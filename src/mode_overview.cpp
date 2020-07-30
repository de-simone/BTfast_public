#include "run_modes.h"

#include "account.h"
#include "utils_params.h"   // single_parameter_combination
#include "utils_time.h"     // current_datetime_str

#include <cstdlib>          // std::system
#include <fstream>          // std::ofstream
#include <iostream>         // std::cout


// ------------------------------------------------------------------------- //
/*! Overview of Market main features (no trade)
*/
void mode_overview( BTfast &btf, std::unique_ptr<DataFeed> &datafeed,
                    const param_ranges_t &parameter_ranges,
                    const std::string &overview_file )
{
    std::cout<< "    Run Mode   : Market overview\n\n";
    std::cout << utils_time::current_datetime_str() + " | "
              << "Running Market Overview \n";
    // Extract single parameter combination from parameter_ranges
    // (only the <Start> value is taken)
    parameters_t parameter_combination {
        utils_params::single_parameter_combination(parameter_ranges) };
    // Initialize Account
    Account account { btf.initial_balance() };

    // Parse data and collect market info, without strategy signals
    btf.run_overview( account, datafeed, parameter_combination );

    //--- Write to overview_file
    std::ofstream outfile;
    outfile.open( overview_file );

    // (Date, End-of-Day price)
    for( auto p: btf.eod_prices() ){
        outfile << p.first.tostring() <<", "<< p.second <<"\n";
    }
    outfile << "\n\n";

    // Volume for each hour
    for( int i=0; i < btf.volume_hour().size(); i++ ){
        outfile << i <<", " << btf.volume_hour().at(i) <<"\n";
    }
    outfile << "\n\n";

    // Sum of range Close-Open (in ticks) for each Day of Week
    for( int i=0; i < btf.co_range_dow().size(); i++ ){
        outfile << i+1 <<", " << btf.co_range_dow().at(i) <<"\n";
    }
    outfile << "\n\n";

    // Daily range H-L (in USD)
    for( auto r: btf.hl_range() ){
        outfile << r <<"\n";
    }
    outfile << "\n\n";

    outfile.close();
    std::cout<< "\nOverview info written on file: " << overview_file << "\n";
    //---

    // Execute script for gnuplot and open the PNG file
    std::string command { "./bin/PlotMktOverview" };
    std::system(command.c_str());
}
