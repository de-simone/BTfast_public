#ifndef POSITION_SIZER_H
#define POSITION_SIZER_H

#include "account.h"
#include "instruments.h"

#include <string>       // std::string


/*!
Class providing risk management functionalities by conputing
position sizing for orders.

Called by SignalHandler::signal_to_order.

Member Variables:
- symbol_: instance of instrument class
- mm_type_: type of money management/position sizing:
            "fixed-size", "fixed-notional", ...
- fixed_size_: number of contracts to be used in "fixed-size" type
- value_at_risk_: value at risk to be used in "fixed-notional" type

*/



// ------------------------------------------------------------------------- //
// Class for managing open trades

class PositionSizer {

    const std::string &ps_type_;
    const Instrument &symbol_;
    int num_contracts_ {1};
    double risk_fraction_ {0.1};

    public:
        // constructor
        PositionSizer( const std::string &ps_type, const Instrument &symbol,
                       int num_contracts, double risk_fraction );

        int compute_quantity( double price, double position_size_factor,
                              const Account &account ) const;

};






#endif
