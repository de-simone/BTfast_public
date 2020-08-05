#ifndef REGIMES_H
#define REGIMES_H

#include <array>        // std::array
#include <deque>        // std::deque

// ------------------------------------------------------------------------- //
/*! Market Regimes (volatility, directionality)
*/
bool MktRegime ( int reg_num,
                 const std::array<double, 6>& OpenD,
                 const std::array<double, 6>& HighD,
                 const std::array<double, 6>& LowD,
                 const std::array<double, 6>& CloseD,
                 const std::deque<double>& atrD ) ; // ATR on daily data

#endif
