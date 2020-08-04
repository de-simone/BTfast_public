#include "validation.h"

#include "account.h"
#include "btfast.h"             // type aliases
#include "performance.h"
#include "utils_fileio.h"       // write_strategies_to_file
#include "utils_math.h"         // percentile, nearest_int
#include "utils_params.h"       // extract_parameters_from_strategies,
                                // strategy_attribute_by_name
#include "utils_time.h"         // current_datetime_str

#include <algorithm>    // std::max_element, std::min_element, std::remove, std::unique
#include <cmath>        // std::erfc, std::sqrt
#include <cstdio>       // printf
#include <iostream>     // std::cout
#include <iterator>     // std::distance
//#include <mutex>        // std::mutex
//#include <omp.h>        // openMP
#include <utility>      // std::make_pair

// ------------------------------------------------------------------------- //
/*! Constructor
*/
Validation::Validation( BTfast &btf,
                        std::unique_ptr<DataFeed> &datafeed,
                        const std::vector<strategy_t> &strategies_to_validate,
                        const param_ranges_t &parameter_ranges,
                        const std::string &selected_file,
                        const std::string &validated_file,
                        const std::string &fitness_metric,
                        const std::string &data_dir,
                        const std::string &data_file_oos,
                        int max_variation_pct, int num_noise_tests,
                        const std::string &noise_file )

: btf_ {btf},
  datafeed_ {datafeed},
  strategies_to_validate_ {strategies_to_validate},
  parameter_ranges_ {parameter_ranges},
  selected_file_ {selected_file},
  validated_file_ {validated_file},
  fitness_metric_ {fitness_metric},
  data_dir_ {data_dir},
  data_file_oos_ {data_file_oos},
  max_variation_ { max_variation_pct / 100.0 },
  num_noise_tests_ {num_noise_tests},
  noise_file_ {noise_file}
{
    //date_i_ = btf.first_date_parsed();
    //date_f_ = btf.last_date_parsed();

    /*
    // Compute Final Date of IS
    if( OOSfraction_ > 0 && OOSfraction_ < 1 ){
        int totdays { date_f_.DaysDiff(btf.first_date_parsed()) };
        if( totdays <= 0 ){
            std::cout<<">>> ERROR: invalid OOS fraction (validation)\n";
            exit(1);
        }
        int ISdays { (int) ((1-OOSfraction_)*totdays)  };
        Date final_date_IS { date_i_.add_days(ISdays) };
        if( final_date_IS >= date_f_ ){
            std::cout <<">>> ERROR: In-sample window ends after "
                      << "final date (validation)\n";
            exit(1);
        }
        // OOS starts on business day after IS ends
        //Date initial_date_OOS { final_date_IS.add_bdays(1) };
    }
    */
}



