#include "datafeed_csv.h"

#include <filesystem>   // std::filesystem
#include <iostream>     // std::cout
#include <unistd.h>     // access



// ------------------------------------------------------------------------- //
/*! Constructor
*/

HistoricalBarsCSV::HistoricalBarsCSV(const Instrument &symbol,
                                     const std::string &timeframe,
                                     const std::string &data_dir,
                                     const std::string &data_file,
                                     int csv_format,
                                     Date start_date, Date end_date)
: DataFeed{ symbol, timeframe },
  data_dir_{data_dir}, data_file_{data_file},
  csv_format_{csv_format},
  start_date_{start_date}, end_date_{end_date}
{
    // set complete path to data file
    data_file_path_ = data_dir_ + "/" + data_file_;
}




// ------------------------------------------------------------------------- //
/*! Open CSV file
*/
void HistoricalBarsCSV::open_data_connection()
{
    // Check if csv_file exists
    if( access( data_file_path_.c_str(), F_OK ) != 0 ){

        std::cout << "\nAvailable data files:\n";
        for( const auto& entry:
                std::filesystem::recursive_directory_iterator(data_dir_) ){

            // extract filename from path
            std::string fname { entry.path().string().erase(0,
                                                    data_dir_.length()) };
            // ignore filenames starting with '/.'
            if( fname.rfind("/.", 0) != 0 ){
                std::cout << fname << "\n";
            }
        }
        std::cout << "\n>>> ERROR: CSV file does not exist (datafeed): "
                  << data_file_path_ << "\nAvailable data files listed above.\n";

        exit(1);
    }

    // Open file stream in reading mode
    infile_ = fopen(data_file_path_.c_str(), "r");

    // Count total number of lines
    /*
    char ch;
    int lines {0};
    fscanf(infile_, "%*[^\n]\n");  // skip first line
    while( !feof(infile_) ){
        ch = fgetc(infile_);
        if( ch == '\n' ){
            lines++;
        }
    }
    tot_bars_ = lines;
    */

    // Reset cursor to the beginning
    reset_cursor();

}


// ------------------------------------------------------------------------- //
/*! Close CSV file
*/
void HistoricalBarsCSV::close_data_connection()
{
    fclose(infile_);                         // close file stream
}


// ------------------------------------------------------------------------- //
/*! Reset pointer to beginning of file.
    Reset continue_parsing_ to true.
    Function called when initializing backtest.
*/
void HistoricalBarsCSV::reset_cursor()
{

    fseek(infile_, 0, SEEK_SET);
    fscanf(infile_, "%*[^\n]\n");            // skip first line

    continue_parsing_ = true;
}


// ------------------------------------------------------------------------- //
/*! Get next bar from datafeed, until all bars are parsed (use CSV)
*/
void HistoricalBarsCSV::stream_next_bar()
{

    char buffer[200];
    int y,m,d,hh,mm,vup,vdn,vol;
    int volume;
    double op,hi,lo,cl;

    //---
    if( fgetc(infile_) != EOF ){            // get next bar

        fseek(infile_, -1, SEEK_CUR);     // move stream backward by 1 to undo 'fgetc'
        fgets(buffer, 200, infile_);      // read 1 line (200 chars)

        switch( csv_format_ ){
            case 1:         // intraday data from TradeStation
                            // MM/DD/YYYY,HH:MM,O,H,L,C,Vup,Vdn
                sscanf(buffer, "%2d/%2d/%4d,%2d:%2d,%lf,%lf,%lf,%lf,%d,%d",
                        &m, &d, &y, &hh, &mm, &op, &hi, &lo, &cl, &vup, &vdn);
                volume = vup + vdn;
                break;

            case 2:         // daily data from TradeStation
                            // MM/DD/YYYY,HH:MM,O,H,L,C,V,OI
                sscanf(buffer, "%2d/%2d/%4d,%2d:%2d,%lf,%lf,%lf,%lf,%d,%*d",
                       &m, &d, &y, &hh, &mm, &op, &hi, &lo, &cl, &vol);
                volume = vol;
                break;

            case 3:         // intraday data from CSV (DXT)
                            // YYYY-MM-DD,HH:MM,O,H,L,C,Vol(int)
                sscanf(buffer, "%4d-%2d-%2d,%2d:%2d,%lf,%lf,%lf,%lf,%d",
                       &y, &m, &d, &hh, &mm, &op, &hi, &lo, &cl, &vol);
                volume = vol;
                break;
            /*
            case 4:         // intraday data from MatLab (DXT)
                            // YYYY-MM-DD HH:MM:SS,O,H,L,C,Vol(float)
                double vol_float;
                sscanf(buffer, "%4d-%2d-%2d %2d:%2d:%*d,%lf,%lf,%lf,%lf,%lf",
                       &y, &m, &d, &hh, &mm, &op, &hi, &lo, &cl, &vol_float);
                volume =  (int) vol_float;
                break;
            */
            default:        // invalid csv_format
                std::cout << ">>> ERROR: invalid CSV format (datafeed) "
                          << csv_format_ << "\n";
                exit(1);
        }


        // select date range
        DateTime timestamp {y,m,d,hh,mm};
        if( timestamp.date() >= start_date_
            && timestamp.date() <= end_date_ ){
            // create new bar event
            Event new_bar { symbol_, timestamp, timeframe_,
                            op, hi, lo, cl, volume };
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
std::unique_ptr<DataFeed> HistoricalBarsCSV::clone() const
{
    return( std::make_unique<HistoricalBarsCSV>(*this) );
}
