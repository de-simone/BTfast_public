#include "exits.h"

// ------------------------------------------------------------------------- //
bool ExitCondition( int exit_num, const std::deque<Event>& data1,
                   const Time& CurrentTime,
                   const Time& CurrentDOW, const Time& OneBarBeforeClose,
                   int tf_mins, int co_mins )
{

    bool result {false};

    switch( exit_num ){
        
        case 1:
        // One bar before close of session at end of DAY,
        // or at open of next session if session ends earlier than usual
            result = ( CurrentTime == OneBarBeforeClose
                      || ( (data1[0].timestamp().time()
                           - data1[1].timestamp().time()).tot_minutes() >
                         co_mins + tf_mins )
                     );
            break;

        case 2:
        // One bar before close of session at end of WEEK,
        // or at open of next session if session ends earlier than usual
            result = ( CurrentDOW == 5 &&
                      ( CurrentTime == OneBarBeforeClose
                        || ( (data1[0].timestamp().time()
                             - data1[1].timestamp().time()).tot_minutes() >
                            co_mins + tf_mins ) )
                     );
            break;

    }

    return(result);
}
