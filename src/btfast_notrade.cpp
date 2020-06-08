#include "btfast.h"

#include "position_sizer.h"
#include "utils_math.h"
#include "utils_print.h"    // print_progress

#include <array>            // std::array
#include <iostream>         // std::cout
//#include <stdexcept>        // std::invalid_argument
#include <string>
#include <vector>           // std::vector



//-------------------------------------------------------------------------- //
/*! Parse data without strategy signals
    account: reference to a Account object initialized just before
             running run_notrade().
    datafeed: smart pointer to DataFeed object

*/

void BTfast::run_notrade( Account &account,
                          std::unique_ptr<DataFeed> &datafeed,
                          const parameters_t& strategy_params )
{


    // Initalize Events Queue
    std::deque<Event> events_queue;
    // Initialize smart pointer to object derived from DataFeed base class
    //std::unique_ptr<DataFeed> datafeed {nullptr};
    // Initialize smart pointer to object derived from ExecutionHandler base class
    std::unique_ptr<ExecutionHandler> execution_handler {nullptr};
    // Initialize smart pointer to object derived from Strategy base class
    std::unique_ptr<Strategy> strategy {nullptr};
    // Initialize Price Collection
    // (maps containing lists of bars for each symbol/tf)
    PriceCollection price_collection { symbol_, timeframe_,
                                       max_bars_back_, false };
    // Initialize Position Handler
    PositionHandler position_handler { account };
    // Initialize Position Sizer
    PositionSizer position_sizer { ps_type_, symbol_,
                                   num_contracts_, risk_fraction_ };
    // Initialize Signals (vector of signals (max 2),filled by signal_handler)
    std::vector<Event> signals {};
    // Initialize Signal Handler (handling strategy signals)
    SignalHandler signal_handler { signals, signals,
                                   position_handler, position_sizer };


    // Initialize all components for backtest
    initialize_backtest( events_queue, datafeed, execution_handler, strategy,
                         price_collection, position_handler, signal_handler,
                         strategy_params );

    int bar_count {0};

    //--- Start loop 
    while ( datafeed->continue_parsing() ) {

        if( events_queue.empty() ){    // queue is empty
            // get next bar from datafeed and append it to end of queue
            datafeed->stream_next_bar();

        }
        else{                           // queue not empty

            // get first event on queue and remove it from queue
            Event event = events_queue.front();
            events_queue.pop_front();

            if( event.event_type() != "NONE" ){

                if( event.event_type() == "BAR" ){
                    //std::cout << event.tostring() << "\n";
                    // Update bar counter and print progress
                    bar_count += 1;
                    // Set actual initial/final dates
                    if( bar_count == 1 ){
                        first_date_parsed_ = event.timestamp().date();
                    }
                    else if( bar_count == datafeed->tot_bars() ){
                        last_date_parsed_ = event.timestamp().date();
                    }

                    if( print_progress_ ){
                        utils_print::print_progress( bar_count );
                    }

                    // Update bar collections with latest bar
                    price_collection.on_bar(event);

                }
            }
        }
    }
    //--- End loop

    // Close connection to datafeed
    datafeed->close_data_connection();

    bar_counter_ = bar_count;
    //price_collection.print_bars();

    //<<< start testing
    /*
    int run_mode;
    try{
        run_mode = std::stoi( "" );
        std::cout<<"yes\n";
    }
    catch (const std::invalid_argument& er) {
        std::cout<<"no\n";
        std::cerr << "invalid argument error: " << er.what() << '\n';
    }
    */
    /*
    std::vector<Event> evs{};
    evs.push_back( Event{symbol_, DateTime {}, "M15", 0,0,0,0,0});
    evs.push_back(Event{symbol_, DateTime {}, "M15", 1,1,1,1,1});
    std::cout<< "before: \n";
    for( auto ev: evs){
        std::cout<<  ev.tostring()<<"\n";
    }
    //evs.clear();
    evs.erase(evs.begin());
    std::cout<< "after : \n";
    for( auto ev: evs){
        std::cout<<  ev.tostring()<<"\n";
    }

    std::array<Event,2> evs{};
    evs.back() = Event{symbol_, DateTime {}, "M15", 0,0,0,0,0};
    evs.back() = Event{symbol_, DateTime {}, "M15", 1,1,1,1,1};

    evs.fill(Event{});
    for( auto ev: evs){
        std::cout<< ev.tostring()<<"\n";
    }
     */
    //<<< end testing
}
