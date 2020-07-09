/*****************************************************************************/
/*!
 \mainpage BTfast: Back-Test fast!

 \section intro_sec Introduction

 BTfast is a software written in C++ to do event-driven backtesting,
 optimization and validation of trading systems.

 @version     v1.0
 @author      Andrea De Simone

 Copyright (c) 2019-2020, Andrea De Simone - All rights reserved.<br/>

 \subsection Data Storage
        - CSV files


 * Required files:
        - Configuration file: 'settings.xml'
        - Data file, in 'data_dir' directory
        - Strategy files: 'Strategy/strategyname.cpp',
                          'Strategy/strategyname.h',
                          'Strategy/StrategyName.xml'
        - Contract specifications: 'src/instruments.cpp'


                                          -------------------------------------
                                              Created by Prof. Andrea De Simone
                                        Copyright 2019 (c). All rights reserved
                                                         Started on: 2019-11-22
                                                        Last update: 2020-07-09
                                          -------------------------------------

 * TO DO:

    - dynamic position sizing in mastercode
    - trade management: form of dynamic position sizing increasing/decreasing
      position during the trade (not before entry).
    - check implementation of LIMIT orders with TradeStation
    - deal with more than 1 symbol (e.g. market breadth).
      datafeed sends to queue std::array<Event,10>, where storing simultaneous
      bars of up to 10 different symbols. Then PriceCollection runs over array
      to update all bar lists.
   - genetic programming ( create bool nodes like HighD[1] > LowD[5],
                          out of building blocks )
   - add commission/slippage to execution

   [- does the call to append_to_optim_results (in run_parallel_optimization)
       need #pragma omp critical (as in genetic) ?
   ]

   [- multiple comparison, benjamini-hochberg]
   [- genetic optim: avoid computing the fitness of the same strategy (individual)
     multiple times]
    [- fix IS/OOS dates. define:
        initial_date_IS (=input_start_date), final_date_IS,
        initial_date_OOS1, final_date_OOS1,
        initial_date_OOS2 (=final_date_OOS1+1 bday), final_date_OOS2(=input_end_date)
    ]
    [- Bootstrap to get standard error on performance metrics]
    [- replace OpenMP parallelization with C++17 for_each loop and
        execution policy: std::execution::par_unseq
        (currently: error when #including <execution> )]
    [- error handling: replace exit(1) with try/throw/catch exceptions]
    [- IQfeed/IB API]
    [
    - perf decay filter: avg delay between peaks. Calcola il tempo medio in cui il TS genera
        nuovi massimi della equity line.
        Se il TS e' in ritardo (cioe' dopo questo tempo medio non ha generato un nuovo max),
        staccalo. Riattacca il TS quando ha generato un nuovo massimo.
    - Portfolio analysis: correlation matrix of equity lines of multiple strategies
    - Portfolio general director: monitor all systems, redistribute resources real time,
       decide whether to weight a strategy more or less or even switch it off
       (see http://www.systemsontheroad.com/#!STARTING-A-HEDGE-FUND-4-THE-BIG-PICTURE/c18q6/56411f070cf2e1ca27908c84 )
     ]
 *****************************************************************************/

#include "account.h"
#include "btfast.h"         // type aliases (parameters_t, strategy_t)
#include "datafeed.h"
#include "instruments.h"
#include "run_modes.h"      // mode_notrade, mode_single_bt, mode_optimization,
                            // mode_factory, mode_factory_sequential, mode_overview

#include "utils_fileio.h"   // read_config_file, read_param_file, read_strategies_from_file
//#include "utils_math.h"
//#include "utils_optim.h"    //  append_to_optim_results
//#include "utils_params.h"   // single_parameter_combination, cartesian_product
#include "utils_print.h"    // print_header, print_footer, show_backtest_results
#include "utils_time.h"     // actual_start_date, actual_end_date


#include <iostream>     // std::cout
#include <chrono>       // std::chrono
#include <memory>       // std::unique_ptr
#include <string>       // std::string


///////////////////////////////////////////////////////////////////////////////
/////////////////////////    BEGIN OF MAIN PROGRAM    /////////////////////////
///////////////////////////////////////////////////////////////////////////////

