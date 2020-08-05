#include "transaction.h"

#include <cstring>  // strlen

// ------------------------------------------------------------------------- //
/*! Constructor
*/
Transaction::Transaction(std::string strategy_name, Instrument symbol,
                        std::string side, int quantity,
                        DateTime entry_time, double entry_price,
                        DateTime exit_time,  double exit_price,
                        double mae, double mfe, int bars_in_trade,
                        double net_pl, double cumul_pl )

: ticket_{0}, strategy_name_{strategy_name}, symbol_{symbol},
  side_{side}, quantity_{quantity},
  entry_time_{entry_time}, exit_time_{exit_time},
  entry_price_{entry_price}, exit_price_{exit_price},
  mae_{mae}, mfe_{mfe}, bars_in_trade_{bars_in_trade},
  net_pl_{net_pl}, cumul_pl_{cumul_pl}
{}



//-------------------------------------------------------------------------- //
/*! String representation
*/
std::string Transaction::tostring() const {

    char buffer[300];
    int digits = symbol_.digits();
    std::string float_format = "%8." + std::to_string(digits) + "f ";
    std::string tr_format1 = "%6.5s %3d %18.17s " + float_format +"%10.2f\n";
    std::string tr_format2 = "%18s %18.17s " + float_format +"%19.2f";

    sprintf(buffer, tr_format1.c_str(),
            side_.c_str(), quantity_, entry_time_.tostring().c_str(),
            entry_price_, net_pl_);

    sprintf(buffer + strlen(buffer), tr_format2.c_str(),
            "",exit_time_.tostring().c_str(), exit_price_, cumul_pl_ );

    return(buffer);
}
