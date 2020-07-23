#ifndef PERFORMANCE_H
#define PERFORMANCE_H

#include "transaction.h"

#include <unordered_map> // std::unordered_map
#include <vector>       // std::vector


/*!
Class providing real-time status of account,
and storing Transaction history (vector of Transaction() objects)


Member Variables
- transactions_ : vector of Transaction objects
- profits_: vector of profit/loss
- metrics_all_: map of metrics (name,value) for all trades
- metrics_long_: map of metrics (name,value) for long trades
- metrics_short_: map of metrics (name,value) for short trades
- initial_balance_ : initial balance
- ndays_: number of days parsed from file


*/



// ------------------------------------------------------------------------- //
// Class for Events

class Performance {


    double initial_balance_ {100000.0};     ///< initial balance
    int ndays_ {1};                         ///< number of days parsed from file
    std::vector<Transaction> transactions_ {};///< vector of Transaction objects
    std::vector<double> profits_ {};        ///< vector of profit/loss
    std::unordered_map<std::string,double> metrics_all_{}; ///< map of metrics (name,value) for all trades
    std::unordered_map<std::string,double> metrics_long_{}; ///< map of metrics (name,value) for long trades
    std::unordered_map<std::string,double> metrics_short_{}; ///< map of metrics (name,value) for short trades




    public:
        // constructor
        Performance( double initial_balance, int ndays,
                     std::vector<Transaction> transactions);

        void compute_metrics();
        void compute_metrics(
                        const std::vector<Transaction> &transactions,
                        std::unordered_map<std::string,double> &metrics );

        void drawdown( const std::vector<double> &profits,
                       const std::vector<Date>& dates_vec,
                       std::unordered_map<std::string,double> &metrics );
        void avgticks( const std::vector<double> &profits,
                       const std::vector<int> &lots, double tick_value,
                       std::unordered_map<std::string,double> &metrics );
        void max_consec_win_loss( const std::vector<double> &profits,
                        std::unordered_map<std::string,double> &metrics );
        void zscore( const std::vector<double> &profits,
                     std::unordered_map<std::string,double> &metrics );
        void cagr( const std::vector<double> &profits,
                        std::unordered_map<std::string,double> &metrics,
                        int nyears );
        void rsquared( const std::vector<double> &profits,
                        std::unordered_map<std::string,double> &metrics );

        void print_performance_report();
        void write_performance_to_file( std::string fname,
                            std::string paramfile,
                            std::string strategy_name, std::string symbol_name,
                            std::string timeframe, Date date_i, Date date_f );
        void compute_and_print_performance();

        // Getters
        std::vector<Transaction> transactions() const { return(transactions_); }
        double initial_balance() const { return(initial_balance_); }
        int ntrades() const { return( (int) metrics_all_.at("ntrades") ); }
        double avgticks() const { return( metrics_all_.at("avg_ticks")); }
        double winperc() const { return( metrics_all_.at("win_perc") ); }
        double profitfactor() const { return( metrics_all_.at("profit_factor") ); }
        double npmdd() const { return( metrics_all_.at("netpl_maxdd") ); }
        double expectancy() const { return( metrics_all_.at("expectancy") ); }
        double zscore() const { return( metrics_all_.at("zscore") ); }
        double netpl() const { return( metrics_all_.at("net_pl") ); }
        double avgtrade() const { return( metrics_all_.at("avg_trade")); }
        double stdticks() const { return( metrics_all_.at("std_ticks") ); }


        // Setters
        //void set_initial_balance( double bal ){ initial_balance_ = bal; }
        void set_transactions( std::vector<Transaction> transactions ){
            transactions_ = transactions;
        }


};






#endif
