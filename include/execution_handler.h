#ifndef EXECUTION_HANDLER_H
#define EXECUTION_HANDLER_H

#include "events.h"


#include <deque>        // std::deque
#include <memory>       // std::unique_ptr


// ------------------------------------------------------------------------- //
/*!
Abstract base class for all Execution modes (simulated, broker, etc)

Member Variables:
- events_queue_: pointer to events_queue,
                 initialized in BTfast.run_backtest() and set via method.

*/

class ExecutionHandler {

    protected:
        std::deque<Event> *events_queue_{nullptr};


    public:
        // Constructor
        ExecutionHandler();
        // Pure virtual Destructor (requires a function body)
        virtual ~ExecutionHandler() = 0;

        // Getters
        std::deque<Event>* events_queue() const { return(events_queue_); };

        // Link queue to events queue by pointer
        void set_events_queue(std::deque<Event> *events_queue);

        // Pure virtual functions (overridden by derived objects)
        virtual bool include_commissions() const  = 0;
        virtual int slippage() const = 0;
        virtual void on_order( const Event &order ) const = 0;

};




// ------------------------------------------------------------------------- //
/*! Instantiate Execution Handler corresponding to 'datafeed_type' and
    assign unique_ptr to that object.
*/
void select_execution( std::unique_ptr<ExecutionHandler>& execution_ptr,
                        bool include_commissions, int slippage );

#endif
