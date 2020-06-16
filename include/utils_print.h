#ifndef UTILS_PRINT_H
#define UTILS_PRINT_H

#include "account.h"
#include "events.h"
#include "performance.h"

#include <chrono>           // std::chrono
#include <queue>            // std::queue


// Set of Utility functions for printing


namespace utils_print {

    // --------------------------------------------------------------------- //
    /*! Print header on stdout
    */
    void print_header( std::string strategy_name,  std::string symbol_name,
                       std::string timeframe, std::string data_file );

     // -------------------------------------------------------------------- //
     /*! Print footer on stdout
     */
     void print_footer( std::string strategy_name, std::string symbol_name,
                        std::string timeframe, std::string data_file,
                        Date date_i, Date date_f, int day_counter, int bar_counter,
                        std::chrono::high_resolution_clock::time_point t1);

    // --------------------------------------------------------------------- //
    /*! Print all elements in events queue
    */
    void print_queue(std::queue<Event> q);

    // --------------------------------------------------------------------- //
    /*! Print bars parsed
    */
    void print_progress(int count);

    // --------------------------------------------------------------------- //
    /*! Control the printing on stdout and on file of the results of backtest
    */
    void show_backtest_results(
                    const Account &account, Performance &performance,
                    std::string strategy_name, std::string symbol_name,
                    std::string timeframe, Date date_i, Date date_f,
                    bool print_trade_list, bool print_performance_report,
                    bool show_plot, std::string paramfile,
                    std::string trade_list_file,
                    std::string performance_file, std::string profits_file );

    // --------------------------------------------------------------------- //
    /*! Print strategy parameters
    */
    //void print_strategy_params(strategy_t strat);
}


#endif
