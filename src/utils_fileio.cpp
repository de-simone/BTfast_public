#include "utils_fileio.h"

#include "xmlParser.h"  // XML parsing library

#include <algorithm>    // std::remove
#include <cstring>      // strcmp
#include <fstream>      // std::fstream, open, close
#include <iostream>     // std::cout
#include <sstream>      // stringstream



// ------------------------------------------------------------------------- //
//  Read input settings from configuration XML file

void utils_fileio::read_config_file(
                        std::string config_file,
                        std::string &main_dir,
                        int &run_mode,
                        std::string &strategy_name, std::string &symbol_name,
                        std::string &timeframe, std::string &input_start_date,
                        std::string &input_end_date,
                        std::string &data_dir, std::string &data_file,
                        int &csv_format, std::string &datafeed_type,
                        bool &print_progress,
                        bool &print_performance_report, bool &print_trade_list,
                        bool &write_trades_to_file, std::string &fitness_metric,
                        int &population_size, int &generations,
                        int &max_bars_back, double &initial_balance,
                        std::string &position_size_type,
                        int &num_contracts, double &risk_fraction,
                        bool &include_commissions, int &slippage,
                        std::string &data_file_oos,
                        int &max_variation_pct, int &num_noise_tests )
{
    std::string node_name {""};
    std::string node_value {"-"};
    XMLNode xMainNode{ XMLNode::openFileHelper(config_file.c_str(),
                                                "Settings") };
    // Number of <Input> nodes
    int n_inputs = xMainNode.nChildNode("Input");

    XMLNode xNode {};

    // Loop over all <Input> nodes
    for( int i = 0; i<n_inputs; i++){

        xNode = xMainNode.getChildNode("Input", i);

        // Extract Name and Value elements
        node_name = xNode.getChildNode("Name").getText();
        if( xNode.getChildNode("Value").getText() != NULL ){
            node_value = xNode.getChildNode("Value").getText();
        }
        else{
            std::cout<< ">>> ERROR: empty value in settings (read_param_file)\n";
            exit(1);
        }

        // Print Name/Value elements
        //cout << node_name << "  =  " << node_value << endl;

        // Assign values to variables
        if( node_name == "MAIN_DIR" ){
            main_dir = node_value;                                  // string
        }
        else if( node_name == "RUN_MODE" ){
            try{
                run_mode = std::stoi( node_value );                 // int
            }
            catch (const std::invalid_argument& er) {
                std::cerr << ">>> ERROR: invalid input for RUN_MODE\n";
                exit(1);
            }
        }
        else if( node_name == "STRATEGY_NAME" ){
            strategy_name = node_value;                             // string
        }
        else if( node_name == "SYMBOL_NAME" ){
            symbol_name = node_value;                               // string
        }
        else if( node_name == "TIMEFRAME" ){
            timeframe = node_value;                                 // string
        }
        else if( node_name == "START_DATE" ){
            input_start_date = node_value;                          // string
        }
        else if( node_name == "END_DATE" ){
            input_end_date = node_value;                            // string
        }
        else if( node_name == "DATA_DIR" ){
            data_dir = node_value;                                  // string
        }
        else if( node_name == "DATA_FILE" ){
            data_file = node_value;                                 // string
        }
        else if( node_name == "CSV_FORMAT" ){
            try{
                csv_format = std::stoi( node_value );               // int
            }
            catch (const std::invalid_argument& er) {
                std::cerr << ">>> ERROR: invalid input for CSV_FORMAT\n";
                exit(1);
            }
        }
        else if( node_name == "DATAFEED_TYPE" ){
            datafeed_type = node_value;                             // string
        }
        else if( node_name == "PRINT_PROGRESS" ){
            try{
                print_progress = std::stoi( node_value );           // int
            }
            catch (const std::invalid_argument& er) {
                std::cerr << ">>> ERROR: invalid input for PRINT_PROGRESS\n";
                exit(1);
            }
        }
        else if( node_name == "PRINT_PERFORMANCE_REPORT" ){
            try{
                print_performance_report = std::stoi( node_value ); // int
            }
            catch (const std::invalid_argument& er) {
                std::cerr << ">>> ERROR: invalid input for PRINT_PERFORMANCE_REPORT\n";
                exit(1);
            }
        }
        else if( node_name == "PRINT_TRADE_LIST" ){
            try{
                print_trade_list = std::stoi( node_value );         // int
            }
            catch (const std::invalid_argument& er) {
                std::cerr << ">>> ERROR: invalid input for PRINT_TRADE_LIST\n";
                exit(1);
            }
        }
        else if( node_name == "WRITE_TRADES_TO_FILE" ){
            try{
                write_trades_to_file = std::stoi( node_value );     // int
            }
            catch (const std::invalid_argument& er) {
                std::cerr << ">>> ERROR: invalid input for WRITE_TRADES_TO_FILE\n";
                exit(1);
            }
        }
        else if( node_name == "FITNESS_METRIC" ){
            fitness_metric = node_value ;                           // string
        }
        else if( node_name == "POPULATION_SIZE" ){
            try{
                population_size = std::stoi( node_value );          // int
            }
            catch (const std::invalid_argument& er) {
                std::cerr << ">>> ERROR: invalid input for POPULATION_SIZE\n";
                exit(1);
            }
            // population size must be even
            if( population_size%2 != 0){
                population_size += 1;
            }
        }
        else if( node_name == "GENERATIONS" ){
            try{
                generations = std::stoi( node_value );              // int
            }
            catch (const std::invalid_argument& er) {
                std::cerr << ">>> ERROR: invalid input for GENERATIONS\n";
                exit(1);
            }
        }
        else if( node_name == "MAX_BARS_BACK" ){
            try{
                max_bars_back = std::stoi( node_value );            // int
            }
            catch (const std::invalid_argument& er) {
                std::cerr << ">>> ERROR: invalid input for MAX_BARS_BACK\n";
                exit(1);
            }
        }
        else if( node_name == "INITIAL_BALANCE" ){
            try{
                initial_balance = std::stod( node_value );          // double
            }
            catch (const std::invalid_argument& er) {
                std::cerr << ">>> ERROR: invalid input for INITIAL_BALANCE\n";
                exit(1);
            }
        }
        else if( node_name == "POSITION_SIZE_TYPE" ){
            position_size_type = node_value;                        // string
        }
        else if( node_name == "NUM_CONTRACTS" ){
            try{
                num_contracts = std::stoi( node_value );            // int
            }
            catch (const std::invalid_argument& er) {
                std::cerr << ">>> ERROR: invalid input for NUM_CONTRACTS\n";
                exit(1);
            }
        }
        else if( node_name == "RISK_FRACTION" ){
            try{
                risk_fraction = std::stod( node_value );            // double
            }
            catch (const std::invalid_argument& er) {
                std::cerr << ">>> ERROR: invalid input for RISK_FRACTION\n";
                exit(1);
            }
        }
        else if( node_name == "INCLUDE_COMMISSIONS" ){
            try{
                include_commissions = std::stoi( node_value );      // int
            }
            catch (const std::invalid_argument& er) {
                std::cerr << ">>> ERROR: invalid input for INCLUDE_COMMISSIONS\n";
                exit(1);
            }
        }
        else if( node_name == "SLIPPAGE" ){
            try{
                slippage = std::stoi( node_value );                 // int
            }
            catch (const std::invalid_argument& er) {
                std::cerr << ">>> ERROR: invalid input for SLIPPAGE\n";
                exit(1);
            }
        }
        else if( node_name == "DATA_FILE_OOS" ){
            data_file_oos = node_value;                             // string
        }
        else if( node_name == "MAX_VARIATION_PCT" ){
            try{
                max_variation_pct = std::stoi( node_value );        // int
            }
            catch (const std::invalid_argument& er) {
                std::cerr << ">>> ERROR: invalid input for MAX_VARIATION_PCT\n";
                exit(1);
            }
        }
        else if( node_name == "NOISE_TESTS" ){
            try{
                num_noise_tests = std::stoi( node_value );          // int
            }
            catch (const std::invalid_argument& er) {
                std::cerr << ">>> ERROR: invalid input for NOISE_TESTS\n";
                exit(1);
            }
        }
    }
    // End of loop over <Input> nodes
}

