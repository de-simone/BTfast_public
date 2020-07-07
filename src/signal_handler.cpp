#include "signal_handler.h"

#include <iostream>     // std::cout
#include <algorithm>    // std::min

// ------------------------------------------------------------------------- //
/*! Constructors
*/

SignalHandler::SignalHandler( std::vector<Event> &long_signals,
                              std::vector<Event> &short_signals,
                              const PositionHandler &position_handler,
                              const PositionSizer &position_sizer )

 : long_signals_ {long_signals},
   short_signals_ {short_signals},
   position_handler_ {position_handler},
   position_sizer_ {position_sizer}
 {}

// ------------------------------------------------------------------------- //
/*! Set Queue by pointer
*/
void SignalHandler::set_events_queue(std::deque<Event> *events_queue)
{
    events_queue_ = events_queue;
}



// ------------------------------------------------------------------------- //
/*! Append new incoming signals from signals array to signals vectors
    'long_signals_', 'short_signals_'.

    signals is an array of size=2, of the form
                {Long entry/exit, Short entry/exit}

   Each of the vectors 'long_signals_' and 'short_signals_' typically contains
   only 1 signal.
   The only exception to store 2 signals is when an Entry comes
   right after an Exit, or an Exit comes right after an Entry.

   Check if first signal of 'long_signals_' and 'short_signals_' is triggered,
   and, if yes, convert it into an order event and send it to the queue.

   'barevent' is already 'next bar' after strategy generated the signal.
*/
void SignalHandler::on_signals( const Event &barevent,
                                const std::array<Event,2> &signals )
{

    // Pointer to the vector of long or short signals
    std::vector<Event> *signals_ptr {nullptr};

    //--- Start loop over signals array
    for( int ls = 0; ls < signals.size(); ls++ ){

        const Event &new_signal = signals[ls];
        bool signal_triggered {false};

        //-- Set signals ptr to the corresponding vector
        if( ls == 0 ){      // process LONG entry/exit signals
            signals_ptr = &long_signals_;
        }
        else if( ls == 1 ){ // process SHORT entry/exit signals
            signals_ptr = &short_signals_;
        }
        else{
            std::cout<<">>> ERROR: signals array has size >2 "
                     << "(signal_handler)\n";
            exit(1);
        }
        //--

        //-- new_signal is NONE
        if( new_signal.event_type() == "NONE" ){

            if( !signals_ptr->empty() ){   // not empty signal vector

                handle_first_signal( barevent, new_signal, *signals_ptr,
                                     signal_triggered );
                if( signal_triggered ){
                    break;  // ignore SHORT signal if entered LONG
                }
            }
            else{                       // empty signal vector
                continue;
            }
        }
        //-- END new_signal is NONE

        //-- new_signal is not NONE
        else{
            // Discard new entry signals at the end of the session
            // (for intraday bars)
            if( (new_signal.action() == "BUY"
                 || new_signal.action() == "SELLSHORT")
                && ( new_signal.timestamp().time()
                     == new_signal.symbol().session_close_time() )
                && ( barevent.timeframe() != "D" ) ){

                if( !signals_ptr->empty() ) {  // not empty signal vector

                    handle_first_signal( barevent, Event {}, *signals_ptr,
                                         signal_triggered );
                    if( signal_triggered ){
                        break;  // ignore SHORT signal if entered LONG
                    }
                }
                continue;
            }

            //- Empty signal vector
            if( signals_ptr->empty() ){
                // Accept all Exit signals.
                // Accept Entry signals only if there are no open positions
                if( (    new_signal.action() == "SELL"
                      || new_signal.action() == "BUYTOCOVER" )
                  || position_handler_.open_positions().empty() ){
                    signals_ptr->push_back( new_signal );
                }
            }
            //- END empty signal vector
            //- Not empty signal vector
            else{
                // Append a LONG exit signal after a LONG entry signal,
                // or an entry after an exit
                if( ls == 0 &&
                    (signals_ptr->size() < 2) // no more than 2 simultaneous signals
                  &&(
                     ( signals_ptr->at(0).action() == "BUY"             // Entry
                       && new_signal.action() == "SELL" )               // Exit
                    ||
                    (( signals_ptr->at(0).action() == "SELL"           // Exit
                       || (!short_signals_.empty() &&
                           short_signals_.at(0).action() == "BUYTOCOVER" )) // Exit
                    && ( new_signal.action() == "BUY" ) )              // Entry
                        //|| new_signal.action() == "SELLSHORT" ) )    // Entry
                    )
                  ){
                    signals_ptr->push_back( new_signal );
                }
                // Append a SHORT exit signal after a SHORT entry signal,
                // or an entry after an exit
                else if( ls == 1 &&
                    (signals_ptr->size() < 2) // no more than 2 simultaneous signals
                  &&(
                     ( signals_ptr->at(0).action() == "SELLSHORT"       // Entry
                       && new_signal.action() == "BUYTOCOVER" )         // Exit
                    ||
                     (( signals_ptr->at(0).action() == "BUYTOCOVER"     // Exit
                        || (!long_signals_.empty() &&
                            long_signals_.at(0).action() == "SELL"  ))  // Exit
                     && ( new_signal.action() == "SELLSHORT" ) )        // Entry
                        //|| new_signal.action() == "BUY" )  )          // Entry
                    )
                  ){
                    signals_ptr->push_back( new_signal );
                }

                handle_first_signal( barevent, new_signal, *signals_ptr,
                                     signal_triggered );
                if( signal_triggered ){
                    break;  // ignore SHORT signal if entered LONG
                }

            } //- END not empty signal vector
        } //-- END new_signal is not NONE
    }//--- End for loop
}


