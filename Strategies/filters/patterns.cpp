#include "patterns.h"

#include <algorithm>    // std::max_element, std::min_element
#include <cmath>        // std::abs
//#include <iostream>

using std::max_element;
using std::min_element;
using std::abs;

// ------------------------------------------------------------------------- //
bool Pattern ( int ptn_num,
               std::array<double, 6> OpenD, std::array<double, 6> HighD,
               std::array<double, 6> LowD,  std::array<double, 6> CloseD,
               std::deque<double> atrD )
{

    bool result {false};

    switch( ptn_num ){
        case 0:
            result = true;
            break;

        // ----------------------------------------------------------------- //
        // -------------------   UNGER's BASIC PATTERNS   ------------------ //
        // ----------------------------------------------------------------- //
        case 1:
            result =  abs(OpenD[1]-CloseD[1]) < 0.5 * (HighD[1]-LowD[1]);
            break;
        case 2:
            result = abs(OpenD[1]-CloseD[5]) < 0.5 * (HighD[5]-CloseD[1]);
            break;
        case 3:
            result = ( abs(OpenD[5]-CloseD[1]) < 0.5 * (
                                *max_element(HighD.begin()+1, HighD.end())
                                - *min_element(LowD.begin()+1, LowD.end()) ) );
            break;
        case 4:
            result = (HighD[0]-OpenD[0]) > ((HighD[1]-OpenD[1]) * 1);
            break;
        case 5:
            result = (HighD[0]-OpenD[0]) > ((HighD[1]-OpenD[1]) * 1.5);
            break;
        case 6:
            result = (OpenD[0]-LowD[0])> ((OpenD[1]-LowD[1]) * 1);
            break;
        case 7:
            result = (OpenD[0]-LowD[0])> ((OpenD[1]-LowD[1]) * 1.5);
            break;
        case 8:         // 3 up-closes
            result = (CloseD[1]>CloseD[2] && CloseD[2]>CloseD[3]
                        && CloseD[3]>CloseD[4]);
            break;
        case 9:         // 3 down-closes
            result = (CloseD[1]<CloseD[2] && CloseD[2]<CloseD[3]
                        && CloseD[3]<CloseD[4]);
            break;
        case 10:
            result = (HighD[1]>HighD[2] && LowD[1]>LowD[2]);
            break;
        case 11:
            result = (HighD[1]<HighD[2] && LowD[1]<LowD[2]);
            break;
        case 12:
            result = ( HighD[0] > (LowD[0] + LowD[0]*0.75/100) );
            break;
        case 13:
            result = ( HighD[0] < (LowD[0] + LowD[0]*0.75/100) );
            break;
        case 14:
            result = CloseD[1] > CloseD[2];
            break;
        case 15:
            result = CloseD[1] < CloseD[2];
            break;
        case 16:
            result = CloseD[1] < OpenD[1];
            break;
        case 17:
            result = CloseD[1] > OpenD[1];
            break;
        case 18:
            result = CloseD[1] < (CloseD[2] - CloseD[2]*0.5/100);
            break;
        case 19:
            result = CloseD[1] > (CloseD[2] + CloseD[2]*0.5/100);
            break;
        case 20:
            result = HighD[0] > HighD[1];
            break;
        case 21:
            result = HighD[1] > HighD[5];
            break;
        case 22:
            result = LowD[0] < LowD[1];
            break;
        case 23:
            result = LowD[1] < LowD[5];
            break;
        case 24:
            result = (HighD[1]>HighD[2] && HighD[1]>HighD[3]
                        && HighD[1]>HighD[4]);
            break;
        case 25:
            result = (HighD[1]<HighD[2] && HighD[1]<HighD[3]
                        && HighD[1]<HighD[4]);
            break;
        case 26:
            result = (LowD[1]<LowD[2] && LowD[1]<LowD[3] && LowD[1]<LowD[4]);
            break;
        case 27:
            result = (LowD[1]>LowD[2] && LowD[1]>LowD[3] && LowD[1]>LowD[4]);
            break;
        case 28:
            result = (CloseD[1]>CloseD[2] && CloseD[2]>CloseD[3]
                        && OpenD[0]>CloseD[1]);
            break;
        case 29:
            result = (CloseD[1]<CloseD[2] && CloseD[2]<CloseD[3]
                        && OpenD[0]<CloseD[1]);
            break;
        case 30:        // yesterday's Internal Bar Strength (close near the high)
            result = (CloseD[1]-LowD[1]) > 0.80 * (HighD[1]-LowD[1]);
            break;
        case 31:        // yesterday's Internal Bar Strength (close near the low)
            result = (CloseD[1]-LowD[1]) < 0.20 * (HighD[1]-LowD[1]);
            break;
        case 32:
            result = (OpenD[0]<LowD[1]) || (OpenD[0]>HighD[1]);
            break;
        case 33:
            result = OpenD[0] < (CloseD[1] - CloseD[1]*0.5/100);
            break;
        case 34:
            result = OpenD[0] > (CloseD[1] + CloseD[1]*0.5/100);
            break;
        case 35:
            result = (HighD[0]<HighD[1] && LowD[0]>LowD[1]);
            break;
        case 36:
            result = (HighD[1]-LowD[1]) < ( (HighD[2]-LowD[2])
                                            + (HighD[3]-LowD[3]) )/3;
            break;
        case 37:
            result = ( (HighD[1]-LowD[1]) < (HighD[2]-LowD[2])
                    && (HighD[2]-LowD[2]) < (HighD[3]-LowD[3]) );
            break;
        case 38:
            result = ( HighD[2]>HighD[1] && LowD[2]<LowD[1] );
            break;
        case 39:
            result = ( HighD[2]>HighD[1] || LowD[2]<LowD[1] );
            break;
        case 40:
            result = ( HighD[2]<HighD[1] || LowD[2]>LowD[1] );
            break;
        case 41:
            result = ( CloseD[1]>CloseD[2] && CloseD[2]>CloseD[3] );
            break;
        case 42:
            result = ( CloseD[1]<CloseD[2] && CloseD[2]<CloseD[3] );
            break;


        // ----------------------------------------------------------------- //
        // -----------------------   MY PATTERNS   ------------------------- //
        // ----------------------------------------------------------------- //
        case 43:        // 3 days up
            result = ( CloseD[1]>OpenD[1]  && CloseD[2]>OpenD[2]
                        && CloseD[3]>OpenD[3] );
            break;
        case 44:        // 3 days down
            result = ( CloseD[1]<OpenD[1]  && CloseD[2]<OpenD[2]
                        && CloseD[3]<OpenD[3] );
            break;

        case 45:        // 3 days up, ascending
            result = ( CloseD[1]>OpenD[1]  && CloseD[2]>OpenD[2]
                        && CloseD[3]>OpenD[3]
                        && CloseD[1]>CloseD[2] && CloseD[2]>CloseD[3] );
            break;
        case 46:        // 3 days down, descending
            result = ( CloseD[1]<OpenD[1]  && CloseD[2]<OpenD[2]
                        && CloseD[3]<OpenD[3]
                        && CloseD[1]<CloseD[2] && CloseD[2]<CloseD[3] );
            break;

        case 47:       // 1 bar retracing up
            result = ( CloseD[1]>OpenD[1] && CloseD[2]<OpenD[2]
                        && OpenD[1]<=CloseD[2] && CloseD[1]<OpenD[2]
                        && CloseD[1]>CloseD[2] );
            break;
        case 48:        // 1 bar retracing down
            result = ( CloseD[1]<OpenD[1] && CloseD[2]>OpenD[2]
                        && OpenD[1]>=CloseD[2] && CloseD[1]>OpenD[2]
                        && CloseD[1]<CloseD[2] );
            break;

        case 49:        // 2 bars retracing up
            result = ( CloseD[1]>OpenD[1] && CloseD[2]>OpenD[2]
                        && CloseD[3]<OpenD[3]
                        && OpenD[1]<=CloseD[2] && OpenD[2]<=CloseD[3]
                        && CloseD[1]<OpenD[3]  && CloseD[1]>CloseD[3]
                        && CloseD[2]<OpenD[3]  && CloseD[2]>CloseD[3] );
            break;
        case 50:        // 2 bars retracing down
            result = ( CloseD[1]<OpenD[1] && CloseD[2]<OpenD[2]
                        && CloseD[3]>OpenD[3]
                        && OpenD[1]>=CloseD[2] && OpenD[2]>=CloseD[3]
                        && CloseD[1]>OpenD[3]  && CloseD[1]<CloseD[3]
                        && CloseD[2]>OpenD[3]  && CloseD[2]<CloseD[3] );
            break;
        case 51:        // Retrace towards broken-out yesterday's high
            result = ( OpenD[0]<HighD[1]
                        && HighD[0]>HighD[1] && CloseD[0]<HighD[1] );
            break;
        case 52:        // Retrace towards broken-out yesterday's low
            result = ( OpenD[0]>LowD[1]
                        && LowD[0]<LowD[1] && CloseD[0]>LowD[1] );
            break;
        case 53:        // yesterday's close near 5-day low
            result = ( ( CloseD[1]-*min_element(LowD.begin()+1, LowD.end()) )
                       < 0.20 * ( *max_element(HighD.begin()+1, HighD.end())
                                  - *min_element(LowD.begin()+1, LowD.end()) ) );
            break;
        case 54:        // yesterday's close near 5-day high
            result = ( ( CloseD[1]-*min_element(LowD.begin()+1, LowD.end()) )
                       > 0.80 * ( *max_element(HighD.begin()+1, HighD.end())
                                  - *min_element(LowD.begin()+1, LowD.end()) ) );
            break;
        case 55:        // Volatility-Based Momentum (5) > 1.0
            result = ( abs( CloseD[1]-CloseD[5] )/atrD[1] > 1.0 )
            break;
        case 56:        // Volatility-Based Momentum (5) > 2.0
            result = ( abs( CloseD[1]-CloseD[5] )/atrD[1] > 2.0 )
            break;

    }

    return(result);
}


















