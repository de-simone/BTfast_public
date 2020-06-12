#include "run_modes.h"

#include "account.h"
#include "performance.h"
#include "utils_fileio.h"   // read_strategies_from_file
#include "utils_optim.h"    // append_to_optim_results
#include "utils_params.h"   // single_parameter_combination, cartesian_product,
                            // first_param_from_range, replace_opt_range_by_name
#include "utils_print.h"    // show_backtest_results
#include "utils_math.h"
#include "utils_time.h"     // current_datetime_str
#include "validation.h"

#include <cstdlib>          // std::system
#include <fstream>          // std::ofstream
#include <iostream>         // std::cout

// ------------------------------------------------------------------------- //
/*! No trade
*/
void mode_notrade( BTfast &btf, std::unique_ptr<DataFeed> &datafeed,
                   const param_ranges_t &parameter_ranges )
{
    std::cout << utils_time::current_datetime_str() + " | "
              << "Running in no-trade mode \n";
    // Extract single parameter combination from parameter_ranges
    // (only the <Start> value is taken)
    parameters_t parameter_combination {
        utils_params::single_parameter_combination(parameter_ranges) };
    // Initialize Account
    Account account { btf.initial_balance() };
    // Parse data without strategy signals
    btf.run_notrade( account, datafeed, parameter_combination );

    //<<< tests
    //std::vector<double> v({1,2,3,4,5,6,7,8,9,10});
    //std::vector<double> w({1.1, 2.1, 3.1, 4.1, 5.1, 6.1, 7.1, 8.1, 9.1, 10.1});
    //utils_math::mannwhitney(v, w);
    //<<<
}



// ------------------------------------------------------------------------- //
/*! Single Backtest
*/
void mode_single_bt( BTfast &btf,
                     std::unique_ptr<DataFeed> &datafeed,
                     const param_ranges_t &parameter_ranges,
                     bool print_trade_list, bool print_performance_report,
                     bool show_plot,
                     const std::string &param_file,
                     const std::string &trade_list_file,
                     const std::string &performance_file,
                     const std::string &profits_file )
{
    std::cout << utils_time::current_datetime_str() + " | "
              << "Running Backtest \n";

    // Extract single parameter combination from parameter_ranges
    // (only the <Start> value is taken)
    parameters_t parameter_combination {
        utils_params::single_parameter_combination(parameter_ranges) };

    // Initialize Account
    Account account { btf.initial_balance() };

    // Run single backtest
    btf.run_backtest( account, datafeed, parameter_combination );

    std::cout<<"\n";

    // Initialize Performance object
    Performance performance { btf.initial_balance(), btf.day_counter(),
                              std::vector<Transaction> {} };

    // Load transaction history into performance object
    performance.set_transactions( account.transactions() );

    // Print results on stdout and on file
    utils_print::show_backtest_results( account, performance,
                               btf.strategy_name(), btf.symbol().name(),
                               btf.timeframe(),
                               btf.first_date_parsed(), btf.last_date_parsed(),
                               print_trade_list, print_performance_report,
                               show_plot, param_file, trade_list_file,
                               performance_file, profits_file );

}





// ------------------------------------------------------------------------- //
/*! Optimization, with mode specified by 'optim_mode'
    possible values of 'optim_mode':
      - "parallel"
      - "genetic"
      - "serial"
*/
void mode_optimization( BTfast &btf,
                        std::unique_ptr<DataFeed> &datafeed,
                        param_ranges_t &parameter_ranges,
                        const std::string &optim_mode,
                        const std::string &optim_file,
                        const std::string &param_file,
                        const std::string &fitness_metric,
                        int population_size, int generations )
{

    // Combine 'parameter_ranges' into all parameter combinations
    // [ [("p1", 10), ("p2", 2), ...], [("p1", 10), ("p2", 4), ...] ]
    std::vector<parameters_t> search_space {
                    utils_params::cartesian_product(parameter_ranges) };

    // Initialize vector where storing results of optimization:
    // performance metrics and parameter values of each run, e.g.
    // [ [("metric1", 110.2), ("metric2", 2.1), ("p1", 2.0), ("p2", 21.0), ...],
    //  [("metric1", 121.3), ("metric2", 1.7), ("p1", 4.0), ("p2", 10.0), ...],
    //  ... ]
    std::vector<strategy_t> optim_results {};

    if( optim_mode == "parallel" ){     // Exhaustive Parallel Optimization

        bool sort_results {true};
        bool verbose {true};
        btf.run_parallel_optimization( search_space, optim_results, optim_file,
                                       param_file, fitness_metric,
                                       datafeed, sort_results, verbose );
    }

    else if( optim_mode == "genetic" ){ // Genetic Parallel Optimization

        btf.run_genetic_optimization( search_space, optim_results, optim_file,
                                      param_file, fitness_metric,
                                      datafeed, population_size, generations );
    }

    else if( optim_mode == "serial" ){  // Exhaustive Serial Optimization

        bool sort_results {true};
        bool verbose {true};
        btf.run_optimization( search_space, optim_results, optim_file,
                              param_file, fitness_metric,
                              datafeed, sort_results, verbose );
    }

    else{
        std::cout<<">>>ERROR: invalid optim_mode (mode_optimization).\n";
        exit(1);
    }
}