// ------------------------------------------------------------------------- //
/*! Run full validation process.

        - Selection
        - Validation: OOS metrics test
        - Validation: OOS consistency test (Mann-Whitney on ticks)
        - Validation: Profitability test ("fractN_long", "fractN_short")
        - Validation: Stability test ("epsilon")
        - Validation: Noise test

    Return: number of validated strategies

    Names of optimization variables ("fractN_long", "fractN_short")
    and "Side_switch" must match in MasterCode.xml.

*/
void Validation::run_validation()
{

    if( strategies_to_validate_.empty() ){
        std::cout << "No strategies to validate\n";
        return;
    }
    else if( strategies_to_validate_.size() == 1 ){
        std::cout << "\n" << utils_time::current_datetime_str() + " | "
                  << "Starting Validation of 1 strategy\n";
    }
    else if( strategies_to_validate_.size() > 1 ){
        std::cout << "\n\n" << utils_time::current_datetime_str() + " | "
                  << "Starting Validation of " << strategies_to_validate_.size()
                  << " strategies\n";
    }


    // Initialize vectors storing strategies passing each step
    std::vector<strategy_t> passed_selection {};
    std::vector<strategy_t> passed_validation_1 {};
    std::vector<strategy_t> passed_validation_2 {};
    std::vector<strategy_t> passed_validation_3 {};
    std::vector<strategy_t> passed_validation_4 {};
    std::vector<strategy_t> passed_validation_5 {};

    //--- Selection
    // strategies_to_validate_ -> passed_selection
    selection( strategies_to_validate_, passed_selection );
    num_validated_ = (int) passed_selection.size();
    //---

    //--- Validation - OOS metrics test
    // passed_selection -> passed_validation_1
    OOS_metrics_test( passed_selection, passed_validation_1 );
    num_validated_ = (int) passed_validation_1.size();
    //---

    //--- Validation - OOS consistency test
    // passed_validation_1 -> passed_validation_2
    OOS_consistency_test( passed_validation_1, passed_validation_2 );
    num_validated_ = (int) passed_validation_2.size();
    //---

    //--- Validation - Profitability test
    // passed_validation_2 -> passed_validation_3
    profitability_test( passed_validation_2, passed_validation_3 );
    num_validated_ = (int) passed_validation_3.size();
    //---//

    //--- Validation - Stability test
    // passed_validation_3 -> passed_validation_4
    stability_test( passed_validation_3, passed_validation_4 );
    num_validated_ = (int) passed_validation_4.size();
    //---

    // Write validated strategies before noise test to 'validated_file_prenoise'
    if( num_validated_ > 0 ){
        // remove extension from validated_file_
        std::string validated_file_prenoise { validated_file_ };
        std::string extension { ".csv" };
        std::string::size_type i { validated_file_prenoise.find(extension) };
        if (i != std::string::npos){
            validated_file_prenoise.erase(i, extension.length());
        }
        validated_file_prenoise += "_pre-noise"+extension;  // add suffix
        int control = utils_fileio::write_strategies_to_file(
                                                    validated_file_prenoise, "",
                                                    passed_validation_4,
                                                    btf_.strategy_name(),
                                                    btf_.symbol().name(),
                                                    btf_.timeframe(),
                                                    btf_.first_date_parsed(),
                                                    btf_.last_date_parsed(),
                                                    false );
        if( control == 1 ){
            std::cout << "\nValidated strategies (before noise test) "
                      << "written on file: " << validated_file_prenoise <<"\n";
        }
    }

    //--- Validation - Noise test
    // passed_validation_4 -> passed_validation_5
    noise_test( passed_validation_4, passed_validation_5, false );
    num_validated_ = (int) passed_validation_5.size();
    //---

    printf( "\n>>> N. of strategies passing Full Validation: %d\n",
            num_validated_ );

    // Write validated strategies to 'validated_file_'
    if( num_validated_ > 0 ){
        int control = utils_fileio::write_strategies_to_file(
                                                    validated_file_, "",
                                                    passed_validation_5,
                                                    btf_.strategy_name(),
                                                    btf_.symbol().name(),
                                                    btf_.timeframe(),
                                                    btf_.first_date_parsed(),
                                                    btf_.last_date_parsed(),
                                                    false );
        if( control == 1 ){
            std::cout << "\nValidated strategies written on file: "
                      << validated_file_ <<"\n";
        }
    }
}





// ------------------------------------------------------------------------- //
/*! Perform selection

    Perform test on strategies in 'input_strategies'
    and store those which pass it into 'output_strategies'.
    Write output_strategies to 'selected_file_'.
*/
void Validation::selection( const std::vector<strategy_t> &input_strategies,
                           std::vector<strategy_t> &output_strategies )
{
    if( input_strategies.empty() ){
        return;
    }
    std::cout << "\n" << utils_time::current_datetime_str() + " | "
              << "Running Selection\n";

    // Selected 'input_strategies' stored into 'output_strategies'
    selection_conditions( input_strategies, output_strategies );

    if( output_strategies.size() > 0 ){
        /*if( output_strategies.size() > 100 ){ // Keep at most 100 selected strategies
            output_strategies.resize( max_selected_strategies );
        }*/

        // Write selected strategies to 'selected_file_'
        int control = utils_fileio::write_strategies_to_file(
                                                    selected_file_, "",
                                                    output_strategies,
                                                    btf_.strategy_name(),
                                                    btf_.symbol().name(),
                                                    btf_.timeframe(),
                                                    btf_.first_date_parsed(),
                                                    btf_.last_date_parsed(),
                                                    false );
        if( control == 1 ){
            std::cout << "\nSelected strategies written on file: "
                      << selected_file_ <<"\n";
        }
    }
    printf( "%21s N. of strategies passing Selection: %lu\n", "",
            output_strategies.size() );
}


