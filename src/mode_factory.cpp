#include "run_modes.h"

#include "utils_fileio.h"   // read_strategies_from_file
#include "utils_optim.h"    // remove_duplicates
#include "utils_params.h"   // cartesian_product,
                            // extract_parameters_from_all_strategies
                            // expand_strategies_with_opt_range,
                            // first_parameters_from_range,
                            // parameter_value_by_name,
                            // no_filter_strategy
                            // strategy_attribute_by_name
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
            std::cout<< "    Run Mode   : Strategy Factory (Exhaustive Generation + Validation)\n\n";
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
            std::cout<< "    Run Mode   : Strategy Factory (Genetic Generation + Validation)\n\n";
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
/*! Strategy Factory mode (Sequential Generation + Validation)
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
    double zscore_with_filter {0};
    double zscore_no_filter {0};
    double perf_relative_improvement {0.2};
    strategy_t no_filter_strat {};
    //utils_params::print_param_ranges_t(parameter_ranges);

    // ----------------------    STATEGY GENERATION    --------------------- //

    //--- GENERATION STEP 1
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
    //utils_params::print_parameters_t_vector( search_space );

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
    std::vector<strategy_t> selected_1 {};
    // Instantiate Validation object
    Validation val1 { btf, datafeed, generated_1, selected_file,
                      validated_file, fitness_metric, data_dir,
                      data_file_oos, max_variation_pct, num_noise_tests,
                      noise_file };
    val1.initial_generation_selection( generated_1, selected_1 );
    std::cout << "Number of strategies passing 1st generation step: "
              << selected_1.size() <<"\n";
    if( selected_1.empty() ){
        exit(1);
    }
    //---

    //--- GENERATION STEP 2
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
        selected_2.push_back(no_filter_strat);
        // Append strategy with new filter if it improves metrics
        if( selection_conditions ){
            selected_2.push_back(strat);
        }
    }
    utils_optim::remove_duplicates( selected_2, fitness_metric );
    std::cout << "Number of strategies passing 2nd generation step: "
              << selected_2.size() <<"\n";
    if( selected_2.empty() ){
       exit(1);
    }
    //---

    //--- GENERATION STEP 3
    // Extract strategy parameters from selected_1 to search space
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
    std::cout << "Number of strategies passing 3rd generation step: "
              << selected_3.size() <<"\n";
    if( selected_3.empty() ){
       exit(1);
    }
    //---

    //--- GENERATION STEP 4
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
