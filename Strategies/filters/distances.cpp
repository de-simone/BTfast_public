#include "distances.h"

#include <algorithm>        // std::max_element, std::min_element
#include <cmath>            // std::abs
#include <numeric>          // std::accumulate

using std::max_element;
using std::min_element;
using std::abs;

// ------------------------------------------------------------------------- //
void DistanceCalculation ( int distance_switch,
                           double& distance,
                           double& fract_long, double& fract_short,
                           const std::array<double, 6>& OpenD,
                           const std::array<double, 6>& HighD,
                           const std::array<double, 6>& LowD,
                           const std::array<double, 6>& CloseD,
                           const std::deque<double>& atr )
{

    switch( distance_switch ){

        case 1:                // (High1-Low1)
            distance = ( HighD[1] - LowD[1] );
            break;
        case 2:                // avg of (H-L) over last 5 sessions
            distance = ( std::accumulate(HighD.begin()+1, HighD.end(), 0.0)
                         - std::accumulate(LowD.begin()+1, LowD.end(), 0.0)
                       ) / ( (double) (HighD.size() - 1) );
            break;
        case 3:                // HighestHigh(5) - LowestLow(5)
            distance = ( *max_element( HighD.begin()+1, HighD.end() )
                         - *min_element( LowD.begin()+1, LowD.end() ) );
            break;
        case 4:                // ATR(20 bars)
            distance = atr.front();
            fract_long  = 5 * fract_long;
            fract_short = 5 * fract_short;
            break;

    }

}
