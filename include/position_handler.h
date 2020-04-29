#ifndef POSITION_HANDLER_H
#define POSITION_HANDLER_H

#include "account.h"
#include "events.h"
#include "position.h"

#include <deque>        // std::deque
#include <vector>       // std::vector
#include <string>       // std::string


/*!
Class providing real-time status and handling of open positions.
If there are open positions, it updates the account class on
every incoming bar.

Member Variables
- account_: reference to instance of account class,
            initialized by constructor when instantiating BTfast object.
- events_queue_:  pointer to events_queue,
                 initialized in BTfast.run_backtest() and set via method.
- open_positions_: vector of open positions (instances of Position class)

*/



// ------------------------------------------------------------------------- //
// Class for managing open trades

class PositionHandler {

    Account &account_;
    std::deque<Event> *events_queue_{nullptr}; //<<<
    std::vector<Position> open_positions_ {};


    public:
        // constructor
        PositionHandler( Account &account );

        // set events queue by pointer
        void set_events_queue(std::deque<Event> *events_queue); //<<<

        // Update open positions at new incoming BAR event
        void on_bar( const Event &barevent );
        // Update account on new incoming FILL event received from execution.
        void on_fill( const Event &fillevent );
        // Close all open positions on bar 'barevent'
        void close_all_positions( const Event &barevent );

        // Getters
        Account& account() const { return(account_); }
        std::deque<Event>* events_queue() const { return(events_queue_); }
        std::vector<Position> open_positions() const { return(open_positions_);}

        // Setters
        //void clear_open_positions(){
        //                            open_positions_ = std::vector<Position>{};}
};






#endif
