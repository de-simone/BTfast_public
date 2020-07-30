#include "poi.h"

#include <algorithm>        // std::max_element, std::min_element
#include <cmath>            // std::abs
#include <numeric>          // std::accumulate

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

        case 1:         // Open of today
            POI_long  = OpenD[0];
            POI_short = POI_long;
            break;

        case 2:         // Close of yesterday
            POI_long  = CloseD[1];
            POI_short = POI_long;
            break;

        case 3:         // High/Low of yesterday
            if( BOMR_switch == 1 ){
                POI_long  = HighD[1];
                POI_short = LowD[1];
            }
            else if( BOMR_switch == 2 ){
                POI_long  = LowD[1];
                POI_short = HighD[1];
            }
            break;

        case 4:        // median price of yesterday
            POI_long  = 0.5*(HighD[1] + LowD[1]);
            POI_short = POI_long;
            break;

        case 5:        // Average close of last sessions
            POI_long  = ( std::accumulate(CloseD.begin()+1, CloseD.end(), 0.0)
                            / CloseD.size() );
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

        case 7:        // median price of last 5 sessions
            POI_long  = 0.5*( *max_element( HighD.begin()+1, HighD.end() )
                              + *min_element( LowD.begin()+1, LowD.end() )  );
            POI_short = POI_long;
            break;

        case 8:       // highest high/lowest low of last 5 sessions
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
