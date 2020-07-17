#include "run_modes.h"

#include "account.h"
#include "utils_params.h"   // single_parameter_combination

//#include "utils_math.h"
#include "utils_time.h"     // current_datetime_str

#include <iostream>         // std::cout

// ------------------------------------------------------------------------- //
/*! No trade
*/
void mode_notrade( BTfast &btf, std::unique_ptr<DataFeed> &datafeed,
                   const param_ranges_t &parameter_ranges )
{
    std::cout<< "    Run Mode   : No trade\n\n";
    std::cout << utils_time::current_datetime_str() + " | "
              << "Running in no-trade mode \n";
    // Extract single parameter combination from parameter_ranges
    // (only the <Start> value is taken)
    parameters_t parameter_combination {
        utils_params::single_parameter_combination(parameter_ranges) };
    // Initialize Account
    Account account { btf.initial_balance() };
    // Parse data without strategy signals
    btf.run_notrade( account, datafeed, parameter_combination );

    //<<< tests
    //std::vector<double> v({1,2,3,4,5,6,7,8,9,10});
    //std::vector<double> w({1.1, 2.1, 3.1, 4.1, 5.1, 6.1, 7.1, 8.1, 9.1, 10.1});
    //utils_math::mannwhitney(v, w);

    std::string optim_param_name {"fractN_short"};
    std::vector<int> optimization_range {
                    utils_params::opt_range_by_name( optim_param_name,
                                                     parameter_ranges ) };
    param_ranges_t param_ranges {};
    // Fill 'param_ranges'...
    for( const auto& el: parameter_combination ){
        if( el.first != optim_param_name ){     //  ... with parameters != optim param
            param_ranges.push_back( std::make_pair(
                                el.first, std::vector<int>{el.second} ) );
        }
        else if( el.first == optim_param_name ){ // ... with optim param
            param_ranges.push_back( std::make_pair(
                                optim_param_name, optimization_range ) );
        }
        else{
          std::cout<<">>> ERROR: parameter not found "
                   <<"in strategy parameters (validation).\n";
          exit(1);
        }
    }

    // Cartesian product of all parameter ranges
    std::vector<parameters_t> search_space {
                    utils_params::cartesian_product(param_ranges) };

    utils_params::print_parameters_t_vector(search_space);

    std::vector<parameters_t> search_space_tmp {parameter_combination};
    utils_params::expand_strategies_with_opt_range(
                                        optim_param_name,
                                        parameter_ranges,
                                        search_space_tmp);
    utils_params::print_parameters_t_vector(search_space_tmp);
    //<<<




}
