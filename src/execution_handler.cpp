#include "execution_handler.h"

#include "execution_handler_sim.h"
//#include <cstdlib>      // exit
//#include <iostream>     // std::cout


// ------------------------------------------------------------------------- //
/*! Constructor
*/
ExecutionHandler::ExecutionHandler(){}

// ------------------------------------------------------------------------- //
/*! Body for Pure virtual Destructor
*/
ExecutionHandler::~ExecutionHandler(){}


// ------------------------------------------------------------------------- //
/*! Set Queue by pointer
*/
void ExecutionHandler::set_events_queue(std::deque<Event> *events_queue)
{
    events_queue_ = events_queue;
}








// ------------------------------------------------------------------------- //
/* Instantiate datafeed corresponding to 'datafeed_type' and
   assign unique_ptr to that object.
*/
void select_execution( std::unique_ptr<ExecutionHandler>& execution_ptr,
                        bool include_commissions, int slippage )
{

    execution_ptr = std::make_unique<SimulatedExecution>(include_commissions,
                                                         slippage);
}
