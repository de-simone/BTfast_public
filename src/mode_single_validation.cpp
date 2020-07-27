#include "run_modes.h"

#include "account.h"
#include "performance.h"
#include "utils_optim.h"    // append_to_optim_results
#include "utils_params.h"   // single_parameter_combination
#include "utils_print.h"    // show_backtest_results
#include "utils_time.h"     // current_datetime_str
#include "validation.h"

#include <iostream>         // std::cout


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
    std::cout<< "    Run Mode   : Validation for Single Strategy\n\n";
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

    Performance performance { btf.initial_balance(), 
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
    Validation validation { btf, datafeed, strategy_to_validate, parameter_ranges,
                            selected_file, validated_file, fitness_metric,
                            data_dir, data_file_oos, max_variation_pct,
                            num_noise_tests, noise_file };

    // Run full validation process
    validation.run_validation();
    // --------------------------------------------------------------------- //
}
