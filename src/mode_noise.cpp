#include "run_modes.h"

#include "account.h"
#include "performance.h"
#include "utils_optim.h"    // append_to_optim_results
#include "utils_params.h"   // single_parameter_combination

#include <cstdlib>          // std::system

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
