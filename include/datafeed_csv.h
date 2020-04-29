#ifndef DATAFEED_CSV_H
#define DATAFEED_CSV_H

#include "datafeed.h"

/*!
Read historical bars from a single CSV file,
and stream them to the provided events queue.

Member Variables:
- csv_format_: option to deal with different formats of input CSV file
    1 = intraday data exported from TradeStation
    2 = daily data exported from TradeStation
    3 = data exported from MatLab
- csv_file_: name of csv file
- start_date_: start date, included (from settings)
- end_date_: end date, included (from settings)
- continue_parsing_: switch to control parsing
- tot_bars_: total number of bars to parse (not defined for CSV)
  [unable to get the number of bars after date selection]
- infile_: pointer to a FILE object that identifies the stream

*/




class HistoricalBarsCSV : public DataFeed {

    std::string type_ {"CSV"};
    std::string csv_file_ {""};
    int csv_format_ {1};
    Date start_date_ {};
    Date end_date_ {};
    bool continue_parsing_ {true};
    //int tot_bars_ {1};
    FILE *infile_;


    public:
        // Constructor
        HistoricalBarsCSV(const Instrument &symbol,
                          std::string timeframe,
                          std::string data_file,
                          int csv_format,
                          Date start_date, Date end_date);

    private:
        // Functions overriding the base class pure virtual functions
        std::string type() const override { return(type_); }
        std::string data_file() const override { return(csv_file_); }
        int csv_format() const override { return(csv_format_); }
        Date start_date() const override { return(start_date_); }
        Date end_date() const override { return(end_date_); }
        bool continue_parsing() const override { return(continue_parsing_); }
        int tot_bars() const override { return(1); } // not defined
        void open_data_connection() override;
        void close_data_connection() override;
        void reset_cursor() override;
        void stream_next_bar() override;

        void set_start_date(Date d) override { start_date_=d; }
        void set_end_date(Date d) override { end_date_=d; }
        void set_data_file(std::string f) override { csv_file_ = f; }

        std::unique_ptr<DataFeed> clone() const override;
};




#endif
