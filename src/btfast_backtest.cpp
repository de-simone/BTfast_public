#include "btfast.h"

#include "position_sizer.h"
#include "utils_print.h"    // print_progress
#include "utils_trade.h"    // FeaturesExtraction

#include <array>            // std::array
#include <iostream>         // std::cout
//#include <string>           // std::string

//-------------------------------------------------------------------------- //
/*! Initialize variables and class instances for a new backtest
*/
void BTfast::initialize_backtest (
                                 std::deque<Event> &events_queue,
                                 std::unique_ptr<DataFeed> &datafeed_ptr,
                                 std::unique_ptr<ExecutionHandler> &execution_ptr,
                                 std::unique_ptr<Strategy> &strategy_ptr,
                                 PriceCollection &price_coll,
                                 PositionHandler &pos_handler,
                                 SignalHandler &sig_handler,
                                 const parameters_t& strategy_params )
{
    // Clear bars in PriceCollection object
    price_coll.clear_bars();

    // Link events queue to Position Handler
    pos_handler.set_events_queue(&events_queue);
    // Link events queue to Signal Handler
    sig_handler.set_events_queue(&events_queue);

    // Link events queue to Datafeed
    datafeed_ptr->set_events_queue(&events_queue);
    // Open connection to datafeed
    datafeed_ptr->open_data_connection();

    // Initialize object for simulated execution, derived from ExecutionHandler,
    // and wrap it into the smart pointer 'execution_ptr'
    select_execution( execution_ptr, include_commissions_, slippage_ );
    // Link events queue to Execution Handler
    execution_ptr->set_events_queue(&events_queue);

    // Initialize Strategy derived object corresponding to 'strategy_name_'
    // and wrap it into the smart pointer 'strategy_ptr'
    select_strategy( strategy_ptr, strategy_name_,
                     symbol_, timeframe_, max_bars_back_ );

    // Load combination of strategy parameters into strategy object
    if( !strategy_params.empty() ){
        strategy_ptr -> set_param_values( strategy_params );
    }
    else{
        std::cout<< ">>> ERROR: empty parameters vector (initialize_backtest).\n";
        exit(1);
    }
}


//-------------------------------------------------------------------------- //
/*! Run single backtest
    account: reference to a Account object initialized just before
             running run_backtest().
    datafeed: smart pointer to DataFeed object
    strategy_params (const ref): combination of strategy parameters.

*/

void BTfast::run_backtest( Account &account,
                           std::unique_ptr<DataFeed> &datafeed,
                           const parameters_t& strategy_params )
{
    // Initalize Events Queue
    std::deque<Event> events_queue;
    // Initialize smart pointer to object derived from ExecutionHandler base class
    std::unique_ptr<ExecutionHandler> execution_handler {nullptr};
    // Initialize smart pointer to object derived from Strategy base class
    std::unique_ptr<Strategy> strategy {nullptr};
    // Initialize Price Collection
    // (maps containing lists of bars for each symbol/tf)
    PriceCollection price_collection { symbol_, timeframe_,
                                       max_bars_back_, random_noise_ };
    // Initialize Position Handler
    PositionHandler position_handler { account };
    // Initialize Position Sizer
    PositionSizer position_sizer { ps_type_, symbol_,
                                   num_contracts_, risk_fraction_ };

    // Initialize array of candidate signals filled by Strategy
    // 1st entry: Entry/exit LONG signals. 2nd entry: Entry/exit SHORT signals.
    std::array<Event, 2> signals {};
    // Initialize vectors of signals (max 2,each), filled by SignalHandler
    std::vector<Event> long_signals {};
    std::vector<Event> short_signals {};
    // Initialize Signal Handler (handling strategy signals)
    SignalHandler signal_handler { long_signals, short_signals,
                                   position_handler, position_sizer };

    // Initialize counters of parsed bars/days
    int bar_count {0};
    int day_count {0};
    // Initialize variable storing the latest bar parsed
    Event last_bar {};
    // Initialize actual initial/final dates parsed from file
    Date first_date_parsed {};
    Date last_date_parsed {};

    // Initialize all components for backtest
    initialize_backtest( events_queue, datafeed, execution_handler, strategy,
                         price_collection, position_handler, signal_handler,
                         strategy_params);

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
                    //--- Count bars and days elapsed
                    bar_count += 1;         // update counter of bars parsed
                    last_bar = event;       // set last bar
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
                    if( print_progress_ ){   // print progress
                        utils_print::print_progress( bar_count );
                    }
                    //---

                    // Update bar collections with latest bar
                    price_collection.on_bar(event);
                    // Update open positions and account status
                    position_handler.on_bar(event);
                    // Compute strategy signals, store them into signals array
                    strategy->compute_signals( price_collection,
                                               position_handler, signals );
                    //std::cout<< signals.at(0).tostring() << "\n";
                    // Handle strategy signals
                    signal_handler.on_signals( event, signals );
                }

                else if( event.event_type() == "ORDER" ){
                    //std::cout << event.tostring() << "\n";
                    execution_handler->on_order(event);
                }

                else if( event.event_type() == "FILL" ){
                    //std::cout<< event.tostring() << "\n";
                    // Update account with new fill
                    position_handler.on_fill(event);

                    //<<<
                    /*
                    // Extract market features before entry and write them to file
                    if( event.action()=="BUY" || event.action()=="SELLSHORT" ){
                        utils_trade::FeaturesExtraction( price_collection,
                                                         "tmp/features.csv" );
                    }
                    */
                    //<<<

                }

            }
        }
    }
    //--- End loop

    // Close all open positions on last bar parsed
    position_handler.close_all_positions( last_bar );

    // Close connection to datafeed
    datafeed->close_data_connection();

    //price_collection.print_bars();

    // Set member variables
    bar_counter_ = bar_count;
    day_counter_ = day_count;
    first_date_parsed_ = first_date_parsed;
    last_date_parsed_  = last_date_parsed;

}
