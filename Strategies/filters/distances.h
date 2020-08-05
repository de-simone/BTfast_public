#ifndef DISTANCES_H
#define DISTANCES_H

#include <array>        // std::array
#include <deque>        // std::deque

// ------------------------------------------------------------------------- //
/*! Point of Inititation for computing price excursions
*/
void DistanceCalculation ( int distance_switch,
                           double& distance,
                           double& fract_long, double& fract_short,
                           const std::array<double, 6>& OpenD,
                           const std::array<double, 6>& HighD,
                           const std::array<double, 6>& LowD,
                           const std::array<double, 6>& CloseD,
                           const std::deque<double>& atr );


#endif