// ------------------------------------------------------------------------- //
/*! Validation for Single Strategy
*/
void mode_single_validation( BTfast &btf,
                             std::unique_ptr<DataFeed> &datafeed,
                             const param_ranges_t &parameter_ranges,
                             const std::string &param_file,
                             const std::string &selected_file,
                             const std::string &validated_file,
                             const std::string &fitness_metric,
                             const std::string &data_dir,
                             const std::string &data_file_oos,
                             int max_variation_pct, int num_noise_tests,
                             const std::string &noise_file )
{
    // ----------------------------    BACKTEST   -------------------------- //
    std::cout << utils_time::current_datetime_str() + " | "
              << "Running Backtest \n";

    // Extract single parameter combination from parameter_ranges
    // (only the <Start> value is taken)
    parameters_t parameter_combination {
        utils_params::single_parameter_combination(parameter_ranges) };

    // Backtest of single strategy
    Account account { btf.initial_balance() };

    btf.run_backtest( account, datafeed, parameter_combination );

    Performance performance { btf.initial_balance(), btf.day_counter(),
                              std::vector<Transaction> {} };

    performance.set_transactions( account.transactions() );

    // Print results on stdout and on file
    utils_print::show_backtest_results( account, performance,
                               btf.strategy_name(), btf.symbol().name(),
                               btf.timeframe(),
                               btf.first_date_parsed(), btf.last_date_parsed(),
                               false, true,
                               false, param_file, "", "", "" );

    // Initialize vector to store results of backtest
    std::vector<strategy_t> strategy_to_validate {};
    // Fill 'strategy_to_validate' with performance metrics and parameters
    utils_optim::append_to_optim_results( strategy_to_validate,
                                          performance,
                                          parameter_combination );
    // --------------------------------------------------------------------- //

    // ---------------------------    VALIDATION   ------------------------- //
    // Instantiate Validation object
    Validation validation { btf, datafeed,
                            strategy_to_validate, selected_file,
                            validated_file, fitness_metric,
                            data_dir, data_file_oos, max_variation_pct,
                            num_noise_tests, noise_file };

    // Run full validation process
    validation.run_validation();
    // --------------------------------------------------------------------- //
}






