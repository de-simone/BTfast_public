#ifndef INSTRUMENTS_H
#define INSTRUMENTS_H

#include "datetime.h"   // Time


/*!
Define classes for each instrument Symbol

Member Variables:
- name_: root name
- contract_unit_: size of 1 contract in units of underlying asset
- margin_: initial margin (in USD)
- commission_: round-turn commission (in USD)
- tick_size_: size of minimum price movement
- tick_value_: value of 1 tick in USD per contract
- session_open_time_: *exchange* time of mkt open (when no DST in place)
- session_close_time_: *exchange* time of mkt close (when no DST in place)
- settlement_time_: *exchange* time of daily settlement
- two_days_session_: whether or not the session spans 2 days
   (e.g. 6pm-5pm day after)
- transaction_cost_: transaction costs in USD for round-turn trade:
                     commission+2*tick_value
transaction_cost_ticks_: transaction costs in ticks for round-turn trade:
                     commission/tick_value + 2
- big_point_value_: market value of a full point move in the price:
                  = tick_value/tick_size
- digits_: number of decimal digits of tick size

*/



// ------------------------------------------------------------------------- //
// Class for instruments

class Instrument {

    std::string name_ {""};
    int contract_unit_ {1};
    double margin_ {1000};
    double commission_ {3.0};
    double tick_size_ {0.01};
    double tick_value_ {10.0};
    Time session_open_time_ {18,0};
    Time session_close_time_ {17,0};
    Time settlement_time_ {14,00};

    bool two_days_session_ {true};          ///< set by set_session_type()
    double transaction_cost_ {50};          ///< set by set_transaction_cost()
    double transaction_cost_ticks_ {2.5};   ///< set by set_transaction_cost()
    double big_point_value_ {10000.0};      ///< set by set_big_point_value()
    int digits_ {1};                        ///< set by set_digits()


    public:
        // Constructors
        Instrument();
        Instrument( std::string symbol_name );

        // Getters
        std::string name() const { return(name_); }
        int contract_unit() const { return(contract_unit_); }
        double margin() const { return(margin_); }
        double commission() const { return(commission_); }
        double tick_size() const { return(tick_size_); }
        double tick_value() const { return(tick_value_); }
        Time session_open_time() const { return(session_open_time_); }
        Time session_close_time() const { return(session_close_time_); }
        Time settlement_time() const { return(settlement_time_); }
        double transaction_cost() const { return(transaction_cost_); }
        double transaction_cost_ticks() const { return(transaction_cost_ticks_);}
        double big_point_value() const { return(big_point_value_); }
        int digits() const { return(digits_); }
        bool two_days_session() const { return(two_days_session_); }


    private:
        void fill_members();
        void set_session_type();
        void set_transaction_cost();
        void set_big_point_value();
        void set_digits();

};






#endif
