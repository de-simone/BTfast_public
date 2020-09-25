#include "utils_trade.h"


#include <algorithm>    // std::max_element, std::min_element
#include <array>    // std::array
#include <fstream>     // std::ofstream
#include <iostream> // std::cout

using std::max_element;
using std::min_element;


// ------------------------------------------------------------------------- //
// Return Market position of current position: 0=flat, 1=long, -1=short

int utils_trade::MarketPosition( const PositionHandler &position_handler )
{
    int mp {0};
    if( !position_handler.open_positions().empty() ){
        if( position_handler.open_positions().back().side() == "LONG" ){
            mp = 1;
        }
        else if( position_handler.open_positions().back().side() == "SHORT" ){
            mp = -1;
        }
    }
    else{
        mp = 0;
    }
    return(mp);
}

// ------------------------------------------------------------------------- //
// Find the position with side "LONG" or "SHORT" and corresponding to
//    strategy 'strat_name', inside the vector of open positions

Position utils_trade::find_pos_to_close( const std::string& side,
                                         const std::string& strat_name,
                                 const std::vector<Position>& open_positions )
{
    Position result {};
    if( side == "LONG" || side == "SHORT" ){
        for( const Position& pos : open_positions ){
            if( pos.side() == side && pos.strategy_name() == strat_name ){
                result = pos;
                break;
            }
        }
    }
    return(result);
}



// ------------------------------------------------------------------------- //
// Compute the number of entries during current session
// (deprecated because slow)
/*
int utils_trade::EntriesInSession( const DateTime &timestamp,
                                    const Instrument &symbol,
                                    const PositionHandler &position_handler)
{
    // positions still open during current session
    int open_trades{0};
    for( auto pos: position_handler.open_positions() ){
        if(
            ( symbol.two_days_session()         // session spans two days
             &&(
                 // opened previous day
                 ( ( timestamp.date() == pos.entry_time().add_bdays(1).date() )
                 && ( timestamp.time() <= symbol.session_close_time() )
                 && ( pos.entry_time().time() >= symbol.session_open_time() )
                 )
                // opened same day
              || ( ( timestamp.date() == pos.entry_time().date() )
                 &&( ( timestamp.time() <= symbol.session_close_time()
                     && pos.entry_time().time()<=symbol.session_close_time() )
                   ||( timestamp.time() >= symbol.session_open_time()
                     && pos.entry_time().time()>=symbol.session_open_time() ) )
                 )
             )
            )

            ||( !symbol.two_days_session()     // session spans one day
                && ( timestamp.date() == pos.entry_time().date() )
            )
        ){
            open_trades += 1;
        }
    }

    //trades open and closed during current session
    int closed_trades{0};
    for( auto tr: position_handler.account().transactions() ){
        if(
            ( symbol.two_days_session()         // session spans two days
             &&(
                 // opened previous day
                 ( ( timestamp.date() == tr.entry_time().add_bdays(1).date() )
                 && ( timestamp.time() <= symbol.session_close_time() )
                 && ( tr.entry_time().time() >= symbol.session_open_time() )
                 )
                // opened same day
              || ( ( timestamp.date() == tr.entry_time().date() )
                 &&( ( timestamp.time() <= symbol.session_close_time()
                     && tr.entry_time().time()<=symbol.session_close_time() )
                   ||( timestamp.time() >= symbol.session_open_time()
                     && tr.entry_time().time()>=symbol.session_open_time() ) )
                 )
             )
            )

            ||( !symbol.two_days_session()     // session spans one day
                && ( timestamp.date() == tr.entry_time().date() )
            )
        ){
            closed_trades += 1;
        }
    }
    return(open_trades + closed_trades);
}
*/


// --------------------------------------------------------------------- //
// Extract features from price_collection from recent history before
// trade entry, and write them on file 'fname'