// ------------------------------------------------------------------------- //
/*! Handle first (front) signal of 'signals' vector,
    on upcoming 'barevent' and 'new_signal' events.
    Set signal_triggered to true in case a signal is triggered.
*/
void SignalHandler::handle_first_signal( const Event &barevent,
                                         const Event &new_signal,
                                         std::vector<Event> &signals_vec,
                                         bool &signal_triggered )
{
    // Check if first signal in vector 'signals_vec' is triggered
    double order_price { is_triggered( barevent, signals_vec.at(0) ) };

    if( order_price != 0.0 ){
        // Triggered: convert first signal in vector to an order event
        signal_to_order( barevent, signals_vec.at(0), order_price );
        // If LONG entry is triggered, clear the vector of SHORT signals
        if( signals_vec.at(0).action() == "BUY" ){
            short_signals_.clear();
        }
        // If SHORT entry is triggered, clear the vector of LONG signals
        else if( signals_vec.at(0).action() == "SELLSHORT" ){
            long_signals_.clear();
        }
        // Remove first signal from signals vector
        signals_vec.erase(signals_vec.begin());
        signal_triggered = true;
    }
    else{
        // Not triggered, but new signal still in place:
        // Update first signal with new one
        if( new_signal.event_type() != "NONE" ){
            signals_vec.at(0) = new_signal;
        }
        // Not triggered, and signal disappeared:
        // Remove first signal from signals vector
        else{
            signals_vec.erase(signals_vec.begin());
        }
        signal_triggered = false;
    }
}




//-------------------------------------------------------------------------- //
/*! Convert the signal event 'signal' to an Order event at 'order_price'
    and append the order to the events queue.
*/
void SignalHandler::signal_to_order( const Event &bar, const Event &signal,
                                     double order_price )
{
    int quantity {0};
    // Compute position size for ENTRY signal
    if( signal.action() == "BUY" || signal.action() == "SELLSHORT" ){
        quantity = position_sizer_.compute_quantity( bar.close(),
                                                signal.position_size_factor(),
                                                position_handler_.account() );
    }
    // Quantity to close carried by EXIT signal
    else if( signal.action() == "SELL" || signal.action() == "BUYTOCOVER" ){
        quantity = signal.quantity_to_close();
    }

    // Convert signal to order if quantity is >0
    if( quantity > 0 ){
        // Adjust full stop loss and take profit based on quantity
        // ( signal only carries SL/TP per single contract )
        double stoploss { signal.stoploss() * quantity };
        double takeprofit { signal.takeprofit() * quantity };

        // Create order event
        Event new_order { signal.symbol(), bar.timestamp(),
                          signal.action(), signal.order_type(),
                          order_price, quantity, signal.strategy_name(),
                          stoploss, takeprofit, 0 };

        // Append order to events queue
        events_queue_->push_back( new_order );
    }
}





//-------------------------------------------------------------------------- //
/*! Check whether signal is triggered by new bar event.
    Returns 'order_price'!=0 if it is triggered, otherwise 0.
*/
double SignalHandler::is_triggered( const Event &bar, const Event &signal )
{
    double order_price {0.0};
    double entry_level { signal.suggested_price() };

    //--- Entry price for BUY STOP signal
    if( signal.action() == "BUY" && signal.order_type() == "STOP" ){

        if( bar.open() >= entry_level ){
            order_price = bar.open();
        }
        else if( bar.open() < entry_level ){
            if( bar.high() >= entry_level ){
                order_price = entry_level;
            }
            else{       // O<H<stop, stop price not reached by bar
                return(0.0);    // signal not Triggered
            }
        }
    }
    //---

    //--- Entry price for SELLSHORT STOP signal
    else if( signal.action() == "SELLSHORT" && signal.order_type() == "STOP" ){

        if( bar.open() <= entry_level ){
            order_price = bar.open();
        }
        else if( bar.open() > entry_level ){
            if( bar.low() <= entry_level ){
                order_price = entry_level;
            }
            else{       // O>L>stop, stop price not reached by bar
                return(0.0);    // signal not Triggered
            }
        }
    }
    //---

    //--- Entry price for BUY LIMIT signal
    else if( signal.action() == "BUY" && signal.order_type() == "LIMIT" ){

        if( bar.open() <= entry_level ){
            order_price = bar.open();
        }
        else if( bar.open() > entry_level ){
            if( bar.low() <= entry_level ){
                order_price = entry_level;
            }
            else{       // O>L>limit, limit price not reached by bar
                return(0.0);    // signal not Triggered
            }
        }
    }
    //---

    //--- Entry price for SELLSHORT LIMIT signal
    else if( signal.action() == "SELLSHORT" && signal.order_type() == "LIMIT"){

        if( bar.open() >= entry_level ){
            order_price = bar.open();
        }
        else if( bar.open() < entry_level ){
            if( bar.high() >= entry_level ){
                order_price = entry_level;
            }
            else{       // O<H<limit, limit price not reached by bar
                return(0.0);    // signal not Triggered
            }
        }
    }
    //---

    //--- Entry/Exit price for MARKET signal (at the open of next bar)
    else if( signal.order_type() == "MARKET" ){
        order_price = bar.open();
    }
    //---

    else{
        std::cout<< ">>> ERROR: order action/type not implemented"
                << " (signal_handler)" << std::endl;
        return(0.0);
    }

    return(order_price);
}








