#include "TA_indicators.h"

#include <iostream>
#include <algorithm>    // std::max, std::min
#include <cmath>        // std::abs, std::pow

#include "utils_math.h"      // utils_math

using std::max;
using std::min;
//using std::max_element;
//using std::min_element;
using std::abs;
using std::pow;

// ------------------------------------------------------------------------- //
// Rate of Change
// Formula: ROC[t] = ( Price[t]/ Price[t-length] - 1 ) * 100
// applied_price: OPEN, HIGH, LOW, CLOSE
//
// Fill 'out' deque with indicator values (size <= 'max_bars_back')

void ROC( std::deque<double> &out, const std::deque<Event>& bar,
          bool make_new_entry, int max_bars_back,
          int length, std::string applied_price )
{

    // Check if input is valid
    if( length <= 0 || max_bars_back <= 0 ){
        std::cout<<">>> ERROR: Invalid argument for ROC indicator\n";
        exit(1);
    }
    // At least 'length' bars in history
    if( bar.size() <= length ){
        return;
    }

    //--- Start Calculation
    double new_value {0.0};

    if( applied_price == "OPEN" ){
        new_value = ( bar.at(0).open()/bar.at(length).open() - 1 ) * 100;
    }
    else if( applied_price == "HIGH" ){
        new_value = ( bar.at(0).high()/bar.at(length).high() - 1 ) * 100;
    }
    else if( applied_price == "LOW" ){
        new_value = ( bar.at(0).low()/bar.at(length).low() - 1 ) * 100;
    }
    else if( applied_price == "CLOSE" ){
        new_value = ( bar.at(0).close()/bar.at(length).close() - 1 ) * 100;
    }
    else{
        std::cout<<">>> ERROR: Invalid argument for ROC indicator\n";
        exit(1);
    }
    //--- End Calculation

    if( make_new_entry ){   // insert new value in front of deque
        if( out.size() >= max_bars_back ){
            out.pop_back();     // delete last value
        }
        out.push_front(new_value);
    }
    else{                   // update front value in deque
        out.front() = new_value;
    }

}


// ------------------------------------------------------------------------- //
// True Range
// = Max(current_high, previous_close) - Min(current_low, previous_close)
//
// Fill 'out' deque with indicator values (size <= 'max_bars_back')

void TrueRange( std::deque<double> &out, const std::deque<Event>& bar,
                bool make_new_entry, int max_bars_back )
{
    // Check if input is valid
    if( max_bars_back <= 0 ){
        std::cout<<">>> ERROR: Invalid argument for TrueRange indicator\n";
        exit(1);
    }

    //--- Start Calculation
    double new_value {0.0};

    if( bar.size() == 0 ){
        return;
    }
    else if( bar.size() == 1 ){ // just (High-Low) on first bar
        new_value = bar.at(0).high() - bar.at(0).low();
    }
    else{
        new_value = max( bar.at(0).high(), bar.at(1).close() )
                    - min( bar.at(0).low(), bar.at(1).close() );
    }
    //--- End Calculation

    if( make_new_entry ){   // insert new value in front of deque
        if( out.size() >= max_bars_back ){
            out.pop_back();     // delete last value
        }
        out.push_front(new_value);
    }
    else{                   // update front value in deque
        out.front() = new_value;
    }
}


// ------------------------------------------------------------------------- //
// Average True Range
// Recursive formula: ATR[t] = alpha * TR[t] + (1-alpha) * ATR[t-1],
// where alpha = 1/length and the intial value is simple avg of TrueRange.
//
// Fill 'out' deque with indicator values (size <= 'max_bars_back')

