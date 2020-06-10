#include "btfast.h"

#include "position_sizer.h"
#include "utils_math.h"
#include "utils_print.h"    // print_progress

#include <array>            // std::array
#include <iostream>         // std::cout
#include <fstream>          // std::ofstream
//#include <stdexcept>        // std::invalid_argument
#include <string>
#include <utility>          // std::pair
#include <vector>           // std::vector



// ------------------------------------------------------------------------- //
/*! Parse data and collect market info, without strategy signals
    account: reference to a Account object initialized just before
             running run_overview().
    datafeed: smart pointer to DataFeed object
*/

void BTfast::run_overview( Account &account,
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

    // Initialize counters of parsed bars/days
    int bar_count {0};
    int day_count {0};
    // Initialize actual initial/final dates parsed from file
    Date first_date_parsed {};
    Date last_date_parsed {};
    Date CurrentDate {};
    Time CurrentTime {};
    int CurrentDOW {0};
    double SessionOpenPrice {0.0};
    bool NewSession {false};
    std::array<double, 2> OpenD {};
    std::array<double, 2> HighD {};
    std::array<double, 2> LowD {};
    std::array<double, 2> CloseD {};



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

            if( event.event_type() != "NONE" && event.event_type() == "BAR" ){

                //std::cout << event.tostring() << "\n";
                // Update bar counter and print progress
                bar_count += 1;
                // Set actual initial/final dates
                if( bar_count == 1 ){
                    first_date_parsed = event.timestamp().date();
                    last_date_parsed  = event.timestamp().date();
                    day_count += 1;
                }
                else{
                    if( !(event.timestamp().date() == last_date_parsed )){
                        day_count += 1; // update counter of days parsed
                    }
                    last_date_parsed = event.timestamp().date();
                }

                // Update bar collections with latest bar
                price_collection.on_bar(event);

                //-- Extract bars from price collection
                // dataD: session (daily) bars
                const std::deque<Event>& data1D =
                            price_collection.bar_collection().at(
                                            symbol_.name()).at("D");

                // data1: intraday bars (= data1D for timeframe="D")
                const std::deque<Event>& data1 =  ( timeframe_=="D" )
                    ? price_collection.bar_collection().at(
                                            symbol_.name()).at("D")
                    : price_collection.bar_collection().at(
                                            symbol_.name()).at(timeframe_);
                if( data1.empty() || data1D.empty() ){
                    continue;
                }
                //--

                //-- Time attributes of current reference bar
                CurrentTime = data1[0].timestamp().time();
                CurrentDate = data1[0].timestamp().date();
                CurrentDOW  = data1[0].timestamp().weekday();
                //--

                //-- OHLC of current session and previous session
                if( data1D.size() < OpenD.size() ){
                    continue;
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

                //-- Identify new session
                if( SessionOpenPrice != OpenD[0] ){
                    NewSession = true;
                    SessionOpenPrice = OpenD[0];
                }
                else{
                    NewSession = false;
                }

                // ------------   START COLLECTING MARKET INFO   ----------- //
                // Volume for each hour
                volume_hour_.at( CurrentTime.hour() ) += data1[0].volume();

                // Daily Range H-L for each day of week
                if( NewSession ){

                    int dow {0};
                    if( symbol_.two_days_session() ){
                        dow = data1D[0].timestamp().weekday();
                    }
                    else{
                        dow = data1D[1].timestamp().weekday();
                    }

                    co_range_dow_.at( dow-1 ) += ( CloseD[1]-OpenD[1] )
                                                    / symbol_.tick_size();

                    hl_range_.push_back( (HighD[1]-LowD[1])
                                            / symbol_.tick_size() );

                }
                // -------------   END COLLECTING MARKET INFO   ------------ //

            }
        }
    }
    //--- End loop

    // Close connection to datafeed
    datafeed->close_data_connection();

    // Set member variables
    bar_counter_ = bar_count;
    day_counter_ = day_count;
    first_date_parsed_ = first_date_parsed;
    last_date_parsed_  = last_date_parsed;


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