// ------------------------------------------------------------------------- //
/*! Apply selection conditions to initially generated strategies (1st step)
    in 'input_strategies' (during sequential generation)
    and fill 'output_strategies'

    Names/Number/Order of performance metrics must be matched among:
        - utils_params::extract_parameters_from_single_strategy
        - utils_params::extract_metrics_from_single_strategy
        - utils_optim::append_to_optim_results
        - utils_optim::sort_by_metric
        - utils_optim::sort_by_ntrades, utils_optim::sort_by_avgtrade, etc
        - utils_fileio::write_strategies_to_file
        - Individual::compute_individual_fitness
        - Validation::initial_generation_selection
        - Validation::selection_conditions
        - Validation::noise_test
        - mode_factory_sequential (run_modes)
*/
void Validation::initial_generation_selection(
                            const std::vector<strategy_t> &input_strategies,
                            std::vector<strategy_t> &output_strategies )
{

    //-- Loop over input_strategies
    for( const auto& strat: input_strategies ){
        // Read metrics from optimization results
        double Ntrades { utils_params::strategy_attribute_by_name("Ntrades",
                                                                    strat) };
        double AvgTicks { utils_params::strategy_attribute_by_name("AvgTicks",
                                                                    strat) };
        //double WinPerc { utils_params::strategy_attribute_by_name("WinPerc",
        //                                                            strat ) };
        double PftFactor { utils_params::strategy_attribute_by_name("PftFactor",
                                                                    strat) };
        double NpMdd { utils_params::strategy_attribute_by_name("NP/MDD",
                                                                    strat) };
        double Expectancy { utils_params::strategy_attribute_by_name("Expectancy",
                                                                    strat) };
        double Zscore { utils_params::strategy_attribute_by_name("Z-score",
                                                                    strat) };
        // Selection Conditions

        bool condition1 { Ntrades > 300 };
        bool condition2 { AvgTicks > 5 };
        bool condition3 { NpMdd > 2.0 };
        bool condition4 { PftFactor > 1.1 };
        bool condition5 { Expectancy > 0.05 };
        bool condition6 { Zscore > 0.5 };
        // Combine all conditions
        bool selection_conditions = ( condition1 && condition2 && condition3
                                    && condition4 && condition5 && condition6 );
        //bool selection_conditions { Ntrades > 400 }; //<<< test
        // Append selected strategies to output
        if( selection_conditions ){
            output_strategies.push_back(strat);
        }
    }
    //-- End loop over input_strategies
}

// ------------------------------------------------------------------------- //
/*! Apply selection conditions to 'input_strategies' and fill 'output_strategies'

    Names/Number/Order of performance metrics must be matched among:
        - utils_params::extract_parameters_from_single_strategy
        - utils_params::extract_metrics_from_single_strategy
        - utils_optim::append_to_optim_results
        - utils_optim::sort_by_metric
        - utils_optim::sort_by_ntrades, utils_optim::sort_by_avgtrade, etc
        - utils_fileio::write_strategies_to_file
        - Individual::compute_individual_fitness
        - Validation::initial_generation_selection
        - Validation::selection_conditions
        - Validation::noise_test
        - mode_factory_sequential (run_modes)
*/
void Validation::selection_conditions(
                            const std::vector<strategy_t> &input_strategies,
                            std::vector<strategy_t> &output_strategies )
{

    // number of optimization tests (unique strategies)
    //size_t Ntests { input_strategies.size() };

    //-- Loop over input_strategies
    for( const auto& strat: input_strategies ){
        // Read metrics from optimization results
        double Ntrades { utils_params::strategy_attribute_by_name("Ntrades",
                                                                    strat) };
        double AvgTicks { utils_params::strategy_attribute_by_name("AvgTicks",
                                                                    strat) };
        //double WinPerc { utils_params::strategy_attribute_by_name("WinPerc",
        //                                                            strat) };
        double PftFactor { utils_params::strategy_attribute_by_name("PftFactor",
                                                                    strat) };
        double NpMdd { utils_params::strategy_attribute_by_name("NP/MDD",
                                                                    strat) };
        double Expectancy { utils_params::strategy_attribute_by_name("Expectancy",
                                                                    strat) };
        double Zscore { utils_params::strategy_attribute_by_name("Z-score",
                                                                    strat) };
        //double NetPL { utils_params::strategy_attribute_by_name("NetPL",
        //                                                            strat) };
        //double AvgTrade { utils_params::strategy_attribute_by_name("AvgTrade",
        //                                                            strat) };

        // one-sided p-value
        // p = 1-Phi(Z) = Phi(-Z)=(1/2)Erfc[x/sqrt(2)],  Phi = CDF(N(0,1))
        //double pvalue = 0.5*std::erfc( Zscore/std::sqrt(2.0) );

        // Selection Conditions
        bool condition1 { Ntrades > 20 * (btf_.day_counter() / 252.0) };
        bool condition2 { AvgTicks > 12 };//4*btf_.symbol().transaction_cost_ticks()};
        //bool condition2 { AvgTrade > 3 * btf_.symbol().transaction_cost() };
        bool condition3 { NpMdd > 4.0 };
        bool condition4 { PftFactor > 1.2 };
        bool condition5 { Expectancy > 0.1 };
        //bool condition6 { pvalue < 0.1 / Ntests  }; // multiple comparison (bonferroni)
        //condition6 = pvalue<=0.01;
        bool condition6 { Zscore > 2.0 };
        // Combine all conditions
        bool selection_conditions = ( condition1 && condition2 && condition3
                                    && condition4 && condition5 && condition6 );
        // Append selected strategies to output
        if( selection_conditions ){
            output_strategies.push_back(strat);
        }
    }
    //-- End loop over input_strategies
}


