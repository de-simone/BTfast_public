#ifndef STRATEGY_H
#define STRATEGY_H

#include "events.h"
#include "instruments.h"
#include "position_handler.h"
#include "price_collection.h"

#include <array>        // std::array
#include <deque>        // std::deque
#include <memory>       // std::unique_ptr, std::make_unique
#include <string>       // std::string
#include <vector>       // std::vector





// ------------------------------------------------------------------------- //
/*!
Abstract base class for all strategies

Member Variables:
- name_: name of strategy
- symbol_: instance of instrument class.
- timeframe_: timeframe
- max_bars_back_: max number of values stored in each indicator (if any)

*/

class Strategy {

    protected:
        std::string name_ {""};
        Instrument symbol_ {};
        std::string timeframe_ {""};
        int max_bars_back_ {100};


    public:
        // Constructor
        Strategy( std::string name, Instrument symbol,
                  std::string timeframe, int max_bars_back );
        // Pure virtual Destructor (requires a function body)
        virtual ~Strategy() = 0;

        // Getters
        std::string name() const { return(name_); };
        Instrument symbol() const { return(symbol_); };
        std::string timeframe() const { return(timeframe_); }

        // Find parameter value by its parameter name, in the parameter set
        int find_param_value_by_name( std::string p_name,
                    const std::vector< std::pair<std::string,int> >&
                                        parameter_set);

        // pure virtual functions (overridden by derived strategies)
        virtual int digits() const = 0;

        virtual void set_param_values(
                    const std::vector< std::pair<std::string,int> >&
                                        parameter_set ) = 0;
        virtual int preliminaries( const std::deque<Event>& data1,
                                   const std::deque<Event>& data1D,
                                   const PositionHandler& position_handler )=0;
        virtual void compute_entry( const std::deque<Event>& data1,
                                    const std::deque<Event>& data1D,
                                    const PositionHandler& position_handler,
                                    std::array<Event, 2> &signals) = 0;
        virtual void compute_exit( const std::deque<Event>& data1,
                                   const std::deque<Event>& data1D,
                                   const PositionHandler& position_handler,
                                   std::array<Event, 2> &signals ) = 0;
        virtual void compute_signals(
                                const PriceCollection& price_collection,
                                const PositionHandler& position_handler,
                                std::array<Event, 2> &signals ) = 0;
        /*
        virtual Event compute_entry(const std::deque<Event>& data1,
                                    const std::deque<Event>& data1D,
                                    const PositionHandler& position_handler )=0;
        virtual Event compute_exit(const std::deque<Event>& data1,
                                   const std::deque<Event>& data1D,
                                   const PositionHandler& position_handler)=0;
        virtual Event compute_signals(
                                const PriceCollection& price_collection,
                                const PositionHandler& position_handler) = 0;
        */
};





// ------------------------------------------------------------------------- //
/*! Instantiate strategy corresponding to 'strategy_name' and
    assign unique_ptr to that object.
*/
void select_strategy( std::unique_ptr<Strategy>& strategy_ptr,
                      std::string strategy_name, const Instrument &symbol,
                      std::string timeframe, int max_bars_back );


#endif
