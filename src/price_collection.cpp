#include "price_collection.h"

#include "utils_random.h"   // add_gaussian_noise

#include <iostream>         // std::cout


// ------------------------------------------------------------------------- //
/*! Constructor
*/
PriceCollection::PriceCollection(const Instrument &symbol, std::string timeframe,
                                 int max_bars_back, bool random_noise )
: symbol_name_{symbol.name()},
  timeframe_{timeframe},
  max_bars_back_{max_bars_back},
  random_noise_{random_noise}
{
    if( timeframe_ != "D" ){
        // exclude the first character of timeframe (="M")
        // to get minutes of intraday timeframe
        int mins = std::stoi( timeframe_.substr(1, std::string::npos) );
        // time of 1 timeframe bar
        delta_ = Time {mins/60, mins%60};
    }

    /*
    // Initialize deques for given symbol_name and different timeframes
    if( timeframe_ == "D" ){
        bar_collection_[symbol_name_] = { { "D", std::deque<Event>{} }
                                        };

    }
    else{
        bar_collection_[symbol_name_] = { { "D", std::deque<Event>{} },
                                          {timeframe_,std::deque<Event>{}}
                                        };
    }
    */
}

//-------------------------------------------------------------------------- //
/*! Actions on new incoming bar event
*/
void PriceCollection::on_bar(Event &barevent){

    //-- Modify bar by adding gaussian noise
    if( random_noise_ ){
        utils_random::add_gaussian_noise( barevent );
    }
    //--

    //-- Handle intraday bars
    if( timeframe_ != "D" && barevent.timeframe() != "D"){

        // delete last bar if deque length exceeds max_bars_back
        if( bar_collection_[barevent.symbol().name()]
                    [barevent.timeframe()].size() >= max_bars_back_ ){
            bar_collection_[barevent.symbol().name()]
                        [barevent.timeframe()].pop_back();
        }
        // append new intraday bar in front of deque bar_collection_
        bar_collection_[barevent.symbol().name()]
                                [barevent.timeframe()].push_front(barevent);

        update_D_bars(barevent);
    }
    //--

    //-- Handle session ("daily") bars
    else if( timeframe_ == "D" && barevent.timeframe() == "D" ) {

        // delete last bar if deque length exceeds max_bars_back
        if( bar_collection_[barevent.symbol().name()]["D"].size()
                                                    >= max_bars_back_){
            bar_collection_[barevent.symbol().name()]["D"].pop_back();
        }
        // append new session bar in front of deque bar_collection_
        bar_collection_[barevent.symbol().name()]["D"].push_front(barevent);

    }
    //--
}


//-------------------------------------------------------------------------- //
/*! Update bar collection for 'symbol' with daily timeframe
    ('day' means 'trading session')
*/
void PriceCollection::update_D_bars(const Event &barevent){

    // time at bar open/close
    Time bar_close_time = barevent.timestamp().time();
    Time bar_open_time = (barevent.timestamp().time() - delta_);

    // reference to daily bars, as alias
    std::deque<Event>& bar_list = bar_collection_[
                                                barevent.symbol().name()]["D"];

    // Create new bar at the Open of the session
    if( bar_open_time == barevent.symbol().session_open_time() ){

        // delete last bar if deque length exceeds max_bars_back
        if( bar_list.size() >= max_bars_back_){
            bar_list.pop_back();
        }

        Event new_D_bar {barevent.symbol(), barevent.timestamp(),
                         timeframe_, barevent.open(), barevent.high(),
                         barevent.low(), barevent.close(), barevent.volume() };
        // Insert new bar at the front of the deque
        bar_list.push_front(new_D_bar);
    }
    // Update current daily bar
    else if( !bar_list.empty() ){   // deque is not empty

        if( (barevent.symbol().two_days_session()    // session spans two days
             && (
                    ( bar_close_time > barevent.symbol().session_open_time()
                      && bar_open_time <= Time {23,59,59} )
                 || ( bar_close_time <= barevent.symbol().session_close_time()
                      && bar_open_time >= Time {0,0,0} )
                )
            )
            ||
            ( !barevent.symbol().two_days_session()   // session spans 1 day
              && (bar_close_time >  barevent.symbol().session_open_time())
              && (bar_close_time <= barevent.symbol().session_close_time())
            )
        ){
            // temporary value of current Close is latest close
            bar_list.at(0).set_close( barevent.close() );

            // if new bar has higher High or lower Low,
            // update High and Low of the current daily bar
            if( barevent.high() > bar_list.at(0).high()) {
                bar_list.at(0).set_high( barevent.high() );
            }
            if( barevent.low() < bar_list.at(0).low()) {
                bar_list.at(0).set_low( barevent.low() );
            }

            // Add volume cumulatively
            bar_list.at(0).set_volume( bar_list.at(0).volume()
                                        + barevent.volume() );

            // Update timestamp with that of latest incoming bar
            bar_list.at(0).set_timestamp( barevent.timestamp() );

        }
    }
}




//-------------------------------------------------------------------------- //
/*! Print all bars in map bar_collection
*/
void PriceCollection::print_bars(){

    for (auto s: bar_collection_){
		std::cout << s.first << std::endl;
        for( auto el : s.second ){
            std::cout << "  " << el.first << std::endl;
            // reverse order (latest bar printed last)
            for( auto it = el.second.rbegin(); it!=el.second.rend();it++ ){
                std::cout << "     " << (*it).tostring() << std::endl;
            }
        }
    }
}



//-------------------------------------------------------------------------- //
/*! Clear bar_collection_ deque for symbol_name_ and all timeframes
*/
void PriceCollection::clear_bars(){

    if( timeframe_ == "D" ){
        bar_collection_[symbol_name_]["D"].clear();
    }
    else{
        bar_collection_[symbol_name_][timeframe_].clear();
        bar_collection_[symbol_name_]["D"].clear();
    }

}
