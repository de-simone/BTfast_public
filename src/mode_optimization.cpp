#include "run_modes.h"

#include "utils_params.h"   // cartesian_product

#include <iostream>         // std::cout

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

        std::cout<< "    Run Mode   : Exhaustive Parallel Optimization\n\n";
        bool sort_results {true};
        bool verbose {true};
        btf.run_parallel_optimization( search_space, optim_results, optim_file,
                                       param_file, fitness_metric,
                                       datafeed, sort_results, verbose );
    }

    else if( optim_mode == "genetic" ){ // Genetic Parallel Optimization
        std::cout<< "    Run Mode   : Genetic Parallel Optimization\n\n";
        btf.run_genetic_optimization( search_space, optim_results, optim_file,
                                      param_file, fitness_metric,
                                      datafeed, population_size, generations );
    }

    else if( optim_mode == "serial" ){  // Exhaustive Serial Optimization
        std::cout<< "    Run Mode   : Exhaustive Serial Optimization\n\n";
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
