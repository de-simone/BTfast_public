#ifndef POSITION_H
#define POSITION_H

#include "events.h"
//#include <instruments.h>

#include <datetime.h>   // Time
#include <string>       // std::string


/*!
Currently open (running) trades

Member Variables
- strategy_name_: name of the strategy which generated the signal
- symbol_: object of instrument class
- side_: "LONG" or "SHORT"
- quantity_: number of contracts/lots
- entry_time_: entry time
- entry_price_: entry price
- stoploss_: stop loss in USD per contract
- takeprofit_: take profit in USD per contract
- ticket_: ticket assigned by broker (random number in backtest)
- mae_: Max Adverse Excursion, in USD per contract (updated on every bar)
- mfe_: Max Favourable excursion, in USD per contract (updated on every bar)
- bars_in_trade_: number of bars since trade entry
- days_in_trade_: number of days (sessions) since trade entry
- pl_: Profit/Loss of the trade (updated on every bar)
- keep_open_: keep position open, or close it because SL/TP hit

*/



// ------------------------------------------------------------------------- //
// Class for open (running) trades

class Position  {

    std::string strategy_name_{""};
    Instrument symbol_;
    std::string side_ {""};
    int quantity_ {0};
    DateTime entry_time_ {};
    double entry_price_ {0.0};
    double stoploss_ {0.0};
    double takeprofit_ {0.0};
    int ticket_{0};
    double mae_ {0.0};
    double mfe_ {0.0};
    int bars_in_trade_ {1};
    int days_in_trade_ {0};
    double pl_ {0.0};
    bool keep_open_ {true};


    public:
        // Constructors
        // Empty position (0 arguments)
        Position();
        // Valid position (9 arguments)
        Position(std::string strategy_name, Instrument symbol_,
                    std::string side, int quantity,
                    DateTime entry_time, double entry_price,
                    double stoploss, double takeprofit, int ticket );

        // update position at new incoming bar
        void update_position( Event barevent );

        // string representation
        std::string tostring();

        // Getters
        std::string strategy_name() const { return(strategy_name_); }
        Instrument symbol() const { return(symbol_); }
        std::string side() const { return(side_); }
        int quantity() const { return(quantity_); }
        DateTime entry_time() const { return(entry_time_); }
        double entry_price() const { return(entry_price_); }
        double stoploss() const { return(stoploss_); }
        double takeprofit() const { return(takeprofit_); }
        int ticket() const { return(ticket_); }
        double mae() const { return(mae_); }
        double mfe() const { return(mfe_); }
        int bars_in_trade() const { return(bars_in_trade_); }
        int days_in_trade() const { return(days_in_trade_); }
        double pl() const { return(pl_); }
        bool keep_open() const { return(keep_open_); }

        // Setters
        void set_pl( double pl ) { pl_ = pl; }
        //void set_bars_in_trade( int bars ) { bars_in_trade_ = bars; }

        // Overloading comparison function
        bool operator==(const Position& p) const ;
};






#endif
