#include "execution_handler_sim.h"

#include "utils_random.h"   // rand_generator

#include <iostream>         // std::cout


// ------------------------------------------------------------------------- //
/*! Constructor
*/

SimulatedExecution::SimulatedExecution( bool include_commissions,
                                        int slippage)

: ExecutionHandler{},
  include_commissions_{include_commissions},
  slippage_{slippage}
{}




// ------------------------------------------------------------------------- //
/*! Transform an ORDER event into a FILL event.
    Trivial execution: order->fill
*/
void SimulatedExecution::on_order( const Event &order ) const
{
    int ticket {0};
    // Entry order
    if( order.action() == "BUY" || order.action() == "SELLSHORT" ){
        // random integer (simulate broker assignment)
        std::uniform_int_distribution<> distr1(1,10000);
        ticket = distr1(utils_random::rand_generator);
    }
    // Exit order
    else if( order.action() == "SELL" || order.action() == "BUYTOCOVER" ){
        // ticket of position to close (carried by order event)
        ticket = order.ticket();
    }
    else{
        std::cout << ">>>ERROR: order event action " << order.action()
                    << " not recognized (execution)" << std::endl;
        exit(1);
    }

    // price offset wrt to ideal order price (can be positive or negative)
    double slipped_price {0};
    if( slippage_ > 0 ){
        std::uniform_int_distribution<> distr2(-slippage_, slippage_);
        slipped_price = distr2(utils_random::rand_generator)
                        * order.symbol().tick_size();
    }
    // round-turn commission cost by broker
    double commission {0.0};
    if( include_commissions_ ){
       commission = order.symbol().commission();
    }

    // price at which order is filled
    double fill_price = order.suggested_price() + slipped_price;

    // Create a Fill event
    Event new_fill { order.symbol(), order.timestamp(),
                     order.action(), order.order_type(),
                     fill_price, order.quantity(),
                     order.strategy_name(), order.stoploss(),
                     order.takeprofit(), ticket, commission };

    // Append fill event to events queue
    events_queue_->push_back( new_fill );

}
