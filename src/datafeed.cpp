#include "datafeed.h"

#include "datafeed_csv.h"
//#include "datafeed_sqlite.h"


//#include <cstdlib>      // exit
#include <iostream>     // std::cout


// ------------------------------------------------------------------------- //
/*! Constructor
*/
DataFeed::DataFeed( const Instrument &symbol, std::string timeframe )
:  symbol_{symbol}, timeframe_{timeframe}{}

// ------------------------------------------------------------------------- //
/*! Body for Pure virtual Destructor
*/
DataFeed::~DataFeed(){}


// ------------------------------------------------------------------------- //
/*! Set Queue by pointer
*/
void DataFeed::set_events_queue(std::deque<Event> *events_queue)
{
    events_queue_ = events_queue;
}








// ------------------------------------------------------------------------- //
/* Instantiate datafeed corresponding to 'datafeed_type' and
    assign unique_ptr to that object.
*/
void select_datafeed( std::unique_ptr<DataFeed>& datafeed_ptr,
                      std::string datafeed_type,
                      const Instrument &symbol, std::string timeframe,
                      const std::string &data_dir,
                      const std::string &data_file,
                      int csv_format, Date start_date, Date end_date )
{
    if( datafeed_type == "CSV" ){
        datafeed_ptr = std::make_unique<HistoricalBarsCSV> (
                                                    symbol, timeframe,
                                                    data_dir, data_file,
                                                    csv_format,
                                                    start_date, end_date );
    }
    /*
    else if( datafeed_type == "SQLite" ){
        datafeed_ptr = std::make_unique<HistoricalBarsSQLite> (
                                                    symbol, timeframe,
                                                    data_file,
                                                    start_date, end_date);
    }
    */
    else{
        std::cout << ">>> ERROR: invalid datafeed type (select_datafeed).\n";
        exit(1);
    }
}
