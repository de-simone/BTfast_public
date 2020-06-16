#ifndef UTILS_PARAMS_H
#define UTILS_PARAMS_H

#include "btfast.h"         // strategy_t, parameters_t, param_ranges_t


//#include <queue>            // std::queue


// Set of Utility functions, for handling parameters containers


namespace utils_params {

    // --------------------------------------------------------------------- //
    /*!  Print content of parameters_t type
        [ ("p1", 2), ("p2", 7), ... ]
    */
    void print_parameters_t( const parameters_t& p );

    // --------------------------------------------------------------------- //
    /*!  Print content of vector of parameters_t type
        [ [ ("p1", 2), ("p2", 7), ... ],
          [ ("p1", 4), ("p2", 11), ... ], ... ]
    */
    void print_parameters_t_vector( const std::vector<parameters_t>& v );

    // --------------------------------------------------------------------- //
    /*!  Print content of param_ranges_t type
        [ ("p1", [10]), ("p2", [2,4,6,8]), ... ]
    */
    void print_param_ranges_t( const param_ranges_t& par_range );

    // --------------------------------------------------------------------- //
    /*!  From vector v (as returned by utils_fileio::read_param_file() ),

            v =  [ ("p1", [10]), ("p2", [2,4,6,8]), ... ]

         extract first element of std::vector<int>:

            [ ("p1", 10), ("p2", 2), ... ]

         Used to get single set of parameter values read from XML:
            for <Input> parameters:     single fixed value
            for <OptRange> parameters:  start value only
    */
    parameters_t single_parameter_combination( const param_ranges_t &v );

    // --------------------------------------------------------------------- //
    /*!  From vector v (as returned by utils_fileio::read_param_file() ),

            v =  [ ("p1", [10]), ("p2", [2,4,6,8]), ... ]

         return Cartesian product of all parameter combinations

            [ [("p1", 10), ("p2", 2), ...], [("p1", 10), ("p2", 4), ...] ]
    */
    std::vector<parameters_t> cartesian_product( param_ranges_t &v );

    // --------------------------------------------------------------------- //
    /*!  Extract only the parameter values from all strategies in 'source'
         (ignoring the entries with performance metrics)
         and cast them double -> int. Copy the result into 'dest'.
    */
    void extract_parameters_from_all_strategies(
                                        const std::vector<strategy_t> &source,
                                        std::vector<parameters_t> &dest );

    // --------------------------------------------------------------------- //
    /*!  Extract only the parameter values from strategy 'source'
         (ignoring the entries with performance metrics)
         and cast them double -> int. Copy the result into 'dest'.
    */
    void extract_parameters_from_single_strategy( const strategy_t &source,
                                                   parameters_t &dest );


    // --------------------------------------------------------------------- //
    /*!  Extract parameter range vector from 'par_range' corresponding to
         name 'par_name' and expand strategy 'source' with
         'par_name' replaced by optimization range.
         Output vector replaces 'base_strats'
    */
    void expand_strategies_with_opt_range(
                                    const std::string &par_name,
                                    const param_ranges_t &par_range,
                                    std::vector<parameters_t> &base_strats);

    // --------------------------------------------------------------------- //
    /*! Extract attribute (metric or paramter) named 'attr_name'
        from single strategy 'source'
    */
    double strategy_attribute_by_name( const std::string &attr_name,
                                       const strategy_t &source );

    // --------------------------------------------------------------------- //
    /*! Set parameter value in 'parameters' corresponding to name 'par_name'
        to value 'new_value'
    */
    void set_parameter_value_by_name( const std::string &par_name,
                                      parameters_t& parameters, int new_value );

    // --------------------------------------------------------------------- //
    /*! Get first parameter value from 'source' corresponding to
        name 'par_name'
    */
    int parameter_value_by_name( const std::string &par_name,
                                 const param_ranges_t &source );

    // --------------------------------------------------------------------- //
    /*!  From full range for all parameters 'source',

            source =  [ ("p1", [10]), ("p2", [2,4,6,8]), ... ]

         return param_ranges_t with just the first element of std::vector<int>:

            [ ("p1", [10]), ("p2", [2]), ... ]
    */
    //param_ranges_t first_param_from_range( const param_ranges_t &source );

    // --------------------------------------------------------------------- //
    /*!  From full range for all parameters 'source',

            source =  [ ("p1", [10]), ("p2", [2,4,6,8]), ... ]

         fill 'dest' with just the first element of std::vector<int>:

            [ [ ("p1", 10), ("p2", 2), ... ] ]
    */
    std::vector<parameters_t> first_parameters_from_range(
                                            const param_ranges_t &par_range );

    // --------------------------------------------------------------------- //
    /*!  Extract parameter range vector from 'source' corresponding to
         name 'par_name' and replace parameter vector in 'dest'
    */
    void replace_opt_range_by_name( const std::string &par_name,
                                    const param_ranges_t &source,
                                    param_ranges_t &dest );

    // --------------------------------------------------------------------- //
    /*!  Extract the parameter values from all strategies in 'source'
         (ignoring the entries with performance metrics)
         and return parameter ranges.
    */
    param_ranges_t param_ranges_from_all_strategies(
                                        const std::vector<strategy_t> &source );

    // --------------------------------------------------------------------- //
    /*!  Find strategy in 'source' equal to 'ref_strat' except with filter
        'filter_name' equal to 0.
    */
    strategy_t no_filter_strategy( std::string filter_name,
                                   strategy_t ref_strat,
                                   const std::vector<strategy_t> &source );


    // --------------------------------------------------------------------- //
    /*!  Extract parameter range vector from all strategies in 'source'
         name 'par_name' and replace parameter vector in 'dest'
    */
    /*
    void extract_parameter_ranges_from_all_strategies(
                                    const std::vector<strategy_t> &source,
                                    param_ranges_t &dest );
    */
    // --------------------------------------------------------------------- //
    /*!  Extract parameter range vector from 'source' corresponding to
         name 'par_name' and make a new parameter vector in 'dest' where
         'par_name' is the only optimization parameter
         (all others are initialized with their starting value)
    */
    /*
    void extract_single_opt_range_by_name( const std::string &par_name,
                                           const param_ranges_t &source,
                                           param_ranges_t &dest );
    */
}



#endif
