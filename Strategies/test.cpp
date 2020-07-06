#include "test.h"

#include "filters/TA_indicators.h"   // ROC
#include "utils_math.h" // modulus, round_double
#include "utils_trade.h"      // MarketPosition

// ------------------------------------------------------------------------- //
/*! Constructor
*/
Test::Test( std::string name, Instrument symbol,
            std::string timeframe, int max_bars_back )
: Strategy{ name, symbol, timeframe, max_bars_back },
  digits_ { symbol_.digits() }
{
    if( timeframe_ != "D" ){
        int mins = std::stoi( timeframe_.substr(1, std::string::npos) );
        Time delta {mins/60, mins%60}; // time difference of 1 timeframe bar
        OneBarBeforeClose_ = symbol_.session_close_time() - delta;
    }

}




//-------------------------------------------------------------------------- //
/*! Set values of the input parameters (for optimization), by setting the
    correspondence with the names appearing in XML param file.
    Recall: all parameters in XML are INTEGER.
*/
void Test::set_param_values(
                        const std::vector< std::pair<std::string,int> >&
                                parameter_set )
{
    // Find parameter value in parameter_set by its name
    // (as it appears in XML file)
    fractN_ = find_param_value_by_name( "fractN", parameter_set );

}




//-------------------------------------------------------------------------- //
/*!
    Variable definitions and preliminary calculations for computing signals.
    Variables passed as const reference (to avoid object copies and
    prevent modifications).
    Return: 1 if all OK; 0 if not enough session bars in history.
*/

int Test::preliminaries( const std::deque<Event>& data1,
                         const std::deque<Event>& data1D,
                         const PositionHandler& position_handler)
{
    // Check if bar collections are not empty
    if( data1.empty() || data1D.empty() ){
        return(0);
    }

    // Time attributes of current reference bar
    CurrentTime_ = data1[0].timestamp().time();
    CurrentDate_ = data1[0].timestamp().date();
    CurrentDOW_  = data1[0].timestamp().weekday();

    // Market Position
    MarketPosition_ = utils_trade::MarketPosition(position_handler);

    // Entries in current session (slow, deprecated)
    //EntriesToday_ = utils_trade::EntriesInSession( data1[0].timestamp(),
    //                                                symbol_, position_handler);

    //-- OHLC of current session and previous 5 sessions
    if( data1D.size() < OpenD_.size() ){   // daily bar collection long enough
        return(0);
    }
    else{
        for( int j = 0; j < OpenD_.size(); j++ ){
            OpenD_[j]  = data1D[j].open();
            HighD_[j]  = data1D[j].high();
            LowD_[j]   = data1D[j].low();
            CloseD_[j] = data1D[j].close();
        }
    }
    //--

    //-- Enable/disable trading
    // Enable trading at the start of new session
    if( SessionOpenPrice_ != OpenD_[0] ){ // Identify first bar of the day
        TradingEnabled_ = true;
        NewSession_ = true;
        SessionOpenPrice_ = OpenD_[0];
    }
    else{
        NewSession_ = false;   //<<<
    }
    // Disable trading if already in trade, until end of session
    if( MarketPosition_ != 0 ){
        TradingEnabled_ = false;
    }
    //--

    //-- Update Indicator Values
    /*
    bool make_new_entry {true};         // if computing indicator on intraday (data1)
    //bool make_new_entry {NewSession_}; // if computing indicators on "D" (data1D)
    ROC( ROC_, data1, make_new_entry, max_bars_back_, 2, "CLOSE" );
    ROC( ROC_, data1, true, max_bars_back_, 2, "CLOSE" );
    ROC( ROC_, data1D, NewSession_, max_bars_back_, 2, "CLOSE" );
    */
    //--

    return(1);
}