// ------------------------------------------------------------------------- //
/*! Append new incoming signals to signals vector (if empty).
 The vector 'signals_' typically contains only 1 signal.
 The only exception to store 2 signals is when an Entry comes
 right after an Exit, or an Exit comes right after an Entry.

 Then convert the signal to an order sent to the queue.

 'bar' is already 'next bar' after strategy generated the signal.

void SignalHandler::on_signal( const Event &barevent,
                               const Event &new_signal )
{
    //-- new_signal is NONE
    if( new_signal.event_type() == "NONE"){
        if( signals_.empty() ){     // empty signal vector
            return;
        }
        else{                       // not empty signal vector
            handle_first_signal( barevent, new_signal );
        }
    }
    //-- END new_signal is NONE

    //-- new_signal is not NONE
    else{

        // Discard new entry signals at the end of the session
        // (for intraday bars)
        if( (new_signal.action() == "BUY" || new_signal.action() == "SELLSHORT")
            && ( new_signal.timestamp().time()
                 == new_signal.symbol().session_close_time() )
            && ( barevent.timeframe() != "D" ) ){

            if( !signals_.empty() ){
                handle_first_signal( barevent, Event {} );
            }
            return;
        }

        if( signals_.empty() ){         // empty signal vector

            // Accept all Exit signals.
            // Accept Entry signals only if there are no open positions
            if( (    new_signal.action() == "SELL"
                  || new_signal.action() == "BUYTOCOVER" )
              || position_handler_.open_positions().empty() ){

                signals_.push_back( new_signal );
            }
        }
        else{                           // not empty signal vector

            // Append an exit signal after an entry signal,
            // or en exit after an entry
            if( new_signal.event_type() != "NONE" &&
                ( signals_.size() < 2 )    // no more than 2 simultaneous signals
               && (
                ( ( signals_.at(0).action() == "BUY"                    // Entry
                    || signals_.at(0).action() == "SELLSHORT" )         // Entry
                 && ( new_signal.action() == "SELL"                     // Exit
                    || new_signal.action() == "BUYTOCOVER" )  )         // Exit
                ||
                ( ( signals_.at(0).action() == "SELL"                   // Exit
                    || signals_.at(0).action() == "BUYTOCOVER" )        // Exit
                 && ( new_signal.action() == "BUY"                      // Entry
                    || new_signal.action() == "SELLSHORT" )  )          // Entry
                )
              ){
                signals_.push_back( new_signal );
            }
            handle_first_signal( barevent, new_signal );

        }
    }
    //-- END new_signal is not NONE
}
*/

// ------------------------------------------------------------------------- //
/* Handle first signal of 'signals_' vector

void SignalHandler::handle_first_signal( const Event &barevent,
                                         const Event &new_signal )
{
    // Check if first signal of list is triggered
    double order_price { is_triggered( barevent, signals_.at(0) ) };

    if( order_price != 0.0 ){
        // Triggered: convert first signal of list to an order event
        signal_to_order( barevent, signals_.at(0), order_price );
    }
    else{
        // Not triggered, but new signal still in place:
        // Replace first signal with new one
        if( new_signal.event_type() != "NONE" ){
            signals_.at(0) = new_signal;
        }
        // Not triggered, and signal disappeared:
        // Remove first signal from the list
        else{
            signals_.erase(signals_.begin());
        }
    }
}
*/

//-------------------------------------------------------------------------- //
/*
 Convert the signal event 'signal' to an Order event at 'order_price'
 and append the order to the events queue.
 Remove signal from signals_ vector.

void SignalHandler::signal_to_order( const Event &bar, const Event &signal,
                                     double order_price )
{
    int quantity = std::min( 1, signal.suggested_quantity() );

    // Create order event
    Event new_order { signal.symbol(), bar.timestamp(),
                    signal.action(), signal.order_type(),
                    order_price, quantity,
                    signal.strategy_name(),
                    signal.stoploss(), signal.takeprofit(), 0 };


    // Append order to events queue
    events_queue_->push_back( new_order );
    // Remove signal from signals vector
    signals_.erase( std::remove(signals_.begin(), signals_.end(), signal),
                    signals_.end() );
}
*/
