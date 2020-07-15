#include "btfast.h"         // parameters_t, strategy_t

#include "utils_fileio.h"   // write_strategies_to_file
#include "utils_time.h"     // current_datetime_str
#include "utils_optim.h"    //  append_to_optim_results, sort_by_metric

//#include <algorithm>        // std::for_each
#include <chrono>           // std::chrono
#include <cmath>            // std::fmod
//#include <ctime>            // clock_t, time
//#include <execution>        // std::execution::par_unseq
//#include <functional>       // std::bind
#include <iostream>         // std::cout
#include <mutex>            // std::mutex
//#include <new>              // std::nothrow
#include <numeric>          // std::accumulate
#include <omp.h>            // openMP



//-------------------------------------------------------------------------- //
/*! Run Parallelized Exhaustive Optimization over over 'search_space'
    combinations.
    Results stored into 'optim_results' and written to 'optim_file'.

    search_space: combination of parameters to run optimization over (input)
    optim_results: vector where storing optimization results (metrics + params)
    paramfile: XML file with strategy parameter ranges/value.
    optim_file: file where optimization results are written
    fitness_metric: used to sort optimization results in descending order
    datafeed: smart pointer to DataFeed object

*/

void BTfast::run_parallel_optimization(
                                const std::vector<parameters_t> &search_space,
                                        std::vector<strategy_t> &optim_results,
                                        const std::string &optim_file,
                                        const std::string &paramfile,
                                        const std::string &fitness_metric,
                                        std::unique_ptr<DataFeed> &datafeed,
                                        bool sort_results, bool verbose )
{
    // disable printing number of bars parsed
    print_progress_ = false;

    std::mutex mtx;
    int iter {0};

    //--- Start optimization loop
    // *parameter_combination is a single set of parameter values:
    // [ ("p1", 10), ("p2", 2), ... ]
    #pragma omp parallel for
    for( auto parameter_combination = search_space.begin();
         parameter_combination < search_space.end();
         parameter_combination++ ){

    /*std::for_each(  //std::execution::par_unseq,
                    parameter_combinations.begin(),
                    parameter_combinations.end(),
                    std::bind(&BTfast::run_backtest, this )
                    //[this](){ BTfast::run_backtest(this); }
                );*/

        //parameters_t parameter_combination = *it;

        if( verbose ){
            mtx.lock();
            iter++;        // Increment iteration
            std::cout << utils_time::current_datetime_str() + " | "
                      << "(Parallel) Running optimization " << iter << " / "
                      //<< std::distance(search_space.begin(), parameter_combination)+1
                      << search_space.size() << "\n";
            mtx.unlock();
        }

        // Make a copy of DataFeed object and wrap it into a new unique_ptr
        std::unique_ptr<DataFeed> datafeed_copy = datafeed.get()->clone();

        // Initialize Account
        Account account { initial_balance_ };

        // Run backtest (passing strategy parameters of current run)
        run_backtest( account, datafeed_copy, *parameter_combination );

        // Initialize Performance object
        Performance performance { initial_balance_, day_counter_,
                                  std::vector<Transaction> {} };
        // Load transaction history into performance object
        performance.set_transactions( account.transactions() );
        // Compute performance metrics
        performance.compute_metrics();

        // Append performance metrics and parameter combination
        // to optimization results
        //#pragma omp critical
        utils_optim::append_to_optim_results( optim_results, performance,
                                              *parameter_combination );
    }
    //--- End optimization loop

    if( verbose ){
        std::cout << "Optimization Done.\n";
    }

    // Sort in descending order of fitness_metric
    if( sort_results ){
        utils_optim::sort_by_metric( optim_results, fitness_metric );
    }

    // Write optimization results to file 'optim_file'
    int control = utils_fileio::write_strategies_to_file(
                                            optim_file, paramfile,
                                            optim_results, strategy_name_,
                                            symbol_.name(), timeframe_,
                                            first_date_parsed_,
                                            last_date_parsed_, verbose );
    if( control == 1 && verbose ){
        std::cout << "\nOptimization results written on file: "
                  << optim_file <<"\n";
    }
}








