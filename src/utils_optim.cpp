#include "utils_optim.h"

#include <algorithm>    // std::sort
#include <iostream>     // std::cout

// ------------------------------------------------------------------------- //
/* Append to 'optim' the performance metrics and parameter combination of
   an optimization run (stored in 'perform' and 'parameters')

   Names/Number/Order of performance metrics must be matched among:
       - utils_params::extract_parameters_from_single_strategy
       - utils_optim::append_to_optim_results
       - utils_optim::sort_by_metric
       - utils_optim::sort_by_ntrades, utils_optim::sort_by_avgtrade, etc
       - utils_fileio::write_strategies_to_file
       - Individual::compute_individual_fitness
       - Validation::intermediate_selection
       - Validation::selection_conditions
       - mode_factory_sequential (run_modes)

*/
void utils_optim::append_to_optim_results( std::vector<strategy_t> &optim,
                                            const Performance& perform,
                                            const parameters_t& parameters )
{
    strategy_t row;
    // Append performance metrics.
    row.push_back( std::make_pair("Ntrades", perform.ntrades()) );          // 0
    //row.push_back( std::make_pair("NetPL", perform.netpl()) );
    row.push_back( std::make_pair("AvgTicks", perform.avgticks()) );        // 1
    row.push_back( std::make_pair("WinPerc", perform.winperc()) );          // 2
    //row.push_back( std::make_pair("AvgTrade", perform.avgtrade()) );
    row.push_back( std::make_pair("PftFactor", perform.profitfactor()) );   // 3
    row.push_back( std::make_pair("NP/MDD", perform.npmdd()) );             // 4
    row.push_back( std::make_pair("Expectancy", perform.expectancy()) );    // 5
    row.push_back( std::make_pair("Z-score", perform.zscore()) );           // 6

    // Append parameter combination
    for( single_param_t p : parameters ){
        row.push_back( std::make_pair(p.first, (double) p.second) );
    }

    optim.push_back(row);

    /*// print results
    for( auto el: row){
        cout<<el.first<<": " << el.second<<"\n";
    }
    */

}


// ------------------------------------------------------------------------- //
/*! Binary predicate function to compare two strategy_t objects
*/
bool utils_optim::equal_strategies( const strategy_t &a, const strategy_t &b )
{
    bool cond {true};
    if( a.size() != b.size() ){
        return(false);
    }
    else{
        for( int i=0; i<a.size(); i++ ){
             cond = cond && ( a.at(i).second == b.at(i).second );
        }
        return( cond );
    }
}

// ------------------------------------------------------------------------- //
/*! Binary predicate function to compare two parameters_t objects
*/

bool utils_optim::equal_strategy_params( const parameters_t &a,
                                         const parameters_t &b )
{
    bool cond {true};
    if( a.size() != b.size() ){
        return(false);
    }
    else{
        for( int i=0; i<a.size(); i++ ){
             cond = cond && ( a.at(i).second == b.at(i).second );
        }
        return( cond );
    }
}

// ------------------------------------------------------------------------- //
/*! Remove duplicates from 'strategies'.
    Strategies is modified in-place and sorted by 'fitness_metric'
*/

void utils_optim::remove_duplicates( std::vector<strategy_t> &strategies,
                                     const std::string &metric )
{
    if( strategies.empty() ){
        return;
    }
    utils_optim::sort_by_metric( strategies, metric );

    strategies.erase( std::unique( strategies.begin(), strategies.end(),
                                    utils_optim::equal_strategies ),
                      strategies.end() );

}

// ------------------------------------------------------------------------- //
/*! Remove duplicates from parameters (modified in-place)
*/

void utils_optim::remove_duplicates( std::vector<parameters_t> &parameters )
{
    /*
    if( parameters.empty() ){
        return;
    }
    parameters.erase( std::unique( parameters.begin(), parameters.end(),
                                    utils_optim::equal_strategy_params ),
                      parameters.end() );
    */
}