// ------------------------------------------------------------------------- //
/*! metrics OOS > 50% metrics IS
    and at least 75% of (IS+OOS) years are profitable

    Perform test on strategies in 'input_strategies'
    and store those which pass it into 'output_strategies'.
*/
void Validation::OOS_metrics_test( const std::vector<strategy_t>
                                                        &input_strategies,
                                std::vector<strategy_t> &output_strategies )
{
    if( input_strategies.empty() ){
        return;
    }
    std::cout << "\n" << utils_time::current_datetime_str() + " | "
              << "Running OOS Metrics Test\n";

    // Initialize smart pointer to object derived from DataFeed base class
    std::unique_ptr<DataFeed> datafeed_oos { nullptr };
    // Instantiate DataFeed derived object corresponding to 'datafeed_type'
    // and wrap it into the smart pointer 'datafeed'
    select_datafeed( datafeed_oos, datafeed_->type(), btf_.symbol(),
                     btf_.timeframe(), data_dir_, data_file_oos_,
                     datafeed_->csv_format(),
                     datafeed_->start_date(), datafeed_->end_date() );

    //--- Loop over input_strategies
    for( const auto& strat: input_strategies ){
        printf( "%25s Strategy %lu / %lu ","",
                &strat - &input_strategies[0] + 1, input_strategies.size() );
                //std::distance(input_strategies.begin(), &strat) + 1

        // Extract parameters from strat and store them into strat_params
        parameters_t strat_params {};
        utils_params::extract_parameters_from_single_strategy( strat,
                                                               strat_params );
        //-- In-Sample Backtest
        // Initialize Account for IS
        Account account_is { btf_.initial_balance() };
        // Run single backtest
        btf_.run_backtest( account_is, datafeed_, strat_params );
        // Initialize Performance object for IS
        Performance performance_is { btf_.initial_balance(),
                                     std::vector<Transaction> {} };

        if( !account_is.transactions().empty() ){
            // Load transaction history into performance object
            performance_is.set_transactions( account_is.transactions() );
            performance_is.compute_metrics();
        }
        else{
            printf("-failed-\n");
            continue;   // discard this strategy
        }
        int ndays_is { btf_.day_counter() };
        int nyears_is { performance_is.nyears() };
        int profitable_yrs_is { performance_is.profitable_yrs() };
        //-- End In-Sample Backtest

        //-- Out-of-Sample Backtest
        // Initialize Account for OOS
        Account account_oos { btf_.initial_balance() };
        // Run single backtest
        btf_.run_backtest( account_oos, datafeed_oos, strat_params );
        // Initialize Performance object for OOS
        Performance performance_oos { btf_.initial_balance(),
                                     std::vector<Transaction> {} };

        if( !account_oos.transactions().empty() ){
            // Load transaction history into performance object
            performance_oos.set_transactions( account_oos.transactions() );
            performance_oos.compute_metrics();
        }
        else{
            printf("-failed-\n");
            continue;   // discard this strategy
        }
        int ndays_oos { btf_.day_counter() };
        int nyears_oos { performance_oos.nyears() };
        int profitable_yrs_oos { performance_oos.profitable_yrs() };
        //-- End Out-of-Sample Backtest

        // Avg number of trades per day
        double trades_per_day_is {0.0};
        if( ndays_is > 0 ){
            trades_per_day_is = performance_is.ntrades()/(double) ndays_is;
        }
        double trades_per_day_oos {0.0};
        if( ndays_oos > 0 ){
            trades_per_day_oos = performance_oos.ntrades()/(double) ndays_oos;
        }
        // Total number of years
        double nyears { (double) (nyears_is + nyears_oos) };

        //-- Selection Conditions
        // Similar number of trades per day
        bool condition1 {  trades_per_day_oos >= 0.3 * trades_per_day_is
                        && trades_per_day_oos <= 3.0 * trades_per_day_is };
        // NetPL>0 on out-of-sample
        bool condition2 { performance_oos.netpl() > 0.0 };
        // At least 50% AvgTicks on out-of-sample wrt in-sample
        bool condition3 {
                performance_oos.avgticks() >= 0.5 * performance_is.avgticks() };
        // At least 50% NetPL/MaxDD on out-of-sample wrt in-sample
        bool condition4 {
                performance_oos.npmdd() >= 0.5 * performance_is.npmdd() };
        // At least 75% of all years are profitable (AvgTicks>=6)
        bool condition5 { ( profitable_yrs_is + profitable_yrs_oos )
                            / nyears  >= 0.75 };
        //--

        if(condition1 && condition2 && condition3 && condition4 && condition5){
            printf("+PASSED+\n");
            // append passed strategy to output_strategies
            output_strategies.push_back( strat );
        }
        else{
            printf("-failed-\n");
        }
    }
    //--- End loop over input_strategies

    printf( "%21s N. of strategies passing OOS Metrics Test: %lu\n", "",
            output_strategies.size() );
}



