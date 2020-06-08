#include "utils_print.h"

#include "utils_time.h" // current_datetime_str

#include <cmath>        // std::fmod
#include <cstdio>       // printf
#include <iostream>     // std::cout

/*
#include <cstdlib>      // srand, rand
#include <cstring>      // strcat
#include <fstream>      // std::fstream, open, close
#include <iomanip>      // std::setfill, std::setw
#include <numeric>      // std::accumulate
#include <random>       // random_device, mt19937, uniform_int_distribution
#include <string>       // std::getline
#include <sstream>      // ostringstream
#include <stdexcept>    // std::invalid_argument
*/

// ------------------------------------------------------------------------- //
// Print header on stdout
void utils_print::print_header( std::string strategy_name, std::string symbol_name,
                   std::string timeframe, std::string data_file )
{
    printf("\n       ====================");
    printf("\n          >>> BTfast <<<   ");
    printf("\n       ====================\n\n");

    /*
    char timebuf[80];
    time_t now = time(0);
    struct tm *ts = localtime(&now);
    strftime(timebuf, sizeof(timebuf), "%Y-%m-%d %H:%M:%S", ts);
    printf("  [ Started at %s ]\n\n", timebuf);
    */
    //cout << "  [ Started at " << current_datetime_str() << " ]\n\n";

    printf("    Strategy   : %s\n", strategy_name.c_str());
    printf("    Symbol     : %s\n", symbol_name.c_str());
    printf("    TimeFrame  : %s\n", timeframe.c_str());
    printf("    Data File  : %s\n\n", data_file.c_str());
}

// ------------------------------------------------------------------------- //
// Print footer on stdout
void utils_print::print_footer( std::string strategy_name, std::string symbol_name,
                   std::string timeframe, std::string data_file,
                   Date date_i, Date date_f, int day_counter, int bar_counter,
                   std::chrono::high_resolution_clock::time_point t1)
{
    printf("\n");
    printf("    Strategy    : %s\n", strategy_name.c_str());
    printf("    Symbol      : %s\n", symbol_name.c_str());
    printf("    TimeFrame   : %s\n", timeframe.c_str());
    printf("    Data File   : %s\n", data_file.c_str());
    printf("    Date Range  : %s --> %s   (%d Days)\n",
            date_i.tostring().c_str(), date_f.tostring().c_str(), day_counter );
            //date_f.DaysDiff(date_i) );
    printf("    Bars Parsed : %d \n\n", bar_counter );

    std::cout <<"  [ Ended at "<< utils_time::current_datetime_str()<<" ]\n\n";

    // Print execution time
    std::chrono::high_resolution_clock::time_point t2 {    // ... clock ends ticking
                        std::chrono::high_resolution_clock::now() };
    double elapsed_secs { std::chrono::duration_cast<
                        std::chrono::duration<double>>(t2 - t1).count() };

    int mm = (int)(elapsed_secs/60);
    double ss = std::fmod(elapsed_secs, 60);
    int hh = (int)(mm/60);
    mm = std::fmod(mm, 60);
    printf("  [ Elapsed Time = %d h, %d m, %.5f s ]\n\n", hh,mm,ss);
}


// ------------------------------------------------------------------------- //
// Print all elements in events queue and clear it

void utils_print::print_queue(std::queue<Event> q)
{
    while( !q.empty() ){
        std::cout << q.front().tostring()<< "\n";
        q.pop();
    }
}

// ------------------------------------------------------------------------- //
// Print on stdout the number of bars parsed from datafeed.

void utils_print::print_progress(int count)
{

    printf("%21s Bars parsed: %d\r", "", count);
    /*
    int progress = (int)((count/(float)tot_count)*100);
    if( progress%10 == 0 && progress<100 ){
        //cout << "    Progress " << progress << "%\r";
        printf("%21s Progress %3d%%\r", "", progress);
    }
    else if( progress == 100 ){
        //cout << "    Progress " << progress << "%. Done.\n";
        printf("%21s Progress %3d%%. Done.\n", "", progress);
    }
    */
}


// ------------------------------------------------------------------------- //
// Control the printing on stdout and on file of the results of backtest
void utils_print::show_backtest_results(
                const Account &account, Performance &performance,
                std::string strategy_name, std::string symbol_name,
                std::string timeframe, Date date_i, Date date_f,
                bool print_trade_list, bool print_performance_report,
                bool show_plot, std::string paramfile,
                std::string trade_list_file,
                std::string performance_file, std::string profits_file ){

    // Print Trade List on stdout and on file
    if( print_trade_list ){
        account.print_transaction_history();
        account.write_transaction_history_to_file( trade_list_file,
                                            paramfile,
                                            strategy_name, symbol_name,
                                            timeframe, date_i, date_f );
        //<<<
        //account.write_transaction_history_pl_to_file( "tmp/labels.csv" );
        //<<<
    }

    //Compute performance metrics and print performance report on stdout
    if( print_performance_report ){
        performance.compute_and_print_performance();
        performance.write_performance_to_file( performance_file, paramfile,
                                               strategy_name, symbol_name,
                                               timeframe, date_i, date_f );
    }

    // Plot equity curve
    if( show_plot ){
        // Write balance to file
        account.write_equity_to_file( profits_file );
        // Execute script for gnuplot and open the PNG file
        std::string command = "./bin/PlotBalance";
        std::system(command.c_str());
    }
}
