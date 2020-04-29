#include "datetime.h"

#include "utils_math.h" // modulus
#include <cmath>        // floor
#include <cstdlib>      // abs
//#include <cstdio>       // sprintf
#include <iostream>     // std::cout


// ------------------------------------------------------------------------- //
/*! Constructors
*/
DateTime::DateTime (int year, int month, int day,
                    int hour, int minute, double second)
    : year_{year}, month_{month}, day_{day},
      hour_{hour}, minute_{minute}, second_{second}
{
    DateTime::set_leapyear(year_, is_leapyear_);
    DateTime::is_valid();
    DateTime::set_weekday(year_, month_, day_, weekday_);
    DateTime::set_businessday(is_bday_);
}



Date::Date (int year, int month, int day)
    : year_{year}, month_{month}, day_{day}
{
    // Create DateTime object with time 00:00:00
    // (also checks input validity)
    DateTime dt{year,month,day};
    // Retrieve member variables from DateTime object
    weekday_ = dt.weekday();
    is_bday_ = dt.is_bday();
    is_leapyear_ = dt.is_leapyear();
}

Time::Time (int hour, int minute, double second)
    : hour_{hour}, minute_{minute}, second_{second}
{
    Time::is_valid();
}


// ------------------------------------------------------------------------- //
// Calculate Weekday from year,month,day (doomsday algo or zeller's formula)
// and passed it by ref to wday [Mon=1, Tue=2, .., Sun = 7]
void DateTime::set_weekday(int y, int m, int d, int &wday)
{
    if (m == 1 || m == 2) {
        m = m + 12;
        y = y - 1;
    }
    int K = y%100;
    int J = (int)floor(y/100);

    int h = ( d + (int)floor((13 * (m + 1)) / 5) + K + (int)floor(K/4)
           + (int)floor(J/4) + 5*J) % 7 ;

    wday = (h+5)%7 + 1;
}


// ------------------------------------------------------------------------- //
// True if weekeday = 1-5 (mon-fri), false otherwise
void DateTime::set_businessday(bool &is_bus)
{
    switch(weekday_){

        case(1):
        case(2):
        case(3):
        case(4):
        case(5):
            is_bus=true;
            break;
        case(6):
        case(7):
            is_bus=false;
            break;
        default:
            is_bus = true;
    }
}

// ------------------------------------------------------------------------- //
/*! Set leap year
*/
void DateTime::set_leapyear(int y, bool &is_leap)
{
    is_leap = ( (y%4==0) && (y%100!=0) )|| (y%400==0);
}

// ------------------------------------------------------------------------- //
/*! Set object to current date and time
*/
void DateTime::set_current()
{
    time_t t = ::time(NULL);   // get time now
    struct tm *now = localtime( & t ); // convert it into struct tm

    year_ = now->tm_year + 1900;
    month_  = now->tm_mon+1;
    day_ = now->tm_mday;
    hour_ = now->tm_hour;
    minute_ = now->tm_min;
    second_ = now->tm_sec;

}


//-------------------------------------------------------------------------- //
/*! Add 'ndays' days (positive or negative) to DateTime object
    (possible bugs here!)
*/
/*
DateTime DateTime::add_days( int ndays )
{

    if( ndays == 0 ){
        std::cout << ">>> ERROR: argument of add_days must be !=0.\n";
        exit(1);
    }

    int DaysPerMonth[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

    int y {year_};
    int m {month_};
    int d {day_};
    bool is_leap {is_leapyear_};
    int i{0};

    // add positive ndays
    if( ndays > 0 ){
        while( i < ndays ){
            d++;
            if (   ( m != 2 && d > DaysPerMonth[m-1] )
                || ( m == 2 && !is_leap && d > 28 )
                || ( m == 2 && is_leap  && d > 29 ) ) {
                m++;
                d = 1;
                if (m == 13) {
                    m = 1;
                    y++;
                    set_leapyear(y, is_leap);
                }
            }
            i++;
        }
    }

    // substract negative ndays
    else if (ndays < 0) {
        while( i < abs(ndays) ){
            d--;
            if ( d < 1 ) {
                m--;
                d = DaysPerMonth[m-1];
                if( m == 2 && is_leap ){
                    d++;
                }
                if (m == 0) {
                    d = 31;
                    m = 12;
                    y--;
                    set_leapyear(y, is_leap);
                }
            }
            i++;
        }
    }

    DateTime result{y,m,d};
    return(result);
}
*/