// ------------------------------------------------------------------------- //
/*! Strategy Factory mode (Generation + Validation)
*/
void mode_factory( BTfast &btf,
                   std::unique_ptr<DataFeed> &datafeed,
                   param_ranges_t &parameter_ranges,
                   const std::string &optim_mode,
                   const std::string &optim_file,
                   const std::string &param_file,
                   const std::string &selected_file,
                   const std::string &validated_file,
                   const std::string &fitness_metric,
                   int population_size, int generations,
                   const std::string &data_dir,
                   const std::string &data_file_oos,
                   int max_variation_pct, int num_noise_tests,
                   const std::string &noise_file )
{
    int max_num_generations {1};

    // Repeat generation+validation until validated strategies found
    // (only for "genetic" optim_mode)
    for( int i = 0; i < max_num_generations; i++ ){

        // Combine 'parameter_ranges' into all parameter combinations
        std::vector<parameters_t> search_space {
                     utils_params::cartesian_product(parameter_ranges) };

        // Initialize vector to store results of strategy generation
        // (new optimization or imported from file)
        // [ [("metric1", 110.2), ("metric2", 2.1), ("p1", 2.0), ("p2", 21.0), ...],
        std::vector<strategy_t> generated_strategies {};

        // --------------------    STATEGY GENERATION    ------------------- //
        // ----------------    (optimization or from file)    -------------- //
        if( optim_mode == "parallel" ){
            // Exhaustive Parallel Optimization
            bool sort_results {true};
            bool verbose {true};
            btf.run_parallel_optimization( search_space, generated_strategies,
                                           optim_file,  param_file, fitness_metric,
                                           datafeed,
                                           //btf.start_date(),btf.end_date(),
                                           sort_results, verbose );
        }
        else if( optim_mode == "genetic" ){
            std::cout<<"\n--- Strategy Generation N. " << i + 1
                     << " / "<< max_num_generations <<" ---\n";
            // Genetic Parallel Optimization
            btf.run_genetic_optimization( search_space, generated_strategies,
                                          optim_file, param_file, fitness_metric,
                                          datafeed,
                                          //btf.start_date(),btf.end_date(),
                                          population_size, generations );
        }
        else if( optim_mode == "import" ){
            // Import Generation Results from 'optim_file'

            // input START_DATE/END_DATE in settings should not be "0"
            if( datafeed->start_date().year() <= 1990 ||
                datafeed->end_date().year() >= 2090 ){
                std::cout<<">>> ERROR: input START_DATE/END_DATE should not be 0 "
                         <<"(mode_factory).\n"
                         <<"Set them according to the Date Range in file "
                         << optim_file <<"\n";
                exit(1);
            }
            // set dates as those from input settings
            btf.set_first_date_parsed( datafeed->start_date() );
            btf.set_last_date_parsed( datafeed->end_date() );
            // set day_counter as num of days between input start/end dates
            btf.set_day_counter( btf.last_date_parsed().DaysDiff(
                                                   btf.first_date_parsed() ));

            // read results from file and store them into generated_strategies
            generated_strategies = utils_fileio::read_strategies_from_file(
                                                                optim_file );
            std::cout<<"Imported " << generated_strategies.size()
                     <<" strategies from file:\n" << optim_file<<"\n";
        }
        else{
            std::cout<<">>>ERROR: invalid optim_mode (mode_factory).\n";
            exit(1);
        }

        if( generated_strategies.empty() ){
            std::cout << ">>> ERROR: Generation results not available"
                      << " (mode_factory).\n";
            exit(1);
        }
        // ----------------------------------------------------------------- //

        // Remove duplicates strategies from generated_strategies
        utils_optim::remove_duplicates( generated_strategies, fitness_metric );
        std::cout<<"Unique strategies: " << generated_strategies.size() <<"\n";

        // -------------------------    VALIDATION   ----------------------- //
        // Instantiate Validation object
        Validation validation { btf, datafeed,
                                generated_strategies, selected_file,
                                validated_file, fitness_metric,
                                data_dir, data_file_oos, max_variation_pct,
                                num_noise_tests, noise_file };

        // Run full validation process
        validation.run_validation();
        // ----------------------------------------------------------------- //

        // exit loop over generations
        if( optim_mode != "genetic" || validation.num_validated() > 0 ){
            break;
        }
    }
}