void utils_trade::FeaturesExtraction( const PriceCollection &price_collection,
                                      std::string fname )
{

    std::string symbol_name { price_collection.symbol_name() };
    std::string timeframe { price_collection.timeframe() };

    //-- Extract bars from price collection
    // dataD: session (daily) bars
    const std::deque<Event>& data1D =
                price_collection.bar_collection().at(symbol_name).at("D");

    // data1: intraday bars (= data1D for timeframe="D")
    const std::deque<Event>& data1 =
                ( timeframe == "D" ) ? price_collection.bar_collection().at(
                                                symbol_name).at("D")
                                    : price_collection.bar_collection().at(
                                                symbol_name).at(timeframe);
    //--

    //-- Check if bar collections are not empty
    if( data1.empty() || data1D.empty() ){
        return;
    }
    //--

    //-- Time attributes of current reference bar
    //Time CurrentTime { data1[0].timestamp().time() };
    //Date CurrentDate { data1[0].timestamp().date() };
    int CurrentDOW  { data1[0].timestamp().weekday() };
    //--

    //-- OHLC of current and previous 3 sessions
    std::array<double, 3> OpenD {};
    std::array<double, 3> HighD {};
    std::array<double, 3> LowD {};
    std::array<double, 3> CloseD {};

    if( data1D.size() < OpenD.size() ){     // daily bar collection long enough
        return;
    }
    else{
        for( int j = 0; j < OpenD.size(); j++ ){
            OpenD[j]  = data1D[j].open();
            HighD[j]  = data1D[j].high();
            LowD[j]   = data1D[j].low();
            CloseD[j] = data1D[j].close();
        }
    }
    //--

    //-- OHLC of current and previous 3 bars
    std::array<double, 3> Open {};
    std::array<double, 3> High {};
    std::array<double, 3> Low {};
    std::array<double, 3> Close {};

    if( data1.size() < Open.size() ){     // daily bar collection long enough
        return;
    }
    else{
        for( int j = 0; j < Open.size(); j++ ){
            Open[j]  = data1[j].open();
            High[j]  = data1[j].high();
            Low[j]   = data1[j].low();
            Close[j] = data1[j].close();
        }
    }
    //--


    //-- Fill vector of features
    std::vector<double> features {};

    features.push_back( (double) CurrentDOW );  // Day of week
    features.push_back( HighD[1]-LowD[1] ); // range of previous session
    features.push_back( High[1]-Low[1] ); // range of previous bar
    // range of previous sessions
    features.push_back( *max_element(HighD.begin()+1, HighD.end())
                        - *min_element(LowD.begin()+1, LowD.end()) );
    // range of previous bars
    features.push_back( *max_element(High.begin()+1, High.end())
                        - *min_element(Low.begin()+1, Low.end()) );


    for( auto el: OpenD ){
        features.push_back(el);
    }
    for( auto el: HighD ){
        features.push_back(el);
    }
    for( auto el: LowD ){
        features.push_back(el);
    }
    for( auto el: CloseD ){
        features.push_back(el);
    }
    for( auto el: Open ){
        features.push_back(el);
    }
    for( auto el: High ){
        features.push_back(el);
    }
    for( auto el: Low ){
        features.push_back(el);
    }
    for( auto el: Close ){
        features.push_back(el);
    }
    //--

    //-- Write features to file
    std::ofstream outfile;
    outfile.open(fname, std::ofstream::app ); // append
    /*
    std::cout<<"# OpenD0, OpenD1, OpenD2, OpenD3, OpenD4, OpenD5, "
            << "HighD0, HighD1, HighD2, HighD3, HighD4, HighD5, "
            << "LowD0, LowD1, LowD2, LowD3, LowD4, LowD5, "
            << "CloseD0, CloseD1, CloseD2, CloseD3, CloseD4, CloseD5 \n";
    */
    //std::cout<< CurrentDate.tostring()<<" "<<CurrentTime.tostring()<<"\n";

    for( auto f: features ){
        outfile << f <<", ";
    }
    outfile<<"\n";
    outfile.close();
    //--

}
