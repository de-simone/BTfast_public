#ifndef RUN_MODES_H
#define RUN_MODES_H

#include "btfast.h"


/*!
    Set of functions to be run according to the RUN_MODE variable
    selected in settings
*/

//--


// ------------------------------------------------------------------------- //
// No Trade
void mode_notrade( BTfast &btf, std::unique_ptr<DataFeed> &datafeed,
                   const param_ranges_t &parameter_ranges);

// ------------------------------------------------------------------------- //
// Single Backtest
void mode_single_bt( BTfast &btf,
                     std::unique_ptr<DataFeed> &datafeed,
                     const param_ranges_t &parameter_ranges,
                     bool print_trade_list, bool print_performance_report,
                     bool show_plot,
                     const std::string &param_file,
                     const std::string &trade_list_file,
                     const std::string &performance_file,
                     const std::string &profits_file );

 // ------------------------------------------------------------------------- //
 // Optimization, with mode specified by 'optim_mode'
 void mode_optimization( BTfast &btf,
                         std::unique_ptr<DataFeed> &datafeed,
                         param_ranges_t &parameter_ranges,
                         const std::string &optim_mode,
                         const std::string &optim_file,
                         const std::string &param_file,
                         const std::string &fitness_metric,
                         int population_size, int generations );

// ------------------------------------------------------------------------- //
// Validation for Single Strategy (Backtest + Validation)
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
                             const std::string &noise_file );


// ------------------------------------------------------------------------- //
// Strategy Factory (Generation + Validation)
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
                   const std::string &noise_file );


// ------------------------------------------------------------------------- //
// Overview of Market main features (no trade)
void mode_overview( BTfast &btf, std::unique_ptr<DataFeed> &datafeed,
                    const param_ranges_t &parameter_ranges,
                    const std::string &overview_file );




// ------------------------------------------------------------------------- //
// Noise Test for Single Strategy
void mode_noise( BTfast &btf,
                 std::unique_ptr<DataFeed> &datafeed,
                 const param_ranges_t &parameter_ranges,
                 int num_noise_tests, bool show_plot,
                 const std::string &noise_file,
                 const std::string &param_file,
                 const std::string &fitness_metric );

#endif
