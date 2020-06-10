#ifndef BTFAST_H
#define BTFAST_H

#include "datafeed.h"               // select_datafeed
#include "execution_handler.h"
#include "signal_handler.h"
#include "../Strategies/strategy.h" // select_strategy


/*!
Main class for BTfast.
Enscapsulates the settings and components for carrying out
an event-driven backtest/optimization of a single strategy

Member Variables

- strategy_name_: name of strategy
- symbol_: object of Instrument class for requested symbol,
            initialized in main program.
- timeframe_: timeframe
- first_date_parsed_: first date parsed from datafeed
- last_date_parsed_: last date parsed from datafeed
- bar_counter_: counter of number of bars parsed/received
- day_counter: counter of number of days actually parsed from file
- max_bars_back_: max number of intraday bars in price collection,
                    (for each symbol/tf) and in indicator deques.
- initial_balance_: initial account balance
- ps_type_: type of position sizing (money management)
- num_contracts_: number of contracts in "fixed-size" position sizing
- risk_fraction_: fraction to use in "fixed-fractional", "fixed-notional"
                  position sizing
- print_progress_: switch to control printing number of parsed bars on stdout
- include_commissions_: switch to control whether to include commission costs
- slippage_: int number of slippage ticks
- random_noise_: switch to control random noise added to data

*/

//--- Type aliases
// Single parameter combination, pair ("name", int value), e.g. ("p1", 2)
using single_param_t = std::pair<std::string,int>;

// Vector of single_param_t, e.g. [ ("p1", 2), ("p2", 7), ... ]
using parameters_t = std::vector<single_param_t>;

// performance metric and parameters of single strategy,
// in pairs ("name", double value), e.g.:
// [ ("metric1", 110.2), ("metric2", 2.1), ("p1", 2.0), ("p2", 21.0), ...]
using strategy_t = std::vector<std::pair<std::string,double>>;

// Full range for all parameters
// [ ("p1", [10]), ("p2", [2,4,6,8]), ... ]
using param_ranges_t = std::vector<std::pair<std::string,std::vector<int>>>;

//---

// ------------------------------------------------------------------------- //
// Main class for all run modes.

class BTfast {

    const std::string &strategy_name_ {""};
    const Instrument &symbol_;
    const std::string &timeframe_ {""};
    Date first_date_parsed_ {};
    Date last_date_parsed_ {};
    int day_counter_ {0};
    int bar_counter_ {0};
    int max_bars_back_ {100};
    double initial_balance_ {100000};
    const std::string &ps_type_ {""};
    int num_contracts_ {1};
    double risk_fraction_ {0.1};
    bool print_progress_{false};
    bool include_commissions_ {false};
    int slippage_ {0};

    bool random_noise_ {false};

    // Member variables used for Market Overview
    // Volume for each hour
    std::array<int, 24> volume_hour_ {};
    // Sum of range Close-Open (in ticks) for each Day of Week
    std::array<double, 7> co_range_dow_ {};
    //std::vector<std::pair<int, double>> DOWranges {};
    // Daily range H-L (in ticks)
    std::vector<double> hl_range_ {};


    public:
        // Consunique_ptr
        BTfast( const std::string &strategy_name,
                const Instrument &symbol,
                const std::string &timeframe,
                int max_bars_back,
                double initial_balance,
                const std::string &ps_type,
                int num_contracts, double risk_fraction,
                bool print_progress,
                bool include_commissions, int slippage );


        // Initialize variables and class instances for a new backtest
        void initialize_backtest(std::deque<Event> &events_queue,
                                 std::unique_ptr<DataFeed> &datafeed_ptr,
                                 std::unique_ptr<ExecutionHandler> &execution_ptr,
                                 std::unique_ptr<Strategy> &strategy_ptr,
                                 PriceCollection &price_coll,
                                 PositionHandler &pos_handler,
                                 SignalHandler &sig_handler,
                                 const parameters_t& strategy_params );

        // Parse data without strategy signals
        void run_notrade( Account &account,
                          std::unique_ptr<DataFeed> &datafeed,
                          const parameters_t& strategy_params );

        // Run single backtest
        void run_backtest( Account &account,
                           std::unique_ptr<DataFeed> &datafeed,
                           const parameters_t& strategy_params );

        // Run exhaustive parallel optimization
        void run_parallel_optimization(
                                const std::vector<parameters_t> &search_space,
                                        std::vector<strategy_t> &optim_results,
                                        const std::string &optim_file,
                                        const std::string &paramfile,
                                        const std::string &fitness_metric,
                                        std::unique_ptr<DataFeed> &datafeed,
                                        bool sort_results, bool verbose );

        // Run exhaustive serial optimization
        void run_optimization( const std::vector<parameters_t> &search_space,
                               std::vector<strategy_t> &optim_results,
                               const std::string &optim_file,
                               const std::string &paramfile,
                               const std::string &fitness_metric,
                               std::unique_ptr<DataFeed> &datafeed,
                               bool sort_results, bool verbose );

        // Run genetic (parallel) optimization
        void run_genetic_optimization( std::vector<parameters_t> &search_space,
                                       std::vector<strategy_t> &optim_results,
                                       const std::string &optim_file,
                                       const std::string &paramfile,
                                       const std::string &fitness_metric,
                                       std::unique_ptr<DataFeed> &datafeed,
                                       int population_size, int generations );

       // Parse data and collect market info, without strategy signals
       void run_overview( Account &account,
                          std::unique_ptr<DataFeed> &datafeed,
                          const parameters_t& strategy_params );


        // Getters
        const std::string& strategy_name() const { return(strategy_name_); }
        const Instrument& symbol() const { return(symbol_); }
        const std::string& timeframe() const { return(timeframe_); }
        int bar_counter() const { return(bar_counter_); }
        int day_counter() const { return(day_counter_); }
        const Date& first_date_parsed() const { return(first_date_parsed_); }
        const Date& last_date_parsed() const { return(last_date_parsed_); }
        double initial_balance() const { return(initial_balance_); }
        const std::array<int, 24>& volume_hour() const { return(volume_hour_); }
        const std::array<double, 7>& co_range_dow() const { return(co_range_dow_); }
        const std::vector<double>& hl_range() const { return(hl_range_); }

        // Setters
        void set_random_noise( bool value ) { random_noise_ = value; }
        void set_first_date_parsed( Date d ) { first_date_parsed_ = d; }
        void set_last_date_parsed( Date d ) { last_date_parsed_ = d; }
        void set_day_counter( int c ) { day_counter_ = c; }

};






#endif