int main () {

    // -----------------------   INITIALIZATIONS   ------------------------- //
    //--- Main program variables
    std::chrono::high_resolution_clock::time_point t1 {
            std::chrono::high_resolution_clock::now() };  ///< Starting clock time
    int run_mode {0};                       ///< Run Mode (backtest or optimization)
    int csv_format {1};                     ///< Data format of CSV file
    int max_bars_back {100};                ///< Max number of bars to keep in history
    int slippage {0};                       ///< max number of slippage ticks
    int population_size {100};              ///< Number of individuals in population (GA)
    int generations {10};                   ///< Max number of generations (GA)
    int num_contracts {1};                  ///< Number of contracts to use in "fixed-size" position size
    int max_variation_pct {30};             ///< Percentage of max variation for stability test
    int num_noise_tests {100};              ///< Number of noise tests
    bool print_progress {true};             ///< Print backtest progress on stdout
    bool print_performance_report {false};  ///< Print perf report on stdout
    bool print_trade_list {false};          ///< Print list of trades on stdout
    bool write_trades_to_file {false};      ///< Write trade history to file and show plot
    bool include_commissions {false};       ///< Include or not commissions
    double initial_balance {100000.0};      ///< Initial account balance
    double risk_fraction {0.1};             ///< Fraction to use in "fixed-fractional", "fixed-notional" position size

    std::string config_file {"settings.xml"};   ///<< XML configuration file
    std::string strategy_name {""};             ///< Name of the strategy
    std::string symbol_name {""};               ///< Name of the symbol
    std::string timeframe {""};                 ///< Main Timeframe of the strategy
    std::string input_start_date {""};          ///< Start date from settings (included)
    std::string input_end_date {""};            ///< End date from settings (included)
    std::string fitness_metric {""};            ///< Metric to sort optimization and for GA
                                                ///< (used in utils.cpp and btfast_genetic.cpp)
    std::string main_dir {""};                  ///< Path to main BTfast dir
    std::string data_dir {""};                  ///< Path to directory containing data
    std::string datafeed_type {""};             ///< Type of datafeed
    std::string data_file {""};                 ///< File containing data
    std::string data_file_oos {""};             ///< FIle containing out-of-sample data
    std::string position_size_type {""};        ///< Type of position size (money management)
    //--- End main program variables

    // unbuffer output
    // (to call command: './run > log.txt &' with live update of log file)
    std::cout << std::unitbuf;

    // Read configuration settings from XML config_file
    utils_fileio::read_config_file(
                    config_file, main_dir, run_mode, strategy_name,
                    symbol_name, timeframe, input_start_date, input_end_date,
                    data_dir, data_file, csv_format, datafeed_type,
                    print_progress, print_performance_report, print_trade_list,
                    write_trades_to_file, fitness_metric,
                    population_size, generations,
                    max_bars_back, initial_balance,
                    position_size_type, num_contracts, risk_fraction,
                    include_commissions, slippage,
                    data_file_oos, max_variation_pct, num_noise_tests );

    //--- Define paths and result files
    //std::string data_dir { main_dir + "/BarData" } ; ///< Path to directory containing data
    std::string result_dir { main_dir + "/Results" }; ///< Path to directory containing results
    std::string profits_file { result_dir + "/profits.csv" };  ///< Path to profits file (needed by gnuplot)
    std::string noise_file { result_dir + "/noise.csv" };  ///< Path to file with noise test results (needed by gnuplot)
    std::string overview_file { result_dir + "/mkt_overview.csv" };  ///< Path to file with overview results (needed by gnuplot)
    std::string trade_list_file { result_dir + "/transactions_"
                                    + strategy_name + "_" + symbol_name
                                    + "_" + timeframe + ".csv" }; ///< Path to transaction list file
    std::string performance_file { result_dir + "/performance_"
                                    + strategy_name + "_" + symbol_name
                                    + "_" + timeframe + ".txt" }; ///< Path to performance report file
    std::string optim_file {result_dir + "/optimization_"
                            + strategy_name + "_" + symbol_name
                            + "_" + timeframe + ".csv" }; ///< Path to optimization results file
    std::string selected_file { result_dir + "/selected_" + strategy_name
                                + "_" + symbol_name
                                + "_" + timeframe + ".csv" };   ///< Path to selected strategies file
    std::string validated_file { result_dir + "/validated_" + strategy_name
                                + "_" + symbol_name
                                + "_" + timeframe + ".csv" };   ///< Path to validated strategies file

    std::string param_file { "Strategies/" + strategy_name + ".xml" };  ///< File with strategy parameters
    //---

    //--- Start/End Dates
    // Transform input date strings into date objects
    Date start_date { utils_time::actual_start_date( input_start_date ) };
    Date end_date { utils_time::actual_end_date( input_end_date ) };
    // Check order of dates
    if( start_date > end_date ){
        std::cout << ">>> ERROR: start date after end date.\n";
        exit(1);
    }
    //---
    // --------------------------------------------------------------------- //

    // Print header
    utils_print::print_header( strategy_name,symbol_name,timeframe,data_file );

    // Read parameter values/range from strategy XML file
    // e.g.: [ ("p1", [10]), ("p2", [2,4,6,8]), ... ]
    param_ranges_t parameter_ranges {
                                 utils_fileio::read_param_file(param_file) };

    // ------------------------   INSTANTIATIONS   ------------------------- //
    // Instantiate Instrument object
    Instrument symbol { symbol_name };

    // Initialize smart pointer to object derived from DataFeed base class
    std::unique_ptr<DataFeed> datafeed { nullptr };
    // Instantiate DataFeed derived object corresponding to 'datafeed_type'
    // and wrap it into the smart pointer 'datafeed'
    select_datafeed( datafeed, datafeed_type, symbol, timeframe,
                     data_dir, data_file, csv_format, start_date, end_date );

    // Instantiate main class object
    BTfast btf { strategy_name, symbol, timeframe,
                 max_bars_back, initial_balance, position_size_type,
                 num_contracts, risk_fraction, print_progress,
                 include_commissions, slippage };
    // --------------------------------------------------------------------- //


    // ----------------------    RUN MODE SWITCH    ------------------------ //
    switch( run_mode ){

        // -------------------------   NO-TRADE   -------------------------- //
        case 0: {
            mode_notrade( btf, datafeed, parameter_ranges );
            break;
        }
        // ----------------------------------------------------------------- //

        // ----------------------   SINGLE BACKTEST   ---------------------- //
        case 1: {
            mode_single_bt( btf, datafeed, parameter_ranges,
                            print_trade_list, print_performance_report,
                            write_trades_to_file, param_file, trade_list_file,
                            performance_file, profits_file );
            break;
        }
        // ----------------------------------------------------------------- //

        // -----------------------   OPTIMIZATION   ------------------------ //
        case 2:
            // Exhaustive Parallel Optimization
            mode_optimization( btf, datafeed, parameter_ranges, "parallel",
                               optim_file, param_file, fitness_metric,
                               population_size, generations );
            break;

        case 22:
            // Genetic Parallel Optimization
            mode_optimization( btf, datafeed, parameter_ranges, "genetic",
                               optim_file, param_file, fitness_metric,
                               population_size, generations );
            break;

        case 222:
            // Exhaustive Serial Optimization
            mode_optimization( btf, datafeed, parameter_ranges, "serial",
                               optim_file, param_file, fitness_metric,
                               population_size, generations );
            break;
        // ----------------------------------------------------------------- //

        // ------------------------   VALIDATION   ------------------------- //
        // Validation of single strategy
        case 3:
            mode_single_validation( btf, datafeed, parameter_ranges, param_file,
                                    selected_file, validated_file,
                                    fitness_metric, data_dir,
                                    data_file_oos, max_variation_pct,
                                    num_noise_tests, noise_file );

            break;
        // ----------------------------------------------------------------- //


        // ---------------------   STRATEGY FACTORY   ---------------------- //
        case 4:
            // Sequential Exhaustive Parallel Optimization
            mode_factory_sequential( btf, datafeed, parameter_ranges,
                                      optim_file, param_file, selected_file,
                                      validated_file, fitness_metric,
                                      population_size, generations,
                                      data_dir, data_file_oos, max_variation_pct,
                                      num_noise_tests, noise_file );
            break;

        case 44:
            // Exhaustive Parallel Optimization
            mode_factory( btf, datafeed, parameter_ranges, "parallel",
                          optim_file, param_file, selected_file,
                          validated_file, fitness_metric,
                          population_size, generations,
                          data_dir, data_file_oos, max_variation_pct,
                          num_noise_tests, noise_file );
            break;

        case 444:
            // Genetic Parallel Optimization
            mode_factory( btf, datafeed, parameter_ranges, "genetic",
                          optim_file, param_file, selected_file,
                          validated_file, fitness_metric,
                          population_size, generations,
                          data_dir, data_file_oos, max_variation_pct,
                          num_noise_tests, noise_file );
            break;

        case 4444:
            // Import generation results from file
            mode_factory( btf, datafeed, parameter_ranges, "import",
                          optim_file, param_file, selected_file,
                          validated_file, fitness_metric,
                          population_size, generations,
                          data_dir, data_file_oos, max_variation_pct,
                          num_noise_tests, noise_file );
            break;
        // ----------------------------------------------------------------- //


        // ----------------------   MARKET OVERVIEW   ---------------------- //
        case 6: {
            mode_overview( btf, datafeed, parameter_ranges, overview_file );
            break;
        }
        // ----------------------------------------------------------------- //

        // --------------------------   NOISE TEST   ----------------------- //
        /*
        case 5:
            mode_noise( btf, datafeed, parameter_ranges,
                        num_noise_tests, write_trades_to_file,
                        noise_file, param_file, fitness_metric );
            break;
        */
        // ----------------------------------------------------------------- //


        default:
            std::cout<< ">>> ERROR: invalid RUN_MODE "<<run_mode<< " (main)\n";
            return(0);
    }
    // --------------------------------------------------------------------- //



    // -------------------------   FINALIZATION   -------------------------- //
    utils_print::print_footer( strategy_name, symbol_name, timeframe,
                               data_file, btf.first_date_parsed(),
                               btf.last_date_parsed(), btf.day_counter(),
                               btf.bar_counter(), t1 );
    // --------------------------------------------------------------------- //

    return(0);
}
///////////////////////////////////////////////////////////////////////////////
//////////////////////////    END OF MAIN PROGRAM    //////////////////////////
///////////////////////////////////////////////////////////////////////////////