// ------------------------------------------------------------------------- //
/* Read parameter values/ranges from XML parameter file
   and store them into vector of vectors (returned by function).
   Each sub-vector represents a complete set of parameter values:
            for <Input> parameters:     single fixed value
            for <OptRange> parameters:  vector of values start...stop (step)
*/

param_ranges_t utils_fileio::read_param_file( std::string paramfile )
{

    param_ranges_t result {};
    std::string node_name {""};
    int node_value {0};
    int node_start {0};
    int node_stop {0};
    int node_step {0};

    XMLNode xMainNode{ XMLNode::openFileHelper(paramfile.c_str(),"Inputs") };
    // Number of fixed parameters (<Input> nodes)
    int n_fixed = xMainNode.nChildNode("Input");
    // Number of optimized parameters (<OptRange> nodes)
    int n_optimized = xMainNode.nChildNode("OptRange");

    XMLNode xNode {};
    // Range of values for one parameter
    std::vector<int> param_vals{};

    // Loop over all nodes
    for( int i = 0; i<n_fixed+n_optimized; i++){

        // Clear set of parameters
        param_vals.clear();
        // Get next node
        xNode = xMainNode.getChildNode(i);
        // Extract Name
        node_name = xNode.getChildNode("Name").getText();

        // Parameter is fixed (<Input> node)
        if( strcmp(xNode.getName(), "Input") == 0 ){
            if( xNode.getChildNode("Value").isEmpty() ){
                std::cout<<">>> ERROR: invalid node in XML (read_param_file)\n";
                exit(1);
            }
            node_value = std::stoi( xNode.getChildNode("Value").getText() );
            param_vals.push_back(node_value);
        }
        // Parameter is optimized (<OptRange> node)
        else if( strcmp(xNode.getName(), "OptRange") == 0 ){
            if( xNode.getChildNode("Start").isEmpty() ){
                std::cout<<">>> ERROR: invalid node in XML (read_param_file)\n";
                exit(1);
            }
            if( xNode.getChildNode("Stop").isEmpty() ){
                std::cout<<">>> ERROR: invalid node in XML (read_param_file)\n";
                exit(1);
            }
            if( xNode.getChildNode("Step").isEmpty() ){
                std::cout<<">>> ERROR: invalid node in XML (read_param_file)\n";
                exit(1);
            }
            node_start = std::stoi( xNode.getChildNode("Start").getText() );
            node_stop  = std::stoi( xNode.getChildNode("Stop").getText() );
            node_step  = std::stoi( xNode.getChildNode("Step").getText() );
            do {
                param_vals.push_back(node_start);
                node_start += node_step;
            } while( node_start <= node_stop );

        }
        // Pair parameter values with names
        result.push_back(std::make_pair(node_name,param_vals));
    }
    // End of loop over all nodes

    /* // print results
    for( auto r: result){
        std::cout << r.first <<": \n";
        for( auto el: r.second){
            std::cout << "      "<< el<<" ";
        }
        std::cout << "\n";
    }
    */
    return(result);
}