// ------------------------------------------------------------------------- //
/*! OOS and IS ticks from same distribution (Mann-Whitney two-sample test)

    Perform test on strategies in 'input_strategies'
    and store those which pass it into 'output_strategies'.
*/
void Validation::OOS_consistency_test( const std::vector<strategy_t>
                                                        &input_strategies,
                                    std::vector<strategy_t> &output_strategies )
{
    if( input_strategies.empty() ){
        return;
    }
    std::cout << "\n" << utils_time::current_datetime_str() + " | "
              << "Running OOS Consistency Test\n";

    // Initialize smart pointer to object derived from DataFeed base class
    std::unique_ptr<DataFeed> datafeed_oos { nullptr };
    // Instantiate DataFeed derived object corresponding to 'datafeed_type'
    // and wrap it into the smart pointer 'datafeed'
    select_datafeed( datafeed_oos, datafeed_->type(), btf_.symbol(),
                     btf_.timeframe(), data_dir_, data_file_oos_,
                     datafeed_->csv_format(),
                     datafeed_->start_date(), datafeed_->end_date() );

    //--- Loop over input_strategies
    for( const auto& strat: input_strategies ){

        printf( "%25s Strategy %lu / %lu ","",
                                        &strat - &input_strategies[0] + 1,
                                        input_strategies.size() );

        // Extract parameters from strat and store them into strat_params
        parameters_t strat_params {};
        utils_params::extract_parameters_from_single_strategy( strat,
                                                               strat_params );
        //-- In-Sample Backtest
        // Initialize Account for IS
        Account account_is { btf_.initial_balance() };
        // Run single backtest
        btf_.run_backtest( account_is, datafeed_, strat_params );
        // Load vector of IS ticks for each trade
        std::vector<double> ticks_is {};
        for( auto tr: account_is.transactions() ){
            ticks_is.push_back( tr.ticks() );
        }
        if( ticks_is.empty() ){
            printf("-failed-\n");
            continue;   // discard this strategy
        }
        //-- End In-Sample Backtest

        //-- Out-of-Sample Backtest
        // Initialize Account for OOS
        Account account_oos { btf_.initial_balance() };
        // Run single backtest
        btf_.run_backtest( account_oos, datafeed_oos, strat_params );
        // Load vector of OOS ticks for each trade
        std::vector<double> ticks_oos {};
        for( auto tr: account_oos.transactions() ){
            ticks_oos.push_back( tr.ticks() );
        }
        if( ticks_oos.empty() ){
            printf("-failed-\n");
            continue;   // discard this strategy
        }
        //-- End Out-of-Sample Backtest

        double pvalue { utils_math::mannwhitney(ticks_is, ticks_oos) };
        if( pvalue >= 0.05 ){
            printf("+PASSED+\n");
            // append passed strategy to output_strategies
            output_strategies.push_back( strat );
        }
        else{
            printf("-failed-\n");
        }
    }
    //--- End loop over input_strategies

    printf( "%21s N. of strategies passing OOS Consistency Test: %lu\n", "",
            output_strategies.size() );
}





