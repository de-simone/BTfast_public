#ifndef PRICECOLLECTION_H
#define PRICECOLLECTION_H

#include "events.h"

#include <unordered_map>    // std::unordered_map
#include <deque>

/*!
Price Collection: nested unordered map containing lists of bars for
each symbol/timeframe.

bar_collection_:   { "symbol_name", {"tf", deque<Event>} }

to be called as bar_collection_["symbol_name"]["timeframe"]

bar_collection_["symbol_name"]["timeframe"] is a deque of Bar Events

Member Variables:
- symbol_name_: name of instrument symbol
- timeframe_: timeframe
- max_bars_back_: max number of bars in deque, for each symbol/tf.
- random_noise_: switch to control random noise added to data
- bar_collection_: nested unordered_map { "symbol_name", {"tf", deque<Event>} }
- delta_: time difference (in hours,mins) of 1 timeframe bar
- [tf_list_: list of requested timeframes]


*/



// ------------------------------------------------------------------------- //
// Class for closed trades

class PriceCollection {

    std::string symbol_name_;
    std::string timeframe_ {""};
    int max_bars_back_{100};
    bool random_noise_ {false};
    std::unordered_map< std::string,
        std::unordered_map< std::string, std::deque<Event> > > bar_collection_;
    Time delta_ {};


    public:
        // constructor
        PriceCollection( const Instrument &symbol, std::string timeframe,
                        int max_bars_back, bool random_noise);


        // Actions on new incoming bar event
        void on_bar(Event &barevent);
        // Update collection of session ("D") bars
        void update_D_bars(const Event &barevent);
        // Print all bars in map bar_collection
        void print_bars();
        // Clear deques for symbol_name_
        void clear_bars();


        // Getters
        int max_bars_back() const { return(max_bars_back_); }
        std::string symbol_name() const { return(symbol_name_); }
        std::string timeframe() const { return(timeframe_); }


        const std::unordered_map< std::string,
            std::unordered_map< std::string, std::deque<Event> > >
            & bar_collection() const { return( bar_collection_ ); };


};






#endif
