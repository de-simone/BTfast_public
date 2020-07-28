#include "run_modes.h"

#include "account.h"
#include "performance.h"
#include "utils_optim.h"    // append_to_optim_results
#include "utils_params.h"   // single_parameter_combination
#include "utils_time.h"     // current_datetime_str
#include "validation.h"

#include <cstdlib>          // std::system
#include <iostream>         // std::cout

// ------------------------------------------------------------------------- //
/*! Noise Test for Single Strategy (adding gaussian noise to price data)
*/
void mode_noise( BTfast &btf,
                 std::unique_ptr<DataFeed> &datafeed,
                 const param_ranges_t &parameter_ranges,
                 int num_noise_tests, bool write_trades_to_file,
                 const std::string &noise_file,
                 const std::string &param_file,
                 const std::string &fitness_metric )
{
    std::cout<< "    Run Mode   : Single Noise Test\n\n";

    // ----------------------------    BACKTEST   -------------------------- //
    std::cout << utils_time::current_datetime_str() + " | "
              << "Running Backtest \n";

    // Extract single parameter combination from parameter_ranges
    // (only the <Start> value is taken)
    parameters_t parameter_combination {
        utils_params::single_parameter_combination(parameter_ranges) };


    //-- Backtest on original price data (without noise)
    Account account { btf.initial_balance() };
    btf.run_backtest( account, datafeed, parameter_combination );
    Performance performance { btf.initial_balance(),
                              std::vector<Transaction> {} };
    performance.set_transactions( account.transactions() );
    performance.compute_metrics();
    // Initialize vector to store results of backtest
    std::vector<strategy_t> strategy_to_validate {};
    utils_optim::append_to_optim_results( strategy_to_validate, performance,
                                          parameter_combination );
    //--

    // --------------------------------------------------------------------- //

    // ---------------------------    VALIDATION   ------------------------- //
    // Instantiate Validation object
    Validation validation { btf,datafeed, strategy_to_validate,parameter_ranges,
                            "", "", fitness_metric, "", "", 0,
                            num_noise_tests, noise_file };

    // Run full validation process
    std::vector<strategy_t> passed_test {};
    validation.noise_test( strategy_to_validate, passed_test, true );
    // --------------------------------------------------------------------- //

    // Plot distributions of performance metrics under noise test
    if( write_trades_to_file ){
        // Execute script for gnuplot and open the PNG file
        std::string command = "./bin/PlotNoiseDistributions";
        system(command.c_str());
    }
}