//-------------------------------------------------------------------------- //
/*! Add 'ndays' business days (positive or negative), skipping weekend,
   to DateTime object
*/
/*
DateTime DateTime::add_bdays(int ndays){

    if( ndays == 0 ){
        std::cout << ">>> ERROR: argument of add_days must be !=0.\n";
        exit(1);
    }

    int DaysPerMonth[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

    int y {year_};
    int m {month_};
    int d {day_};
    bool is_leap {is_leapyear_};
    int wday {weekday_};

    int i{0};

    // add positive ndays
    if( ndays > 0 ){
        while( i < ndays ){
            d++;
            if (   ( m != 2 && d > DaysPerMonth[m-1] )
                || ( m == 2 && !is_leap && d > 28 )
                || ( m == 2 && is_leap  && d > 29 ) ) {
                m++;
                d = 1;
                if (m == 13) {
                    m = 1;
                    y++;
                    set_leapyear(y, is_leap);
                }
            }
            set_weekday(y,m,d, wday);
            // skip weekends
            if( wday != 6 && wday != 7 ){
                i++;
            }
        }
    }

    // substract negative ndays
    else if (ndays < 0) {
        while( i < abs(ndays) ){
            d--;
            if ( d < 1 ) {
                m--;
                d = DaysPerMonth[m-1];

                if( m == 2 && is_leap ){
                    d++;
                }
                if (m == 0) {
                    d = 31;
                    m = 12;
                    y--;
                    set_leapyear(y, is_leap);
                }
            }
            set_weekday(y,m,d, wday);
            // skip weekends
            if( wday != 6 && wday != 7 ){
                i++;
            }
        }
    }

    DateTime result{y,m,d};
    return(result);
}
*/

