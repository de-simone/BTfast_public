#include "utils_params.h"

#include <algorithm>    // std::reverse
#include <numeric>      // std::accumulate


// ------------------------------------------------------------------------- //
//  Extract first element of std::vector<int> in each pair of v
parameters_t utils_params::single_parameter_combination(const param_ranges_t &v)
{
    parameters_t result {};
    for( auto elem: v){
        if( !elem.second.empty() ){
            result.push_back( std::make_pair(elem.first, elem.second.at(0)) );
        }
    }
    return(result);
}


// ------------------------------------------------------------------------- //
//  Cartesian product of variable number of vector<int>, stored in a vector v
std::vector<parameters_t> utils_params::cartesian_product(param_ranges_t &v )
{
    //vector<vector<int>> result;
    std::vector<parameters_t> result;
    //vector<int> row(v.size());
    parameters_t row(v.size());

    //auto product = []( int a, vector<int>& b ) { return( a*b.size() ); };
    auto product = []( int a, std::pair<std::string,std::vector<int>>& b ) {
                                            return( a*b.second.size() ); };
    const int totsize = std::accumulate( v.begin(), v.end(), 1, product );

    for( int n=0 ; n<totsize ; ++n ) {
        div_t q { n, 0 };
        row.clear();

        for( int i=v.size()-1 ; i>=0 ; i-- ) {
            //q = div( q.quot, v[i].size() );
            q = div( q.quot, v[i].second.size() );
            //row.push_back( v[i][q.rem] );
            row.push_back( std::make_pair( v[i].first, v[i].second[q.rem] ));
        }
        // reverse order in row
        std::reverse(row.begin(),row.end());
        // insert row into result vector
        result.push_back(row);

    }
    /* // print results
    for( auto el: result){
        for(auto p : el){
            cout<<p.first<<", " << p.second<<"\n";
        }
        cout<<"\n";
    }
    */

    return(result);
}


// --------------------------------------------------------------------- //
/*  Extract only the parameter values from  strategy  'source'
    (ignoring the entries with performance metrics)
    and cast them double -> int. Copy the result into 'dest'.

    Names/Number/Order of performance metrics must be matched among:
        - utils_params::extract_parameters_from_single_strategy
        - utils_optim::append_to_optim_results
        - utils_optim::sort_by_metric
        - utils_optim::sort_by_ntrades, utils_optim::sort_by_avgtrade, etc
        - utils_fileio::write_strategies_to_file
        - Individual::compute_individual_fitness
        - Validation::selection_conditions

*/
void utils_params::extract_parameters_from_single_strategy(
                                                    const strategy_t &source,
                                                    parameters_t &dest )
{
    // clear destination vector
    dest.clear();
    // loop over strategy values (metrics or parameters)
    for(std::pair<std::string,double> p : source){
        if(    p.first == "Ntrades"  || p.first == "AvgTicks"
            || p.first == "WinPerc" || p.first == "PftFactor"
            //|| p.first == "AvgTrade" || p.first == "NetPL"
            || p.first == "NP/MDD"
            || p.first == "Expectancy" || p.first == "Z-score" ){
            // skip performance metrics
            continue;
        }
        else{
            // cast and append
            dest.push_back(std::make_pair(p.first, (int) p.second));
        }
    }
}


// --------------------------------------------------------------------- //
/*  Extract only the parameter values from  all stratgies in 'source'
    (ignoring the entries with performance metrics)
    and cast them double -> int. Copy the result into 'dest'.

    Names of performance metrics must match those in:
    utils_optim::append_to_optim_results
*/
void utils_params::extract_parameters_from_all_strategies(
                                    const std::vector<strategy_t> &source,
                                    std::vector<parameters_t> &dest )
{
    // clear destination vector
    dest.clear();
    // loop over source strategies
    for( strategy_t el: source ){
        parameters_t row;
        utils_params::extract_parameters_from_single_strategy( el, row);
        dest.push_back(row);
    }
    /*
    // print dest
    for( auto el: dest){
        for(auto p : el){
            std::cout<<p.first<<", " << p.second<<"  ";
        }
        std::cout<<"\n";
    }
    */
}