// ------------------------------------------------------------------------- //
// Copy (append) content of file 'sourcefile' to file 'destfile'
// with each line starting with '#'
void utils_fileio::copy_file_to_file( std::string sourcefile,
                                      std::string destfile )
{

    FILE *outfile, *infile;
    char buffer[200];

    infile  = fopen(sourcefile.c_str(),  "r");
    outfile = fopen(destfile.c_str(),  "a");

    fprintf(outfile,"#<%s>\n", sourcefile.c_str());

    while( fgetc(infile) != EOF ){

        fseek(infile, -1, SEEK_CUR);
        fgets(buffer, 200, infile);

        if( strncmp(buffer,"*",1) != 0 ){     // ignore lines starting with *
            fprintf(outfile,"# %s",buffer);
        }
    }
    fclose(infile);

    fprintf(outfile,"#</%s>\n#\n", sourcefile.c_str());
    fclose(outfile);

}


// ------------------------------------------------------------------------- //
/*! Write strategies results (e.g. from optimization results) to file 'fname'.
    if fname == "" does nothing.

    It writes 7 metrics.

    Names/Number/Order of performance metrics must be matched among:
        - utils_params::extract_parameters_from_single_strategy
        - utils_params::extract_metrics_from_single_strategy
        - utils_optim::append_to_optim_results
        - utils_optim::sort_by_metric
        - utils_optim::sort_by_ntrades, utils_optim::sort_by_avgtrade, etc
        - utils_fileio::write_strategies_to_file
        - Individual::compute_individual_fitness
        - Validation::intermediate_selection
        - Validation::selection_conditions
        - mode_factory_sequential (run_modes)

    Return 1 if OK, 0 otherwise
*/

