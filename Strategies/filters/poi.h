#ifndef POI_H
#define POI_H

#include <array>        // std::array

// ------------------------------------------------------------------------- //
/*! Point of Inititation for computing price excursions
*/
void PointOfInititation( int poi_num, int BOMR_switch,
                         double &POI_long, double &POI_short,                         
                         const std::array<double, 6>& OpenD,
                         const std::array<double, 6>& HighD,
                         const std::array<double, 6>& LowD,
                         const std::array<double, 6>& CloseD );

#endif
