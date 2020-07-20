#ifndef EXITS_H
#define EXITS_H

#include "datetime.h"
#include "events.h"
#include "position.h"

#include <deque>    // std::deque
#include <vector>   // std::vector

// ------------------------------------------------------------------------- //
/*! Conditions for exit, controlled by switch 'exit_num'

    tf_mins_: minutes of timeframe (for intraday)
    co_mins_: minutes between close of a session and open of next one
    new_session: true at the start of a new session (day), otherwise false
*/

bool ExitCondition( int exit_num, const std::deque<Event>& data1,
                   const std::string& strategy_name,
                   const std::vector<Position>& open_positions,
                   const Time& CurrentTime, const Time& CurrentDOW,
                   const Time& OneBarBeforeClose,
                   int exit_days, int exit_bars,
                   int tf_mins, int co_mins, bool new_session );


#endif
