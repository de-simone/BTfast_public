#include "ng6.h"

#include "filters/patterns.h"           // Pattern
#include "filters/time_filter.h"        // TimeFilter
#include "filters/TA_indicators.h"      // ATR
#include "utils_math.h"                 // modulus, round_double
#include "utils_trade.h"                // MarketPosition

#include <algorithm>    // std::max_element, std::min_element
#include <cmath>        // std::abs,std::pow
#include <iostream>     // std::cout

using std::max_element;
using std::min_element;
using std::abs;
using std::pow;

// ------------------------------------------------------------------------- //
/*! Constructor
*/
NG6::NG6( std::string name, Instrument symbol,
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

    // duration of session (in minutes)
    int session_duration = 60*utils_math::modulus(
                                     symbol_.session_close_time().hour()
                                     - symbol_.session_open_time().hour(), 24);
    // duration of T-segment window (in minutes)
    T_segment_duration_ = session_duration / 3;
}




//-------------------------------------------------------------------------- //
/*! Set values of the input parameters (for optimization), by setting the
    correspondence with the names appearing in XML param file.
    Recall: all parameters in XML are INTEGER.
*/
void NG6::set_param_values(
                        const std::vector< std::pair<std::string,int> >&
                                parameter_set )
{
    // Find parameter value in parameter_set by its name (as appear in XML file)
    MyStop_ = find_param_value_by_name( "MyStop", parameter_set );
    //fractN_ = find_param_value_by_name( "fractN", parameter_set );
    fractN_long_  = find_param_value_by_name( "fractN_long", parameter_set );
    fractN_short_ = find_param_value_by_name( "fractN_short", parameter_set );
    volabin_ = find_param_value_by_name( "VolatilityBin", parameter_set );
    epsilon_ = find_param_value_by_name( "epsilon", parameter_set );
}




//-------------------------------------------------------------------------- //
/*!
    Variable definitions and preliminary calculations for computing signals.
    Variables passed as const reference (to avoid object copies and
    prevent modifications).
    Return: 1 if all OK; 0 if not enough session bars in history.
*/

