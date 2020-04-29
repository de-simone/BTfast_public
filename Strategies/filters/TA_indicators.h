#ifndef TA_INDICATORS_H
#define TA_INDICATORS_H

#include "events.h"

#include <deque>        // std::deque
#include <string>

// ------------------------------------------------------------------------- //
/*! Rate Of Change
*/
void ROC( std::deque<double> &out, const std::deque<Event>& bar,
          bool make_new_entry, int max_bars_back,
          int length, std::string applied_price );

// ------------------------------------------------------------------------- //
/*! True Range
*/
void TrueRange( std::deque<double> &out, const std::deque<Event>& bar,
                bool make_new_entry, int max_bars_back );

// ------------------------------------------------------------------------- //
/*! Average True Range
*/
void ATR( std::deque<double> &out, const std::deque<Event>& bar,
          bool make_new_entry, int max_bars_back, int length );

// ------------------------------------------------------------------------- //
/*! Average Directional Index 
*/
void ADX( std::deque<double> &out, const std::deque<Event>& bar,
          bool make_new_entry, int max_bars_back, int length );

// ------------------------------------------------------------------------- //
/*! Highest High of last 'length' bars
*/
void HighestHigh( std::deque<double> &out, const std::deque<Event>& bar,
                  bool make_new_entry, int max_bars_back, int length );


// ------------------------------------------------------------------------- //
/*! Lowest Low of last 'length' bars
*/
void LowestLow( std::deque<double> &out, const std::deque<Event>& bar,
                bool make_new_entry, int max_bars_back, int length );

#endif
