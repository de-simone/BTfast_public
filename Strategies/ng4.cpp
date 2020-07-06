#include "ng4.h"

#include "filters/patterns.h"   // Pattern, OppositePattern
#include "filters/TA_indicators.h"
#include "utils_math.h" // modulus, round_double
#include "utils_time.h" // CalcTime
#include "utils_trade.h"      // MarketPosition

#include <algorithm>    // std::max_element, std::min_element
#include <cmath>        // std::abs,std::pow

using std::max_element;
using std::min_element;
using std::abs;
using std::pow;

// ------------------------------------------------------------------------- //
/*! Constructor
*/
NG4::NG4( std::string name, Instrument symbol,
                        std::string timeframe, int max_bars_back )
: Strategy{ name, symbol, timeframe, max_bars_back },
  digits_ { symbol_.digits() }
{

    if( timeframe_ != "D" ){
        int mins = std::stoi( timeframe_.substr(1, std::string::npos) );
        Time delta {mins/60, mins%60}; // time difference of 1 timeframe bar
        OneBarBeforeClose_ = symbol_.session_close_time() - delta;
    }

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
void NG4::set_param_values(
                        const std::vector< std::pair<std::string,int> >&
                                parameter_set )
{
    // Find parameter value in parameter_set by its name (as appear in XML file)
    MyStop_ = find_param_value_by_name( "MyStop", parameter_set );
    fractN_ = find_param_value_by_name( "fractN", parameter_set );
    epsilon_ = find_param_value_by_name( "epsilon", parameter_set );
}




//-------------------------------------------------------------------------- //
/*!
    Variable definitions and preliminary calculations for computing signals.
    Variables passed as const reference (to avoid object copies and
    prevent modifications).
    Return: 1 if all OK; 0 if not enough session bars in history.
*/

int NG4::preliminaries( const std::deque<Event>& data1,
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


    return(1);
}

//-------------------------------------------------------------------------- //
/*! Define Entry rules and fill 'signals' array
*/
void NG4::compute_entry( const std::deque<Event>& data1,
                                const std::deque<Event>& data1D,
                                const PositionHandler& position_handler,
                                std::array<Event, 2> &signals )
{
    // --------------------------    T-SEGMENTS    ------------------------- //
    int Tsegment {0};
    if ( CurrentTime_ > symbol_.session_open_time()
         && CurrentTime_ <= utils_time::CalcTime(symbol_.session_open_time(),
                                                 T_segment_duration_) ) {
            Tsegment = 1;
    }
    else if ( CurrentTime_ > utils_time::CalcTime(symbol_.session_open_time(),
                                                  T_segment_duration_)
             && CurrentTime_ <= utils_time::CalcTime(symbol_.session_open_time(),
                                                     2*T_segment_duration_) ){
            Tsegment = 2;
    }
    else if ( CurrentTime_ > utils_time::CalcTime(symbol_.session_open_time(),
                                                  2*T_segment_duration_)
             && CurrentTime_ <= symbol_.session_close_time() ) {
            Tsegment = 3;
    }
    // --------------------------------------------------------------------- //

    // --------------------    POINT OF INITIATION    ---------------------- //
    double POI_long {  0.5*(HighD_[0] + LowD_[0]) }; // 2
    double POI_short {  0.5*(HighD_[0] + LowD_[0]) };// 2
    // --------------------------------------------------------------------- //

    // ------------------------    BREAKOUT LEVELS    ---------------------- //
    double fract { std::pow(2,fractN_) * 0.1 }; // 2^fractN_ / 10
    fract = fract * ( 1 + epsilon_/20.0 );      // epsilon=1 means 5% variation

    double BO_level_long { utils_math::round_double(
                                    POI_long + fract * (HighD_[1] - LowD_[1]),
                                                    digits_) };
    double BO_level_short { utils_math::round_double(
                                    POI_short - fract * (HighD_[1] - LowD_[1]),
                                                    digits_) };
    // --------------------------------------------------------------------- //

    // --------------------------    TIME FILTER    ------------------------ //
    bool FilterT { true };
    FilterT = true; // 0
    // --------------------------------------------------------------------- //

    // ---------------------------    FILTER 1    -------------------------- //
    bool Filter1_short { true };
    bool Filter1_long { true };

    Filter1_long =(HighD_[0]-OpenD_[0]) > ((HighD_[1]-OpenD_[1]) * 1.5); // 15

    Filter1_short = CloseD_[1] > CloseD_[2]; // 24
    //Filter1_short = data1[0].low() != LowD_[0]; // Not Low of day // 8
    //Filter1_short = (HighD_[0]-OpenD_[0]) > ((HighD_[1]-OpenD_[1]) * 1);
    // Decreasing Volume (9)
    //Filter1_short = data1[0].volume() < data1[1].volume()
    //                        && data1[1].volume() < data1[2].volume();
    // --------------------------------------------------------------------- //

    // ----------------------    COMBINE ALL FILTERS    -------------------- //
    bool All_filters_long  { FilterT && Filter1_long };
    bool All_filters_short { FilterT && Filter1_short };
    // --------------------------------------------------------------------- //

    // ------------------------    ENTRY RULES    -------------------------- //
    bool EnterLong  = ( TradingEnabled_ && All_filters_long );

    bool EnterShort = ( TradingEnabled_ && All_filters_short );
    // --------------------------------------------------------------------- //


    ////////////////////////  DO NOT EDIT THIS BLOCK  /////////////////////////
    //////////////////////////     OPEN TRADES     ////////////////////////////
    if( EnterLong ){
        signals[0] = Event { symbol_, data1[0].timestamp(),
                             "BUY", "STOP", BO_level_long,
                             1.0, 0, name_, (double) MyStop_, 0.0 };
    }

    if( EnterShort ){
        signals[1] = Event { symbol_, data1[0].timestamp(),
                             "SELLSHORT", "STOP", BO_level_short,
                             1.0, 0, name_, (double) MyStop_, 0.0 };
    }
    ///////////////////////////////////////////////////////////////////////////
}


//-------------------------------------------------------------------------- //
/*! Define Exit rules and fill 'signals' array
*/
void NG4::compute_exit( const std::deque<Event>& data1,
                               const std::deque<Event>& data1D,
                               const PositionHandler& position_handler,
                               std::array<Event, 2> &signals )
{
    // ------------------------    EXIT RULES    --------------------------- //
    bool ExitLong   = ( MarketPosition_> 0
                       && ( CurrentTime_ == OneBarBeforeClose_ ) );

    bool ExitShort  = ( MarketPosition_< 0
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

void NG4::compute_signals( const PriceCollection& price_collection,
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