// ------------------------------------------------------------------------- //
/*! TS profitable across at least 80% of all parameter combinations.
    It checks all combination of the parameter named "optim_param_name"
    in strategy.

    Perform test on strategies in 'input_strategies'
    and store those which pass it into 'output_strategies'.
*/
void Validation::profitability_test(
                            const std::vector<strategy_t>& input_strategies,
                                std::vector<strategy_t>& output_strategies )
{
    if( input_strategies.empty() ){
        return;
    }
    std::cout << "\n" << utils_time::current_datetime_str() + " | "
              << "Running Profitability Test \n";

    //--- Loop over input_strategies
    for( const auto& strat: input_strategies ){

        printf( "%25s Strategy %lu / %lu ","",
                &strat - &input_strategies[0] + 1, input_strategies.size() );

        // Extract parameters from strat and store them into strat_params
        parameters_t strat_params {};
        utils_params::extract_parameters_from_single_strategy( strat,
                                                               strat_params );
        // Store value of Side switch (1=Long, 2=Short, 3=Both)
        int side_switch { utils_params::parameter_by_name( "Side_switch",
                                                           strat_params) };
        bool test_passed {false};
        switch( side_switch ){
            case 1:
                test_passed = check_profitability(strat_params, "fractN_long");
                break;
            case 2:
                test_passed = check_profitability(strat_params, "fractN_short");
                break;
            case 3:
                test_passed = (
                            check_profitability(strat_params, "fractN_long")
                        && check_profitability(strat_params, "fractN_short") );
                break;
        }
        if( test_passed ){
            printf("+PASSED+\n");
            // append passed strategy to output_strategies
            output_strategies.push_back( strat );
        }
        else{
            printf("-failed-\n");
        }
    }
    //--- End loop over input_strategies

    printf( "%21s N. of strategies passing Profitability Test: %lu\n", "",
            output_strategies.size() );
}



// ------------------------------------------------------------------------- //
/*! Check profitability of single strategy by varying 'optim_param_name':
    >= 80% of all runs must be profitable
*/
bool Validation::check_profitability( const parameters_t& strat_params,
                                      const std::string &optim_param_name )
{
    bool result {false};

    std::cout << "( " << optim_param_name << " ) ";

    // Initialize 'search_space' with single strategy parameters
    std::vector<parameters_t> search_space {strat_params};
    // Fill 'search_space' with combintations of 'optim_param_name'
    utils_params::expand_strategies_with_opt_range(
                      optim_param_name, parameter_ranges_, search_space );
    // Initialize vector where storing optimization results of running
    std::vector<strategy_t> optim_results {};
    // Run optimization over optimization parameter
    btf_.run_parallel_optimization( search_space, optim_results, "", "",
                                    fitness_metric_, datafeed_,true,false);

    // Fill vector of AvgTicks from backtests over optim param
    std::vector<double> metric {};
    for( const strategy_t& opres: optim_results ){
        for( const auto& el: opres ){
            if( el.first == "AvgTicks" ){
                metric.push_back( el.second );
            }
        }
    }
    // Check if metric vector is empty
    if( metric.empty() ){
        std::cout<<">>> ERROR: empty metric vector (validation).\n";
        exit(1);
    }
    // Count profitable runs (with AvgTicks > transaction costs)
    int profitable_runs {0};
    for( double at: metric ){
        if( at > btf_.symbol().transaction_cost_ticks() ){
            profitable_runs++;
        }
    }
    // at least 80% of all runs must be profitable
    result = ( profitable_runs >= 0.8 * metric.size() );

    return( result );
}



