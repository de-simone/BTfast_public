#ifndef TEST_H
#define TEST_H

#include "../Strategies/strategy.h"
#include <array>    // std::array

/*
For a new strategy, follow these steps:

    > in strategy.cpp:
        1. add #include "strategyname.h"
        2. add strategy in select_strategy() function
    > copy template.h -> strategyname.h and edit:
        1. Constructor name
        2. Class definition: Indicators, Initialization of Input Parameters

    > copy template.cpp -> strategyname.cpp and edit:
        1. add #include "strategyname.h"
        2. change name to all methods (before scope resolution ::)
        3. in set_param_values(): set input parameters with same names
                                  as they appear in XML parameter file
        3. edit method: preliminaries() (section 'Update Indicator Values')
        4. edit method: compute_entry()
        5. edit method: compute_exit()

    > copy template.xml -> StrategyName.xml and edit parameters.
      StrategyName should match name in settings.xml.
      ALL parameters in XML need to be INTEGER (int type).

*/


// ------------------------------------------------------------------------- //
/*!

Simple test strategy for comparison with TradeStation and TS-REX

Member Variables:

- name_: name of strategy  (inherited from base class Strategy)
- symbol_: instance of instrument class  (inherited from base class Strategy)
- timeframe_: timeframe  (inherited from base class Strategy)

- digits_: rounding digits of symbol_
- OneBarBeforeClose_: Time of 1 timeframe before session close
                        (close of 2nd-to-last bar)
- CurrentDate_: date of current bar
- CurrentTime_: time of current bar
- CurrentDOW_: day of week of current bar
- MarketPosition_: market position (1: long, -1: short, 0: flat)
- TradingEnabled_: enable/disable trading during session.
                    E.g. forbids multiple trades in same session.
- SessionOpenPrice_: opening price of the current session
- NewSession_: true at the start of a new session (day), otherwise false
- OpenD_, ... , CloseD_: array of OHLC of current and previous 5 sessions
- ROC_: deque with indicator ROC
-----

Strategy Parameters:

- Ncontracts_: Number of contracts to trade
- MyStop_: Stop-Loss in USD per contract
- fract_: Fraction for breakout

*/

class Test : public Strategy {

    int digits_{1};
    Time OneBarBeforeClose_ {};
    Date CurrentDate_ {};
    Time CurrentTime_ {};
    int CurrentDOW_ {0};
    int MarketPosition_ {0};
    bool TradingEnabled_ {true};
    double SessionOpenPrice_ {0.0};
    bool NewSession_ {false};
    std::array<double, 6> OpenD_ {};
    std::array<double, 6> HighD_ {};
    std::array<double, 6> LowD_ {};
    std::array<double, 6> CloseD_ {};


    // ---     Indicators     --- //
    std::deque<double> ROC_ {};
    // -------------------------- //

    // --- Initialization of Input Parameters --- //
    //  (default values, may be replaced by XML)  //
    int Ncontracts_ {1};
    int MyStop_ {0};
    int fractN_ {2};
    // ------------------------------------------ //


    public:
        // Constructor
        Test( std::string name, Instrument symbol,
              std::string timeframe, int max_bars_back );

    private:
        // Functions overriding the base class virtual functions

        int digits() const override { return(digits_); }

        // Set values of parameters (for optimization)
        void set_param_values(
                const std::vector< std::pair<std::string,int> >&
                        parameter_set ) override;
        // Variable definitions and preliminary calculations
        int preliminaries( const std::deque<Event>& data1,
                           const std::deque<Event>& data1D,
                           const PositionHandler& position_handler ) override;
        // Define Entry signals
        void compute_entry( const std::deque<Event>& data1,
                            const std::deque<Event>& data1D,
                            const PositionHandler& position_handler,
                            std::array<Event, 2> &signals ) override;
        // Define Exit signals
        void compute_exit( const std::deque<Event>& data1,
                           const std::deque<Event>& data1D,
                           const PositionHandler& position_handler,
                           std::array<Event, 2> &signals ) override;
        // Handle computation of Entry/Exit signals
        void compute_signals( const PriceCollection& price_collection,
                              const PositionHandler& position_handler,
                              std::array<Event, 2> &signals ) override;

};




#endif
