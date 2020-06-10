#include "validation.h"

#include "account.h"
#include "btfast.h"             // type aliases
#include "performance.h"
#include "utils_fileio.h"       // write_strategies_to_file
#include "utils_math.h"         // percentile, nearest_int
#include "utils_params.h"       // extract_parameters_from_strategies
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
                        //const param_ranges_t &parameter_ranges,
                        const std::vector<strategy_t> &strategies_to_validate,
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
        - Validation: Profitability test on "fractN_long"
        - Validation: Profitability test on "fractN_short"
        - Validation: Stability test
        - Validation: Noise test

    Return: number of validated strategies

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
    std::vector<strategy_t> passed_validation_6 {};


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

    //--- Validation - Profitability test 1
    // passed_validation_2 -> passed_validation_3
    profitability_test( passed_validation_2, passed_validation_3,
                        "fractN_long", 0,5,1);
    num_validated_ = (int) passed_validation_3.size();
    //---//

    //--- Validation - Profitability test 2
    // passed_validation_3 -> passed_validation_4
    profitability_test( passed_validation_3, passed_validation_4,
                        "fractN_short", 0,5,1);
    num_validated_ = (int) passed_validation_4.size();
    //---//

    //--- Validation - Stability test
    // passed_validation_4 -> passed_validation_5
    stability_test( passed_validation_4, passed_validation_5 );
    num_validated_ = (int) passed_validation_5.size();
    //---

    //--- Validation - Noise test
    // passed_validation_5 -> passed_validation_6
    noise_test( passed_validation_5, passed_validation_6 );
    num_validated_ = (int) passed_validation_6.size();
    //---

    printf( "\n>>> N. of strategies passing Full Validation: %d\n",
            num_validated_ );

    // Write validated strategies to 'validated_file_'
    if( num_validated_ > 0 ){
        int control = utils_fileio::write_strategies_to_file(
                                                    validated_file_, "",
                                                    passed_validation_6,
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

        /*// Keep at most 100 selected strategies
        size_t max_selected_strategies {100};
        if( output_strategies.size() > max_selected_strategies ){
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
/*! Apply selection conditions

    Names/Number/Order of performance metrics must be matched among:
        - utils_params::extract_parameters_from_single_strategy
        - utils_optim::append_to_optim_results
        - utils_optim::sort_by_metric
        - utils_optim::sort_by_ntrades, utils_optim::sort_by_avgtrade, etc
        - utils_fileio::write_strategies_to_file
        - Individual::compute_individual_fitness
        - Validation::selection_conditions
*/
void Validation::selection_conditions(
                            const std::vector<strategy_t> &input_strategies,
                            std::vector<strategy_t> &output_strategies )
{
    double Ntrades {0};
    double AvgTicks {0};
    double WinPerc {0};
    //double NetPL {0};
    //double AvgTrade {0};
    double PftFactor {0};
    double NpMdd {0};
    double Expectancy {0};
    double Zscore {0};

    // number of optimization tests (unique strategies)
    size_t Ntests { input_strategies.size() };

    //-- Loop over generated strategies
    for( auto it = input_strategies.begin();
              it != input_strategies.end(); it++ ){

        // Read metrics from optimization results
        for( auto optrun = it->begin(); optrun != it->end(); optrun++ ){
            if( optrun->first == "Ntrades" ){
                Ntrades = optrun->second;
            }
            //else if( optrun->first == "NetPL" ){
            //    NetPL = optrun->second;
            //}
            else if( optrun->first == "AvgTicks" ){
                AvgTicks = optrun->second;
            }
            else if( optrun->first == "WinPerc" ){
                WinPerc = optrun->second;
            }
            //else if( optrun->first == "AvgTrade" ){
            //    AvgTrade = optrun->second;
            //}
            else if( optrun->first == "PftFactor" ){
                PftFactor = optrun->second;
            }
            else if( optrun->first == "NP/MDD" ){
                NpMdd = optrun->second;
            }
            else if( optrun->first == "Expectancy" ){
                Expectancy = optrun->second;
            }
            else if( optrun->first == "Z-score" ){
                Zscore = optrun->second;
            }
        }

        // one-sided p-value
        // p = 1-Phi(Z) = Phi(-Z)=(1/2)Erfc[x/sqrt(2)],  Phi = CDF(N(0,1))
        double pvalue = 0.5*std::erfc( Zscore/std::sqrt(2.0) );

        // Selection Conditions
        bool condition1 { Ntrades > 60 };// 40 * (btf_.day_counter() / 252.0) };
        bool condition2 { AvgTicks > 12 };//4*btf_.symbol().transaction_cost_ticks()};
        //bool condition2 { AvgTrade > 3 * btf_.symbol().transaction_cost() };
        bool condition3 { NpMdd > 4.0 };
        bool condition4 { PftFactor > 1.2 };
        bool condition5 { Expectancy > 0.1 };
        bool condition6 { pvalue < 0.1 / Ntests  }; // multiple comparison (bonferroni)
        //condition6 = pvalue<=0.01;
        condition6 = Zscore > 2.5;

        // Combine all conditions
        bool selection_conditions = ( condition1 && condition2 && condition3
                                    && condition4 && condition5 && condition6 );
        // Append selected strategies to output
        if( selection_conditions ){
            output_strategies.push_back(*it);
        }
    }
    //-- End loop over generated strategies
}



// ------------------------------------------------------------------------- //
/*! metrics OOS > 50% metrics IS

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
    for( auto strat = input_strategies.begin();
              strat < input_strategies.end(); ++strat ){

        printf( "%25s Strategy %lu / %lu ","",
                std::distance(input_strategies.begin(), strat) + 1,
                input_strategies.size() );

        // Extract parameters from *strat and store them into strat_params
        parameters_t strat_params {};
        utils_params::extract_parameters_from_single_strategy( *strat,
                                                               strat_params );
        //-- In-Sample Backtest
        // Initialize Account for IS
        Account account_is { btf_.initial_balance() };
        // Run single backtest
        btf_.run_backtest( account_is, datafeed_, strat_params );
        // Initialize Performance object for IS
        Performance performance_is { btf_.initial_balance(), btf_.day_counter(),
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
        //-- End In-Sample Backtest

        //-- Out-of-Sample Backtest
        // Initialize Account for OOS
        Account account_oos { btf_.initial_balance() };
        // Run single backtest
        btf_.run_backtest( account_oos, datafeed_oos, strat_params );
        // Initialize Performance object for OOS
        Performance performance_oos { btf_.initial_balance(), btf_.day_counter(),
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

        //-- Selection Conditions
        bool condition1 {  trades_per_day_oos >= 0.5 * trades_per_day_is
                        && trades_per_day_oos <= 2.0 * trades_per_day_is };
        bool condition2 { performance_oos.netpl() > 0.0 };
        bool condition3 {
                performance_oos.avgticks() > 0.5 * performance_is.avgticks() };
        bool condition4 {
                performance_oos.npmdd() > 0.5 * performance_is.npmdd() };
        //--
        if( condition1 && condition2 && condition3 && condition4 ){
            printf("+PASSED+\n");
            // append passed strategy to output_strategies
            output_strategies.push_back( *strat );
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
    for( auto strat = input_strategies.begin();
              strat < input_strategies.end(); ++strat ){

        printf( "%25s Strategy %lu / %lu ","",
                std::distance(input_strategies.begin(), strat) + 1,
                input_strategies.size() );

        // Extract parameters from *strat and store them into strat_params
        parameters_t strat_params {};
        utils_params::extract_parameters_from_single_strategy( *strat,
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
            output_strategies.push_back( *strat );
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
void Validation::profitability_test( const std::vector<strategy_t>
                                                        &input_strategies,
                                std::vector<strategy_t> &output_strategies,
                                const std::string &optim_param_name,
                                int start, int stop, int step )
{
    if( input_strategies.empty() ){
        return;
    }
    std::cout << "\n" << utils_time::current_datetime_str() + " | "
              << "Running Profitability Test over parameter "
              << optim_param_name <<" ("<<start<<".."<<stop<<":"<<step<<")\n";

    // Range of values for optimization parameter
    std::vector<int> optimization_range {};
    do {
        optimization_range.push_back(start);
        start += step;
    } while( start <= stop );

    /*
    // Extract range of optimization parameter "optim_param_name"
    // from original parameter_ranges_
    for( auto el: parameter_ranges_ ){
        if( el.first == optim_param_name ){
            optimization_range = el.second ;
        }
    }
    // Check if metric vector is empty
    if( optimization_range.empty() ){
        std::cout<<">>> ERROR: empty optimization_range vector. "
                 << "Check name of optimization parameter "
                 << "in Validation::profitability_test.\n";
        exit(1);
    }
    */

    //--- Loop over input_strategies
    for( auto strat = input_strategies.begin();
              strat < input_strategies.end(); ++strat ){

        printf( "%25s Strategy %lu / %lu ","",
                std::distance(input_strategies.begin(), strat) + 1,
                input_strategies.size() );

        // Extract parameters from *strat and store them into strat_params
        parameters_t strat_params {};
        utils_params::extract_parameters_from_single_strategy( *strat,
                                                               strat_params );

        param_ranges_t param_ranges {};
        // Fill 'param_ranges'...
        for( single_param_t el: strat_params ){
            if( el.first != optim_param_name ){     //  ... with parameters != optim param
                param_ranges.push_back( std::make_pair(
                                                el.first,
                                                std::vector<int>{el.second} ) );
            }
            else if( el.first == optim_param_name ){ // ... with optim param
                param_ranges.push_back( std::make_pair( optim_param_name,
                                                        optimization_range ) );
            }
            else{
              std::cout<<">>> ERROR: epsilon parameter not found "
                       <<"in strategy parameters (validation).\n";
              exit(1);
            }
        }

        // Cartesian product of all parameter ranges
        std::vector<parameters_t> search_space {
                        utils_params::cartesian_product(param_ranges) };

        // Initialize vector where storing optimization results of running
        std::vector<strategy_t> optim_results {};

        // Run optimization over optimization parameter
        btf_.run_parallel_optimization( search_space,
                                        optim_results, "", "",
                                        fitness_metric_, datafeed_,
                                        true, false );

        // Fill vector of AvgTicks from backtests over optim param
        std::vector<double> metric {};
        for( strategy_t opres: optim_results ){
            for( auto el: opres ){
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
        if( profitable_runs >= 0.8 * metric.size() ){
            printf("+PASSED+\n");
            // append passed strategy to output_strategies
            output_strategies.push_back( *strat );
        }
        else{
            printf("-failed-\n");
        }
    }
    //--- End loop over input_strategies

    printf( "%21s N. of strategies passing Profitability Test over %s: %lu\n",
            "", optim_param_name.c_str(), output_strategies.size() );
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
    for( auto strat = input_strategies.begin();
              strat < input_strategies.end(); ++strat ){

        printf( "%25s Strategy %lu / %lu ","",
                std::distance(input_strategies.begin(), strat) + 1,
                input_strategies.size() );

        // Extract parameters from *strat and store them into strat_params
        parameters_t strat_params {};
        utils_params::extract_parameters_from_single_strategy( *strat,
                                                               strat_params );

        param_ranges_t parameter_ranges {};
        // Fill 'parameter_ranges'...
        for( single_param_t el: strat_params ){
            if( el.first != "epsilon" ){        //  ... with strategy parameters
                parameter_ranges.push_back(
                                    std::make_pair( el.first,
                                                std::vector<int>{el.second} ) );
            }
            else if( el.first == "epsilon" ){   // ... with epsilons
                parameter_ranges.push_back(
                                    std::make_pair( "epsilon", eps_values ));
            }
            else{
                std::cout<<">>> ERROR: epsilon parameter not found "
                         <<"in strategy parameters (validation).\n";
                exit(1);
            }
        }

        // Cartesian product of all epsilons
        std::vector<parameters_t> search_space {
                        utils_params::cartesian_product(parameter_ranges) };

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
            output_strategies.push_back( *strat );
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
/*! Perf metric of strategy on original data between
                    mean - 2*std   and   mean + 2*stdev

    Perform test on strategies in 'input_strategies'
    and store those which pass it into 'output_strategies'.
*/
void Validation::noise_test( const std::vector<strategy_t>
                                                        &input_strategies,
                             std::vector<strategy_t> &output_strategies )
{
    if( input_strategies.empty() ){
        return;
    }
    std::cout << "\n" << utils_time::current_datetime_str() + " | "
              << "Running Noise Test\n";

    //--- Loop over input_strategies
    for( auto strat = input_strategies.begin();
              strat < input_strategies.end(); ++strat ){

        printf( "%25s Strategy %lu / %lu ","",
                std::distance(input_strategies.begin(), strat) + 1,
                input_strategies.size() );

        // Initialize vector where storing optimization results of running
        std::vector<strategy_t> noise_results {};

        // Add original strategy as first entry of noise_results
        noise_results.push_back( *strat );

        // Extract parameters from *strat and store them into strat_params
        parameters_t strat_params {};
        utils_params::extract_parameters_from_single_strategy( *strat,
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
                                        false, false);
        //--

        //-- Fill vector of 'perf_metric_name' from backtests over noise_results
        std::string perf_metric_name { "NP/MDD" };
        std::vector<double> perf_metric {};
        /*
        for( auto el: *strat ){
            if( el.first == perf_metric_name ){
                perf_metric.push_back( el.second );
            }
        }
        */
        for( strategy_t res: noise_results ){
            for( auto el: res ){
                if( el.first == perf_metric_name ){
                    perf_metric.push_back( el.second );
                }
            }
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

        //double percentile_low  { utils_math::percentile( perf_metric, 0.15 ) };
        //double percentile_high { utils_math::percentile( perf_metric, 0.85 ) };
        double lower_level { utils_math::mean( perf_metric )
                             - 2 * utils_math::stdev( perf_metric ) };
        double upper_level { utils_math::mean( perf_metric )
                             + 2 * utils_math::stdev( perf_metric ) };

        if( lower_level < upper_level
         && original_metric >= lower_level
         && original_metric <= upper_level ){
            printf("+PASSED+\n");
            // append passed strategy to output_strategies
            output_strategies.push_back( *strat );
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
        for( auto optrun = it->begin(); optrun != it->end(); optrun++ ){
            if( optrun->first == "Ntrades" ){
                Ntrades = optrun->second;
            }
            else if( optrun->first == "NetPL" ){
                NetPL = optrun->second;
            }
            else if( optrun->first == "AvgTrade" ){
                AvgTrade = optrun->second;
            }
            else if( optrun->first == "PftFactor" ){
                PftFactor = optrun->second;
            }
            else if( optrun->first == "NP/MDD" ){
                NpMdd = optrun->second;
            }
            else if( optrun->first == "Expectancy" ){
                Expectancy = optrun->second;
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