void ATR( std::deque<double> &out, const std::deque<Event>& bar,
          bool make_new_entry, int max_bars_back,
          int length )
{
    // Check if input is valid
    if( length <= 0 || max_bars_back <= 0 ){
        std::cout<<">>> ERROR: Invalid argument for ATR indicator\n";
        exit(1);
    }
    // At least 'length' bars in history
    if( bar.size() <= length ){
        return;
    }

    //--- Start Calculation
    double new_value {0.0};
    double prev_value {0.0};
    double true_range {0.0};
    double alpha {1.0 / length};    // smoothing factor

    //-- Calculation of previous indicator value
    if( out.size() == 0  ) {
        // first true range is just High-Low
        prev_value = alpha * ( bar.at(length).high() - bar.at(length).low() );
        // simple average of true range
        for(int k = length - 1; k >= 1; k-- ){
            true_range = max( bar.at(k).high(), bar.at(k+1).close() )
                        - min( bar.at(k).low(), bar.at(k+1).close() );
            prev_value = prev_value + alpha * true_range;
        }
    }
    else{
        prev_value = out.front();
    }
    //--

    true_range = max( bar.at(0).high(), bar.at(1).close() )
                - min( bar.at(0).low(), bar.at(1).close() );

    new_value = (1-alpha) * prev_value + alpha * true_range;
    //--- End Calculation


    if( make_new_entry ){   // insert new value in front of deque
        if( out.size() >= max_bars_back ){
            out.pop_back();     // delete last value
        }
        out.push_front(new_value);
    }
    else{                   // update front value in deque
        out.front() = new_value;
    }
}

// ------------------------------------------------------------------------- //
// Average Directional Index (ADX)
//
//          moveUp      = current High - previous High;
//          moveDown    = previous Low - current Low;
//          +DM         = Theta(moveUP-moveDown) * Max(moveUP,0);
//          -DM         = Theta(moveDown-moveUP) * Max(moveDown,0);
//          +DMavg      = SmoothedAvg( +DM, n);
//          -DMavg      = SmoothedAvg( -DM, n);
//            [ +DI         = ( +DMavg / ATR ) * 100; ]
//            [ -DI         = ( -DMavg / ATR ) * 100; ]
//            [ DX          = 100 * |(+DI) - (-DI)|/[(+DI) + (-DI)] ]
//          DX          = 100 * |(+DMavg) - (-DMavg)|/[(+DMavg) + (-DMavg)];
//          ADX         = SmoothedAvg( DX, n);
//
// Recursive formula for smoothed averages:
//
//          (+DMavg[t]) = alpha * (+DM[t]) + (1-alpha) * (+DMavg[t-1]),
//          (-DMavg[t]) = alpha * (-DM[t]) + (1-alpha) * (-DMavg[t-1]),
//
//          ADX[t]   = alpha * DX[t] + (1-alpha) * ADX[t-1],
//
// where alpha = 1/length and the intial values are simple avg of +DM,-DM, DX.
//
// Fill 'out' deque with indicator values (size <= 'max_bars_back')

