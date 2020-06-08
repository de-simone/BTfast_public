#include "time_filters.h"

#include "utils_time.h"                 // CalcTime

// ------------------------------------------------------------------------- //
bool TimeFilter_DOW( int filter_num, int CurrentDOW )
{
    bool result {false};
    switch( filter_num ){
        case 0:
            result = true;
            break;
        case 1:        // Not Mondays
            result = CurrentDOW != 1;
            break;
        case 2:        // Not Tuesdays
            result = CurrentDOW != 2;
            break;
        case 3:        // Not Wednesdays
            result = CurrentDOW != 3;
            break;
        case 4:        // Not Thursdays
            result = CurrentDOW != 4;
            break;
        case 5:        // Not Fridays
            result = CurrentDOW != 5;
            break;
    }

    return(result);
}


// ------------------------------------------------------------------------- //
bool TimeFilter_Intraday( int filter_num,
                          const Date &CurrentDate, const Time &CurrentTime,
                          int CurrentDOW, const Instrument &symbol,
                          int T_segment_duration )
{

    Time session_open_time { symbol.session_open_time() };
    Time session_close_time { symbol.session_close_time() };
    Time settlement_time { symbol.settlement_time() };

    // --------------------------    T-SEGMENTS    ------------------------- //
    int Tsegment {0};
    if ( CurrentTime > session_open_time
         && CurrentTime <= utils_time::CalcTime( session_open_time,
                                                 T_segment_duration ) ){
            Tsegment = 1;
    }
    else if ( CurrentTime > utils_time::CalcTime( session_open_time,
                                                   T_segment_duration )
             && CurrentTime <= utils_time::CalcTime( session_open_time,
                                                     2*T_segment_duration )){
            Tsegment = 2;
    }
    else if ( CurrentTime > utils_time::CalcTime( session_open_time,
                                                  2*T_segment_duration )
             && CurrentTime <= session_close_time ) {
            Tsegment = 3;
    }
    // --------------------------------------------------------------------- //

    // Before/After daily settlement time
    bool after_settlement { CurrentTime >= settlement_time
                            && CurrentTime <= session_close_time };


    bool result {false};
    switch( filter_num ){
        case 0:
            result = true;
            break;
        case 1:        // T-segment 1
            result = Tsegment == 1;
            break;
        case 2:        // Not T-segment 1
            result = Tsegment != 1;
            break;
        case 3:        // T-segment 2
            result = Tsegment == 2;
            break;
        case 4:        // Not T-segment 2
            result = Tsegment != 2;
            break;
        case 5:       // T-segment 3
            result = Tsegment == 3;
            break;
        case 6:       // Not T-segment 3
            result = Tsegment != 3;
            break;
        case 7:       // Not T-segment 1 and before settlement
            result = (Tsegment != 1) && !(after_settlement);
            break;
        case 8:       // Not T-segment 2 and before settlement
            result = (Tsegment != 2) && !(after_settlement);
            break;
        case 9:       // T-segment 3 and before settlement
            result = (Tsegment == 3) && !(after_settlement);
            break;
        case 10:
            result = CurrentTime >= Time(8,0)
                            && CurrentTime < settlement_time;
            break;

        //<<< tests
        /*
        case 16:
            result = //CurrentTime >= Time(8,0)
                    //     && CurrentTime < settlement_time &&
                     ( CurrentDOW == 2 || CurrentDOW == 3 );
            break;
        case 17:
            result = CurrentTime >= Time(8,0)
                         && CurrentTime < settlement_time &&
                     (CurrentDOW == 2 || CurrentDOW == 3);
            break;
        case 18:
            result = CurrentTime >= Time(8,0)
                         && CurrentTime < settlement_time
                         && (CurrentDOW != 5);
            break;
        */
        //<<<
    }

    return(result);
}
