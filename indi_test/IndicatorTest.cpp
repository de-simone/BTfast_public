/*
   Test of indicators (check results with IndicatorTest.xlsx)

   Compile as:

        gcc -o IndicatorTest.o
        gcc -std=c++17 -I IndicatorTest.cpp events.cpp instruments.cpp datetime.cpp
            -o IndicatorTest.o

   Execute as: ./IndicatorTest.o
*/

#include <deque>
#include <iostream>
#include <string>
#include <vector>

#include "events.h"
#include "TA_indicators.h"


//-------------------------------------------------------------------------

int main()
{

    int dim = 30;
    double arrayC[dim], arrayO[dim], arrayH[dim], arrayL[dim];

    arrayC[0] = 29.872; // earliest
    arrayC[1] = 30.2381;
    arrayC[2] = 30.0996;
    arrayC[3] = 28.9028;
    arrayC[4] = 28.9225;
    arrayC[5] = 28.4775;
    arrayC[6] = 28.5566;
    arrayC[7] = 27.5576;
    arrayC[8] = 28.4675;
    arrayC[9] = 28.2796;
    arrayC[10] = 27.4882;
    arrayC[11] = 27.231;
    arrayC[12] = 26.3507;
    arrayC[13] = 26.3309;
    arrayC[14] = 27.0333;
    arrayC[15] = 26.2221;
    arrayC[16] = 26.0144;
    arrayC[17] = 25.4605;
    arrayC[18] = 27.0333;
    arrayC[19] = 27.4487;
    arrayC[20] = 28.3586;
    arrayC[21] = 28.4278;
    arrayC[22] = 27.9530;
    arrayC[23] = 29.0116;
    arrayC[24] = 29.3776;
    arrayC[25] = 29.3576;
    arrayC[26] = 28.9107;
    arrayC[27] = 30.6149;
    arrayC[28] = 30.0502;
    arrayC[29] = 30.1890; // latest (bar index = 0)

    arrayO[0] = 29.41;  // earliest
    arrayO[1] = 29.87;
    arrayO[2] = 30.23;
    arrayO[3] = 30.09;
    arrayO[4] = 28.90;
    arrayO[5] = 28.92;
    arrayO[6] = 28.47;
    arrayO[7] = 28.55;
    arrayO[8] = 27.55;
    arrayO[9] = 28.46;
    arrayO[10] = 28.27;
    arrayO[11] = 27.48;
    arrayO[12] = 27.23;
    arrayO[13] = 26.35;
    arrayO[14] = 26.33;
    arrayO[15] = 27.03;
    arrayO[16] = 26.22;
    arrayO[17] = 26.01;
    arrayO[18] = 25.46;
    arrayO[19] = 27.03;
    arrayO[20] = 27.44;
    arrayO[21] = 28.35;
    arrayO[22] = 28.42;
    arrayO[23] = 27.95;
    arrayO[24] = 29.01;
    arrayO[25] = 29.37;
    arrayO[26] = 29.35;
    arrayO[27] = 28.91;
    arrayO[28] = 30.61;
    arrayO[29] = 30.05;   // latest (bar index = 0)

    arrayH[0] = 30.1983; // earliest
    arrayH[1] = 30.2776;
    arrayH[2] = 30.4458;
    arrayH[3] = 29.3478;
    arrayH[4] = 29.3477;
    arrayH[5] = 29.2886;
    arrayH[6] = 28.8334;
    arrayH[7] = 28.7346;
    arrayH[8] = 28.6654;
    arrayH[9] = 28.8532;
    arrayH[10] = 28.6356;
    arrayH[11] = 27.6761;
    arrayH[12] = 27.2112;
    arrayH[13] = 26.8651;
    arrayH[14] = 27.409;
    arrayH[15] = 26.9441;
    arrayH[16] = 26.5189;
    arrayH[17] = 26.5189;
    arrayH[18] = 27.0927;
    arrayH[19] = 27.686;
    arrayH[20] = 28.4477;
    arrayH[21] = 28.5267;
    arrayH[22] = 28.6654;
    arrayH[23] = 29.0116;
    arrayH[24] = 29.8720;
    arrayH[25] = 29.8028;
    arrayH[26] = 29.7529;
    arrayH[27] = 30.6546;
    arrayH[28] = 30.5951;
    arrayH[29] = 30.7635; // latest (bar index = 0)

    arrayL[0] = 29.4072; // earliest
    arrayL[1] = 29.3182;
    arrayL[2] = 29.9611;
    arrayL[3] = 28.7443;
    arrayL[4] = 28.5566;
    arrayL[5] = 28.4081;
    arrayL[6] = 28.0818;
    arrayL[7] = 27.4289;
    arrayL[8] = 27.6565;
    arrayL[9] = 27.8345;
    arrayL[10] = 27.3992;
    arrayL[11] = 27.0927;
    arrayL[12] = 26.1826;
    arrayL[13] = 26.1332;
    arrayL[14] = 26.6277;
    arrayL[15] = 26.1332;
    arrayL[16] = 25.4307;
    arrayL[17] = 25.3518;
    arrayL[18] = 25.876;
    arrayL[19] = 26.964;
    arrayL[20] = 27.1421;
    arrayL[21] = 28.0123;
    arrayL[22] = 27.8840;
    arrayL[23] = 27.9928;
    arrayL[24] = 28.7643;
    arrayL[25] = 29.1402;
    arrayL[26] = 28.7127;
    arrayL[27] = 28.9290;
    arrayL[28] = 30.0304;
    arrayL[29] = 29.3863; // latest (bar index = 0)

    //--

    /////////////////////////////////////////////

    Instrument symbol {"NG"};
    std::deque<Event> bar{};
    std::deque<double> indicator {};
    bool period_change {true};
    int max_bars_back {100};
    //int length {0};

    for(int i = 0; i < dim; i++){
        // Append bars filled with array (O,H,L,C) to queue
        Event new_bar{ symbol, DateTime {}, "M15",
                       arrayO[i], arrayH[i], arrayL[i], arrayC[i],0};
        bar.push_front( new_bar );


        //--- Indicator test
        //ROC( indicator, bar, period_change, max_bars_back, 5, "CLOSE" );
        //TrueRange( indicator, bar, period_change, max_bars_back );
        //ATR( indicator, bar, period_change, max_bars_back, 14 );
        ADX( indicator, bar, period_change, max_bars_back, 14 );
        //HighestHigh( indicator, bar, period_change, max_bars_back, 5 );
        //LowestLow( indicator, bar, period_change, max_bars_back, 5 );
        //---





        std::cout<<i<<": "<<new_bar.open()<<", "<<new_bar.high()<<", "
                          <<new_bar.low()<<", "<< new_bar.close();
        if(!indicator.empty()){
             std::cout<< " --> "<< indicator.front()<<"\n";
        }
        else{
            std::cout<<"\n";
        }
    }
    //--







    std::cout<<"\n";
    return(0);
}