//-------------------------------------------------------------------------- //
/*! Run Sequential Exhaustive Optimization over 'search_space' combinations.
    Results stored into 'optim_results' and written to 'optim_file'.

    search_space: combination of parameters to run optimization over (input)
    optim_results: vector where storing optimization results (metrics + params)
    paramfile: XML file with strategy parameter ranges/value.
    optim_file: file where optimization results are written
    fitness_metric: used to sort optimization results in descending order
    datafeed: smart pointer to DataFeed object

*/

void BTfast::run_optimization( const std::vector<parameters_t> &search_space,
                               std::vector<strategy_t> &optim_results,
                               const std::string &optim_file,
                               const std::string &paramfile,
                               const std::string &fitness_metric,
                               std::unique_ptr<DataFeed> &datafeed,
                               bool sort_results, bool verbose )
{
    // disable printing number of bars parsed
    print_progress_ = false;

    std::vector<double> iteration_times(5);  // store first 5 iteration times
    int hh {0};
    int mm {0};
    double ss {0.0};
    double single_iter_time {0.0};
    double avg_time {0.0};
    double remaining_time {0.0};
    std::chrono::high_resolution_clock::time_point t1, t2;
    int iter {0};

    //--- Start optimization loop
    // parameter_combination (single set of values for all parameters):
    // [ ("p1", 10), ("p2", 2), ... ]
    for(parameters_t parameter_combination: search_space){

        if( verbose ){
            iter++;     // Increment iteration
            std::cout << utils_time::current_datetime_str() + " | "
                      << "Running optimization " << iter << " / "
                      << search_space.size() << "\n";
        }

        // Starts computing elapsed time
        t1 = std::chrono::high_resolution_clock::now();

        // Initialize Account
        Account account { initial_balance_ };

        // Run backtest (passing strategy parameters of current run)
        run_backtest( account, datafeed, parameter_combination );

        // Initialize Performance object
        Performance performance { initial_balance_, day_counter_,
                                  std::vector<Transaction> {} };
        // Load transaction history into performance object
        performance.set_transactions( account.transactions() );
        // Compute performance metrics
        performance.compute_metrics();

        // Append performance metrics and parameter combination
        // to optimization results
        utils_optim::append_to_optim_results(optim_results, performance,
                                             parameter_combination);


        //- Compute and print remaining time
        if( verbose ){
            if( iter <= 5 ){
                t2 = std::chrono::high_resolution_clock::now() ;
                single_iter_time = std::chrono::duration_cast<
                                            std::chrono::duration<double>>
                                                                (t2 - t1).count();
                iteration_times.push_back( single_iter_time );
                remaining_time = (search_space.size()-iter) * single_iter_time;
            }
            else if( iter == 6 ){
                // avg iteration time over first 5 iterations
                avg_time = accumulate( iteration_times.begin(),
                                       iteration_times.end(),
                                       0.0 ) / iteration_times.size() ;
                // time remaining for completing optimization
                remaining_time = (search_space.size()-iter) * avg_time;
            }
            else{
                remaining_time = (search_space.size()-iter) * avg_time;
            }
            mm = (int)(remaining_time / 60);
            ss = fmod(remaining_time, 60);
            hh = (int)(mm/60);
            mm = fmod(mm, 60);
            printf("%21s Remaining Time: %d h, %d m, %.5f s\n", "", hh, mm, ss);
        }
        //-

    }
    //--- End optimization loop
    if( verbose ){
        std::cout << "Optimization Done.\n";
    }

    // Sort in descending order of fitness_metric
    if( sort_results ){
        utils_optim::sort_by_metric( optim_results, fitness_metric );
    }

    // Write optimization results to file 'optim_file' and on stdout 
    int control = utils_fileio::write_strategies_to_file(
                                            optim_file, paramfile,
                                            optim_results, strategy_name_,
                                            symbol_.name(), timeframe_,
                                            first_date_parsed_,
                                            last_date_parsed_, verbose );
    if( control == 1 && verbose ){
        std::cout << "\nOptimization results written on file: "
                  << optim_file <<"\n";
    }
}