//-------------------------------------------------------------------------- //
/*! Define Entry rules and fill 'signals' array
*/
void Test::compute_entry( const std::deque<Event>& data1,
                          const std::deque<Event>& data1D,
                          const PositionHandler& position_handler,
                          std::array<Event, 2> &signals )
{
    // ---------------------------    FILTER 1    -------------------------- //
    bool Filter1_long  = (  data1[0].close() > data1[1].close()
                         && data1[1].close() > data1[1].open()
                         && data1[2].close() > data1[2].open() );

    bool Filter1_short = (  data1[0].close() < data1[1].close()
                         && data1[1].close() < data1[1].open()
                         && data1[2].close() < data1[2].open() );
    //Filter1_short = Filter1_long;
    // Do not compute Entries if no filter is triggered
    if( !Filter1_long && !Filter1_short ){
        return;
    }
    // --------------------------------------------------------------------- //

    // --------------------------    TIME FILTER    ------------------------ //
    bool MyTimeWindow = (  CurrentDate_ >= Date(2014,4,1)
                        && CurrentDate_ <= Date(2014,4,28)
                        && CurrentTime_ >= Time(6,0)
                        && CurrentTime_ <= Time(11,0)
                        );
    //MyTimeWindow = true;
    // --------------------------------------------------------------------- //

    // ------------------------    BREAKOUT LEVELS    ---------------------- //
    double fract { fractN_ * 0.1 };
    double BO_level_long  = utils_math::round_double(
                             OpenD_[0] + fract * (HighD_[1] - LowD_[1]),
                                                    digits_);
    double BO_level_short = utils_math::round_double(
                             OpenD_[0] - fract * (HighD_[1] - LowD_[1]),
                                                    digits_);
    // --------------------------------------------------------------------- //

    // ------------------------    ENTRY RULES    -------------------------- //
    bool EnterLong = ( TradingEnabled_ // MarketPosition_ == 0 && EntriesToday_ < 1
                       && MyTimeWindow && Filter1_long );
    //EnterLong = false;

    bool EnterShort = ( TradingEnabled_ //MarketPosition_ == 0 && EntriesToday_ < 1
                        && MyTimeWindow && Filter1_short );
    //EnterShort = false;
    // --------------------------------------------------------------------- //


    ////////////////////////  DO NOT EDIT THIS BLOCK  /////////////////////////
    //////////////////////////     OPEN TRADES     ////////////////////////////
    if( EnterLong ){
        signals[0] = Event { symbol_, data1[0].timestamp(),
                             "BUY", "STOP", BO_level_long,
                             1.0, Ncontracts_, name_,
                             (double) MyStop_ * Ncontracts_, 0.0 };
    }

    if( EnterShort ){
        signals[1] = Event { symbol_, data1[0].timestamp(),
                             "SELLSHORT", "STOP", BO_level_short,
                             1.0, Ncontracts_, name_,
                             (double) MyStop_ * Ncontracts_, 0.0 };
    }
    ///////////////////////////////////////////////////////////////////////////
}



//-------------------------------------------------------------------------- //
/*! Define Exit rules and fill 'signals' array
*/
void Test::compute_exit( const std::deque<Event>& data1,
                         const std::deque<Event>& data1D,
                         const PositionHandler& position_handler,
                         std::array<Event, 2> &signals )
{
    // ------------------------    EXIT RULES    --------------------------- //
    bool ExitLong  = ( MarketPosition_ > 0
                      && ( CurrentTime_ == OneBarBeforeClose_ ) );
    /*
    ExitLong = false;
    bool ExitLong  = ( MarketPosition_ > 0
                      && (position_handler.open_positions().back().side()
                            == "LONG" )
                      && (position_handler.open_positions().back().bars_in_trade()
                           == 3 ) );
    */

    bool ExitShort  = ( MarketPosition_ < 0
                        && ( CurrentTime_ == OneBarBeforeClose_ ) );
    // --------------------------------------------------------------------- //

    ////////////////////////  DO NOT EDIT THIS BLOCK  /////////////////////////
    //////////////////////////     CLOSE TRADES     ///////////////////////////
    if( ExitLong ){
        // identify long position to close
        Position long_pos_to_close {};
        for( Position pos : position_handler.open_positions() ){
            if( pos.side() == "LONG" ){
                long_pos_to_close = pos;
                break;
            }
        }
        // long position to close has been found
        if( long_pos_to_close.quantity() > 0 ){
            signals[0] = Event { symbol_, data1[0].timestamp(),
                                 "SELL", "MARKET", data1[0].close(),
                                 1.0, long_pos_to_close.quantity(),
                                 name_, 0.0, 0.0 };
        }
    }

    if( ExitShort ){
        // identify short position to close
        Position short_pos_to_close {};
        for( Position pos : position_handler.open_positions() ){
            if( pos.side() == "SHORT" ){
                short_pos_to_close = pos;
                break;
            }
        }
        // short position to close has been found
        if( short_pos_to_close.quantity() > 0 ){
            signals[1] = Event { symbol_, data1[0].timestamp(),
                                 "BUYTOCOVER", "MARKET", data1[0].close(),
                                 1.0, short_pos_to_close.quantity(),
                                 name_, 0.0, 0.0 };
        }
    }
    ///////////////////////////////////////////////////////////////////////////
}

//-------------------------------------------------------------------------- //
/*! Handle computation of Entry/Exit signals and fill 'signals' array
    with Signal event (or NONE).

    First entry of 'signals' array:  entry/exit signals for LONG trades
    Second entry of 'signals' array: entry/exit signals for SHORT trades
*/

void Test::compute_signals( const PriceCollection& price_collection,
                            const PositionHandler& position_handler,
                            std::array<Event, 2> &signals )
{
    signals.fill(Event {}); // initialized to NONE events

    //--- Extract bars from price collection
    // dataD: session (daily) bars
    const std::deque<Event>& data1D =
                price_collection.bar_collection().at(symbol_.name()).at("D");

    // data1: intraday bars (= data1D for timeframe="D")
    const std::deque<Event>& data1 =
                ( timeframe_=="D" ) ? price_collection.bar_collection().at(
                                                        symbol_.name()).at("D")
                                    : price_collection.bar_collection().at(
                                            symbol_.name()).at(timeframe_);
    //---


    if( preliminaries( data1, data1D, position_handler ) == 0 ){
        return;
    }

    if( MarketPosition_ == 0 ){
        compute_entry( data1, data1D, position_handler, signals );
    }
    else{
        compute_exit( data1, data1D, position_handler, signals );
    }
}
