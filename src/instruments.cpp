#include "instruments.h"

#include "utils_math.h" // round_double
#include <cmath>        // std::abs
#include <iostream>     // std::cout


// ------------------------------------------------------------------------- //
/*! Constructors
*/
// None instrument (0 arguments)
Instrument::Instrument(): name_{"none"}{}

// Instrument identified by its name (1 argument)
Instrument::Instrument( std::string symbol_name )
    : name_{ symbol_name } {

    fill_members(); // fill member variables according to symbol name
    set_session_type();
    set_transaction_cost();
    set_big_point_value();
    set_digits();

}


// ------------------------------------------------------------------------- //
/*! Determine whether session spans two days (true) or one day (false)
*/
void Instrument::set_session_type(){

    if( session_open_time_ > session_close_time_ ) {
        two_days_session_ = true;
    }
    else{
        two_days_session_ = false;
    }
}

// ------------------------------------------------------------------------- //
/*! Set transaction costs in USD for instrument:
    round-turn commission + 2 ticks slippage
*/
void Instrument::set_transaction_cost() {

    transaction_cost_ = commission_ + 2 * tick_value_;
    transaction_cost_ticks_ = utils_math::round_double(
                                    commission_/tick_value_ + 2.0, 2);
}

// ------------------------------------------------------------------------- //
/*! Compute big point value
*/
void Instrument::set_big_point_value() {

    big_point_value_ = tick_value_ / tick_size_;
}

// ------------------------------------------------------------------------- //
/*! Count number of decimal digits of tick_size
*/
void Instrument::set_digits(){

    int count = 0;
    double num = std::abs(tick_size_);
    num = num - int(num);

    while( std::abs(num) >= 0.0000001 ){

        num = num * 10;
        count = count + 1;
        num = num - int(num);
    }
    digits_ = count;
}


// ------------------------------------------------------------------------- //
/*! Fill member variables according to the symbol name
*/
void Instrument::fill_members (){

    // Soybean Oil - CBOT (Chicago)
    if( name_ == "BO" ){
        contract_unit_ = 60000; // pounds
        margin_ = 848;
        commission_ = 3.0;
        tick_size_ = 0.01;
        tick_value_ = 6.0;
        session_open_time_ = Time {19,0};
        session_close_time_ = Time {13,20};
        settlement_time_ = Time {13,15};
    }

    // Corn - CBOT (Chicago)
    else if( name_ == "C" ){
        contract_unit_ = 5000; // Bushels
        margin_ = 990;
        commission_ = 3.0;
        tick_size_ = 0.25;
        tick_value_ = 12.5;
        session_open_time_ = Time {19,0};
        session_close_time_ = Time {13,20};
        settlement_time_ = Time {13,15};
    }

    // E-mini EUR/USD - CME (Chicago)
    else if( name_ == "E7" ){
        contract_unit_ = 62500; // euro
        margin_ = 1252;
        commission_ = 3.0;
        tick_size_ = 0.0001;
        tick_value_ = 6.25;
        session_open_time_ = Time {17,0};
        session_close_time_ = Time {16,0};
        settlement_time_ = Time {14,00};
    }

    // Gold - COMEX (NY)
    else if( name_ == "GC" ){
        contract_unit_ = 100; // troy ounces
        margin_ = 6600;
        commission_ = 3.0;
        tick_size_ = 0.1;
        tick_value_ = 10.0;
        session_open_time_ = Time {18,0};
        session_close_time_ = Time {17,0};
        settlement_time_ = Time {13,30};
    }

    // Natural Gas (Henry Hub) - NYMEX (NY)
    else if( name_ == "NG" ){
        contract_unit_ = 10000; // MBtu
        margin_ = 2200;
        commission_ = 3.0;
        tick_size_ = 0.001;
        tick_value_ = 10.0;
        session_open_time_ = Time {18,0};
        session_close_time_ = Time {17,0};
        settlement_time_ = Time {14,30};
    }

    // E-mini Russell 2000 index - CME (Chicago)
    else if( name_ == "RTY" ){
        contract_unit_ = 50; // 50 x index value
        margin_ = 6380;
        commission_ = 3.0;
        tick_size_ = 0.1;
        tick_value_ = 5.0;
        session_open_time_ = Time {17,0};
        session_close_time_ = Time {16,0};
        settlement_time_ = Time {14,30};
    }

    // Wheat (SRW) - CBOT (Chicago)
    else if( name_ == "W" ){
        contract_unit_ = 5000; // Bushels
        margin_ = 1375;
        commission_ = 3.0;
        tick_size_ = 0.25;
        tick_value_ = 12.5;
        session_open_time_ = Time {19,0};
        session_close_time_ = Time {13,20};
        settlement_time_ = Time {13,15};
    }








    else{
        std::cout << ">>> ERROR: invalid instrument symbol.\n";
        exit(1);
    }
}
