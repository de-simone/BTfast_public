#include "utils_params.h"

#include "utils_optim.h"    // remove_duplicates

#include <algorithm>    // std::reverse, std::sort, std::unique, std::max_element
#include <numeric>      // std::accumulate
#include <iostream>     // std::cout
#include <utility>      // std::make_pair

// --------------------------------------------------------------------- //
/*!  Print content of parameters_t type
    [ ("p1", 2), ("p2", 7), ... ]
*/
void utils_params::print_parameters_t( const parameters_t& p )
{
    for( const auto& elem: p ){
        std::cout<< elem.first<<": "<< elem.second<<"\n";
    }
    std::cout<<"\n";
}

// --------------------------------------------------------------------- //
/*!  Print content of strategy_t type
    [ ("metric1", 2.3), ..., ("p1", 2), ("p2", 7), ... ]
*/
void utils_params::print_strategy_t( const strategy_t& s )
{
    for( const auto& elem: s ){
        //std::cout<< elem.first<<": "<< elem.second<<"\n";
        std::cout<<elem.second<<", ";
    }
    std::cout<<"\n";
}

// --------------------------------------------------------------------- //
/*!  Print content of vector of parameters_t type
    [ [ ("p1", 2), ("p2", 7), ... ],
      [ ("p1", 4), ("p2", 11), ... ], ... ]
*/
void utils_params::print_parameters_t_vector(const std::vector<parameters_t>& v)
{
    for( const auto& p: v ){
        std::cout<<"\n";
        for( const auto& elem: p){
            //std::cout<< elem.first<<": "<< elem.second<<"\n";
            std::cout<<elem.second<<", ";
        }
    }
    std::cout<<"\n";
}

// --------------------------------------------------------------------- //
/*!  Print content of vector of strategy_t type
    [ [ ("p1", 2), ("p2", 7), ... ],
      [ ("p1", 4), ("p2", 11), ... ], ... ]
*/
void utils_params::print_strategy_t_vector( const std::vector<strategy_t>& v )
{
    for( const auto& p: v ){
        std::cout<<"\n";
        for( const auto& elem: p){
            //std::cout<< elem.first<<": "<< elem.second<<"\n";
            std::cout<<elem.second<<", ";
        }
    }
    std::cout<<"\n";
}

// --------------------------------------------------------------------- //
/*!  Print content of param_ranges_t type
    [ ("p1", [10]), ("p2", [2,4,6,8]), ... ]
*/
void utils_params::print_param_ranges_t( const param_ranges_t& par_range )
{
    for( const auto& p: par_range ){
        std::cout<<"\n"<<p.first<<": ";
        for( const auto& elem: p.second){
            std::cout<< elem <<", ";
        }
    }
    std::cout<<"\n";
}



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
/*  Extract only the parameter values from  all strategies in 'source'
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


// --------------------------------------------------------------------- //
/*  Extract only the parameter values from strategy 'source'
    (ignoring the entries with performance metrics)
    and cast them double -> int. Copy the result into 'dest'.

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
        - Validation::noise_test
        - mode_factory_sequential (run_modes)

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
/*  Extract only the performance metrics values from strategy 'source'
    (ignoring the entries with parameters metrics)
    and store them into vector 'dest'.

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
        - mode_factory_sequential
*/
void utils_params::extract_metrics_from_single_strategy(
                                                    const strategy_t &source,
                                                    std::vector<double> &dest )
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
            // append to destination
            dest.push_back( p.second );
        }
    }
}




// --------------------------------------------------------------------- //
/*!  Extract parameter range vector from 'par_range' corresponding to
     name 'par_name' and expand all base strategies 'base_strats' with
     'par_name' replaced by optimization range.
     Output vector replaces 'base_strats'.
*/

void utils_params::expand_strategies_with_opt_range(
                                    const std::string &par_name,
                                    const param_ranges_t &par_range,
                                    std::vector<parameters_t> &base_strats)
{
    std::vector<parameters_t> result;
    // find 'par_name' in 'par_range' and fill optrange vector
    std::vector<int> optrange { utils_params::opt_range_by_name( par_name,
                                                                 par_range ) };

    for( const auto& elem: base_strats ){
        /// append original base strategy
        result.push_back(elem);
        // append strategy with modified value of 'par_name'
        for( const auto& param: optrange ){
            // avoid duplication by checking if 'param' value already exists
            if( param != utils_params::parameter_by_name(par_name, elem) ){
                parameters_t new_entry { elem };
                set_parameter_value_by_name( par_name, new_entry, param );
                result.push_back(new_entry);
            }
        }
    }
    //utils_optim::remove_duplicates( result );
    base_strats = result;
}





// --------------------------------------------------------------------- //
/*! Extract attribute (metric or paramter) named 'attr_name'
    from single strategy 'source'
*/
double utils_params::strategy_attribute_by_name( const std::string &attr_name,
                                                 const strategy_t &source )
{
    for( const auto& elem : source ){
        if( elem.first == attr_name ){
            return(elem.second);
        }
    }
    return(0.0);    // returned value if 'attr_name' is not found
}

