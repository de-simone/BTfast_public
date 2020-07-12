#ifndef REGIMES_H
#define REGIMES_H

#include <array>        // std::array
#include <deque>        // std::deque

// ------------------------------------------------------------------------- //
/*! Market Regimes (volatility, directionality)
*/
bool MktRegime ( int reg_num,
                 std::array<double, 6> OpenD, std::array<double, 6> HighD,
                 std::array<double, 6> LowD,  std::array<double, 6> CloseD,
                 std::deque<double> atrD ); // ATR on daily data

#endif
