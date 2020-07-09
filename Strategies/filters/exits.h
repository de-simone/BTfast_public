#ifndef EXITS_H
#define EXITS_H

#include "datetime.h"
#include "events.h"

#include <deque>    // std::deque

// ------------------------------------------------------------------------- //
/*! Conditions for exit, controlled by switch 'exit_num'

    tf_mins_: minutes of timeframe (for intraday)
    co_mins_: minutes between close of a session and open of next one
*/

bool ExitCondition( int exit_num, const std::deque<Event>& data1,
                   const Time& CurrentTime,
                   const Time& CurrentDOW, const Time& OneBarBeforeClose,
                   int tf_mins, int co_mins );


#endif