// --------------------------------------------------------------------- //
/*! Extract maximum value of the metric named 'metric_name'
    over vector of strategies 'source'
*/
double utils_params::max_strategy_metric_by_name(
                                    const std::string &metric_name,
                                    const std::vector<strategy_t> &source )
{
    std::vector<double> metric_vec {};
    for( const auto& strat: source ){
        for( const auto& elem : strat ){
            if( elem.first == metric_name ){
                metric_vec.push_back(elem.second);
            }
        }
    }
    double result {0.0};
    if( !metric_vec.empty() ){
        result = *std::max_element( metric_vec.begin()+1, metric_vec.end() );
    }
    return(result);
}


// --------------------------------------------------------------------- //
/*! Set parameter value in 'parameters' corresponding to name 'par_name'
    to value 'new_value'
*/
void utils_params::set_parameter_value_by_name( const std::string &par_name,
                                    parameters_t& parameters, int new_value )
{
    for( auto& elem: parameters){
        if( elem.first == par_name ){
            elem.second = new_value;
            break;
        }
    }
}

// --------------------------------------------------------------------- //
/*! Set parameter value in 'strategies' corresponding to name 'par_name'
    to value 'new_value'
*/
/*
void utils_params::set_strategy_parameter_value_by_name(
                                    const std::string &par_name,
                                    strategy_t& strategies, int new_value )
{
    for( auto& elem: strategies){
        if( elem.first == par_name ){
            elem.second = (double) new_value;
            break;
        }
    }
}
*/

// --------------------------------------------------------------------- //
/*! Extract value of parameter  named 'par_name' from parameters_t  'source'
*/
int utils_params::parameter_by_name( const std::string &par_name,
                                     const parameters_t &source )
{
    for( const auto& elem : source ){
        if( elem.first == par_name ){
            return(elem.second);
        }
    }
    return(0);    // returned value if 'par_name' is not found
}

// --------------------------------------------------------------------- //
/*! Get first parameter value from 'source' corresponding to
    name 'par_name'
*/
int utils_params::parameter_value_by_name( const std::string &par_name,
                                           const param_ranges_t &source )
{
    int result {};
    bool elem_found {false};
    for( const auto& elem: source){
        if( elem.first == par_name ){
            // retrieve first parameter value
            result =  elem.second.at(0);
            elem_found = true;
            break;
        }
    }
    // check if 'par_name' was found in 'dest'
    if( !elem_found ){
        std::cout<< ">>> ERROR: Parameter name " << par_name
                 << " not found in destination (replace_opt_range_by_name)\n";
        exit(1);
    }

    return(result);
}

// --------------------------------------------------------------------- //
/*!  From full range for all parameters 'source',

        source =  [ ("p1", [10]), ("p2", [2,4,6,8]), ... ]

     return param_ranges_t with just the first element of std::vector<int>:

        [ ("p1", [10]), ("p2", [2]), ... ]
*/
/*
param_ranges_t utils_params::first_param_from_range(const param_ranges_t &source)
{
    param_ranges_t result {};
    for( const auto& elem: source ){
        if( !elem.second.empty() ){
            // retrieve first element
            std::vector<int> first_p {elem.second.at(0)};
            result.push_back( std::make_pair(elem.first, first_p) );
        }
    }
    return(result);
}
*/

// --------------------------------------------------------------------- //
/*!  From full range for all parameters 'source',

        source =  [ ("p1", [10]), ("p2", [2,4,6,8]), ... ]

     return std::vector<parameters_t> with just the first
     element of std::vector<int>:

        [ [ ("p1", 10), ("p2", 2), ... ] ]
*/
std::vector<parameters_t> utils_params::first_parameters_from_range(
                                            const param_ranges_t &par_range )
{
    parameters_t pars {};
    for( const auto& elem: par_range ){
        if( !elem.second.empty() ){
            // retrieve first element
            pars.push_back( std::make_pair(elem.first, elem.second.at(0)) );
        }
    }
    return( std::vector<parameters_t> {pars} );

}

// --------------------------------------------------------------------- //
//  Extract  parameter range vector from 'source' corresponding to
//  name 'par_name'
std::vector<int> utils_params::opt_range_by_name( const std::string &par_name,
                                                const param_ranges_t &source )
{
    std::vector<int> result {};
    // find 'par_name' in source and fill result vector
    for( const auto& elem: source ){
        if( elem.first == par_name ){
            result = elem.second;
            break;
        }
    }
    // check if 'par_name' was found in 'source'
    if( result.empty() ){
        std::cout<< ">>> ERROR: Invalid parameter name (opt_range_by_name)\n";
        exit(1);
    }

    return( result );
}

// --------------------------------------------------------------------- //
/*!  Extract parameter range vector from 'source' corresponding to
     name 'par_name' and replace parameter vector in 'dest'
*/
void utils_params::replace_opt_range_by_name( const std::string &par_name,
                                              const param_ranges_t &source,
                                              param_ranges_t &dest )
{

    // find 'par_name' in source and fill optrange vector
    std::vector<int> optrange { utils_params::opt_range_by_name( par_name,
                                                                 source ) };

    // replace parameter vector in 'dest' with 'optrange'
    bool elem_found {false};
    for( auto& elem: dest ){
        if( elem.first == par_name){
            elem_found = true;
            elem.second = optrange;
            break;
        }
    }
    // check if 'par_name' was found in 'dest'
    if( !elem_found ){
        std::cout<< ">>> ERROR: Parameter name " << par_name
                 << " not found in destination (replace_opt_range_by_name)\n";
        exit(1);
    }

}


