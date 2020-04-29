#ifndef TIME_FILTER_H
#define TIME_FILTER_H

#include "datetime.h"
#include "instruments.h"

// ------------------------------------------------------------------------- //
/*! Conditions for time filter
*/
bool TimeFilter( int filter_num,
                 const Date &CurrentDate, const Time &CurrentTime,
                 int CurrentDOW,
                 const Instrument &symbol, int T_segment_duration );





#endif
