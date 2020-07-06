#ifndef EVENTS_H
#define EVENTS_H

#include "instruments.h"

/*!
Define Event class

Member Variables for all event types
- event_type_: "NONE", "BAR", "SIGNAL", "ORDER", "FILL"
- symbol_: object of instrument class
- timestamp_ (DateTime object)

Member Variables for BAR event:
- timeframe_
- open_
- high_
- low_
- close_
- volume_

Member Variables for SIGNAL/ORDER/FILL event:
- action_: 'BUY'/'SELL' or 'SELLSHORT'/'BUYTOCOVER'
- order_type_: 'STOP', 'LIMIT', 'MARKET'
- strategy_name_: name of the strategy which generated the signal
- stoploss_: stop loss in USD per contract
- takeprofit_: take profit in USD per contract

Member variable for SIGNAL/ORDER event
- suggested_price_: price at which ideally place market order

Member variable for SIGNAL event
- position_size_factor_: factor controlling scaling up/dn the position dynamically
                            (used by PositionSizer)
- quantity_to_close_: quantity of contracts to close on exit signals

Member variable for ORDER/FILL event
- quantity_: the quantity of contracts (lots/shares) to transact.
- ticket_: ticket ID to identify uniquely the trade.
            It is assigned by broker when opening position.

Member variables for FILL event
- fill_price_: the price at which the trade was filled
                (typicall different from suggested_price)
- commission_: the brokerage commission for carrying out the trade.

*/



// ------------------------------------------------------------------------- //
// Class for Events

class Event {

    //--- Member variables for all event types
    std::string event_type_ {""};
    Instrument symbol_ {};
    DateTime timestamp_ {};
    //---

    //--- Member variables for BAR event
    std::string timeframe_{""};
    double open_ {0.0};
    double high_ {0.0};
    double low_ {0.0};
    double close_ {0.0};
    int volume_ {0};
    //---

    //--- Member variables for SIGNAL/ORDER/FILL event
    std::string action_{""};
    std::string order_type_{""};
    std::string strategy_name_{""};
    double stoploss_{0.0};
    double takeprofit_{0.0};
    //---

    // Member variable for SIGNAL/ORDER event
    double suggested_price_{0.0};
    // Member variable for SIGNAL event
    double position_size_factor_ {0.0};
    int quantity_to_close_ {0};
    // Member variable for ORDER/FILL event
    int quantity_{0};
    int ticket_{0};
    // Member variables for FILL event
    double fill_price_{0.0};
    double commission_{0.0};



    public:
        // NONE event (0 arguments)
        Event();

        // BAR event constructor (8 arguments)
        Event( Instrument symbol_, DateTime timestamp,
               std::string timeframe, double open, double high, double low,
               double close, int volume );

        // SIGNAL event constructor (10 arguments)
        Event( Instrument symbol, DateTime timestamp,
               std::string action, std::string order_type,
               double suggested_price, double position_size_factor,
               int quantity_to_close,
               std::string strategy_name, double stoploss, double takeprofit );

        // ORDER event constructor (10 arguments)
        Event( Instrument symbol, DateTime timestamp,
               std::string action, std::string order_type,
               double suggested_price, int quantity,
               std::string strategy_name, double stoploss, double takeprofit,
               int ticket );

        // FILL event constructor (11 arguments)
        Event( Instrument symbol, DateTime timestamp,
               std::string action, std::string order_type,
               double fill_price, int quantity,
               std::string strategy_name, double stoploss, double takeprofit,
               int ticket, double commission );


        // string representations
        std::string tostring() const;

        // Overloading comparison function
        bool operator==(const Event& ev) const ;

        // Re-establish order of high/low in bar as max/min prices
        void reorder_OHLC( double new_open, double new_high,
                           double new_low, double new_close );

        // Getters
        std::string event_type() const { return(event_type_); }
        Instrument symbol() const { return(symbol_); }
        DateTime timestamp() const { return(timestamp_); }
        std::string timeframe() const {return(timeframe_); }
        double open() const { return(open_); }
        double high() const { return(high_); }
        double low() const { return(low_); }
        double close() const { return(close_); }
        int volume() const { return(volume_); }
        std::string action() const { return(action_); }
        std::string order_type() const { return(order_type_); }
        std::string strategy_name() const { return(strategy_name_); }
        double stoploss() const { return(stoploss_) ; }
        double takeprofit() const { return(takeprofit_); }
        double suggested_price() const { return(suggested_price_); }
        int quantity_to_close() const { return(quantity_to_close_); }
        double position_size_factor() const { return(position_size_factor_); }
        int quantity() const { return(quantity_); }
        int ticket() const { return(ticket_); }
        double fill_price() const { return(fill_price_); }
        double commission() const { return(commission_); }

        // Setters
        void set_open( double op ) { open_ = op; }
        void set_high( double hi ) { high_ = hi; }
        void set_low( double lo ) { low_ = lo; }
        void set_close( double cl ) { close_ = cl; }
        void set_volume( int vol ) { volume_ = vol; }
        void set_timestamp( DateTime t ) { timestamp_ = t; }
        void set_stoploss( double sl ) { stoploss_ = sl; }
        void set_takeprofit( double tp ) { takeprofit_ = tp; }
};






#endif
