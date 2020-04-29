#ifndef POI_H
#define POI_H

#include <array>        // std::array

// ------------------------------------------------------------------------- //
/*! Point of Inititation for computing price excursions
*/
void PointOfInititation ( double &POI_long, double &POI_short,
                          int poi_num, int BOMR_switch,
                    std::array<double, 6> OpenD, std::array<double, 6> HighD,
                    std::array<double, 6> LowD,  std::array<double, 6> CloseD );

#endif