// ------------------------------------------------------------------------- //
/*! Binary functions for sorting in descending order of metrics

Names/Number/Order of performance metrics must be matched among:
    - utils_params::extract_parameters_from_single_strategy
    - utils_optim::append_to_optim_results
    - utils_optim::sort_by_metric
    - utils_optim::sort_by_ntrades, utils_optim::sort_by_avgtrade, etc
    - utils_fileio::write_strategies_to_file
    - Individual::compute_individual_fitness
    - Validation::intermediate_selection
    - Validation::selection_conditions
    - mode_factory_sequential (run_modes)
*/

bool utils_optim::sort_by_ntrades(const strategy_t& a, const strategy_t& b)
{
    return( a[0].second > b[0].second );
}
/*bool utils_optim::sort_by_netpl(const strategy_t& a, const strategy_t& b)
{
    return( a[1].second > b[1].second );
}*/
bool utils_optim::sort_by_avgticks(const strategy_t& a, const strategy_t& b)
{
    return( a[1].second > b[1].second );
}
bool utils_optim::sort_by_winperc(const strategy_t& a, const strategy_t& b)
{
    return( a[2].second > b[2].second );
}
/*
bool utils_optim::sort_by_avgtrade(const strategy_t& a, const strategy_t& b)
{
    return( a[2].second > b[2].second );
}
*/
bool utils_optim::sort_by_profitfactor(const strategy_t& a, const strategy_t& b)
{
    return( a[3].second > b[3].second );
}
bool utils_optim::sort_by_npmdd(const strategy_t& a, const strategy_t& b)
{
    return( a[4].second > b[4].second );
}
bool utils_optim::sort_by_expectancy(const strategy_t& a, const strategy_t& b)
{
    return( a[5].second > b[5].second );
}
bool utils_optim::sort_by_zscore(const strategy_t& a, const strategy_t& b)
{
    return( a[6].second > b[6].second );
}

// ------------------------------------------------------------------------- //
/*! Sort optimization results by fitness metric

    Names/Number/Order of performance metrics must be matched among:
        - utils_params::extract_parameters_from_single_strategy
        - utils_optim::append_to_optim_results
        - utils_optim::sort_by_metric
        - utils_optim::sort_by_ntrades, utils_optim::sort_by_avgtrade, etc
        - utils_fileio::write_strategies_to_file
        - Individual::compute_individual_fitness
        - Validation::intermediate_selection
        - Validation::selection_conditions
        - mode_factory_sequential (run_modes)
*/

void utils_optim::sort_by_metric( std::vector<strategy_t> &optim,
                                  std::string metric )
{
    if( metric == "Ntrades" ){
        std::sort( optim.begin(), optim.end(), utils_optim::sort_by_ntrades);
    }
    //else if( metric == "NetPL" ){
    //    std::sort( optim.begin(), optim.end(), utils_optim::sort_by_netpl);
    //}
    else if( metric == "AvgTicks" ){
        std::sort( optim.begin(), optim.end(), utils_optim::sort_by_avgticks);
    }
    else if( metric == "WinPerc" ){
        std::sort( optim.begin(), optim.end(), utils_optim::sort_by_winperc);
    }
    /*else if( metric == "AvgTrade" ){
        std::sort( optim.begin(), optim.end(), utils_optim::sort_by_avgtrade);
    }*/
    else if( metric == "ProfitFactor" ){
        std::sort( optim.begin(), optim.end(), utils_optim::sort_by_profitfactor);
    }
    else if( metric == "NP/MDD" ){
        std::sort( optim.begin(), optim.end(), utils_optim::sort_by_npmdd);
    }
    else if( metric == "Expectancy" ){
        std::sort( optim.begin(), optim.end(), utils_optim::sort_by_expectancy);
    }
    else if( metric == "Z-score" ){
        std::sort( optim.begin(), optim.end(), utils_optim::sort_by_zscore );
    }
    else{
        std::cout<<">>> ERROR: invalid metric (utils_optim::sort_by_metric).\n";
        exit(1);
    }

}
