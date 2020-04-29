#ifndef ACCOUNT_H
#define ACCOUNT_H

#include "transaction.h"

#include <datetime.h>   // Time
#include <string>       // std::string
#include <vector>       // std::vector


/*!
Class providing real-time status of account,
and storing Transaction history (vector of Transaction() objects)

Member Variables
- initial_balance_: initial balance
- balance_: consolidated balance
- transactions_: vector of transaction objects
[- df: pandas dataframe containing 'exit_time', 'net_pl' 'Bars_in_trade'
    of all transactions]
[- strategy_name_: strategy name]

*/



// ------------------------------------------------------------------------- //
// Class for Events

class Account {

    double initial_balance_ {0.0};
    double balance_ {0.0};
    std::vector<Transaction> transactions_ {};

    //std::string strategy_name_{""};



    public:
        // constructor
        Account( double initial_balance );

        void add_transaction_to_history(Transaction new_trade);
        void print_transaction_history() const;
        void print_transaction_history_pl() const;
        void write_transaction_history_to_file( std::string fname,
                        std::string paramfile,
                        std::string strategy_name, std::string symbol_name,
                        std::string timeframe, Date date_i, Date date_f ) const;
        void write_transaction_history_pl_to_file( std::string fname ) const;
        void write_equity_to_file( std::string fname ) const;

        double largest_loss() const;

        // Getters
        double initial_balance() const { return(initial_balance_); }
        double balance() const { return(balance_); }
        std::vector<Transaction> transactions() const {return(transactions_);}
        //std::string strategy_name() const { return(strategy_name_); }

        // Setters
        void update_balance( double bal ){ balance_ += bal; }
        void reset( double initial_balance );

};






#endif
