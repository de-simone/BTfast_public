#include "poi.h"

#include <algorithm>    // std::max_element, std::min_element
#include <cmath>        // std::abs

using std::max_element;
using std::min_element;
using std::abs;

// ------------------------------------------------------------------------- //
void PointOfInititation( double &POI_long, double &POI_short,
                         int poi_num, int BOMR_switch,
                    std::array<double, 6> OpenD, std::array<double, 6> HighD,
                    std::array<double, 6> LowD,  std::array<double, 6> CloseD )
{
    switch( poi_num ){

        case 1:
            POI_long  = OpenD[0];
            POI_short = POI_long;
            break;
        case 2:
            POI_long  = CloseD[1];
            POI_short = POI_long;
            break;
        case 3:
            if( BOMR_switch == 1 ){
                POI_long  = HighD[1];
                POI_short = LowD[1];
            }
            else if( BOMR_switch == 2 ){
                POI_long  = LowD[1];
                POI_short = HighD[1];
            }
            break;
        case 4:        // 50% retracement
            POI_long  = 0.5*(HighD[1] + LowD[1]);
            POI_short = POI_long;
            break;
        case 5:        // average price of yesterday
            POI_long  = (CloseD[1] + HighD[1] + LowD[1])/3;
            POI_short = POI_long;
            break;
        case 6:        // max/min close of last 5 sessions
            if( BOMR_switch == 1 ){
                POI_long  = *max_element( CloseD.begin()+1, CloseD.end() );
                POI_short = *min_element( CloseD.begin()+1, CloseD.end() );
            }
            else if( BOMR_switch == 2 ){
                POI_long  = *min_element( CloseD.begin()+1, CloseD.end() );
                POI_short = *max_element( CloseD.begin()+1, CloseD.end() );
            }
            break;
        case 7:       // highest high/lowest low of last 5 sessions
            if( BOMR_switch == 1 ){
                POI_long  = *max_element( HighD.begin()+1, HighD.end() );
                POI_short = *min_element( LowD.begin()+1, LowD.end() );
            }
            else if( BOMR_switch == 2 ){
                POI_long  = *min_element( LowD.begin()+1, LowD.end() );
                POI_short = *max_element( HighD.begin()+1, HighD.end() );
            }
            break;

        // case 9:  0.5*(avg(high,5) + avg(low,5) )
        // case 10:  avg(close,5)
    }

}
