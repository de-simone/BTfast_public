#include "utils_time.h"

#include <cmath>        // std::abs
#include <cstdio>       // sscanf
#include <iomanip>      // std::setfill, std::setw
#include <iostream>     // std::cout
#include <sstream>      // ostringstream

/*
//#include <algorithm>    // std::reverse, std::sort, std::remove


#include <cstdlib>      // srand, rand
#include <cstring>      // strcat
#include <fstream>      // std::fstream, open, close


#include <numeric>      // std::accumulate
#include <random>       // random_device, mt19937, uniform_int_distribution
#include <string>       // std::getline

#include <stdexcept>    // std::invalid_argument
*/


// ------------------------------------------------------------------------- //
// Return string of current datetime, as YYYY-MM-DD HH:MM:SS
std::string utils_time::current_datetime_str()
{
    std::ostringstream result;

    DateTime now{};
    now.set_current();

    result  << now.tostring() + ":"
            << std::setfill('0') << std::setw(2) << now.second();
    return( result.str() );
}


//-------------------------------------------------------------------------- //
// Add/subtract 'nmins' from initial_time ('nmins' may be >= 60)

Time utils_time::CalcTime(Time initial_time, int nmins)
{

    if( nmins == 0 ){
        std::cout << ">>> ERROR: argument of Calctime must be !=0";
        exit(1);
    }

    Time result {};
    Time duration {std::abs(nmins)/60, (std::abs(nmins)%60+60)%60};
    if( nmins>0 ){
        result = initial_time+duration;
    }
    else{
        result = initial_time-duration;
    }
    return(result);
}

// ------------------------------------------------------------------------- //
// Convert 'datestr' (format YYYY-MM-DD) into Date object

Date utils_time::str2date(std::string datestr)
{
    int y,m,d;
    sscanf(datestr.c_str(), "%4d-%2d-%2d", &y, &m, &d);
    return( Date {y,m,d} );
}


// --------------------------------------------------------------------- //
// Get actual start date from 'input_start_date' (format YYYY-MM-DD)

Date utils_time::actual_start_date( const std::string &input_start_date )
{
    // Actual start/end dates from input strings
    if( input_start_date != "0" ){
        return( utils_time::str2date(input_start_date) );
    }
    else{
        return( Date {1900,1,1} );  // far in the past
    }
}

// --------------------------------------------------------------------- //
// Get actual end date from 'input_end_date' (format YYYY-MM-DD)

Date utils_time::actual_end_date( const std::string &input_end_date )
{
    if( input_end_date != "0" ){
        return( utils_time::str2date(input_end_date) );
    }
    else{
        return( Date {2100,1,1} );    // far in the future
    }
}
