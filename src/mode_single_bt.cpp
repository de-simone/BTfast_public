#include "run_modes.h"

#include "account.h"
#include "performance.h"
#include "utils_params.h"   // single_parameter_combination
#include "utils_print.h"    // show_backtest_results
#include "utils_time.h"     // current_datetime_str

#include <iostream>         // std::cout



// ------------------------------------------------------------------------- //
/*! Single Backtest
*/
void mode_single_bt( BTfast &btf,
                     std::unique_ptr<DataFeed> &datafeed,
                     const param_ranges_t &parameter_ranges,
                     bool print_trade_list, bool print_performance_report,
                     bool write_trades_to_file,
                     const std::string &param_file,
                     const std::string &trade_list_file,
                     const std::string &performance_file,
                     const std::string &profits_file )
{
    std::cout<< "    Run Mode   : Single Backtest\n\n";
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
    Performance performance { btf.initial_balance(), 
                              std::vector<Transaction> {} };

    // Load transaction history into performance object
    performance.set_transactions( account.transactions() );

    // Print results on stdout and on file
    utils_print::show_backtest_results( account, performance,
                               btf.strategy_name(), btf.symbol().name(),
                               btf.timeframe(),
                               btf.first_date_parsed(), btf.last_date_parsed(),
                               print_trade_list, print_performance_report,
                               write_trades_to_file, param_file, trade_list_file,
                               performance_file, profits_file );

}
