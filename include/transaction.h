#ifndef TRANSACTION_H
#define TRANSACTION_H

#include "instruments.h"


/*!
Closed trades to be stored in transaction list


Member Variables
- ticket_: ticket assigned by broker (random number in backtest)
- strategy_name_: name of the strategy which generated the signal
- symbol_name_: the instrument symbol name
- side_: 'LONG' or 'SHORT'
- quantity_: number of contracts/lots
- entry_time_: entry time
- entry_price_: entry price
- exit_time_: exit time
- exit_price_: exit price
- mae_: Max Adverse Excursion, in ticks
- mfe_: Max Favourable excursion, in ticks
- bars_in_trade_: number of bars in trade
- net_pl_: Net Profit/Loss ( = gross_pl - commmissions)
- cumul_pl_: cumulative Net P/L up to this transaction included

*/



// ------------------------------------------------------------------------- //
// Class for closed trades

class Transaction {

    int ticket_{0};
    std::string strategy_name_{""};
    Instrument symbol_ ;
    std::string side_ {""};
    int quantity_ {0};
    DateTime entry_time_ {};
    DateTime exit_time_ {};
    double entry_price_ {0.0};
    double exit_price_ {0.0};
    double mae_ {0.0};
    double mfe_ {0.0};
    int bars_in_trade_ {1};
    double net_pl_ {0.0};
    double cumul_pl_ {0.0};


    public:
        // constructor
        Transaction(std::string strategy_name, Instrument symbol_,
                    std::string side, int quantity,
                    DateTime entry_time, double entry_price,
                    DateTime exit_time,  double exit_price,
                    double mae, double mfe, int bars_in_trade,
                    double net_pl, double cumul_pl );


        // string representations
        std::string tostring() const;

        // Getters
        int ticket() const { return(ticket_); }
        std::string strategy_name() const { return(strategy_name_); }
        Instrument symbol() const { return(symbol_); }
        std::string side() const { return(side_); }
        int quantity() const { return(quantity_); }
        DateTime entry_time() const { return(entry_time_); }
        DateTime exit_time() const { return(exit_time_); }
        double entry_price() const { return(entry_price_); }
        double exit_price() const { return(exit_price_); }
        double mae() const { return(mae_); }
        double mfe() const { return(mfe_); }
        int bars_in_trade() const { return(bars_in_trade_); }
        double net_pl() const { return(net_pl_); }
        double cumul_pl() const { return(cumul_pl_); }
        double ticks() const {return(net_pl_/(quantity_*symbol_.tick_value()));}
};






#endif