// ------------------------------------------------------------------------- //
/*! Strategy Factory mode (Generation + Validation)
*/
void mode_factory_sequential( BTfast &btf,
                              std::unique_ptr<DataFeed> &datafeed,
                              param_ranges_t &parameter_ranges,
                              //const std::string &optim_mode,
                              const std::string &optim_file,
                              const std::string &param_file,
                              const std::string &selected_file,
                              const std::string &validated_file,
                              const std::string &fitness_metric,
                              int population_size, int generations,
                              const std::string &data_dir,
                              const std::string &data_file_oos,
                              int max_variation_pct, int num_noise_tests,
                              const std::string &noise_file )
{
    // Check if strategy name is MasterCode
    if( btf.strategy_name() != "MasterCode" ){
        std::cout << ">>> ERROR: Sequential strategy generation available"
                  << " only for MasterCode (mode_factory_sequential).\n";
        exit(1);
    }
    // Store value of Side switch (1=Long, 2=Short, 3=Both)
    int side_switch { utils_params::parameter_value_by_name("Side_switch",
                                                            parameter_ranges) };
    // Initialize vector of cartesian product
    std::vector<parameters_t> search_space {};

    bool selection_conditions {false};

    // ----------------------    STATEGY GENERATION    --------------------- //

    //--- GENERATION STEP 1
    // Initial parameter set with just the first element of each
    // parameter in parameter_ranges
    param_ranges_t p_ranges_1 {
                    utils_params::first_param_from_range(parameter_ranges) };

    // Add optimization parameters
    utils_params::replace_opt_range_by_name( "POI_switch", parameter_ranges,
                                             p_ranges_1 );
    utils_params::replace_opt_range_by_name( "Distance_switch",parameter_ranges,
                                             p_ranges_1 );
    switch( side_switch ){
        case 1:
            utils_params::replace_opt_range_by_name( "fractN_long",
                                                     parameter_ranges,
                                                     p_ranges_1 );
            break;
        case 2:
            utils_params::replace_opt_range_by_name( "fractN_short",
                                                     parameter_ranges,
                                                     p_ranges_1 );
            break;
        case 3:
            utils_params::replace_opt_range_by_name( "fractN_long",
                                                     parameter_ranges,
                                                     p_ranges_1 );
            utils_params::replace_opt_range_by_name( "fractN_short",
                                                     parameter_ranges,
                                                     p_ranges_1 );
            break;
        default:
            std::cout << ">>> ERROR: Invalid Side_switch parameter in XML"
                      << " (mode_factory_sequential).\n";
            exit(1);
    }

    // Combine parameter ranges into all parameter combinations
    search_space = utils_params::cartesian_product(p_ranges_1);

    /*for( auto elem: p_ranges_2 ){
        std::cout<<elem.first<<"\n";
        for( auto p: elem.second ){
            std::cout<< p<<"\n";
        }
    }
    for( auto v: search_space ){
         for( auto elem: v ){
             std::cout<<elem.first<<"   "<<elem.second<<"\n";
         }
         std::cout<<"\n";
    }*/

    // Exhaustive Parallel Optimization
    std::vector<strategy_t> generated_1 {};
    btf.run_parallel_optimization( search_space, generated_1,
                                   optim_file,  param_file, fitness_metric,
                                   datafeed, true, true );
    if( generated_1.empty() ){
        std::cout<<">>> ERROR: no strategy generated (mode_factory_sequential)\n";
        exit(1);
    }
    //---
    //--- SELECTION STEP 1
    double AvgTicks_1 {8};
    double Zscore_1 {1.1};
    std::vector<strategy_t> selected_1 {};
    // Instantiate Validation object
    Validation val1 { btf, datafeed, generated_1, selected_file,
                      validated_file, fitness_metric, data_dir,
                      data_file_oos, max_variation_pct, num_noise_tests,
                      noise_file };
    val1.intermediate_selection(generated_1, selected_1);
    std::cout << "Number of strategies passing 1st generation step: "
              << selected_1.size() <<"\n";
    if( selected_1.empty() ){
        exit(1);
    }
    //---


    //--- GENERATION STEP 2
    // Extract parameter ranges from selected_1
    param_ranges_t p_ranges_2 {
        utils_params::param_ranges_from_all_strategies(selected_1) };
    // Add optimization parameter
    utils_params::replace_opt_range_by_name( "DOW_switch", parameter_ranges,
                                             p_ranges_2 );
    // Combine parameter ranges into all parameter combinations
    search_space = utils_params::cartesian_product(p_ranges_2);
    // Exhaustive Parallel Optimization
    std::vector<strategy_t> generated_2 {};
    btf.run_parallel_optimization( search_space, generated_2,
                                   optim_file,  param_file, fitness_metric,
                                   datafeed, true, true );
    if( generated_2.empty() ){
         std::cout<<">>> ERROR: no strategy generated (mode_factory_sequential)\n";
         exit(1);
    }
    //---
    //--- SELECTION STEP 2
    std::vector<strategy_t> selected_2 {};
    double AvgTicks_2 {0};
    double Zscore_2 {0};
    for( auto it = generated_2.begin();
              it != generated_2.end(); it++ ){
        // Read metrics from optimization results
        for( auto optrun = it->begin(); optrun != it->end(); optrun++ ){
            if( optrun->first == "AvgTicks" ){
                AvgTicks_2 = optrun->second;
            }
            else if( optrun->first == "Z-score" ){
                Zscore_2 = optrun->second;
            }
        }
        selection_conditions = ( AvgTicks_2 > AvgTicks_1
                                 && Zscore_2 > Zscore_1 );
        if( selection_conditions ){
            selected_2.push_back(*it);
        }
    }
    std::cout << "Number of strategies passing 2nd generation step: "
              << selected_2.size() <<"\n";
    if( selected_2.empty() ){
       exit(1);
    }
    //---

    //--- GENERATION STEP 3
    // Extract parameter ranges from selected_1
    param_ranges_t p_ranges_3 {
        utils_params::param_ranges_from_all_strategies(selected_2) };
    // Add optimization parameter
    utils_params::replace_opt_range_by_name( "Intraday_switch", parameter_ranges,
                                             p_ranges_3 );
    // Combine parameter ranges into all parameter combinations
    search_space = utils_params::cartesian_product(p_ranges_3);
    // Exhaustive Parallel Optimization
    std::vector<strategy_t> generated_3 {};
    btf.run_parallel_optimization( search_space, generated_3,
                                   optim_file,  param_file, fitness_metric,
                                   datafeed, true, true );
    if( generated_3.empty() ){
         std::cout<<">>> ERROR: no strategy generated (mode_factory_sequential)\n";
         exit(1);
    }
    //---
    //--- SELECTION STEP 3
    std::vector<strategy_t> selected_3 {};
    double AvgTicks_3 {0};
    double Zscore_3 {0};
    for( auto it = generated_3.begin();
              it != generated_3.end(); it++ ){
        // Read metrics from optimization results
        for( auto optrun = it->begin(); optrun != it->end(); optrun++ ){
            if( optrun->first == "AvgTicks" ){
                AvgTicks_3 = optrun->second;
            }
            else if( optrun->first == "Z-score" ){
                Zscore_3 = optrun->second;
            }
        }
        selection_conditions = ( AvgTicks_3 > AvgTicks_2
                                 && Zscore_3 > Zscore_2 );
        if( selection_conditions ){
            selected_3.push_back(*it);
        }
    }
    std::cout << "Number of strategies passing 3rd generation step: "
              << selected_3.size() <<"\n";
    if( selected_3.empty() ){
       exit(1);
    }
    //---


    //--- GENERATION STEP 4
    // Extract parameter ranges from selected_1
    param_ranges_t p_ranges_4 {
        utils_params::param_ranges_from_all_strategies(selected_3) };
    // Add optimization parameter
    switch( side_switch ){
        case 1:
            utils_params::replace_opt_range_by_name( "Filter1L_switch",
                                                     parameter_ranges,
                                                     p_ranges_4 );
            break;
        case 2:
            utils_params::replace_opt_range_by_name( "Filter1S_switch",
                                                     parameter_ranges,
                                                     p_ranges_4 );
            break;
        case 3:
            utils_params::replace_opt_range_by_name( "Filter1L_switch",
                                                     parameter_ranges,
                                                     p_ranges_4 );
            utils_params::replace_opt_range_by_name( "Filter1S_switch",
                                                     parameter_ranges,
                                                     p_ranges_4 );
            break;
        default:
            std::cout << ">>> ERROR: Invalid Side_switch parameter in XML"
                      << " (mode_factory_sequential).\n";
            exit(1);
    }
    // Combine parameter ranges into all parameter combinations
    search_space = utils_params::cartesian_product(p_ranges_4);
    // Exhaustive Parallel Optimization
    std::vector<strategy_t> generated_4 {};
    btf.run_parallel_optimization( search_space, generated_4,
                                   optim_file,  param_file, fitness_metric,
                                   datafeed, true, true );
    if( generated_4.empty() ){
         std::cout<<">>> ERROR: no strategy generated (mode_factory_sequential)\n";
         exit(1);
    }
    //---
    //--- SELECTION STEP 4
    std::vector<strategy_t> selected_4 {};
    double AvgTicks_4 {0};
    double Zscore_4 {0};
    for( auto it = generated_3.begin();
              it != generated_3.end(); it++ ){
        // Read metrics from optimization results
        for( auto optrun = it->begin(); optrun != it->end(); optrun++ ){
            if( optrun->first == "AvgTicks" ){
                AvgTicks_4 = optrun->second;
            }
            else if( optrun->first == "Z-score" ){
                Zscore_4 = optrun->second;
            }
        }
        selection_conditions = ( AvgTicks_4 > AvgTicks_3
                                 && Zscore_4 > Zscore_3 );
        if( selection_conditions ){
            selected_4.push_back(*it);
        }
    }
    std::cout << "Number of strategies passing 4th generation step: "
              << selected_4.size() <<"\n";
    if( selected_4.empty() ){
       exit(1);
    }
    //---

    // --------------------------------------------------------------------- //


    // ---------------------------    VALIDATION   ------------------------- //
    // Instantiate Validation object
    Validation validation { btf, datafeed, selected_4, selected_file,
                            validated_file, fitness_metric,
                            data_dir, data_file_oos, max_variation_pct,
                            num_noise_tests, noise_file };
    // Run full validation process
    validation.run_validation();
    // --------------------------------------------------------------------- //
}








