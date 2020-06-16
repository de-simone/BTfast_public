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
    //<<<
}
