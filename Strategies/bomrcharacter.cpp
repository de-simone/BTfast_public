#include "bomrcharacter.h"

#include "filters/exits.h"      // ExitCondition
#include "filters/TA_indicators.h"
#include "utils_math.h" // round_double
#include "utils_trade.h"        // MarketPosition

//#include <iostream>//<<<

// ------------------------------------------------------------------------- //
/*! Constructor
*/
BOMRcharacter::BOMRcharacter( std::string name, Instrument symbol,
                              std::string timeframe, int max_bars_back )
: Strategy{ name, symbol, timeframe, max_bars_back },
  digits_ { symbol_.digits() }
{
    if( timeframe_ != "D" ){
        // Get minutes from "Mx"
        tf_mins_ = std::stoi( timeframe_.substr(1, std::string::npos) );
        Time delta {tf_mins_/60, tf_mins_%60}; // time difference of 1 timeframe bar
        OneBarBeforeClose_ = symbol_.session_close_time() - delta;
    }

    // duration of after session, from close to open (in minutes)
    co_mins_ = (symbol_.session_open_time()
                - symbol_.session_close_time()).tot_minutes();

}




//-------------------------------------------------------------------------- //
/*! Set values of the input parameters (for optimization), by setting the
    correspondence with the names appearing in XML param file.
    Recall: all parameters in XML are INTEGER.
*/
void BOMRcharacter::set_param_values(
                            const std::vector< std::pair<std::string,int> >&
                                parameter_set )
{
    // Find parameter value in parameter_set by its name
    // (as it appears in XML file)
    //fractN_ = find_param_value_by_name( "fractN", parameter_set );
    MyStop_ = find_param_value_by_name( "MyStop", parameter_set );
    BOMR_switch_ = find_param_value_by_name( "BOMR_switch", parameter_set );
    Nbars_ = find_param_value_by_name( "Nbars", parameter_set );
    Xbars_ = find_param_value_by_name( "Xbars", parameter_set );

}




//-------------------------------------------------------------------------- //
/*!
    Variable definitions and preliminary calculations for computing signals.
    Variables passed as const reference (to avoid object copies and
    prevent modifications).
    Return: 1 if all OK; 0 if not enough session bars in history.
*/

int BOMRcharacter::preliminaries( const std::deque<Event>& data1,
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

    //-- OHLC of current session and previous 5 sessions
    if( data1D.size() < OpenD_.size()){    // daily bar collection long enough
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
        NewSession_ = false;
    }
    // Disable trading if already in trade, until end of session
    if( MarketPosition_ != 0 ){
        TradingEnabled_ = false;
    }
    //--

    //-- Update Indicator Values

    //bool make_new_entry {true};         // if computing indicator on intraday (data1)
    bool make_new_entry {NewSession_}; // if computing indicators on "D" (data1D)
    HighestHigh( highesthigh_, data1, make_new_entry, max_bars_back_, Nbars_);
    LowestLow( lowestlow_, data1, make_new_entry, max_bars_back_, Nbars_);

    // Require at least Nbars_ of history
    if( highesthigh_.size() < Nbars_ +1 ){
        return(0);
    }
    //--


    return(1);
}


//-------------------------------------------------------------------------- //
/*! Define Entry rules and fill 'signals' array
*/
void BOMRcharacter::compute_entry( const std::deque<Event>& data1,
                                const std::deque<Event>& data1D,
                                const PositionHandler& position_handler,
                                std::array<Event, 2> &signals )
{

    // -------------------------    ENTRY LEVELS    ------------------------ //
    // Breakout levels
    double BO_level_long  = highesthigh_[0];
    double BO_level_short = lowestlow_[0];
    // Mean-reverting levels
    double MR_level_long  = lowestlow_[0];
    double MR_level_short = highesthigh_[0];

    // --------------------------------------------------------------------- //

    // ------------------------    ENTRY RULES    -------------------------- //
    bool EnterLong  = ( TradingEnabled_ );
    bool EnterShort = ( TradingEnabled_ );
    // --------------------------------------------------------------------- //


    ////////////////////////  DO NOT EDIT THIS BLOCK  /////////////////////////
    //////////////////////////     OPEN TRADES     ////////////////////////////
    if( EnterLong ){

        if( BOMR_switch_ == 1 ){
            signals[0] = Event { symbol_, data1[0].timestamp(),
                                 "BUY", "STOP", BO_level_long,
                                 1.0, 0, name_, (double) MyStop_ , 0.0 };
        }
        else if( BOMR_switch_ == 2 ){
            signals[0] = Event { symbol_, data1[0].timestamp(),
                                 "BUY", "LIMIT", MR_level_long,
                                 1.0, 0, name_, (double) MyStop_ , 0.0 };
        }
    }

    if( EnterShort ){
        if( BOMR_switch_ == 1 ){
            signals[1] = Event { symbol_, data1[0].timestamp(),
                                 "SELLSHORT", "STOP", BO_level_short,
                                 1.0, 0, name_, (double) MyStop_, 0.0 };
        }
        else if( BOMR_switch_ == 2 ){
            signals[1] = Event { symbol_, data1[0].timestamp(),
                                 "SELLSHORT", "LIMIT", MR_level_short,
                                 1.0, 0, name_, (double) MyStop_, 0.0 };
        }
    }
    ///////////////////////////////////////////////////////////////////////////
}



//-------------------------------------------------------------------------- //
/*! Define Exit rules and fill 'signals' array
*/
void BOMRcharacter::compute_exit( const std::deque<Event>& data1,
                                const std::deque<Event>& data1D,
                                const PositionHandler& position_handler,
                                std::array<Event, 2> &signals )
{
    // ------------------------    EXIT RULES    --------------------------- //
    // Exit after 'Xbars_' bars
    bool ExitLong   = ( MarketPosition_> 0
                        && ExitCondition( 4, data1, name_,
                                          position_handler.open_positions(),
                                          CurrentTime_, CurrentDOW_,
                                          OneBarBeforeClose_, Xbars_, Xbars_+1,
                                          tf_mins_, co_mins_, NewSession_ ) );

    bool ExitShort  = ( MarketPosition_< 0
                        && ExitCondition( 4, data1, name_,
                                          position_handler.open_positions(),
                                          CurrentTime_, CurrentDOW_,
                                          OneBarBeforeClose_, Xbars_, Xbars_+1,
                                          tf_mins_, co_mins_, NewSession_ ) );

    // --------------------------------------------------------------------- //





    ////////////////////////  DO NOT EDIT THIS BLOCK  /////////////////////////
    //////////////////////////     CLOSE TRADES     ///////////////////////////
    if( ExitLong ){
        // identify long position to close
        Position long_pos_to_close {};
        for( const Position& pos : position_handler.open_positions() ){
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
        for( const Position& pos : position_handler.open_positions() ){
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

void BOMRcharacter::compute_signals( const PriceCollection& price_collection,
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