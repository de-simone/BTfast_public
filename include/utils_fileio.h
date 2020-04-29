#ifndef UTILS_FILEIO_H
#define UTILS_FILEIO_H

#include "btfast.h"         // strategy_t alias
#include "datetime.h"

#include <string>           // std::string


// Set of Utility functions for file input/output



namespace utils_fileio {

    // --------------------------------------------------------------------- //
    /*! Read input settings from configuration XML file
        and store them into variables passed by reference.
    */
    void read_config_file( std::string config_file, std::string &main_dir,
                        int &run_mode,
                        std::string &strategy_name, std::string &symbol_name,
                        std::string &timeframe, std::string &input_start_date,
                        std::string &input_end_date,
                        std::string &data_dir, std::string &data_file,
                        int &csv_format, std::string &datafeed_type,
                        bool &print_progress,
                        bool &print_performance_report, bool &print_trade_list,
                        bool &show_plot, std::string &fitness_metric,
                        int &population_size, int &generations,
                        int &max_bars_back, double &initial_balance,
                        std::string &position_size_type,
                        int &num_contracts, double &risk_fraction,
                        bool &include_commissions, int &slippage,
                        std::string &data_file_oos,
                        int &max_variation_pct, int &num_noise_tests );

    // --------------------------------------------------------------------- //
    /*! Read parameter values/ranges from  XML parameter file
        and store them into vector
    */
    param_ranges_t read_param_file( std::string paramfile );


    // --------------------------------------------------------------------- //
    /*! Copy (append) content of file 'inputfile' to file 'outputfile'
        with each line starting with '#'
    */
    void copy_file_to_file( std::string sourcefile, std::string destfile );

    // --------------------------------------------------------------------- //
    /*! Write strategies (e.g. from optimization results) stored in 'optim'
        to CSV file.
        Return: 1 if OK, 0 otherwise
    */
    int write_strategies_to_file( std::string fname, std::string paramfile,
                                   const std::vector<strategy_t> &optim,
                                   std::string strategy_name,
                                   std::string symbol_name,
                                   std::string timeframe,
                                   const Date &date_i, const Date &date_f,
                                   bool verbose );

    // --------------------------------------------------------------------- //
    /*! Read strategies (metrics+parameters) from CSV file 'filename'
    */
    std::vector<strategy_t> read_strategies_from_file( std::string filename );

}



#endif
