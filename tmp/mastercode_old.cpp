#include "mastercode.h"

#include "filters/patterns.h"        // Pattern, OppositePattern
#include "filters/time_filter.h"     // TimeFilter
#include "filters/TA_indicators.h"   // ATR
#include "utils_math.h"                 // modulus, round_double
#include "utils_trade.h"                // MarketPosition

#include <algorithm>                    // std::max_element, std::min_element
#include <cmath>                        // std::abs,std::pow
#include <iostream>                     // std::cout

using std::max_element;
using std::min_element;
using std::abs;
using std::pow;

// ------------------------------------------------------------------------- //
/*! Constructor
*/
MasterCode::MasterCode( std::string name, Instrument symbol,
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
/*! Set values of the input parameters, by setting the
    correspondence with the names appearing in XML param file.
    Recall: all parameters in XML are INTEGER.
*/
void MasterCode::set_param_values(
                        const std::vector< std::pair<std::string,int> >&
                                parameter_set )
{
    // Find parameter value in parameter_set by its name (as appear in XML file)
    Side_switch_ = find_param_value_by_name( "Side_switch", parameter_set );
    MyStop_ = find_param_value_by_name( "MyStop", parameter_set );
    POI_switch_ = find_param_value_by_name( "POI_switch", parameter_set );
    Distance_switch_ = find_param_value_by_name( "Distance_switch",
                                                parameter_set );
    FilterT_switch_ = find_param_value_by_name( "FilterT_switch",
                                                parameter_set );
    Filter1L_switch_ = find_param_value_by_name( "Filter1L_switch",
                                                 parameter_set );
    Filter1S_switch_ = find_param_value_by_name( "Filter1S_switch",
                                                 parameter_set );
    //fractN_ = find_param_value_by_name( "fractN", parameter_set );
    fractN_long_  = find_param_value_by_name( "fractN_long", parameter_set );
    fractN_short_ = find_param_value_by_name( "fractN_short", parameter_set );

    epsilon_ = find_param_value_by_name( "epsilon", parameter_set );
}




//-------------------------------------------------------------------------- //
/*!
    Variable definitions and preliminary calculations for computing signals.
    Variables passed as const reference (to avoid object copies and
    prevent modifications).
    Return: 1 if all OK; 0 if not enough session bars in history.
*/

int MasterCode::preliminaries( const std::deque<Event>& data1,
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
    /*
    bool make_new_entry {true};         // if computing indicator on intraday (data1)
    //bool make_new_entry {NewSession_}; // if computing indicators on "D" (data1D)
    ROC( ROC_, data1, true, max_bars_back_, 2, "CLOSE" );
    ROC( ROC_, data1D, NewSession_, max_bars_back_, 2, "CLOSE" );
    */
    ATR( ATR_, data1, true, max_bars_back_, 10 );
    //--

    return(1);
}

//-------------------------------------------------------------------------- //
/*! Define Entry rules and fill 'signals' array
*/
void MasterCode::compute_entry( const std::deque<Event>& data1,
                                const std::deque<Event>& data1D,
                                const PositionHandler& position_handler,
                                std::array<Event, 2> &signals )
{

    // --------------------    POINT OF INITIATION    ---------------------- //
    double POI_long {0.0};
    double POI_short {0.0};
    switch( POI_switch_ ){
        case 1:
            POI_long  = OpenD_[0];
            POI_short = POI_long;
            break;
        case 2:
            POI_long  = 0.5*(HighD_[0] + LowD_[0]);
            POI_short = POI_long;
            break;
        case 3:
            POI_long  = CloseD_[1];
            POI_short = POI_long;
            break;
        case 4:
            POI_long  = HighD_[1];
            POI_short = LowD_[1];
            break;
        case 5:        // 50% retracement
            POI_long  = 0.5*(HighD_[1] + LowD_[1]);
            POI_short = POI_long;
            break;
        case 6:        // average price of yesterday
            POI_long  = (CloseD_[1] + HighD_[1] + LowD_[1])/3;
            POI_short = POI_long;
            break;
        case 7:{        // max/min of last 5 closes
            POI_long  = *max_element( CloseD_.begin()+1, CloseD_.end() );
            POI_short = *min_element( CloseD_.begin()+1, CloseD_.end() );
            break;
        }
        case 8:{      // highest/lowest of last 5 sessions
            POI_long  = *max_element( HighD_.begin()+1, HighD_.end() );
            POI_short = *min_element( LowD_.begin()+1, LowD_.end() );
            break;
        }
    }
    // --------------------------------------------------------------------- //

    // ------------------------    BREAKOUT LEVELS    ---------------------- //
    //double fract { std::pow(2,fractN_ ) * 0.1 };    // 2^fractN_ / 10
    double fract_long { std::pow(2,fractN_long_) * 0.1 }; // 2^fractN_ / 10
    double fract_short { std::pow(2,fractN_short_) * 0.1 }; // 2^fractN_ / 10
    fract_long = fract_long * ( 1 + epsilon_/20.0 ); // epsilon=1 means 5% variation
    fract_short = fract_short * ( 1 + epsilon_/20.0 ); // epsilon=1 means 5% variation

    double BO_level_long {0.0};
    double BO_level_short {0.0};

    switch( Distance_switch_ ){
        case 1:                // POI +/- f * (High1-Low1)
            BO_level_long  = utils_math::round_double(
                                POI_long + fract_long * (HighD_[1] - LowD_[1]),
                                                       digits_ );
            BO_level_short = utils_math::round_double(
                                POI_short - fract_short * (HighD_[1] - LowD_[1]),
                                                       digits_ );
            break;
        case 2:                // POI +/- f * ATR(10)
            BO_level_long  = utils_math::round_double(
                                POI_long + fract_long * ATR_.front(),
                                                       digits_ );
            BO_level_short = utils_math::round_double(
                                POI_short - fract_short * ATR_.front(),
                                                       digits_ );
            break;
        /*
        case 3:                // POI +/- f * |Close1-Open1|
            BO_level_long  = utils_math::round_double(
                                POI_long + fract * abs(CloseD_[1] - OpenD_[1]),
                                                    digits_);
            BO_level_short = utils_math::round_double(
                                POI_short - fract * abs(CloseD_[1] - OpenD_[1]),
                                                    digits_);
            break;
        */
    }
    //std::cout<<"HighD0/LowD0: "<< HighD_[0]<<", "<< LowD_[0]<<"\n";
    //std::cout<<"HighD1/LowD1: "<< HighD_[1]<<", "<< LowD_[1]<<"\n";
    //std::cout<<"BO_level_long = "<<BO_level_long<<"\n";
    // --------------------------------------------------------------------- //

    // --------------------------    TIME FILTER    ------------------------ //
    bool FilterT { TimeFilter( FilterT_switch_,
                               CurrentDate_, CurrentTime_, CurrentDOW_,
                               symbol_, T_segment_duration_ ) };
    // --------------------------------------------------------------------- //

    // ---------------------------    FILTER 1    -------------------------- //
    bool Filter1_long {false};
    bool Filter1_short {false};

    switch( Filter1L_switch_ ){
        case 0:
            Filter1_long  = true;
            break;
        case 1:
            Filter1_long  = data1[0].close() > CloseD_[1];
            break;
        case 2:
            Filter1_long  = data1[0].close() < CloseD_[1];
            break;
        case 3:
            Filter1_long  = data1[0].close() > OpenD_[1];
            break;
        case 4:
            Filter1_long  = data1[0].close() < OpenD_[1];
            break;
        case 5:
            Filter1_long  = data1[0].close() > OpenD_[0];
            break;
        case 6:
            Filter1_long  = data1[0].close() < OpenD_[0];
            break;
        case 7:             // High of day
            Filter1_long  = data1[0].high() == HighD_[0];
            break;
        case 8:             // Not High of day
            Filter1_long  = data1[0].high() != HighD_[0];
            break;
        case 9:             // Increasing volumes
            Filter1_long  = (  data1[0].volume() > data1[1].volume()
                            && data1[1].volume() > data1[2].volume() );
            break;
        case 10:            // Decreasing volumes
            Filter1_long  = (  data1[0].volume() < data1[1].volume()
                            && data1[1].volume() < data1[2].volume() );
            break;
        case 11 ... 50: {   // 11-50: Pattern Base
            Filter1_long  = Pattern ( Filter1L_switch_ - 10,
                                      OpenD_, HighD_, LowD_, CloseD_ );
            break;
        }
    }

    switch( Filter1S_switch_ ){
        case 0:
            Filter1_short = true;
            break;
        case 1:
            Filter1_short = data1[0].close() > CloseD_[1];
            break;
        case 2:
            Filter1_short = data1[0].close() < CloseD_[1];
            break;
        case 3:
            Filter1_short = data1[0].close() > OpenD_[1];
            break;
        case 4:
            Filter1_short = data1[0].close() < OpenD_[1];
            break;
        case 5:
            Filter1_short = data1[0].close() > OpenD_[0];
            break;
        case 6:
            Filter1_short = data1[0].close() < OpenD_[0];
            break;
        case 7:             // Low of day
            Filter1_short = data1[0].low() == LowD_[0];
            break;
        case 8:             // Not Low of day
            Filter1_short = data1[0].low() != LowD_[0];
            break;
        case 9:             // Increasing volumes
            Filter1_short = (  data1[0].volume() > data1[1].volume()
                            && data1[1].volume() > data1[2].volume() );
            break;
        case 10:            // Decreasing volumes
            Filter1_short = (  data1[0].volume() < data1[1].volume()
                            && data1[1].volume() < data1[2].volume() );
            break;
        case 11 ... 50: {   // 11-50: Pattern Base
            Filter1_short = Pattern ( //OppositePattern(Filter1S_switch_),
                                      Filter1S_switch_ - 10,
                                      OpenD_, HighD_, LowD_, CloseD_ );
            break;
        }
    }
    // --------------------------------------------------------------------- //

    // ----------------------    COMBINE ALL FILTERS    -------------------- //
    bool All_filters_long  { FilterT && Filter1_long };
    bool All_filters_short { FilterT && Filter1_short };
    // --------------------------------------------------------------------- //

    // ------------------------    ENTRY RULES    -------------------------- //
    bool EnterLong  = ( TradingEnabled_
                        && ( Side_switch_ == 1 || Side_switch_ == 3 )
                        && All_filters_long );

    bool EnterShort = ( TradingEnabled_
                        && ( Side_switch_ == 2 || Side_switch_ == 3 )
                        && All_filters_short );
    // --------------------------------------------------------------------- //


    ////////////////////////  DO NOT EDIT THIS BLOCK  /////////////////////////
    //////////////////////////     OPEN TRADES     ////////////////////////////
    if( EnterLong ){
        signals[0] = Event { symbol_, data1[0].timestamp(),
                             "BUY", "STOP", BO_level_long,
                             Ncontracts_, name_,
                             (double) MyStop_ * Ncontracts_ , 0.0 };
    }

    if( EnterShort ){
        signals[1] = Event { symbol_, data1[0].timestamp(),
                             "SELLSHORT", "STOP", BO_level_short,
                             Ncontracts_, name_,
                             (double) MyStop_ * Ncontracts_, 0.0 };
    }
    ///////////////////////////////////////////////////////////////////////////
}


//-------------------------------------------------------------------------- //
/*! Define Exit rules and fill 'signals' array
*/
void MasterCode::compute_exit( const std::deque<Event>& data1,
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

void MasterCode::compute_signals( const PriceCollection& price_collection,
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
