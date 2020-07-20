#include "strategy.h"

#include "bomrcharacter.h"
#include "mastercode.h"
#include "ng1.h"
#include "ng2.h"
#include "ng3.h"
#include "ng4.h"
#include "ng5.h"
#include "ng6.h"
#include "template.h"
#include "test.h"

#include <cstdlib>      // exit
#include <iostream>     // std::cout
//#include <stdexcept>   // std::runtime_error

// ------------------------------------------------------------------------- //
/*! Constructor
*/
Strategy::Strategy( std::string name, Instrument symbol,
                    std::string timeframe, int max_bars_back )
:  name_{name}, symbol_{symbol},
   timeframe_{timeframe}, max_bars_back_{max_bars_back}
{}

// ------------------------------------------------------------------------- //
/*! Body for Pure virtual Destructor
*/
Strategy::~Strategy(){}


// ------------------------------------------------------------------------- //
/*! Find parameter value by its parameter name (as appear in XML file),
    in the parameter combination 'parameter_set'

    'parameter_set' is a single set of values for all parameters, e.g.:
            [ ("p1", 2), ("p2", 10), ... ]

    Return 0 if name not found.
*/
int Strategy::find_param_value_by_name( std::string p_name,
            const std::vector< std::pair<std::string,int> >& parameter_set){

    for( auto it=parameter_set.begin(); it!=parameter_set.end(); it++ ){
        if( p_name == (*it).first ){
            return( (*it).second );
        }
    }
    std::cout << "Parameter " + p_name +" not found in parameter combination. "
              << "Check XML parameter file (Strategy::find_param_value_by_name).\n";
    exit(1);

}






// ------------------------------------------------------------------------- //
/* Instantiate strategy corresponding to 'strategy_name' and
   assign unique_ptr to that object.
*/
void select_strategy( std::unique_ptr<Strategy>& strategy_ptr,
                      std::string strategy_name, const Instrument &symbol,
                      std::string timeframe, int max_bars_back )
{


    if( strategy_name == "template" ){
        strategy_ptr = std::make_unique<Template> ( strategy_name,
                                                    symbol, timeframe,
                                                    max_bars_back );
    }
    else if( strategy_name == "BOMRcharacter" ){
        strategy_ptr = std::make_unique<BOMRcharacter> ( strategy_name,
                                                         symbol, timeframe,
                                                         max_bars_back );
    }
    else if( strategy_name == "MasterCode" ){
        strategy_ptr = std::make_unique<MasterCode> ( strategy_name,
                                                      symbol, timeframe,
                                                      max_bars_back );
    }
    else if( strategy_name == "NG1" ){
        strategy_ptr = std::make_unique<NG1> ( strategy_name,
                                               symbol, timeframe,
                                               max_bars_back );
    }
    else if( strategy_name == "NG2" ){
        strategy_ptr = std::make_unique<NG2> ( strategy_name,
                                               symbol, timeframe,
                                               max_bars_back );
    }
    else if( strategy_name == "NG3" ){
        strategy_ptr = std::make_unique<NG3> ( strategy_name,
                                               symbol, timeframe,
                                               max_bars_back );
    }
    else if( strategy_name == "NG4" ){
        strategy_ptr = std::make_unique<NG4> ( strategy_name,
                                               symbol, timeframe,
                                               max_bars_back );
    }
    else if( strategy_name == "NG5" ){
        strategy_ptr = std::make_unique<NG5> ( strategy_name,
                                               symbol, timeframe,
                                               max_bars_back );
    }
    else if( strategy_name == "NG6" ){
        strategy_ptr = std::make_unique<NG6> ( strategy_name,
                                               symbol, timeframe,
                                               max_bars_back );
    }
    else if( strategy_name == "test" ){
        strategy_ptr = std::make_unique<Test> ( strategy_name,
                                                symbol, timeframe,
                                                max_bars_back );
    }












  else{
      std::cout << ">>> ERROR: invalid strategy name (select_strategy).\n";
      exit(1);
  }
}
