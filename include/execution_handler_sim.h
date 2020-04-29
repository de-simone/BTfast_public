#ifndef EXECUTION_HANDLER_SIM_H
#define EXECUTION_HANDLER_SIM_H

#include "execution_handler.h"


/*!
Class providing simulated (ideal) execution, without broker.

Member Variables
- include_commissions_: switch to include commission costs in fill event
- slippage_: max number of ticks for slippage on entry/exit

*/



class SimulatedExecution : public ExecutionHandler {

    bool include_commissions_{false};
    int slippage_ {0};
    //bool with_commissions_ {false};


    public:
        // Constructor
        SimulatedExecution( bool include_commissions = false,
                            int slippage = 0 );


    private:
        // Functions overriding the base class pure virtual functions
        bool include_commissions() const override { return(include_commissions_); }
        int slippage() const override { return(slippage_); }

        void on_order( const Event &order ) const override;

};




#endif
