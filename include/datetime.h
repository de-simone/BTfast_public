#ifndef DATETIME_H
#define DATETIME_H

#include <string>       // std::string

// ------------------------------------------------------------------------- //
// Date

class Date {
    /*!
    Member Variables:
    - year_: year
    - month_: month (1-12)
    - day_: day of month (1-31)
    - weekday_: day of week (Mon=1, ..., Sun=7)
    - is_bday_: whether or not it is a working day (mon-fri)
    - is_leapyear_: whether or not year is leap
    */
    int year_ {0};
    int month_ {0};
    int day_ {0};
    int weekday_ {0};
    bool is_bday_ {true};
    bool is_leapyear_ {false};

    public:
        Date(int year=1900, int month=1, int day=1);

        //Date add_days(int ndays);
        //Date add_bdays(int ndays);
        int rdn(int y, int m, int d) const;
        int DaysDiff(const Date& date) const;

        // Getters
        int year() const { return(year_); }
        int month() const { return(month_); }
        int day() const { return(day_); }
        int weekday() const { return(weekday_); }
        bool is_bday() const { return(is_bday_); }
        bool is_leapyear() const { return(is_leapyear_); }

        // string representation
        std::string tostring() const;

        // Overloading comparison functions
        bool operator<(const Date& d) const ;
        bool operator>(const Date& d) const;
        bool operator==(const Date& d) const ;
        bool operator<=(const Date& d) const ;
        bool operator>=(const Date& d) const ;
};


// ------------------------------------------------------------------------- //
// Time

class Time {
    /*!
    Member Variables:
    - hour_: hour (0-23)
    - minute_: minute (0-59)
    - second_: second (0-59)
    */
    int hour_ {0};
    int minute_ {0};
    double second_ {0.0};

    public:
        Time(int hour=0, int minute=0, double second=0.0);

        void is_valid();
        int tot_minutes() const { return(hour_*60+minute_); };

        // Getters
        int hour() const { return(hour_); };
        int minute() const { return(minute_); };
        double second() const { return(second_); };

        // string representation
        std::string tostring() const;

        // Overloading comparison functions
        bool operator<(const Time& t) const ;
        bool operator>(const Time& t) const ;
        bool operator==(const Time& t) const;
        bool operator<=(const Time& t) const;
        bool operator>=(const Time& t) const;
        Time operator+(const Time& t) const;
        Time operator-(const Time& t) const;
};


// ------------------------------------------------------------------------- //
// DateTime: timestamp with date and time info
class DateTime {
    /*!
    Member Variables:
    - year_: year
    - month_: month (1-12)
    - day_: day of month (1-31)
    - hour_: hour (0-23)
    - minute_: minute (0-59)
    - second_: second (0-59)
    - weekday_: day of week (Mon=1, ..., Sun=7)
    - is_bday_: whether or not it is a working day (mon-fri)
    - is_leapyear_: whether or not year is leap
    */

    int year_ {1900};
    int month_ {1};
    int day_ {1};
    int hour_ {0};
    int minute_ {0};
    double second_ {0.0};
    int weekday_ {1};    // Mon=1, ..., Sun=7
    bool is_bday_ {true};
    bool is_leapyear_ {false};

    public:
        DateTime(int year=1900, int month=1, int day=1,
                 int hour=0, int minute=0, double second=0.0);

        void is_valid();

        void set_weekday(int y, int m, int d, int &wday);
        void set_businessday(bool &is_bus);
        void set_leapyear(int y, bool &is_leap);
        void set_current();

        //DateTime add_days(int ndays); (possible bugs here)
        //DateTime add_bdays(int ndays);
        int MinutesDiff(const DateTime& dt);

        Date date() const;
        Time time() const;

        // Getters
        int year() const { return(year_); };
        int month() const { return(month_); };
        int day() const { return(day_); };
        int hour() const { return(hour_); };
        int minute() const { return(minute_); };
        double second() const { return(second_); };
        int weekday() const { return(weekday_); };
        bool is_bday() const { return(is_bday_); };
        bool is_leapyear() const { return(is_leapyear_); }

        // string representation
        std::string tostring() const;

        // Overloading comparison functions
        bool operator<(const DateTime& dt) const ;
        bool operator>(const DateTime& dt) const ;
        bool operator==(const DateTime& dt) const ;
        bool operator<=(const DateTime& dt) const ;
        bool operator>=(const DateTime& dt) const ;
};


#endif
