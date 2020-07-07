#include "position.h"

#include <algorithm>    // std::max
#include <iostream>

// ------------------------------------------------------------------------- //
/*! Constructors
*/
// Empty position (0 arguments)
Position::Position(){};

// Valid position (9 arguments)
Position::Position(std::string strategy_name, Instrument symbol,
                    std::string side, int quantity,
                    DateTime entry_time, double entry_price,
                    double stoploss, double takeprofit, int ticket )

: strategy_name_{strategy_name}, symbol_{symbol},
  side_{side}, quantity_{quantity},
  entry_time_{entry_time}, entry_price_{entry_price},
  stoploss_{stoploss}, takeprofit_{takeprofit}, ticket_{ticket},
  mae_{0.0}, mfe_{0.0}, bars_in_trade_{1}, days_in_trade_{0},
  pl_{0.0}, keep_open_{true}
{}



//-------------------------------------------------------------------------- //
/*! Update open position at new incoming bar. (Update Profit/Loss)
    If SL or TP are hit: keep_open=False
    else: position stays open and keep_open=True.
    - barevent: bar event
*/
void Position::update_position( Event barevent ) {

    bool isSL { (stoploss_ != 0.0) };       // true if StopLoss is defined
    bool isTP { (takeprofit_ != 0.0) };     // true if TakeProfit is defined

    // update LONG position
    if( side_ == "LONG" ){
        // pl_ = ( (barvent.close() - entry_price_) * quantity_
        //           * barevent.symbol().big_point_value() )

        mae_ = std::max( mae_, ((entry_price_ - barevent.low())
                            * quantity_
                            * barevent.symbol().big_point_value()) );

        mfe_ = std::max( mfe_, ((barevent.high() - entry_price_)
                            * quantity_
                            * barevent.symbol().big_point_value()) );

        // Check if either SL or TP is defined
        if( isSL || isTP ){
            // Long position hits SL
            bool SLlong = isSL && (mae_ >= stoploss_);
            // Long position hits TP
            bool TPlong = isTP && (mfe_ >= takeprofit_);
            // if SL or TP is hit, close position
            if( SLlong || TPlong ){
                keep_open_ = false;
            }
            else{
                keep_open_ = true;
            }
        }
    }

    // update SHORT position
    if( side_ == "SHORT" ){
        // pl_ = ( (entry_price_-barvent.close()) * quantity_
        //           * barevent.symbol().big_point_value() )
        
        mae_ = std::max( mae_, ((barevent.high()-entry_price_)
                                * quantity_
                                * barevent.symbol().big_point_value()) );

        mfe_ = std::max( mfe_, ((entry_price_ - barevent.low())
                                * quantity_
                                * barevent.symbol().big_point_value()) );

        // Check if either SL or TP is defined
        if( isSL || isTP ){
            // Short position hits SL
            bool SLshort = isSL && (mae_ >= stoploss_);
            // Short position hits TP
            bool TPshort = isTP && (mfe_ >= takeprofit_);
            // if SL or TP is hit, close position
            if( SLshort || TPshort ){
                keep_open_ = false;
            }
            else{
                keep_open_ = true;
            }
        }
    }

    // One more bar in trade
    bars_in_trade_ += 1;

    // One more day (session) in trade
    if( barevent.timestamp().time() == barevent.symbol().session_close_time()){
        days_in_trade_ += 1;
    }
}

//-------------------------------------------------------------------------- //
/*! String representation
*/
std::string Position::tostring()  {

    char buffer[300];

    sprintf(buffer, "%d %s %6.5s %3d %20.19s %10.8s %14.2f\n",
            ticket_, symbol_.name().c_str(),
            side_.c_str(), quantity_,
            entry_time_.tostring().c_str(),
            std::to_string(entry_price_).c_str(), pl_);

    return(buffer);
}


//-------------------------------------------------------------------------- //
/*! Overloading the == operator
*/
bool Position::operator==(const Position& p) const {

    if( (strategy_name_ == p.strategy_name())
        && (symbol_.name() == p.symbol().name())
        && (side_ == p.side())
        && (quantity_ == p.quantity())
        && (entry_time_ == p.entry_time())
        && (entry_price_ == p.entry_price())
        && (ticket_ == p.ticket())
    ){
        return(true);
    }
    else{
        return(false);
    }
}