int utils_fileio::write_strategies_to_file( std::string fname,
                                             std::string paramfile,
                                             const std::vector<strategy_t> &optim,
                                             std::string strategy_name,
                                             std::string symbol_name,
                                             std::string timeframe,
                                             const Date &date_i,
                                             const Date &date_f,
                                             bool verbose )
{
    int result {0};
    if( fname != "" ){

        DateTime now{};
        now.set_current();

        FILE* outfile = fopen(fname.c_str(), "w");
        // Summary
        fprintf(outfile, "# TimeStamp   : %s:%02d\n",
                now.tostring().c_str(),  (int) now.second() );
        fprintf(outfile, "# Strategy    : %s\n", strategy_name.c_str());
        fprintf(outfile, "# Symbol      : %s\n", symbol_name.c_str());
        fprintf(outfile, "# TimeFrame   : %s\n", timeframe.c_str());
        fprintf(outfile, "# Date Range  : %s --> %s\n", date_i.tostring().c_str(),
                                                        date_f.tostring().c_str());
        fprintf(outfile,"#\n");
        fclose(outfile);

        // Copy XML strategy file (if provided)
        if( paramfile != "" ){
            utils_fileio::copy_file_to_file(paramfile, fname);
        }

        fopen(fname.c_str(), "a");
        // Iterate over optimization runs
        char header[500];
        char header_elem[30];
        char row[1000];
        char row_elem[30];
        for( auto it = optim.begin(); it!=optim.end(); it++ ){
            row[0] = '\0';
            header[0] = '\0';

            for( auto optrun = it->begin(); optrun != it->end(); optrun++ ){
                row_elem[0] = '\0';
                header_elem[0] = '\0';
                sprintf(header_elem, "%9s,", optrun->first.c_str());
                strcat(header, header_elem);    // append header_elem to header
                if( optrun == it->begin() ){           // Ntrades is int
                    sprintf(row_elem, "%9d,",(int) optrun->second);
                }
                else if( optrun <= it->begin()+6 ){    // other 6 metrics are double
                    sprintf(row_elem, "%9.2f, ", optrun->second);
                }
                else {                                // strategy params are int
                    sprintf(row_elem, "%11d,",(int) optrun->second);
                }
                strcat(row, row_elem);          // append row_elem to row
            }
            if( it == optim.begin() ){
                fprintf(outfile, "#%s\n", header);  // print on file
                if( verbose ){
                    printf("#%s\n", header);        // print on stdout
                }

            }
            fprintf(outfile, "%s\n", row);  // print on file
            if( verbose ){
                printf("%s\n", row);        // print on stdout
            }
        }
        fclose(outfile);

        result = 1;
    }

    return( result );
}


// ------------------------------------------------------------------------- //
/* Read strategies (metrics+parameters) from 'filename' in CSV format
*/
std::vector<strategy_t> utils_fileio::read_strategies_from_file(
                                                    std::string filename )
{
    std::vector<strategy_t> result {};

    // File pointer
    std::fstream infile;
    // Open an existing file
    infile.open(filename, std::fstream::in);
    std::vector<std::string> row;
    std::string line;
    std::string entry;
    std::string temp;
    // Vector to store header names
    std::vector<std::string> header_names {};

    int uncommented_lines { 0 };
    while( std::getline(infile, line) ){
        // read an entire row and store it into a string variable 'line'

        row.clear();
        // ignore lines starting with "#"
        if(  line.substr(0,1) == "#" ){
            temp = line;    // temporary storing current line
            continue;
        }
        uncommented_lines++;

        // Store header names using last line starting with "#"
        if( uncommented_lines == 1 ){
            std::stringstream header {temp};
            int header_count {0};
            // read every entry of 'header' of a row and store it into 'entry'
            while( std::getline(header, entry, ',') ) {
                if( header_count == 0 ){
                    // Remove '#' character from first header
                    entry = entry.substr(1, std::string::npos);
                }
                // remove empty spaces from 'entry'
                entry.erase( std::remove(entry.begin(), entry.end(), ' '),
                             entry.end());
                // append entry to vector of header names
                header_names.push_back(entry);
                header_count++;
            }
        }

        std::stringstream ss {line};
        // read every entry of 'line' of a row and store it into 'entry'
        while( std::getline(ss, entry, ',') ) {
            // append each entry to the 'row' vector
            row.push_back(entry);

        }
        // remove last element (after last ',')
        //row.pop_back();

        // load row into strategy
        strategy_t strategy{};
        for( size_t i = 0; i < row.size(); i++){
            std::pair<std::string,double> p { std::make_pair(
                                                        header_names.at(i),
                                                        std::stod(row.at(i)))};
            strategy.push_back( p );
        }
        // append strategy to result vector
        result.push_back(strategy);

    }
    infile.close();

    return(result);
}
