#ifndef TIME_FILTERS_H
#define TIME_FILTERS_H

#include "datetime.h"
#include "instruments.h"

// ------------------------------------------------------------------------- //
/*! Conditions for time filters
*/
bool TimeFilter_DOW( int filter_num, int CurrentDOW );

bool TimeFilter_Intraday( int filter_num, const Time &CurrentTime,
                          const Instrument &symbol, int T_segment_duration );




#endif