void ADX( std::deque<double> &out, const std::deque<Event>& bar,
          bool make_new_entry, int max_bars_back, int length )
{
    // Check if input is valid
    if( length <= 0 || max_bars_back <= 0 ){
        std::cout<<">>> ERROR: Invalid argument for ATR indicator\n";
        exit(1);
    }
    // At least 'length' bars in history
    if( bar.size() <= 2*length ){
        return;
    }

    //--- Start Calculation
    double move_up {0.0};
    double move_dn {0.0};
    double pDM {0.0};               // +DM
    double mDM {0.0};               // -DM
    double dx {0.0};                // DX
    static double DMavg_P {0.0};    // +DMavg values (keep previous values)
    static double DMavg_M {0.0};    // -DMavg values (keep previous values)
    double prev_value {0.0};        // ADX values
    double new_value {0.0};         // ADX values
    double alpha {1.0 / length};    // smoothing factor
    int k {0};

    //-- Calculation of previous indicator value
    if( out.size() == 0  ) {

        // First +DMavg/-DMavg are simple averages of +DM/-DM
        for( k = 2 * length - 1; k >= length; k-- ){

            move_up = bar.at(k).high() - bar.at(k+1).high();
            move_dn = bar.at(k+1).low() - bar.at(k).low();
            pDM = utils_math::theta( move_up - move_dn ) * max( move_up, 0.0 );
            mDM = utils_math::theta( move_dn - move_up ) * max( move_dn, 0.0 );
            DMavg_P = DMavg_P + pDM * alpha;
            DMavg_M = DMavg_M + mDM * alpha;
        }
        dx = abs( DMavg_P - DMavg_M )/( DMavg_P + DMavg_M ) * 100;
        prev_value = alpha * dx;


        // Simple avg of DX
        for( k = length - 1; k >= 1; k-- ){

            move_up = bar.at(k).high() - bar.at(k+1).high();
            move_dn = bar.at(k+1).low() - bar.at(k).low();
            pDM = utils_math::theta( move_up - move_dn ) * max( move_up, 0.0 );
            mDM = utils_math::theta( move_dn - move_up ) * max( move_dn, 0.0 );
            DMavg_P = (1-alpha) * DMavg_P + alpha * pDM;
            DMavg_M = (1-alpha) * DMavg_M + alpha * mDM;
            dx = abs( DMavg_P - DMavg_M )/( DMavg_P + DMavg_M ) * 100;
            prev_value  = prev_value + alpha * dx;
        }

    }
    else{
        prev_value = out.front();
    }
    //--

    move_up = bar.at(0).high() - bar.at(1).high();
    move_dn = bar.at(1).low() - bar.at(0).low();
    pDM = utils_math::theta( move_up - move_dn ) * max( move_up, 0.0 );
    mDM = utils_math::theta( move_dn - move_up ) * max( move_dn, 0.0 );
    DMavg_P = (1-alpha) * DMavg_P + alpha * pDM;    // update +DMavg
    DMavg_M = (1-alpha) * DMavg_M + alpha * mDM;    // update -DMavg
    dx = abs( DMavg_P - DMavg_M )/( DMavg_P + DMavg_M ) * 100; // update DX

    new_value = (1-alpha) * prev_value  + alpha * dx;
    //--- End Calculation




    if( make_new_entry ){   // insert new value in front of deque
        if( out.size() >= max_bars_back ){
            out.pop_back();     // delete last value
        }
        out.push_front(new_value);
    }
    else{                   // update front value in deque
        out.front() = new_value;
    }
}



// ------------------------------------------------------------------------- //
// Highest High of last 'length' bars (current bar excluded)
//
// Fill 'out' deque with indicator values (size <= 'max_bars_back')

void HighestHigh( std::deque<double> &out, const std::deque<Event>& bar,
                  bool make_new_entry, int max_bars_back, int length )
{
    // Check if input is valid
    if( length <= 0 || max_bars_back <= 0 ){
        std::cout<<">>> ERROR: Invalid argument for HighestHigh indicator\n";
        exit(1);
    }
    // At least 'length' bars in history
    if( bar.size() <= length ){
        return;
    }

    //--- Start Calculation
    double new_value { bar.at(length).high() };

    for( int k = length; k >= 1 ; k-- ){
        if( bar.at(k).high() > new_value ){
            new_value = bar.at(k).high();
        }
    }
    //--- End Calculation

    if( make_new_entry ){   // insert new value in front of deque
        if( out.size() >= max_bars_back ){
            out.pop_back();     // delete last value
        }
        out.push_front(new_value);
    }
    else{                   // update front value in deque
        out.front() = new_value;
    }
}

// ------------------------------------------------------------------------- //
// Lowest Low of last 'length' bars (current bar excluded)
//
// Fill 'out' deque with indicator values (size <= 'max_bars_back')

void LowestLow( std::deque<double> &out, const std::deque<Event>& bar,
                  bool make_new_entry, int max_bars_back, int length )
{
    // Check if input is valid
    if( length <= 0 || max_bars_back <= 0 ){
        std::cout<<">>> ERROR: Invalid argument for LowestLow indicator\n";
        exit(1);
    }
    // At least 'length' bars in history
    if( bar.size() <= length ){
        return;
    }

    //--- Start Calculation
    double new_value { bar.at(length).low() };

    for( int k = length; k >= 1 ; k-- ){
        if( bar.at(k).low() < new_value ){
            new_value = bar.at(k).low();
        }
    }
    //--- End Calculation

    if( make_new_entry ){   // insert new value in front of deque
        if( out.size() >= max_bars_back ){
            out.pop_back();     // delete last value
        }
        out.push_front(new_value);
    }
    else{                   // update front value in deque
        out.front() = new_value;
    }
}