/*
switch( Filter1L_switch_ ){
    case 0:
        Filter1_long  = true;
        break;
    case 1:
        Filter1_long  = data1[0].close() > CloseD_[1];
        break;
    case 2:
        Filter1_long  = data1[0].close() < CloseD_[1];
        break;
    case 3:
        Filter1_long  = data1[0].close() > OpenD_[1];
        break;
    case 4:
        Filter1_long  = data1[0].close() < OpenD_[1];
        break;
    case 5:
        Filter1_long  = data1[0].close() > OpenD_[0];
        break;
    case 6:
        Filter1_long  = data1[0].close() < OpenD_[0];
        break;
    case 7:             // High of day
        Filter1_long  = data1[0].high() == HighD_[0];
        break;
    case 8:             // Not High of day
        Filter1_long  = data1[0].high() != HighD_[0];
        break;
    case 9:             // Increasing volumes
        Filter1_long  = (  data1[0].volume() > data1[1].volume()
                        && data1[1].volume() > data1[2].volume() );
        break;
    case 10:            // Decreasing volumes
        Filter1_long  = (  data1[0].volume() < data1[1].volume()
                        && data1[1].volume() < data1[2].volume() );
        break;
}

switch( Filter1S_switch_ ){
    case 0:
        Filter1_short = true;
        break;
    case 1:
        Filter1_short = data1[0].close() > CloseD_[1];
        break;
    case 2:
        Filter1_short = data1[0].close() < CloseD_[1];
        break;
    case 3:
        Filter1_short = data1[0].close() > OpenD_[1];
        break;
    case 4:
        Filter1_short = data1[0].close() < OpenD_[1];
        break;
    case 5:
        Filter1_short = data1[0].close() > OpenD_[0];
        break;
    case 6:
        Filter1_short = data1[0].close() < OpenD_[0];
        break;
    case 7:             // Low of day
        Filter1_short = data1[0].low() == LowD_[0];
        break;
    case 8:             // Not Low of day
        Filter1_short = data1[0].low() != LowD_[0];
        break;
    case 9:             // Increasing volumes
        Filter1_short = (  data1[0].volume() > data1[1].volume()
                        && data1[1].volume() > data1[2].volume() );
        break;
    case 10:            // Decreasing volumes
        Filter1_short = (  data1[0].volume() < data1[1].volume()
                        && data1[1].volume() < data1[2].volume() );
        break;
}
*/

