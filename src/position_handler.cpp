#include "position_handler.h"

#include "transaction.h"
#include "utils_math.h"      // round_double

#include <iostream>         // std::cout


// ------------------------------------------------------------------------- //
/*! Constructor
*/

PositionHandler::PositionHandler( Account &account )
 : account_{account} {
}


// ------------------------------------------------------------------------- //
/*! Set Queue by pointer
*/
void PositionHandler::set_events_queue(std::deque<Event> *events_queue){

    events_queue_ = events_queue;
}

//-------------------------------------------------------------------------- //
/*! Update open positions on new incoming BAR event
*/
void PositionHandler::on_bar( const Event &barevent ) {

    // Execute function only if input event is a BAR
    if( barevent.event_type() != "BAR" ){
        return;
    }

    // Check if some position is open ('open_positions_' not empty)
    if( !open_positions_.empty() ){

        //<<< Time bar_close_time { barevent.timestamp().time() };

        // Loop over open open positions
        for( auto& pos : open_positions_){

            // update open position and check whether SL/TP is hit
            pos.update_position( barevent );

            // update account (floating) balance
            //<<<
            /*
            if( (barevent.symbol().two_days_session()    // session spans two days
                 && bar_close_time == Time {0,0,0} )
                ||
                ( !barevent.symbol().two_days_session()   // session spans 1 day
                 && bar_close_time == barevent.symbol().session_close_time()
                )
            ){
                //account_.set_equity( account_.balance() + pos.pl_ );
                account_.add_to_equity( pos.pl_ );
            }
            */
            //<<<


            // if SL or TP is hit, send to queue the order to close position
            if( !pos.keep_open() ){

                if( pos.side() == "LONG") {
                    Event oev {pos.symbol().name(), barevent.timestamp(),
                                "SELL", "MARKET", barevent.close(),
                                pos.quantity(), pos.strategy_name(),
                                0.0, 0.0, pos.ticket()};
                    events_queue_->push_back(oev);
                }
                else if( pos.side() == "SHORT") {
                    Event oev {pos.symbol().name(), barevent.timestamp(),
                                "BUYTOCOVER", "MARKET", barevent.close(),
                                pos.quantity(), pos.strategy_name(),
                                0.0, 0.0, pos.ticket()};
                    events_queue_->push_back(oev);
                }
            }
        }
    }
}



//-------------------------------------------------------------------------- //
/*! Update account on new incoming FILL event received from execution.
*/
void PositionHandler::on_fill( const Event &fillevent ) {

    // Execute function only if input event is a FILL
    if( fillevent.event_type() != "FILL" ){
        return;
    }

    //--- Open new position
    std::string side {""};

    if( fillevent.action() == "BUY" || fillevent.action() == "SELLSHORT" ){
        if( fillevent.action() == "BUY" ){
            side = "LONG";
        }
        if( fillevent.action() == "SELLSHORT" ){
            side = "SHORT";
        }

        // Define new Position object
        Position new_pos { fillevent.strategy_name(), fillevent.symbol(),
                            side, fillevent.quantity(),
                            fillevent.timestamp(), fillevent.fill_price(),
                            fillevent.stoploss(), fillevent.takeprofit(),
                            fillevent.ticket() };
        // Add new position to open_positions
        open_positions_.push_back( new_pos );
    }
    //---

    //--- Close open position
    if( fillevent.action() == "SELL" || fillevent.action() == "BUYTOCOVER" ){

        // Identify position to close by strategy name
        Position pos_to_close {};
        for( const Position& pos : open_positions_ ){
            if( pos.strategy_name() == fillevent.strategy_name() ){
                pos_to_close = pos;
                break;
            }
        }

        if( pos_to_close.quantity() != 0 ){  // position to close found
            // P/L of the position to be closed
            double pos_pl {0.0};
            if( fillevent.action() == "SELL" ){
                pos_pl = (fillevent.fill_price()-pos_to_close.entry_price())
                            * fillevent.quantity()
                            * fillevent.symbol().big_point_value();
            }
            if( fillevent.action() == "BUYTOCOVER" ){
                pos_pl = (pos_to_close.entry_price()-fillevent.fill_price())
                            * fillevent.quantity()
                            * fillevent.symbol().big_point_value();
            }
            // Set pl_ member of the position
            pos_to_close.set_pl(pos_pl);

            // Update account (consolidated) balance
            account_.update_balance( pos_pl );

            // Define new trade
            Transaction new_trade { pos_to_close.strategy_name(),
                                    pos_to_close.symbol(),
                                    pos_to_close.side(),
                                    fillevent.quantity(),
                                    pos_to_close.entry_time(),
                                    pos_to_close.entry_price(),
                                    fillevent.timestamp(),      // exit time
                                    fillevent.fill_price(),     // exit price
                                    utils_math::round_double(
                                                pos_to_close.mae(), 1),
                                    utils_math::round_double(
                                                pos_to_close.mfe(), 1),
                                    pos_to_close.bars_in_trade(),
                                    pos_pl,
                                    account_.balance()
                                                - account_.initial_balance() };

            // Add new trade to history of closed transactions
            account_.add_transaction_to_history( new_trade );
            // Remove position from vector of open positions
            for( size_t i=0; i<open_positions_.size(); i++){
                if( open_positions_[i] == pos_to_close ){
                    open_positions_.erase(open_positions_.begin()+i);
                    break;
                }
            }
        }
        else {
            std::cout << ">>> ERROR: position to close for strategy "
                        << fillevent.strategy_name()
                        << " not found (position handler)"<< std::endl;
            exit(1);
        }
    }
    //---
}

//-------------------------------------------------------------------------- //
/*! Close all open positions on close of bar 'barevent'
*/
void PositionHandler::close_all_positions( const Event &barevent )
{
    // Execute function only if input event is a BAR
    if( barevent.event_type() != "BAR" ){
        return;
    }

    if( !open_positions_.empty() ){

        // Loop over open open positions
        for( auto & pos : open_positions_){

            // P/L of the position to be closed
            double pos_pl {0.0};
            if( pos.side() == "LONG" ){
                pos_pl = (barevent.close()-pos.entry_price())
                            * pos.quantity()
                            * pos.symbol().big_point_value();
            }
            if( pos.side() == "SHORT" ){
                pos_pl = (pos.entry_price()-barevent.close())
                            * pos.quantity()
                            * pos.symbol().big_point_value();
            }
            // Set pl_ member of the position
            pos.set_pl(pos_pl);
            // Update account (consolidated) balance
            account_.update_balance( pos_pl );

            // Define new trade
            Transaction new_trade { pos.strategy_name(),
                                    pos.symbol(),
                                    pos.side(), pos.quantity(),
                                    pos.entry_time(),
                                    pos.entry_price(),
                                    barevent.timestamp(),       // exit time
                                    barevent.close(),           // exit price
                                    utils_math::round_double( pos.mae(), 1),
                                    utils_math::round_double( pos.mfe(), 1),
                                    pos.bars_in_trade(),
                                    pos_pl,
                                    account_.balance()
                                    - account_.initial_balance() };

            // Add new trade to history of closed transactions
            account_.add_transaction_to_history( new_trade );
        }
    }
}
