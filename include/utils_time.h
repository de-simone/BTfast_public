#ifndef UTILS_TIME_H
#define UTILS_TIME_H

#include "datetime.h"

#include <string>           // std::string


// Set of Utility functions for time operations


namespace utils_time {

    // --------------------------------------------------------------------- //
    /*! Return string of current datetime, as YYYY-MM-DD HH:MM:SS
    */
    std::string current_datetime_str();

    // --------------------------------------------------------------------- //
    /*! Add/subtract minutes from initial_time ('nmins' may be >= 60)
    */
    Time CalcTime(Time initial_time, int nmins);

    // --------------------------------------------------------------------- //
    /*! Convert 'datestr' (format YYYY-MM-DD) into Date object
    */
    Date str2date(std::string datestr);

    // --------------------------------------------------------------------- //
    /*! Get actual start date from 'input_start_date' (format YYYY-MM-DD)
    */
    Date actual_start_date( const std::string &input_start_date );

    // --------------------------------------------------------------------- //
    /*! Get actual end date from 'input_end_date' (format YYYY-MM-DD)
    */
    Date actual_end_date( const std::string &input_end_date );

}



#endif