/*
// ------------------------------------------------------------------------- //
// Number of Opposite pattern with respect to input pattern number
int OppositePattern ( int ptn_num )
{
    int opposite {0};
    switch( ptn_num ){
        case 0:
            opposite = 0;
            break;
        case 1:
            opposite = 1;
            break;
        case 2:
            opposite = 2;
            break;
        case 3:
            opposite = 3;
            break;
        case 4:
            opposite = 6;
            break;
        case 5:
            opposite = 7;
            break;
        case 6:
            opposite = 4;
            break;
        case 7:
            opposite = 5;
            break;
        case 8:
            opposite = 9;
            break;
        case 9:
            opposite = 8;
            break;
        case 10:
            opposite = 11;
            break;
        case 11:
            opposite = 10;
            break;
        case 12:
            opposite = 13;
            break;
        case 13:
            opposite = 12;
            break;
        case 14:
            opposite = 15;
            break;
        case 15:
            opposite = 14;
            break;
        case 16:
            opposite = 17;
            break;
        case 17:
            opposite = 16;
            break;
        case 18:
            opposite = 19;
            break;
        case 19:
            opposite = 18;
            break;
        case 20:
            opposite = 22;
            break;
        case 21:
            opposite = 23;
            break;
        case 22:
            opposite = 20;
            break;
        case 23:
            opposite = 21;
            break;
        case 24:
            opposite = 26;
            break;
        case 25:
            opposite = 27;
            break;
        case 26:
            opposite = 24;
            break;
        case 27:
            opposite = 25;
            break;
        case 28:
            opposite = 29;
            break;
        case 29:
            opposite = 28;
            break;
        case 30:
            opposite = 31;
            break;
        case 31:
            opposite = 30;
            break;
        case 32:
            opposite = 32;
            break;
        case 33:
            opposite = 34;
            break;
        case 34:
            opposite = 33;
            break;
        case 35:
            opposite = 35;
            break;
        case 36:
            opposite = 36;
            break;
        case 37:
            opposite = 37;
            break;
        case 38:
            opposite = 38;
            break;
        case 39:
            opposite = 39;
            break;
        case 40:
            opposite = 40;
            break;
    }

    return(opposite);
}
*/