// ------------------------------------------------------------------------- //
/*! Performance must be stable across neighbor parameters (+/- 5%,10%)
    It needs an "epsilon" parameter defined in the strategy,
    with epsilon=1 corresponding to +5% variation.

    Perform stability test on strategies in 'input_strategies'
    and store those which pass it into 'output_strategies'.
*/
void Validation::stability_test( const std::vector<strategy_t>
                                                        &input_strategies,
                                std::vector<strategy_t> &output_strategies )
{
    if( input_strategies.empty() ){
        return;
    }
    std::cout << "\n" << utils_time::current_datetime_str() + " | "
              << "Running Stability Test\n";

    // Create vector with epsilon parameters [-2,-1,0,1,2]
    // corresponding to [-10%,-5%, 0, +5%, +10%] parameter variation
    std::vector<int> eps_values {};
    for( int i=-2; i<=2; i++){
        eps_values.push_back(i);
    }

    //--- Loop over input_strategies
    for( const auto& strat: input_strategies ){

        printf( "%25s Strategy %lu / %lu ","",
                &strat - &input_strategies[0] + 1, input_strategies.size() );

        // Extract parameters from strat and store them into strat_params
        parameters_t strat_params {};
        utils_params::extract_parameters_from_single_strategy( strat,
                                                               strat_params );

        param_ranges_t param_ranges {};
        // Fill 'param_ranges'...
        for( const auto& el: strat_params ){
            if( el.first != "epsilon" ){        //  ... with strategy parameters
                param_ranges.push_back( std::make_pair(
                                    el.first, std::vector<int>{el.second} ) );
            }
            else if( el.first == "epsilon" ){   // ... with epsilons
                param_ranges.push_back( std::make_pair("epsilon", eps_values));
            }
            else{
                std::cout<<">>> ERROR: epsilon parameter not found "
                         <<"in strategy parameters (validation).\n";
                exit(1);
            }
        }

        // Cartesian product of all epsilons
        std::vector<parameters_t> search_space {
                        utils_params::cartesian_product(param_ranges) };

        // Initialize vector where storing optimization results of running
        std::vector<strategy_t> optim_results {};

        // Run optimization over all values of epsilon
        btf_.run_parallel_optimization( search_space,
                                        optim_results, "", "",
                                        fitness_metric_, datafeed_,
                                        true, false );

        // Fill vector of fitness_metric_ from backtests over epsilons
        std::vector<double> metric {};
        for( auto opres: optim_results ){
            for( auto el: opres ){
                if( el.first == fitness_metric_ ){
                    metric.push_back( el.second );
                }
            }
        }
        // Check if metric vector is empty
        if( metric.empty() ){
            std::cout<<">>> ERROR: empty metric vector. "
                     << "Check FITNESS_METRIC (validation).\n";
            exit(1);
        }
        /*
        // Print values of performance metrics over neighborhood
        std::cout<<fitness_metric_<<": ";
        for( double m: metric ){
            std::cout<< m<<", ";
        }
        std::cout<<"\n";
        */
        // Min/Max of performance metric over neighborhood
        double max_metric { *max_element(metric.begin(), metric.end()) };
        double min_metric { *min_element(metric.begin(), metric.end()) };

        // Stability condition ( |1-min/max|<= max_variation_ )
        if( min_metric >= (1-max_variation_) * max_metric ){
            printf("+PASSED+\n");
            // append passed strategy to output_strategies
            output_strategies.push_back( strat );
        }
        else{
            printf("-failed-\n");
        }
    }
    //--- End loop over input_strategies

    printf( "%21s N. of strategies passing Stability Test: %lu\n", "",
            output_strategies.size() );
}




