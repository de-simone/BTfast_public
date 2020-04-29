#ifndef UTILS_PARAMS_H
#define UTILS_PARAMS_H

#include "btfast.h"         // strategy_t, parameters_t, param_ranges_t


//#include <queue>            // std::queue


// Set of Utility functions, for handling parameters containers


namespace utils_params {

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

}



#endif
