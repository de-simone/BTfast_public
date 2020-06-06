#include "datafeed_sqlite.h"

#include <cstdio>       // sprintf
#include <iostream>     // std::cout


// ------------------------------------------------------------------------- //
/*! Constructor
*/

HistoricalBarsSQLite::HistoricalBarsSQLite(const Instrument &symbol,
                                     std::string timeframe,
                                     std::string data_file_path,
                                     Date start_date, Date end_date)
: DataFeed{ symbol, timeframe },
  db_file_{data_file_path},
  start_date_{start_date}, end_date_{end_date}
{}




// ------------------------------------------------------------------------- //
/*! Open CSV file
*/
void HistoricalBarsSQLite::open_data_connection(){

    char query[200];
    sprintf(query,
            "SELECT * FROM %s_%s WHERE TimeStamp BETWEEN date('%s') \
            AND date('%s') ORDER BY TimeStamp;",
            symbol_.name().c_str(), timeframe_.c_str(),
            start_date_.tostring().c_str(), end_date_.tostring().c_str() );

    char count_query[200];
    sprintf(count_query,
            "SELECT COUNT(*) FROM %s_%s WHERE TimeStamp BETWEEN date('%s') \
            AND date('%s') ORDER BY TimeStamp;",
            symbol_.name().c_str(), timeframe_.c_str(),
            start_date_.tostring().c_str(), end_date_.tostring().c_str() );


    // Open DataBase
    int rc = sqlite3_open_v2( db_file_.c_str(), &db_,
                              SQLITE_OPEN_READONLY, NULL);
    if (rc != SQLITE_OK) {
        sqlite3_close(db_);
        std::cout << ">>> ERROR: unable to open SQLite database (datafeed): \n";
        exit(1);
    }

    // Prepare SQL Statement for counting  total number of lines
    sqlite3_prepare_v2(db_, count_query, -1, &stmt_, NULL);
    rc = sqlite3_step(stmt_);
    if (rc != SQLITE_ROW) {
        sqlite3_close(db_);
        std::cout << ">>> ERROR: no data matching request (datafeed): \n";
        exit(1);
    }
    tot_bars_ = sqlite3_column_int(stmt_, 0);

    // Prepare SQL Statement for parsing
    sqlite3_prepare_v2(db_, query, -1, &stmt_, NULL);

    // Reset cursor to the beginning
    reset_cursor();

}


// ------------------------------------------------------------------------- //
/*! Close CSV file
*/
void HistoricalBarsSQLite::close_data_connection(){

    sqlite3_finalize(stmt_);     // Clear Statement and Query
    sqlite3_close(db_);          // Close DataBase
}


// ------------------------------------------------------------------------- //
/*! Reset pointer to beginning of file.
    Reset continue_parsing_ to true.
    Function called when initializing backtest.
*/
void HistoricalBarsSQLite::reset_cursor(){

    sqlite3_reset(stmt_);
    continue_parsing_ = true;
}


// ------------------------------------------------------------------------- //
/*! Get next bar from datafeed, until all bars are parsed (use CSV)
*/
void HistoricalBarsSQLite::stream_next_bar() {

    int y,m,d,hh,mm,vup,vdn; // int vol;
    double op,hi,lo,cl; // double vol_float;
    int volume;

    //--- intraday data from TradeStation
    // MM/DD/YYYY,HH:MM,O,H,L,C,Vup,Vdn

    if( sqlite3_step(stmt_) == SQLITE_ROW ){            // get next bar

        sscanf( (char*) sqlite3_column_text(stmt_, 0),
                "%4d-%2d-%2d %2d:%2d", &y, &m, &d, &hh, &mm );
        op = sqlite3_column_double(stmt_, 1);
        hi = sqlite3_column_double(stmt_, 2);
        lo = sqlite3_column_double(stmt_, 3);
        cl = sqlite3_column_double(stmt_, 4);
        vup = sqlite3_column_int(stmt_, 5);
        vdn = sqlite3_column_int(stmt_, 6);
        volume = vup + vdn;

        // select date range            //<< control dates via query
        DateTime timestamp {y,m,d,hh,mm};
        if( timestamp.date() >= start_date_
            && timestamp.date() <= end_date_ ){
            // create new bar event
            Event new_bar { symbol_, timestamp, timeframe_,
                            op, hi, lo, cl, volume};
            // put bar event on events queue
            events_queue_->push_back( new_bar );
        }

    }
    else{                                   // end of parsing whole date range
        continue_parsing_ = false;
    }
    //---
}


// ------------------------------------------------------------------------- //
/*! Clone object and wrap it into unique ptr 
*/
std::unique_ptr<DataFeed> HistoricalBarsSQLite::clone() const
{
    return( std::make_unique<HistoricalBarsSQLite>(*this) );
}
