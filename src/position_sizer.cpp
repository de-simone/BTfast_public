#include "position_sizer.h"

#include "utils_math.h" // nearest_int

#include <algorithm>    // std::min
#include <cmath>        // std::abs
#include <iostream>     // std::cout


// ------------------------------------------------------------------------- //
/*! Constructor
*/

PositionSizer::PositionSizer( const std::string &ps_type,
                              const Instrument &symbol,
                              int num_contracts, double risk_fraction )
: ps_type_ {ps_type},
  symbol_ {symbol},
  num_contracts_ {num_contracts},
  risk_fraction_ {risk_fraction}
{}


// ------------------------------------------------------------------------- //
/*! Compute position size according to the mm_type variable.
    Return: number of contracts to assign to order
*/
int PositionSizer::compute_quantity(double price, double position_size_factor,
                                    const Account &account) const
{
    int result {1};

    if( ps_type_ == "fixed_size" ){
        result = num_contracts_;
    }
    else if( ps_type_ == "fixed_notional" ){
        // value at risk is risk_fraction_ the initial balance
        double value_at_risk { account.initial_balance() * risk_fraction_ };
        double notional_value { symbol_.contract_unit() * price };
        if( notional_value != 0.0 ){
            result = std::max( 1,
                               utils_math::nearest_int(
                                            value_at_risk/notional_value )
                             );
        }
        else{
            result = num_contracts_;
        }
    }
    else if( ps_type_ == "fixed_fractional" ){
        // n contracts = risk% * balance / |max loss|
        double balance { account.balance() };
        double max_loss { account.largest_loss() };
        if( max_loss != 0.0 ){
            result = std::max( 1,
                               utils_math::nearest_int(
                                risk_fraction_ * balance / std::abs(max_loss) )
                              );
        }
        else{
            result = num_contracts_;
        }
    }

    else{
        std::cout<<">>>ERROR: invalid ps_type (position_sizer).\n";
        exit(1);
    }

    // Scale by position size factor
    result = utils_math::nearest_int( result * position_size_factor );
    
    return(result);
}