int NG6::preliminaries( const std::deque<Event>& data1,
                         const std::deque<Event>& data1D,
                         const PositionHandler& position_handler)
{
    // Invalid preliminaries if bar vectors are empty
    if( data1D.empty() || data1.empty() ){
        return(0);
    }

    // Time attributes of current reference bar
    CurrentTime_ = data1[0].timestamp().time();
    CurrentDate_ = data1[0].timestamp().date();
    CurrentDOW_  = data1[0].timestamp().weekday();

    // Market Position
    MarketPosition_ = utils_trade::MarketPosition(position_handler);

    //-- OHLC of current session and previous 5 sessions
    if( data1D.size() < OpenD_.size() ){    // daily bar collection long enough
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
    ATR( atr_, data1D, NewSession_, max_bars_back_, 10 );
    /*
    // Require at least 100 days of ATR history
    if( atr_.size() < 100 ){
        return(0);
    }
    */
    //--



    return(1);
}

//-------------------------------------------------------------------------- //
/*! Define Entry rules and fill 'signals' array
*/
void NG6::compute_entry( const std::deque<Event>& data1,
                                const std::deque<Event>& data1D,
                                const PositionHandler& position_handler,
                                std::array<Event, 2> &signals )
{

    // --------------------    POINT OF INITIATION    ---------------------- //
    double POI_long  {  CloseD_[1] };                   // 3
    double POI_short {  0.5*(HighD_[0] + LowD_[0]) };     // 2

    /*
    if( CurrentDOW_ == 2 || CurrentDOW_ == 3 ){ // tue,wed
        POI_short =  (CloseD_[1] + HighD_[1] + LowD_[1])/3 ; //6
    }
    */
    // --------------------------------------------------------------------- //

    // ------------------------    BREAKOUT LEVELS    ---------------------- //
    //double fract { std::pow(2,fractN_ ) * 0.1 };  // 2^fractN_ / 10
    /*
    double fract_long { std::pow(2, fractN_long_) * 0.1 };      // 2^fractN_ / 10
    double fract_short { std::pow(2, fractN_short_) * 0.1 };    // 2^fractN_ / 10
    fract_long  = fract_long  * ( 1 + epsilon_/20.0 ); // epsilon=1 means 5% variation
    fract_short = fract_short * ( 1 + epsilon_/20.0 ); // epsilon=1 means 5% variation
    */

    double level_long  = utils_math::round_double(
                               POI_long  + 0.8  * (HighD_[1] - LowD_[1]),
                                                      digits_ );
    double level_short = utils_math::round_double(
                               POI_short - 0.1 * (HighD_[1] - LowD_[1]),
                                                      digits_ );
    /*if( CurrentDOW_ == 2 || CurrentDOW_ == 3 ){ // tue,wed
        level_short = utils_math::round_double(
                               POI_short - 0.4 * (
                                *max_element( HighD_.begin()+1, HighD_.end() )
                                - *min_element( LowD_.begin()+1, LowD_.end() )),
                                                      digits_ );
    }
    */
    // --------------------------------------------------------------------- //

    // --------------------------    TIME FILTER    ------------------------ //
    bool FilterT { CurrentTime_ >= Time(8,0)
                    && CurrentTime_ < symbol_.settlement_time()
                    && CurrentDOW_ != 5     // not on fri
                };
    // --------------------------------------------------------------------- //

    // ---------------------------    FILTER 1    -------------------------- //
    bool Filter1_long { true };
    bool Filter1_short { true };

    Filter1_long = LowD_[1] < LowD_[5]; // 23
    Filter1_short = ( CloseD_[1]>CloseD_[2] &&
                    (CloseD_[2]>CloseD_[3] || CloseD_[3]>CloseD_[4]) );  // 8mod
    /*
    if( CurrentDOW_ == 2 || CurrentDOW_ == 3 ){ // tue,wed
        Filter1_short = (OpenD_[0]-LowD_[0])> ((OpenD_[1]-LowD_[1]) * 1); // 6
    }
    */
    /*Filter1_short = Filter1_short &&
                    Pattern( volabin_, OpenD_, HighD_, LowD_, CloseD_, atr_ );
    */
    // --------------------------------------------------------------------- //

    // ----------------------    COMBINE ALL FILTERS    -------------------- //
    bool All_filters_long  { FilterT && Filter1_long };
    bool All_filters_short { FilterT && CurrentDOW_ != 2 && Filter1_short };

    All_filters_long = false;
    //All_filters_short = false;
    // --------------------------------------------------------------------- //

    // ------------------------    ENTRY RULES    -------------------------- //
    bool EnterLong  = ( TradingEnabled_ && All_filters_long );
    bool EnterShort = ( TradingEnabled_ && All_filters_short );
    // --------------------------------------------------------------------- //


    ////////////////////////  DO NOT EDIT THIS BLOCK  /////////////////////////
    //////////////////////////     OPEN TRADES     ////////////////////////////
    if( EnterLong ){
        /*std::cout<<data1[0].timestamp().tostring()<<"    "
                 <<CloseD_[1]<<", "<<HighD_[1]<<", "<<LowD_[1]<<", "
                 <<LowD_[5]<<", "<<level_long<<"\n";
        std::cout<<"LowD = "<< data1D[0].timestamp().tostring()
                 <<"  "<< data1D[0].low()<<"\n";
        */
        signals[0] = Event { symbol_, data1[0].timestamp(),
                             "BUY", "STOP", level_long,
                             //"BUY", "MARKET", level_long,
                             Ncontracts_, name_,
                             (double) MyStop_ * Ncontracts_, 0.0 };
    }

    if( EnterShort ){
        signals[1] = Event { symbol_, data1[0].timestamp(),
                             "SELLSHORT", "STOP", level_short,
                             //"SELLSHORT", "MARKET", level_short,
                             Ncontracts_, name_,
                             (double) MyStop_ * Ncontracts_, 0.0 };
    }
    ///////////////////////////////////////////////////////////////////////////
}


//-------------------------------------------------------------------------- //
/*! Define Exit rules and fill 'signals' array
*/
void NG6::compute_exit( const std::deque<Event>& data1,
                               const std::deque<Event>& data1D,
                               const PositionHandler& position_handler,
                               std::array<Event, 2> &signals )
{
    // ------------------------    EXIT RULES    --------------------------- //
    // Exit one bar before close of session,
    // or at open of next session if session ends earlier than usual

    bool ExitLong   = ( MarketPosition_> 0
                       && ( CurrentTime_ == OneBarBeforeClose_
                           || ((data1[0].timestamp().time()
                               - data1[1].timestamp().time()).tot_minutes() >
                               co_mins_ + tf_mins_ ) ) );

    bool ExitShort  = ( MarketPosition_< 0
                       && ( CurrentTime_ == OneBarBeforeClose_
                           || ((data1[0].timestamp().time()
                               - data1[1].timestamp().time()).tot_minutes() >
                               co_mins_ + tf_mins_ ) ) );
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
                                 long_pos_to_close.quantity(),
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
                                 short_pos_to_close.quantity(),
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

void NG6::compute_signals( const PriceCollection& price_collection,
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