//-------------------------------------------------------------------------- //
/*! Difference in minutes between two DateTime objects (this-dt)
    within same year.
    Return -1 if t1,t2 have different years.
*/
int DateTime::MinutesDiff(const DateTime& dt){

    if( year_ != dt.year() ){
        return(-1);
    }
    int DaysPerMonth[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
    if( is_leapyear_ ){
        DaysPerMonth[1] = 29;
    }
    // days since year start
    int days1 {0};
    for(int m=1; m<month_; m++ ){
        days1 += DaysPerMonth[m-1];
    }
    days1 += day_;

    int days2 {0};
    for(int k=1; k<dt.month(); k++ ){
        days2 += DaysPerMonth[k-1];
    }
    days2 += dt.day();

    // minutes since year start
    int mins1{ days1*24*60 + hour_*60 + minute_ };          // this
    int mins2{ days2*24*60 + dt.hour()*60 + dt.minute() };  // dt

    return(mins1-mins2);
}

//-------------------------------------------------------------------------- //
/*! Extract Date object from DateTime object
*/
Date DateTime::date() const {

     Date result{year_, month_, day_};

     return(result);
}

//-------------------------------------------------------------------------- //
/*! Extract Time object from DateTime object
*/
Time DateTime::time() const {

     Time result{hour_, minute_, second_};

     return(result);
}


//-------------------------------------------------------------------------- //
/*! Check input validity
*/
void DateTime::is_valid(){

    bool isvalid {true};

    int daysPerMonth[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

    if (  day_ <= 0 || month_ <= 0  || month_ > 12 || year_ < 1900 ) {

        isvalid = false;
    }
    else if ( ( month_ != 2 && day_ > daysPerMonth[month_-1] )
            || ( month_ == 2 && !is_leapyear_ && day_ > 28 )
            || ( month_ == 2 && is_leapyear_  && day_ > 29 ) ){

        isvalid = false;
    }

    if( !isvalid ){
        //std::cout<<year_<<"  "<<month_<<"  "<<day_<<"\n";
        std::cout << ">>> ERROR: invalid inputs for Date.\n";
        exit(1);
    }
    else{
        // Instantiate a Time object to check it is valid (call Time::is_valid)
        Time *dt = new Time{ this->time() };
        // then delete it
        delete dt;
    }
}

//-------------------------------------------------------------------------- //
/*! String representation
*/
std::string DateTime::tostring() const {
    char buffer[22];
    //sprintf(buffer, "%04d-%02d-%02d %02d:%02d:%02.0f",
    //        year_, month_, day_, hour_, minute_, second_);
    sprintf(buffer, "%04d-%02d-%02d %02d:%02d",
            year_, month_, day_, hour_, minute_);
    std::string result{buffer};
    return(result);
}


//-------------------------------------------------------------------------- //
/*! Overloading the < operator
*/
bool DateTime::operator<(const DateTime& dt) const {
    bool result{ (this->date() < dt.date())
                 || ( (this->date() == dt.date())
                       && (this->time() < dt.time()) ) };
    return(result);
}

/*! Overloading the > operator
*/
bool DateTime::operator>(const DateTime& dt) const {
    bool result{ (this->date() > dt.date())
                  || ( (this->date() == dt.date())
                       && (this->time() > dt.time()) ) };
    return(result);
}

/*! Overloading the == operator
*/
bool DateTime::operator==(const DateTime& dt) const {
    bool result{ (this->date() == dt.date())
                 && (this->time() == dt.time()) };
    return(result);
}

// Overloading the <= operator
bool DateTime::operator<=(const DateTime& dt) const {
    bool result = (*this<dt) || (*this==dt) ;
    return(result);
}

/*! Overloading the >= operator
*/
bool DateTime::operator>=(const DateTime& dt) const {
    bool result = (*this>dt) || (*this==dt) ;
    return(result);
}












//-------------------------------------------------------------------------- //
// Add ndays (positive or negative) to Date object
/*
Date Date::add_days(int ndays){

    // Create DateTime object with time 00:00:00
    DateTime dt1 {year_, month_, day_};
    DateTime dt2 = dt1.add_days(ndays);

    Date result { dt2.year(), dt2.month(), dt2.day() };

    return(result);
}
*/

//-------------------------------------------------------------------------- //
// Add 'ndays' business days (positive or negative), skipping weekends,
// to Date object
/*
Date Date::add_bdays(int ndays){

    // Create DateTime object with time 00:00:00
    DateTime dt1 {year_,month_,day_};
    DateTime dt2 = dt1.add_bdays(ndays);

    Date result {dt2.year(), dt2.month(), dt2.day()};

    return(result);
}
*/

//-------------------------------------------------------------------------- //
/*! Rata Die: number of days since date 0001-01-01
*/
int Date::rdn(int y, int m, int d) const
{
    if (m < 3)
        y--, m += 12;

    return(365*y + y/4 - y/100 + y/400 + (153*m - 457)/5 + d - 306);
}

//-------------------------------------------------------------------------- //
/*! Difference in days between two Date objects (this-date),
    extrema included 
*/
int Date::DaysDiff( const Date& date ) const
{
    int result{0};
    int y1 = date.year();
    int m1 = date.month();
    int d1 = date.day();
    int y2 = year_;
    int m2 = month_;
    int d2 = day_;

    if( y1 == 0 || m1 == 0 || d1 == 0
       || y2 == 0 || m2 == 0 || d2 == 0 ){
        result = 1;
    }
    else result = rdn(y2, m2, d2) - rdn(y1, m1, d1) + 1;

    return(result);
}

//-------------------------------------------------------------------------- //
// String representation
std::string Date::tostring() const
{
    char buffer[11];
    sprintf(buffer, "%04d-%02d-%02d", year_, month_, day_);
    std::string result{buffer};
    return(result);
}
// ************************************************************************* //

//-------------------------------------------------------------------------- //
// Overloading the < operator
bool Date::operator<(const Date& d) const
{
    if( year_ == d.year() ) {
        if( month_ == d.month() ){
            return( day_ < d.day() );
        }
        return(month_ < d.month());
    }
    return(year_ < d.year());
}

/*! Overloading the > operator
*/
bool Date::operator>(const Date& d) const
{
    if( year_ == d.year() ) {
        if( month_ == d.month() ){
            return( day_ > d.day() );
        }
        return(month_ > d.month());
    }
    return(year_ > d.year());
}

/*! Overloading the == operator
*/
bool Date::operator==(const Date& d) const
{
    if( (year_ == d.year())
        && (month_ == d.month())
        && (day_ == d.day()) ){
        return(true);
    }
    else{
        return(false);
    }
}

/*! Overloading the <= operator
*/
bool Date::operator<=(const Date& d) const
{
    bool result = (*this<d) || (*this==d) ;
    return(result);
}

/*! Overloading the >= operator
*/
bool Date::operator>=(const Date& d) const
{
    bool result = (*this>d) || (*this==d) ;
    return(result);
}












//-------------------------------------------------------------------------- //
/*! Check input validity
*/

void Time::is_valid()
{
    if( hour_ < 0 || hour_ >= 24
        || minute_ < 0 || minute_ >= 60
        || second_ < 0 || second_ >=60) {

        std::cout << ">>> ERROR: invalid inputs for Time.\n";
        exit(1);
    }
}


//-------------------------------------------------------------------------- //
/*! String representation
*/
std::string Time::tostring() const {
    char buffer[12];
    sprintf(buffer, "%02d:%02d:%02.0f", hour_, minute_, second_);
    std::string result{buffer};
    return(result);
}




//-------------------------------------------------------------------------- //
/*! Overloading the < operator
*/
bool Time::operator<(const Time& t) const
{
    if( hour_ == t.hour() ) {
        if( minute_ == t.minute() ){
            return( second_ < t.second() );
        }
        return(minute_ < t.minute());
    }
    return(hour_ < t.hour());
}

/*! Overloading the > operator
*/
bool Time::operator>(const Time& t) const
{
    if( hour_ == t.hour() ) {
        if( minute_ == t.minute() ){
            return( second_ > t.second() );
        }
        return( minute_ > t.minute() );
    }
    return( hour_ > t.hour() );
}

/*! Overloading the == operator
*/
bool Time::operator==(const Time& t) const
{
    if( (hour_ == t.hour())
        && (minute_ == t.minute())
        && (second_ == t.second()) ){
        return(true);
    }
    else{
        return(false);
    }
}

/*! Overloading the <= operator
*/
bool Time::operator<=(const Time& t) const
{
    bool result { (*this<t) || (*this==t)  };
    return( result );
}

/*! Overloading the >= operator
*/
bool Time::operator>=(const Time& t) const
{
    bool result = (*this>t) || (*this==t) ;
    return( result );
}

/*! Overloading the + operator
*/
Time Time::operator+(const Time& t) const
{

    int tot_hours { hour_ + t.hour() };
    int tot_minutes{ minute_ + t.minute() };
    int tot_seconds{ int(second_ + t.second()) };
    double new_s {double(tot_seconds % 60)};
    //std::cout << tot_minutes/60 << " " << tot_minutes%60 << "\n";
    int new_m { (tot_minutes + tot_seconds/60) % 60};
    int new_h { (tot_hours + (tot_minutes + tot_seconds/60)/60 ) % 24  };
    Time result{new_h, new_m, new_s};
    return(result);
}

/*! Overloading the - operator
*/
Time Time::operator-(const Time& t) const
{

    int diff_hours { hour_ - t.hour() };
    int diff_minutes{ minute_ - t.minute() };
    int diff_seconds{ int(second_ - t.second()) };
    int new_h { utils_math::modulus(diff_hours, 24) };
    int new_m { utils_math::modulus(diff_minutes, 60) };
    double new_s { (double) utils_math::modulus(diff_seconds, 60) };

    if( diff_minutes<0 ) {
        new_h--;
        new_h = (new_h%24+24)%24;
    }
    if( diff_seconds<0 ) {
        new_m--;
        new_m = (new_m%60+60)%60;
    }
    Time result{new_h, new_m, new_s};
    return(result);
}