// ------------------------------------------------------------------------- //
/*! Overview of Market main features (no trade)
*/
void mode_overview( BTfast &btf, std::unique_ptr<DataFeed> &datafeed,
                    const param_ranges_t &parameter_ranges,
                    const std::string &overview_file )
{
    std::cout << utils_time::current_datetime_str() + " | "
              << "Running Market Overview \n";
    // Extract single parameter combination from parameter_ranges
    // (only the <Start> value is taken)
    parameters_t parameter_combination {
        utils_params::single_parameter_combination(parameter_ranges) };
    // Initialize Account
    Account account { btf.initial_balance() };

    // Parse data and collect market info, without strategy signals
    btf.run_overview( account, datafeed, parameter_combination );

    //--- Write to overview_file
    std::ofstream outfile;
    outfile.open( overview_file );

    for( auto p: btf.eod_prices() ){
        outfile << p.first.tostring() <<", "<< p.second <<"\n";
    }
    outfile << "\n\n";

    for( int i=0; i < btf.volume_hour().size(); i++ ){
        outfile << i <<", " << btf.volume_hour().at(i) <<"\n";
    }
    outfile << "\n\n";

    for( int i=0; i < btf.co_range_dow().size(); i++ ){
        outfile << i+1 <<", " << btf.co_range_dow().at(i) <<"\n";
    }
    outfile << "\n\n";

    for( auto r: btf.hl_range() ){
        outfile << r <<"\n";
    }
    outfile << "\n\n";

    outfile.close();
    std::cout<< "\nOverview info written on file: " << overview_file << "\n";
    //---

    // Execute script for gnuplot and open the PNG file
    std::string command { "./bin/PlotMktOverview" };
    std::system(command.c_str());
}



