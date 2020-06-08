#include "mktoverview.h"

#include "filters/TA_indicators.h"
#include "utils_math.h"         // round_double, modulus
#include "utils_trade.h"        //


#include <fstream>      // std::ofstream


// ------------------------------------------------------------------------- //
/*! Constructor
*/
MktOverview::MktOverview( std::string name, Instrument symbol,
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
void MktOverview::set_param_values(
                        const std::vector< std::pair<std::string,int> >&
                                parameter_set )
{
    // Find parameter value in parameter_set by its name
    // (as it appears in XML file)
    //fractN_ = find_param_value_by_name( "fractN", parameter_set );

}




//-------------------------------------------------------------------------- //
/*!
    Variable definitions and preliminary calculations for computing signals.
    Variables passed as const reference (to avoid object copies and
    prevent modifications).
    Return: 1 if all OK; 0 if not enough session bars in history.
*/

int MktOverview::preliminaries( const std::deque<Event>& data1,
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

    if( SessionOpenPrice_ != OpenD_[0] ){ // Identify first bar of the day
        NewSession_ = true;
        SessionOpenPrice_ = OpenD_[0];
    }
    else{
        NewSession_ = false;
    }


    //-- Update Indicator Values
    /*
    bool make_new_entry {true};         // if computing indicator on intraday (data1)
    //bool make_new_entry {NewSession_}; // if computing indicators on "D" (data1D)
    ROC( roc_, data1, make_new_entry, max_bars_back_, 2, "CLOSE" );
    ROC( roc_, data1, true, max_bars_back_, 2, "CLOSE" );
    ROC( roc_, data1D, NewSession_, max_bars_back_, 2, "CLOSE" );
    ATR( atr_, data1, true, max_bars_back_, 10 );
    */
    //--


    return(1);
}


//-------------------------------------------------------------------------- //
/*! Define Entry rules and fill 'signals' array
*/
void MktOverview::compute_entry( const std::deque<Event>& data1,
                              const std::deque<Event>& data1D,
                              const PositionHandler& position_handler,
                              std::array<Event, 2> &signals )
{}



//-------------------------------------------------------------------------- //
/*! Define Exit rules and fill 'signals' array
*/
void MktOverview::compute_exit( const std::deque<Event>& data1,
                             const std::deque<Event>& data1D,
                             const PositionHandler& position_handler,
                             std::array<Event, 2> &signals )
{}


//-------------------------------------------------------------------------- //
/*! Handle computation of Entry/Exit signals and fill 'signals' array
    with Signal event (or NONE).

    First entry of 'signals' array:  entry/exit signals for LONG trades
    Second entry of 'signals' array: entry/exit signals for SHORT trades
*/

void MktOverview::compute_signals( const PriceCollection& price_collection,
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
    else{
        // Collect
        Volumes_.at( CurrentTime_.hour() ) += data1[0].volume();

        if( NewSession_ ){
            std::string fname { "Results/mkt_overview.txt" };
            std::ofstream outfile;
            outfile.open(fname, std::ios_base::app);

            DOWranges_.push_back(std::make_pair( CurrentDOW_,
                                                 HighD_[1] - LowD_[1] ) );

            outfile << utils_math::modulus(CurrentDOW_-1,7)
                    <<", "<<HighD_[1] - LowD_[1]<<"\n";
            outfile.close();

        }
    }

}
