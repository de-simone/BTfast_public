#ifndef PATTERNS_H
#define PATTERNS_H

#include <array>        // std::array
#include <deque>        // std::deque

// ------------------------------------------------------------------------- //
/*! Base Patterns.
*/
bool Pattern ( int ptn_num,
               std::array<double, 6> OpenD, std::array<double, 6> HighD,
               std::array<double, 6> LowD,  std::array<double, 6> CloseD,
               std::deque<double> atrD ); // ATR on daily data

// ------------------------------------------------------------------------- //
/*! Number of Opposite pattern with respect to input pattern number,
    to symmetrize long/short entries.
*/
int OppositePattern ( int ptn_num );



#endif
