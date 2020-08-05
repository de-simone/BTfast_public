#include "run_modes.h"

#include "utils_fileio.h"   // read_strategies_from_file
#include "utils_optim.h"    // remove_duplicates
#include "utils_params.h"   // cartesian_product,
                            // extract_parameters_from_all_strategies,
                            // expand_strategies_with_opt_range,
                            // first_parameters_from_range,
                            // parameter_value_by_name,
                            // set_parameter_value_by_name,
                            // no_filter_strategy,
                            // strategy_attribute_by_name,
                            // max_strategy_metric_by_name
#include "utils_time.h"     // current_datetime_str

#include "validation.h"

#include <iostream>         // std::cout

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

        // Initialize vector to store results of strategy generation
        // (new optimization or imported from file)
        // [ [("metric1", 110.2), ("metric2", 2.1), ("p1", 2.0), ("p2", 21.0), ...],
        std::vector<strategy_t> generated_strategies {};

        // --------------------    STATEGY GENERATION    ------------------- //
        // ----------------    (optimization or from file)    -------------- //
        if( optim_mode == "parallel" ){
            std::cout<< "    Run Mode   : Strategy Factory (Exhaustive Generation + Validation)\n\n";

            // Combine 'parameter_ranges' into all parameter combinations
            std::vector<parameters_t> search_space {
                         utils_params::cartesian_product(parameter_ranges) };
            // Exhaustive Parallel Optimization
            btf.run_parallel_optimization( search_space, generated_strategies,
                                           optim_file, param_file,
                                           fitness_metric, datafeed,
                                           //btf.start_date(),btf.end_date(),
                                           true, true );
        }

        else if( optim_mode == "genetic" ){
            std::cout<< "    Run Mode   : Strategy Factory (Genetic Generation + Validation)\n\n";
            std::cout<<"\n--- Strategy Generation N. " << i + 1
                     << " / "<< max_num_generations <<" ---\n";

            // Combine 'parameter_ranges' into all parameter combinations
            std::vector<parameters_t> search_space {
                        utils_params::cartesian_product(parameter_ranges) };
            // Genetic Parallel Optimization
            btf.run_genetic_optimization( search_space, generated_strategies,
                                          optim_file, param_file,
                                          fitness_metric, datafeed,
                                          //btf.start_date(),btf.end_date(),
                                          population_size, generations );
        }

        else if( optim_mode == "import" ){
            std::cout<< "    Run Mode   : Strategy Factory (Import Generation Results + Validation)\n\n";
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
        Validation validation { btf, datafeed, generated_strategies,
                                parameter_ranges, selected_file,
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
/*! Strategy Factory mode (Sequential Generation + Validation)

        - generation step 1 (POI, Distance, fractN, Exit),
            initial generation selection
        - generation step 2 (DOW), selection step 2
        - generation step 3 (Intraday), selection step 3
        - generation step 4 (Filter1), selection step 4
        - generation + selection step 5 (MktRegime)


    Names/Number/Order of performance metrics must be matched among:
        - utils_params::extract_parameters_from_single_strategy
        - utils_params::extract_metrics_from_single_strategy
        - utils_optim::append_to_optim_results
        - utils_optim::sort_by_metric
        - utils_optim::sort_by_ntrades, utils_optim::sort_by_avgtrade, etc
        - utils_fileio::write_strategies_to_file
        - Individual::compute_individual_fitness
        - Validation::intermediate_selection
        - Validation::selection_conditions
        - Validation::noise_test
        - mode_factory_sequential

    >> Parameter names must match those in MasterCode.xml <<
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
    std::cout<< "    Run Mode   : Strategy Factory (Sequential Generation + Validation)\n\n";
    // Check if strategy name is MasterCode
    if( btf.strategy_name() != "MasterCode" ){
        std::cout << ">>> ERROR: Sequential strategy generation available"
                  << " only for MasterCode (mode_factory_sequential).\n";
        exit(1);
    }
    // Store value of Side switch (1=Long, 2=Short, 3=Both)
    int side_switch { utils_params::parameter_value_by_name("Side_switch",
                                                            parameter_ranges) };
    // Initialize variables
    std::vector<parameters_t> search_space {};
    bool selection_conditions {false};
    double avgticks_with_filter {0};
    double avgticks_no_filter {0};
    // double stdticks_no_filter {0};
    double zscore_with_filter {0};
    double zscore_no_filter {0};
    double perf_relative_improvement {0.2};
    strategy_t no_filter_strat {};
    //utils_params::print_param_ranges_t(parameter_ranges);


    // ----------------------    STATEGY GENERATION    --------------------- //

    //--- GENERATION STEP 1
    std::cout << "\nStarting 1st generation step "
              << "(POI_switch, Distance_switch, fractN, Exit_switch) \n";
    // Initial parameter set with just the first element of each
    // parameter in parameter_ranges
    search_space = utils_params::first_parameters_from_range(parameter_ranges);
    // Add optimization parameter range to each strategy in search space
    utils_params::expand_strategies_with_opt_range(
                            "POI_switch", parameter_ranges, search_space);
    // Add optimization parameter range to each strategy in search space
    utils_params::expand_strategies_with_opt_range(
                            "Distance_switch", parameter_ranges, search_space);
    // Add optimization parameter range to each strategy in search space
    if( side_switch == 1 || side_switch == 3 ){
        utils_params::expand_strategies_with_opt_range(
                            "fractN_long", parameter_ranges, search_space);
    }
    if( side_switch == 2 || side_switch == 3 ){
        utils_params::expand_strategies_with_opt_range(
                            "fractN_short", parameter_ranges, search_space);
    }
    if( side_switch != 1 && side_switch != 2 && side_switch != 3 ){
        std::cout << ">>> ERROR: Invalid Side_switch parameter in XML"
                  << " (mode_factory_sequential).\n";
        exit(1);
    }
    // Add optimization parameter range to each strategy in search space
    utils_params::expand_strategies_with_opt_range(
                            "Exit_switch", parameter_ranges, search_space);
    //utils_params::print_parameters_t_vector( search_space );

    // Exhaustive Parallel Optimization
    std::vector<strategy_t> generated_1 {};
    btf.run_parallel_optimization( search_space, generated_1, optim_file,
                                   param_file, fitness_metric, datafeed,
                                   true, true );
    if( generated_1.empty() ){
        std::cout<<">>> ERROR: no strategy generated (mode_factory_sequential)\n";
        exit(1);
    }
    //---
    //--- SELECTION STEP 1
    std::vector<strategy_t> selected_1 {};
    // Instantiate Validation object
    Validation val1 { btf, datafeed, generated_1, parameter_ranges,
                      selected_file, validated_file, fitness_metric,
                      data_dir, data_file_oos, max_variation_pct,
                      num_noise_tests, noise_file };
    val1.initial_generation_selection( generated_1, selected_1 );
    std::cout << "Number of strategies passing 1st generation step "
              << "(POI_switch, Distance_switch, fractN, Exit_switch) : "
              << selected_1.size() <<"\n\n";
    if( selected_1.empty() ){
        exit(1);
    }
    //---


    //--- GENERATION STEP 2
    std::cout << "\nStarting 2nd generation step (DOW_switch) \n";
    // Extract strategy parameters from selected_1 to search space
    utils_params::extract_parameters_from_all_strategies( selected_1,
                                                          search_space );
    // Add optimization parameter range to each strategy in search space
    utils_params::expand_strategies_with_opt_range(
                                "DOW_switch", parameter_ranges, search_space);
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
    for( const auto& strat: generated_2 ){
        // Find strategy equal to 'strat' except without new filter
        no_filter_strat  = utils_params::no_filter_strategy("DOW_switch",
                                                            strat, generated_2);
        if( no_filter_strat.empty() ){
            continue;   // skip strat if corresponding no_filter_strat not found
        }
        // Metrics of strategy without new filter
        avgticks_no_filter = utils_params::strategy_attribute_by_name(
                                                "AvgTicks", no_filter_strat );
        //stdticks_no_filter = utils_params::strategy_attribute_by_name(
        //                                        "StdTicks", no_filter_strat );
        zscore_no_filter = utils_params::strategy_attribute_by_name(
                                                 "Z-score", no_filter_strat );
        // Metrics of strategy with new filter
        avgticks_with_filter = utils_params::strategy_attribute_by_name(
                                                "AvgTicks", strat );
        zscore_with_filter = utils_params::strategy_attribute_by_name(
                                                "Z-score", strat );
        //selection_conditions = avgticks_with_filter >= avgticks_no_filter
        //          + 1.5*stdticks_no_filter;
        selection_conditions =
            ( avgticks_with_filter >= avgticks_no_filter
                                        * ( 1 + perf_relative_improvement )
            && zscore_with_filter >= zscore_no_filter
                                        * ( 1 + perf_relative_improvement ) );
        // Append strategy without new filter to selected vector
        selected_2.push_back(no_filter_strat);
        // Append strategy with new filter if it improves metrics
        if( selection_conditions ){
            selected_2.push_back(strat);
        }
    }

    utils_optim::remove_duplicates( selected_2, fitness_metric );
    std::cout << "Number of strategies passing 2nd generation step "
              << "(DOW_switch) : "
              << selected_2.size() <<"\n\n";
    if( selected_2.empty() ){
       exit(1);
    }
    //---

    //--- GENERATION STEP 3
    std::cout << "\nStarting 3rd generation step (Intraday_switch) \n";
    // Extract strategy parameters from selected_2 to search space
    utils_params::extract_parameters_from_all_strategies( selected_2,
                                                          search_space );
    // Add optimization parameter range to each strategy in search space
    utils_params::expand_strategies_with_opt_range(
                        "Intraday_switch", parameter_ranges, search_space);
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
    for( const auto& strat: generated_3 ){
        // Find strategy equal to 'strat' except without new filter
        no_filter_strat  = utils_params::no_filter_strategy("Intraday_switch",
                                                            strat, generated_3);
        if( no_filter_strat.empty() ){
            continue;   // skip strat if corresponding no_filter_strat not found
        }
        // Metrics of strategy without new filter
        avgticks_no_filter = utils_params::strategy_attribute_by_name(
                                                "AvgTicks", no_filter_strat );
        zscore_no_filter = utils_params::strategy_attribute_by_name(
                                                 "Z-score", no_filter_strat );
        // Metrics of strategy with new filter
        avgticks_with_filter = utils_params::strategy_attribute_by_name(
                                                "AvgTicks", strat );
        zscore_with_filter = utils_params::strategy_attribute_by_name(
                                                "Z-score", strat );
        selection_conditions =
            ( avgticks_with_filter >= avgticks_no_filter
                                        * ( 1 + perf_relative_improvement )
            && zscore_with_filter >= zscore_no_filter
                                        * ( 1 + perf_relative_improvement ) );
        // Append strategy without new filter to selected vector
        selected_3.push_back(no_filter_strat);
        // Append strategy with new filter if it improves metrics
        if( selection_conditions ){
            selected_3.push_back(strat);
        }
    }
    utils_optim::remove_duplicates( selected_3, fitness_metric );
    std::cout << "Number of strategies passing 3rd generation step "
              << "(Intraday_switch) : "
              << selected_3.size() <<"\n\n";
    if( selected_3.empty() ){
       exit(1);
    }
    //---

    //--- GENERATION STEP 4
    std::cout << "\nStarting 4th generation step (Filter1L_switch, Filter1S_switch) \n";
    // Extract strategy parameters from selected_3 to search space
    utils_params::extract_parameters_from_all_strategies( selected_3,
                                                          search_space );
    // Add optimization parameter range to each strategy in search space
    if( side_switch == 1 || side_switch == 3 ){
        utils_params::expand_strategies_with_opt_range(
                        "Filter1L_switch", parameter_ranges, search_space);
    }
    if( side_switch == 2 || side_switch == 3 ){
        utils_params::expand_strategies_with_opt_range(
                        "Filter1S_switch", parameter_ranges, search_space);
    }
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
    for( const auto& strat: generated_4 ){
        switch( side_switch ){
            case 1:
                // Find strategy equal to 'strat' except without new filter
                no_filter_strat = utils_params::no_filter_strategy(
                                        "Filter1L_switch", strat, generated_4 );
                break;
            case 2:
                // Find strategy equal to 'strat' except without new filter
                no_filter_strat = utils_params::no_filter_strategy(
                                        "Filter1S_switch", strat, generated_4 );
                break;
            case 3:
                // Find strategy equal to 'strat' except without two new filters
                no_filter_strat = utils_params::no_two_filters_strategy(
                                        "Filter1L_switch", "Filter1S_switch",
                                        strat, generated_4 );
                break;
        }
        if( no_filter_strat.empty() ){
            continue;   // skip strat if corresponding no_filter_strat not found
        }
        // Metrics of strategy without new filter(s)
        avgticks_no_filter = utils_params::strategy_attribute_by_name(
                                                "AvgTicks", no_filter_strat );
        zscore_no_filter = utils_params::strategy_attribute_by_name(
                                                "Z-score", no_filter_strat );
        // Metrics of strategy with new filter(s)
        avgticks_with_filter = utils_params::strategy_attribute_by_name(
                                                "AvgTicks", strat );
        zscore_with_filter = utils_params::strategy_attribute_by_name(
                                                "Z-score", strat );
        selection_conditions =
            ( avgticks_with_filter >= avgticks_no_filter
                                        * ( 1 + perf_relative_improvement )
            && zscore_with_filter >= zscore_no_filter
                                        * ( 1 + perf_relative_improvement ) );
        // Append strategy without new filter to selected vector
        selected_4.push_back(no_filter_strat);
        // Append strategy with new filter if it improves metrics
        if( selection_conditions ){
            selected_4.push_back(strat);
        }
    }
    utils_optim::remove_duplicates( selected_4, fitness_metric );
    std::cout << "Number of strategies passing 4th generation step "
              << "(Filter1L_switch, Filter1S_switch) : "
              << selected_4.size() <<"\n\n";
    if( selected_4.empty() ){
       exit(1);
    }
    //---

    std::vector<strategy_t> selected_5 { selected_4 };//<<<
    /*
    //--- GENERATION + SELECTION STEP 5
    std::cout << "\nStarting 5th generation step (MktRegimeL_switch, MktRegimeS_switch) \n";    
    std::vector<strategy_t> selected_5 {};
    for( const auto& selected_strat: selected_4 ){   // loop over selected strategies

        std::cout << utils_time::current_datetime_str() + " | "
                  << "Analyzing Market Regime filters for strategy "
                  << &selected_strat-&selected_4[0] + 1
                  << " / " << selected_4.size() <<"\n";
        // GENERATION STEP 5
        parameters_t selected_strat_params {};
        // Extract parameters from each strategy in selected_4
        utils_params::extract_parameters_from_single_strategy(
                                        selected_strat, selected_strat_params);
        // De-activate DPS (DPS_switch should be 0, already)
        utils_params::set_parameter_value_by_name( "DPS_switch",
                                                   selected_strat_params, 0);
        // Append parameters to search space
        search_space.clear();
        search_space.push_back(selected_strat_params);

        // Add optimization parameter range to each strategy in search space
        if( side_switch == 1 || side_switch == 3 ){
            utils_params::expand_strategies_with_opt_range(
                        "MktRegimeL_switch", parameter_ranges, search_space);
        }
        if( side_switch == 2 || side_switch == 3 ){
            utils_params::expand_strategies_with_opt_range(
                        "MktRegimeS_switch", parameter_ranges, search_space);
        }
        // Exhaustive Parallel Optimization
        std::vector<strategy_t> generated_5 {};
        btf.run_parallel_optimization( search_space, generated_5,
                                       optim_file,  param_file, fitness_metric,
                                       datafeed, true, false );
        if( generated_5.empty() ){
             std::cout<<">>> ERROR: no strategy generated (mode_factory_sequential)\n";
             exit(1);
        }

        // SELECTION STEP 5
        for( auto& strat: generated_5 ){
            switch( side_switch ){
                case 1:
                    // Find strategy equal to 'strat' except without new filter
                    no_filter_strat = utils_params::no_filter_strategy(
                                        "MktRegimeL_switch", strat, generated_5);
                    break;
                case 2:
                    // Find strategy equal to 'strat' except without new filter
                    no_filter_strat = utils_params::no_filter_strategy(
                                        "MktRegimeS_switch", strat, generated_5);
                    break;
                case 3:
                    // Find strategy equal to 'strat' except without two new filters
                    no_filter_strat = utils_params::no_two_filters_strategy(
                                        "MktRegimeL_switch", "MktRegimeS_switch",
                                        strat, generated_5 );
                    break;
            }
            if( no_filter_strat.empty() ){
                continue;   // skip strat if corresponding no_filter_strat not found
            }
            // Metric of strategy without new filter(s)
            avgticks_no_filter = utils_params::strategy_attribute_by_name(
                                                "AvgTicks", no_filter_strat );
            // Metric of strategy with new filter(s)
            avgticks_with_filter = utils_params::strategy_attribute_by_name(
                                                    "AvgTicks", strat );
            // Maximum value of metric over genedated strategies
            double max_avgticks { utils_params::max_strategy_metric_by_name(
                                                    "AvgTicks", generated_5 ) };

            selection_conditions = ( max_avgticks > 0.0
                                && avgticks_with_filter == max_avgticks
                                && max_avgticks >= avgticks_no_filter
                                    * ( 1 + 0.5 * perf_relative_improvement ) );

            if( selection_conditions ){
                // Activate DPS
                parameters_t strat_params {};
                utils_params::extract_parameters_from_single_strategy(strat,
                                                                 strat_params);
                utils_params::set_parameter_value_by_name( "DPS_switch",
                                                           strat_params, 1);
                // Backtest strategy with DPS
                Account account_dps { btf.initial_balance() };
                btf.run_backtest( account_dps, datafeed, strat_params );
                Performance performance_dps { btf.initial_balance(),
                                              std::vector<Transaction> {} };
                performance_dps.set_transactions( account_dps.transactions() );
                performance_dps.compute_metrics();
                std::vector<strategy_t> optim_results {};
                utils_optim::append_to_optim_results( optim_results,
                                                      performance_dps,
                                                      strat_params );
                // Append strategy with new filter and DPS activated
                // if performance metrics are different than the one w/o filter
                if( utils_optim::equal_strategy_metrics( optim_results.at(0),
                                                         no_filter_strat ) ){
                    selected_5.push_back( optim_results.at(0) );
                }
            }
            else{
                // Append strategy without new filter to selected vector
                selected_5.push_back(no_filter_strat);
            }
        }
    } // end loop over selected_4 strategies
    */

    utils_optim::remove_duplicates( selected_5, fitness_metric );
    std::cout << "Number of strategies passing 5th generation step "
              << "(MktRegimeL_switch, MktRegimeS_switch) : "
              << selected_5.size() <<"\n\n";
    if( selected_5.empty() ){
       exit(1);
    }
    //---

    // --------------------------------------------------------------------- //


    // ---------------------------    VALIDATION   ------------------------- //
    // Instantiate Validation object
    Validation validation { btf, datafeed, selected_5, parameter_ranges,
                            selected_file, validated_file, fitness_metric,
                            data_dir, data_file_oos, max_variation_pct,
                            num_noise_tests, noise_file };
    // Run full validation process
    validation.run_validation();
    // --------------------------------------------------------------------- //
}