// ------------------------------------------------------------------------- //
/*! Noise Test for Single Strategy (adding gaussian noise to price data)
*/
void mode_noise( BTfast &btf,
                 std::unique_ptr<DataFeed> &datafeed,
                 const param_ranges_t &parameter_ranges,
                 int num_noise_tests, bool show_plot,
                 const std::string &noise_file,
                 const std::string &param_file,
                 const std::string &fitness_metric )
{
    // Extract single parameter combination from parameter_ranges
    // (only the <Start> value is taken)
    parameters_t parameter_combination {
        utils_params::single_parameter_combination(parameter_ranges) };

    // Initialize vector to store results of noise test
    std::vector<strategy_t> noise_results {};

    //-- Backtest on original price data (without noise)
    Account account { btf.initial_balance() };
    btf.run_backtest( account, datafeed,
                      parameter_combination );
    Performance performance { btf.initial_balance(), btf.day_counter(),
                              std::vector<Transaction> {} };
    performance.set_transactions( account.transactions() );
    performance.compute_metrics();
    utils_optim::append_to_optim_results( noise_results,
                                          performance,
                                          parameter_combination );
    //--

    //-- Backtest with noised data
    // Replicate the same 'parameter_combination' for
    // ('num_noise_tests'-1) times, since 1 run is on original data
    std::vector<parameters_t> search_space {};

    for( int i = 0; i < num_noise_tests - 1; i++ ){
        search_space.push_back(parameter_combination);
    }

    // Each run is with same strategy parameters but with
    // random noise added to price data
    bool sort_results {false};
    bool verbose {false};
    btf.set_random_noise(true);
    btf.run_parallel_optimization( search_space, noise_results, noise_file,
                                   param_file, fitness_metric,
                                   datafeed,
                                   //btf.start_date(),btf.end_date(),
                                   sort_results, verbose );
    //--

    // Plot distributions of performance metrics under noise test
    if( show_plot ){
        // Execute script for gnuplot and open the PNG file
        std::string command = "./bin/PlotNoiseDistributions";
        system(command.c_str());
    }
}
