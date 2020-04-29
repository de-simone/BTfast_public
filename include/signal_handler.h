#ifndef SIGNAL_HANDLER_H
#define SIGNAL_HANDLER_H

#include "position_handler.h"
#include "position_sizer.h"

#include <array>    // std::array
/*!
Class providing real-time handling of the signal from strategy,
and delivers an Order Event to the queue.

Only the following order types are implemented:
    - BUY/SELLSHORT next bar at 'suggested_price' STOP (entry)
    - BUY/SELLSHORT next bar at 'suggested_price' LIMIT (entry)
    - BUY/SELLSHORT, SELL/BUYTOCOVER next bar at MARKET (entry/exit)

Member Variables
- long_signals_: reference to vector of LONG signals, initialized by constructor.
- short_signals_: reference to vector of SHORT signals, initialized by constructor.
- position_handler_: const reference to PositionHandler object,
                        initialized by constructor.
            used to look for open positions.
- events_queue_: pointer to events_queue,
                 initialized in BTfast::run_backtest() and set via method
                 in BTfast::initialize_backtest()

*/



// ------------------------------------------------------------------------- //
// Class for handling signals in real time

class SignalHandler {

    std::vector<Event> &long_signals_;
    std::vector<Event> &short_signals_;
    const PositionHandler &position_handler_;
    const PositionSizer &position_sizer_;
    std::deque<Event> *events_queue_{nullptr};


    public:
        // constructor
        SignalHandler( std::vector<Event> &long_signals,
                       std::vector<Event> &short_signals,
                       const PositionHandler &position_handler,
                       const PositionSizer &position_sizer );

        // set events queue by pointer
        void set_events_queue(std::deque<Event> *events_queue);


        // Append new incoming signals to signals vector (if empty)
        void on_signals( const Event &barevent,
                         const std::array<Event,2> &signals );

        // Handle first signal of 'signals' vector
        void handle_first_signal( const Event &barevent,
                                  const Event &new_signal,
                                  std::vector<Event> &signals,
                                  bool &signal_triggered );
        // Convert signal event to order event at price 'order_price'
        void signal_to_order( const Event &bar, const Event &signal,
                              double order_price );
        // Check whether signal is triggered by new bar event
        double is_triggered( const Event &bar, const Event &signal );


};






#endif
