#ifndef UTILS_OPTIM_H
#define UTILS_OPTIM_H

#include "btfast.h"         // strategy_t alias
#include "performance.h"
//#include "position_handler.h"


// Set of Utility functions for optimization


namespace utils_optim {

    // --------------------------------------------------------------------- //
    // Append to 'optim' the performance metrics and parameter combination of
    // an optimization run (stored in 'perform' and 'parameters')

    void append_to_optim_results(std::vector<strategy_t> &optim,
                                const Performance& perform,
                                const parameters_t& parameters ) ;

    // Binary predicate function to compare two strategy_t objects
    bool equal_strategies( const strategy_t &a, const strategy_t &b );
    // Binary predicate function to compare the metrics of two strategy_t objects    
    bool equal_strategy_metrics( const strategy_t &a, const strategy_t &b );
    // Binary predicate function to compare two parameters_t objects
    //bool equal_strategy_params( const parameters_t &a, const parameters_t &b );

    // Remove duplicates from 'strategies' vector (in-place), sort by metric
    void remove_duplicates( std::vector<strategy_t> &strategies,
                            const std::string &metric );
    // Remove duplicates from parameters (modified in-place)
    //void remove_duplicates( std::vector<parameters_t> &parameters );

    // --------------------------------------------------------------------- //
    // Binary functions for sorting optimization results in descending order
    // of different metrics
    bool sort_by_ntrades(const strategy_t& a, const strategy_t& b);
    //bool sort_by_netpl(const strategy_t& a, const strategy_t& b);
    bool sort_by_avgticks(const strategy_t& a, const strategy_t& b);
    //bool sort_by_avgtrade(const strategy_t& a, const strategy_t& b);
    bool sort_by_profitfactor(const strategy_t& a, const strategy_t& b);
    bool sort_by_winperc(const strategy_t& a, const strategy_t& b);
    bool sort_by_npmdd(const strategy_t& a, const strategy_t& b);
    bool sort_by_expectancy(const strategy_t& a, const strategy_t& b);
    bool sort_by_zscore(const strategy_t& a, const strategy_t& b);

    // --------------------------------------------------------------------- //
    // Sort optimizaiton results by fitness metric
    void sort_by_metric( std::vector<strategy_t> &optim, std::string metric );
}


#endif
