#include "exits.h"

// ------------------------------------------------------------------------- //
bool ExitCondition( int exit_num, const std::deque<Event>& data1,
                    const std::string& strategy_name,
                    const std::vector<Position>& open_positions,
                    const Time& CurrentTime, const Time& CurrentDOW,
                    const Time& OneBarBeforeClose,
                    int tf_mins, int co_mins, bool new_session )
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

        case 3:{
        // Exit after 5 days
            // Find Position to close
            Position pos_to_close {};
            for( const Position& pos : open_positions ){
                if( pos.strategy_name() == strategy_name ){
                    pos_to_close = pos;
                    break;
                }
            }
            result = ( pos_to_close.days_in_trade() == 5 );
            break;
        }


        case 4:{
        // First profitable open
            // Find Position to close
            Position pos_to_close {};
            for( const Position& pos : open_positions ){
                if( pos.strategy_name() == strategy_name ){
                    pos_to_close = pos;
                    break;
                }
            }
            result = ( new_session && pos_to_close.pl() > 0.0 );
            break;
        }

    }

    return(result);
}
