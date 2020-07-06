#include "events.h"

#include <algorithm>    // std::min
#include <cstdio>       // sprintf


// ------------------------------------------------------------------------- //
/*! Constructors
*/
// NONE event (0 arguments)
Event::Event( ) : event_type_{"NONE"}{}

// BAR event constructor (8 arguments)
Event::Event( Instrument symbol, DateTime timestamp,
              std::string timeframe, double open, double high, double low,
              double close, int volume)

: event_type_{"BAR"},
  symbol_{symbol}, timestamp_{timestamp},
  timeframe_{timeframe}, open_{open}, high_{high}, low_{low},
  close_{close}, volume_{volume}
{}


// SIGNAL event constructor (10 arguments)
Event::Event( Instrument symbol, DateTime timestamp,
              std::string action, std::string order_type,
              double suggested_price, double position_size_factor,
              int quantity_to_close,
              std::string strategy_name, double stoploss, double takeprofit)

: event_type_{"SIGNAL"},
  symbol_{symbol}, timestamp_{timestamp},
  action_{action}, order_type_{order_type},
  strategy_name_{strategy_name},
  stoploss_{stoploss},takeprofit_{takeprofit},
  suggested_price_{suggested_price}, position_size_factor_{position_size_factor},
  quantity_to_close_{quantity_to_close}
{}


// ORDER event constructor (10 arguments)
Event::Event( Instrument symbol, DateTime timestamp,
              std::string action, std::string order_type,
              double suggested_price, int quantity,
              std::string strategy_name, double stoploss, double takeprofit,
              int ticket )

: event_type_{"ORDER"},
  symbol_{symbol}, timestamp_{timestamp},
  action_{action}, order_type_{order_type},
  strategy_name_{strategy_name},
  stoploss_{stoploss},takeprofit_{takeprofit},
  suggested_price_{suggested_price}, quantity_{quantity},
  ticket_{ticket}
{}


// FILL event constructor (11 arguments)
Event::Event( Instrument symbol, DateTime timestamp,
              std::string action, std::string order_type,
              double fill_price, int quantity,
              std::string strategy_name, double stoploss, double takeprofit,
              int ticket, double commission)

: event_type_{"FILL"},
  symbol_{symbol}, timestamp_{timestamp},
  action_{action}, order_type_{order_type},
  strategy_name_{strategy_name},
  stoploss_{stoploss},takeprofit_{takeprofit},
  quantity_{quantity},
  ticket_{ticket},
  fill_price_{fill_price},
  commission_{commission}
{}


//-------------------------------------------------------------------------- //
/*! String representations
*/
std::string Event::tostring() const
{

    char buffer[100];
    int digits = symbol_.digits();
    std::string float_format = "%." + std::to_string(digits) + "f, ";

    if( event_type_ == "NONE" ){
        sprintf(buffer, "> %s Event ", event_type_.c_str());
    }
    else if( event_type_ == "BAR" ){
        std::string bar_format = "> %s, " + float_format + float_format
                                + float_format + float_format +"%d";
        sprintf(buffer, bar_format.c_str(), timestamp_.tostring().c_str(),
                open_, high_, low_, close_, volume_ );

    }
    else if( event_type_ == "SIGNAL" ){
        std::string signal_format = "SIGNAL: %s, %s, %s " + float_format;
        sprintf(buffer, signal_format.c_str(), timestamp_.tostring().c_str(),
                action_.c_str(), order_type_.c_str(), suggested_price_);
    }
    else if( event_type_ == "ORDER" ){
        std::string order_format = "ORDER : %s, %s, %d, "+float_format +"%s";
        sprintf(buffer, order_format.c_str(),
                timestamp_.tostring().c_str(), action_.c_str(),
                quantity_, suggested_price_, order_type_.c_str());
    }
    else if( event_type_ == "FILL" ){
        std::string fill_format = "FILL  : %s, %s, %d "+float_format + "%s";
        sprintf(buffer, fill_format.c_str(),
                //ticket_,
                timestamp_.tostring().c_str(), action_.c_str(),
                quantity_, fill_price_, order_type_.c_str() );
    }

    return(buffer);
}

//-------------------------------------------------------------------------- //
/*! Overloading the == operator
    ( used for signal events in signal_handler )
*/
bool Event::operator==(const Event& ev) const
{

    if( (event_type_ == ev.event_type())
        && (timestamp_ == ev.timestamp())
        && (strategy_name_ == ev.strategy_name())
        && (symbol_.name() == ev.symbol().name())
        && (order_type_ == ev.order_type())
        && (action_ == ev.action())
        && (position_size_factor_ == ev.position_size_factor())
        && (suggested_price_ == ev.suggested_price())
    ){
        return(true);
    }
    else{
        return(false);
    }
}

//-------------------------------------------------------------------------- //
/*! Re-establish order of high/low in bar as max/min prices
    and modify the values of this bar accordingly
*/
void Event::reorder_OHLC( double new_open, double new_high,
                          double new_low, double new_close )
{
    // New Open
    if( new_open > high_ ){
        open_  = new_open;
        high_  = new_open;
        low_   = new_low;
        close_ = new_close;
    }
    else if( new_open < low_ ){
        open_  = new_open;
        high_  = new_high;
        low_   = new_open;
        close_ = new_close;
    }
    else{
        open_  = new_open;
        high_  = new_high;
        low_   = new_low;
        close_ = new_close;
    }

    // New Close
    if( new_close > high_ ){
        open_  = new_open;
        high_  = new_close;
        low_   = new_low;
        close_ = new_close;
    }
    else if( new_close < low_ ){
        open_  = new_open;
        high_  = new_high;
        low_   = new_close;
        close_ = new_close;
    }
    else{
        open_  = new_open;
        high_  = new_high;
        low_   = new_low;
        close_ = new_close;
    }

    // New High
    if( new_high < low_ ){
        open_  = new_open;
        high_  = std::max(open_,close_);;
        low_   = new_high;
        close_ = new_close;
    }
    else if( new_high > low_ && new_high < std::max(open_,close_) ){
        open_  = new_open;
        high_  = std::max(open_,close_);
        low_   = new_low;
        close_ = new_close;
    }
    else{
        open_  = new_open;
        high_  = new_high;
        low_   = new_low;
        close_ = new_close;
    }

    // New Low
    if( new_low > high_ ){
        open_  = new_open;
        high_  = new_low;
        low_   = std::min(open_,close_);;
        close_ = new_close;
    }
    else if( new_low < high_ && new_low > std::min(open_,close_) ){
        open_  = new_open;
        high_  = new_high;
        low_   = std::min(open_,close_);;
        close_ = new_close;
    }
    else{
        open_  = new_open;
        high_  = new_high;
        low_   = new_low;
        close_ = new_close;
    }
}
