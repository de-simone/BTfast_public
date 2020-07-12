#include "regimes.h"

#include <algorithm>    // std::max_element, std::min_element
//#include <cmath>        // std::abs
//#include <iostream>

using std::max_element;
using std::min_element;
//using std::abs;

// ------------------------------------------------------------------------- //
bool MktRegime ( int reg_num,
                 std::array<double, 6> OpenD, std::array<double, 6> HighD,
                 std::array<double, 6> LowD,  std::array<double, 6> CloseD,
                 std::deque<double> atrD )
{

    bool result {false};

    // -----------------------    VOLATILITY BINS    ----------------------- //
    int vola_bin {0};

    // Require at least 100 days of ATR history (in strategy::preliminaries())
    if( !atrD.empty() ){
        // Max and Min of atrD over its full history (max_bars_back)
        double vola_max { *max_element(atrD.begin()+1, atrD.end()) };
        double vola_min { *min_element(atrD.begin()+1, atrD.end()) };
        // Split volatility range into 4 bins
        double vola_bin_size { (vola_max-vola_min) / 4.0 };

        if( atrD.front() <= vola_min + vola_bin_size ){
                vola_bin = 1;
        }
        else if( atrD.front() > vola_min + vola_bin_size
                && atrD.front() <= vola_min + 2*vola_bin_size ){
                vola_bin = 2;
        }
        else if( atrD.front() > vola_min + 2*vola_bin_size
                 && atrD.front() <= vola_min + 3*vola_bin_size ){
                vola_bin = 3;
        }
        else if( atrD.front() > vola_min + 3*vola_bin_size ){
                vola_bin = 4;
        }
    }
    // --------------------------------------------------------------------- //

    switch( reg_num ){

        case 1:    // Volatility bin 1
            result = vola_bin == 1;
            break;
        case 2:    // Not Volatility bin 1
            result = vola_bin != 1;
            break;
        case 3:    // Volatility bin 2
            result = vola_bin == 2;
            break;
        case 4:    // Not Volatility bin 2
            result = vola_bin != 2;
            break;
        case 5:    // Volatility bin 3
            result = vola_bin == 3;
            break;
        case 6:    // Not Volatility bin 3
            result = vola_bin != 3;
            break;
        case 7:    // Volatility bin 4
            result = vola_bin == 4;
            break;
        case 8:    // Not Volatility bin 4
            result = vola_bin != 4;
            break;
    }

    return(result);
}