// --------------------------------------------------------------------- //
/*!  Extract the parameter values from all strategies in 'source'
     (ignoring the entries with performance metrics)
     and return parameter ranges.
*/
param_ranges_t utils_params::param_ranges_from_all_strategies(
                                    const std::vector<strategy_t> &source )
{
    param_ranges_t result {};

    // Fill vector of single parameters (one entry per strategy)
    std::vector<parameters_t> single_params;
    utils_params::extract_parameters_from_all_strategies(source,single_params);

    for( const auto& strat: single_params ){
        for( const auto& p: strat ){
            // Check if parameter name is already present in result
            auto it = std::find_if( result.begin(), result.end(),
                [&p](const std::pair<std::string,std::vector<int>>& element){
                                        return element.first == p.first;} );

            if( it != result.end() ){   // parameter name already present
                it->second.push_back( p.second );
            }
            else{                       // parameter name not present
                std::vector<int> new_v { p.second };
                result.push_back( std::make_pair( p.first, new_v ) );
            }
        }
    }
    // Remove duplicates in parameter values for each parameter
    for( auto& param: result ){
        std::sort( param.second.begin(), param.second.end() );
        param.second.erase( std::unique( param.second.begin(),
                                         param.second.end() ),
                            param.second.end() );
    }
    return(result);
}

// --------------------------------------------------------------------- //
/*!  Find strategy in 'source' equal to 'ref_strat' except with filter
    'filter_name' equal to 0.
*/
strategy_t utils_params::no_filter_strategy( std::string filter_name,
                                             strategy_t ref_strat,
                                        const std::vector<strategy_t> &source )
{
    strategy_t result {};
    // Parameters of reference strategy
    parameters_t ref_strat_params {};
    utils_params::extract_parameters_from_single_strategy( ref_strat,
                                                           ref_strat_params);
    for( const auto& strat : source ){
        // Parameters of 'strat'
        parameters_t strat_params {};
        utils_params::extract_parameters_from_single_strategy( strat,
                                                               strat_params);
        // Check if 'strat_params' has same size as 'ref_strat_params'
        if( strat_params.size() != ref_strat_params.size() ){
            return(result);
        }

        // Check whether 'strat_params' and 'ref_strat_params' are equal
        // (except for the filter set to 0)
        bool is_equal {false};
        for( int i = 0; i < strat_params.size(); i++ ){
            if(  ( strat_params.at(i).first == filter_name
                && strat_params.at(i).second == 0 )
              || ( strat_params.at(i).first != filter_name
                && strat_params.at(i).second == ref_strat_params.at(i).second)){
                is_equal =  true;
            }
            else{
                is_equal = false;
                break;
            }
        }

        if( is_equal ){
            result = strat;
            break;
        }
    }
    /*
    for( auto el: ref_strat ){
        std::cout<< el.second<<"  ";
    }
    std::cout<<"\n";
    for( auto el: result ){
        std::cout<<el.second<<"  ";
    }
    std::cout<<"\n\n";
    */
    return(result);
}

// --------------------------------------------------------------------- //
/*!  Find strategy in 'source' equal to 'ref_strat' except with two filters
    'filter_name_1', 'filter_name_2' equal to 0.
*/
strategy_t utils_params::no_two_filters_strategy( std::string filter_name_1,
                                                  std::string filter_name_2,
                                                  strategy_t ref_strat,
                                       const std::vector<strategy_t> &source )
{
   strategy_t result {};
   // Parameters of reference strategy
   parameters_t ref_strat_params {};
   utils_params::extract_parameters_from_single_strategy( ref_strat,
                                                          ref_strat_params);
   for( const auto& strat : source ){
       // Parameters of 'strat'
       parameters_t strat_params {};
       utils_params::extract_parameters_from_single_strategy( strat,
                                                              strat_params);
       // Check if 'strat_params' has same size as 'ref_strat_params'
       if( strat_params.size() != ref_strat_params.size() ){
           return(result);
       }

       // Check whether 'strat_params' and 'ref_strat_params' are equal
       // (except for the filter set to 0)
       bool is_equal {false};
       for( int i = 0; i < strat_params.size(); i++ ){
           if(  ( strat_params.at(i).first == filter_name_1
               && strat_params.at(i).second == 0 )
             || ( strat_params.at(i).first == filter_name_2
               && strat_params.at(i).second == 0 )
             || ( strat_params.at(i).first != filter_name_1
               && strat_params.at(i).first != filter_name_2
               && strat_params.at(i).second == ref_strat_params.at(i).second)){
               is_equal =  true;
           }
           else{
               is_equal = false;
               break;
           }
       }

       if( is_equal ){
           result = strat;
           break;
       }
   }
   return(result);
}