// ------------------------------------------------------------------------- //
/*! Perf metric of strategy on original data in [mean - 2*std , mean + 2*stdev]

    Perform test on strategies in 'input_strategies'
    and store those which pass it into 'output_strategies'.

    If 'write_to_file'=true, write optimization results to 'noise_file_' and
    to stdout (used for noise test of single strategy)
*/
void Validation::noise_test( const std::vector<strategy_t>
                                                        &input_strategies,
                             std::vector<strategy_t> &output_strategies,
                             bool write_to_file )
{
    if( input_strategies.empty() ){
        return;
    }
    std::cout << "\n" << utils_time::current_datetime_str() + " | "
              << "Running Noise Test\n";

    //--- Loop over input_strategies
    for( const auto& strat: input_strategies ){

        printf( "%25s Strategy %lu / %lu ","",
                &strat - &input_strategies[0] + 1, input_strategies.size() );

        // Initialize vector where storing optimization results of running
        std::vector<strategy_t> noise_results {};

        // Add original strategy as first entry of noise_results
        noise_results.push_back( strat );

        // Extract parameters from strat and store them into strat_params
        parameters_t strat_params {};
        utils_params::extract_parameters_from_single_strategy( strat,
                                                               strat_params );

        //-- Backtest with noised data
        // Replicate the same 'strat_params' for 'num_noise_tests-1' times,
        // since 1 run is on original data
        std::vector<parameters_t> search_space {};
        for( int i = 0; i < num_noise_tests_-1; i++ ){
            search_space.push_back(strat_params);
        }

        // Each run is with same strategy parameters but with
        // random noise added to price data
        btf_.set_random_noise(true);
        btf_.run_parallel_optimization( search_space,
                                        noise_results, noise_file_, "",
                                        fitness_metric_, datafeed_,
                                        false, write_to_file);
        //--

        //-- Fill vector of 'perf_metric_name' from backtests over noise_results
        std::string perf_metric_name { "AvgTicks" };
        std::vector<double> perf_metric {};

        for( const strategy_t& res: noise_results ){
            perf_metric.push_back(
                utils_params::strategy_attribute_by_name(perf_metric_name, res)
                                 );
        }
        // Check if metric vector is empty
        if( perf_metric.empty() ){
            std::cout<<">>> ERROR: empty "<< perf_metric_name
                     << " vector (validation).\n";
            exit(1);
        }
        //--

        // first entry of 'perf_metric' is performance metric on original data
        double original_metric { perf_metric.at(0) };

        // mean +/- 2std of performance metric
        double lower_level { utils_math::mean( perf_metric )
                            - 2 * utils_math::stdev( perf_metric ) };
                            //{ utils_math::percentile( perf_metric, 0.05 ) };
        double upper_level { utils_math::mean( perf_metric )
                            + 2 * utils_math::stdev( perf_metric ) };
                            //{ utils_math::percentile( perf_metric, 0.95 ) };
        if( write_to_file ){
            std::cout << "\n";
            std::cout << perf_metric_name <<"  |  original: "<< original_metric
                      << " |  mean +/- 2*std: [ "
                      << lower_level <<" , "<< upper_level<<" ]\n\n";
        }

        if( lower_level < upper_level &&
            original_metric >= lower_level && original_metric <= upper_level ){
            printf("+PASSED+\n");
            // append passed strategy to output_strategies
            output_strategies.push_back( strat );
        }
        else{
            printf("-failed-\n");
        }
    }
    //--- End loop over input_strategies

    printf( "%21s N. of strategies passing Noise Test: %lu\n", "",
            output_strategies.size() );
}



















// ------------------------------------------------------------------------- //
/*! Run the in-sample selected strategies over OOS data.
    Select among 'OOS_run' the strategies satisfying selection criteria
    Store them to 'selected_strategies_' and write them to file 'selected_file_'
*/
/*
void Validation::selection_conditions_OOS( const std::vector<strategy_t>
                                                                    &OOS_run )
{
    double Ntrades {0};
    double NetPL {0};
    double AvgTrade {0};
    double PftFactor {0};
    double NpMdd {0};
    double Expectancy {0};

    //-- Selection Conditions
    for( auto it = OOS_run.begin(); it != OOS_run.end(); it++ ){

        // Read metrics from optimization results
        for( auto elem = it->begin(); elem != it->end(); elem++ ){
            if( elem->first == "Ntrades" ){
                Ntrades = elem->second;
            }
            else if( elem->first == "NetPL" ){
                NetPL = elem->second;
            }
            else if( elem->first == "AvgTrade" ){
                AvgTrade = elem->second;
            }
            else if( elem->first == "PftFactor" ){
                PftFactor = elem->second;
            }
            else if( elem->first == "NP/MDD" ){
                NpMdd = elem->second;
            }
            else if( elem->first == "Expectancy" ){
                Expectancy = elem->second;
            }
        }

        // Selection Conditions
        bool condition1 { Ntrades > 40*(date_f_.DaysDiff(date_i_)/365.0) };
        bool condition2 { NetPL > 0.0 };
        bool condition3 { AvgTrade > 3 * btf_.symbol().transaction_cost() };
        bool condition4 { NpMdd >= 8.0 };
        bool condition5 { PftFactor >= 1.2 };
        bool condition6 { Expectancy >= 0.1 };

        // Combine all conditions
        bool SelectionConditions = ( condition1 && condition2 && condition3
                                  && condition4 && condition5 && condition6 );

        // Append selected strategies
        if( SelectionConditions ){
            selected_strategies_.push_back(*it);
        }
    }
    //--

    std::cout << "N. of strategies passing In-Sample + Out-of-Sample selection: "
              << selected_strategies_.size() <<"\n";
    std::cout<<"\nSelected strategies written on file: "<<selected_file_<<"\n";
}
*/
