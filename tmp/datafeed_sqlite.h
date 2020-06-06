#ifndef DATAFEED_SQLITE_H
#define DATAFEED_SQLITE_H

#include "datafeed.h"

#include <sqlite3.h>

/*!
Read historical bars from a single CSV file,
and stream them to the provided events queue.

Member Variables:

- sqlite_file: path to SQLite database
- start_date_: start date, included (from settings)
- end_date_: end date, included (from settings)
- continue_parsing_: switch to control parsing
- tot_bars_: total number of bars to parse
- db_: pointer to SQLite database
- stmt_: SQLite statement

*/




class HistoricalBarsSQLite : public DataFeed {

    std::string type_ {"SQLite"};
    std::string db_file_ {""};
    Date start_date_ {};
    Date end_date_ {};
    bool continue_parsing_ {true};
    int tot_bars_ {1};
    sqlite3 *db_;
    sqlite3_stmt *stmt_;



    public:
        // Constructor
        HistoricalBarsSQLite(const Instrument &symbol,
                            std::string timeframe,
                            std::string data_file,
                            Date start_date, Date end_date);

    private:
        // Functions overriding the base class pure virtual functions
        std::string type() const override { return(type_); }
        std::string data_file() const override { return(db_file_); }
        int csv_format() const override { return(0); }
        Date start_date() const override { return(start_date_); }
        Date end_date() const override { return(end_date_); }
        bool continue_parsing() const override { return(continue_parsing_); }
        int tot_bars() const override { return(tot_bars_); }
        void open_data_connection() override;
        void close_data_connection() override;
        void reset_cursor() override;
        void stream_next_bar() override;

        void set_start_date(Date d) override { start_date_=d; }
        void set_end_date(Date d) override { end_date_=d; }
        void set_data_file(std::string f) override { db_file_ = f; }

        std::unique_ptr<DataFeed> clone() const override;
};




#endif
