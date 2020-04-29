#ifndef DATAFEED_H
#define DATAFEED_H

#include "events.h"

#include <deque>    // std::deque
#include <memory>   // std::unique_ptr


// ------------------------------------------------------------------------- //
/*!
Abstract base class for all datafeeds

Member Variables:
- symbol_:  object of instrument class
- timeframe_: timeframe
- events_queue_: pointer to events_queue,
                 initialized in BTfast.run_backtest() and set via method.

*/

class DataFeed {

    protected:
        Instrument symbol_;
        std::string timeframe_ {""};
        std::deque<Event> *events_queue_{nullptr};


    public:
        // Constructor
        DataFeed( const Instrument &symbol, std::string timeframe );
        // Pure virtual Destructor (requires a function body)
        virtual ~DataFeed() = 0;

        // Getters
        Instrument symbol() const { return(symbol_); };
        std::string timeframe() const { return(timeframe_); }
        std::deque<Event>* events_queue() const { return(events_queue_); };

        // Link queue to events queue
        void set_events_queue(std::deque<Event> *events_queue);

        // Pure virtual functions (overridden by derived objects)
        virtual std::string type() const = 0;
        virtual std::string data_file() const = 0;
        virtual int csv_format() const = 0;
        virtual Date start_date() const = 0;
        virtual Date end_date() const = 0;
        virtual bool continue_parsing() const = 0;
        virtual int tot_bars() const = 0;
        virtual void open_data_connection() = 0;
        virtual void close_data_connection() = 0;
        virtual void reset_cursor() = 0;
        virtual void stream_next_bar() = 0;

        virtual void set_start_date(Date d) = 0;
        virtual void set_end_date(Date d) = 0;
        virtual void set_data_file(std::string f) = 0;

        virtual std::unique_ptr<DataFeed> clone() const = 0;
};



// ------------------------------------------------------------------------- //
/*! Instantiate datafeed corresponding to 'datafeed_type' and
    assign unique_ptr to that object.
*/
void select_datafeed( std::unique_ptr<DataFeed>& datafeed_ptr,
                      std::string datafeed_type,
                      const Instrument &symbol, std::string timeframe,
                      std::string data_file_path, int csv_format,
                      Date start_date, Date end_date );

#endif
