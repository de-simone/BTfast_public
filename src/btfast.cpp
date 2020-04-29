#include "btfast.h"

#include "utils_time.h"     // str2date

#include <iostream>         // std::cout



// ------------------------------------------------------------------------- //
/*! Constructor
*/
BTfast::BTfast( const std::string &strategy_name,
                const Instrument &symbol,
                const std::string &timeframe,
                int max_bars_back,
                double initial_balance,
                const std::string &ps_type,
                int num_contracts, double risk_fraction,
                bool print_progress,
                bool include_commissions, int slippage )
: strategy_name_ {strategy_name},
  symbol_ {symbol},
  timeframe_ {timeframe},
  max_bars_back_ {max_bars_back},
  initial_balance_ {initial_balance},
  ps_type_ {ps_type},
  num_contracts_ {num_contracts},
  risk_fraction_ {risk_fraction},
  print_progress_ {print_progress},
  include_commissions_ {include_commissions},
  slippage_ {slippage}
{
    /*
    // Actual start/end dates from input strings
    if( input_start_date != "0" ){
        start_date_ = utils_time::str2date(input_start_date);
    }
    else{
        start_date_ = Date {1900,1,1};  // far in the past
    }
    if( input_end_date != "0" ){
        end_date_ = utils_time::str2date(input_end_date);
    }
    else{
        end_date_ = Date {2100,1,1};    // far in the future
    }
    // Check order of dates
    if( start_date_ > end_date_ ){
        std::cout << ">>> ERROR: start date after end date.\n";
        exit(1);
    }
    */
}
