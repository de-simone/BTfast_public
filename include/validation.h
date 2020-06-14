#ifndef VALIDATION_H
#define VALIDATION_H

#include "btfast.h"     // strategy_t alias
#include "instruments.h"

#include <vector>   // std::vector

/*!
Class for Selection and Validation of strategies


Member Variables
- num_validated_: number of strategies passing validation
- btf_: BTfast object passed by const ref from main

- strategies_to_validate_: input strategies to feed into validation process
                          (single strategy, or bunch from optimization or file)
- selected_file_: file to store strategies passing selection
- fitness_metric: performance metric used to compare OOS with IS
- max_variation_: max variation of performance metric allowed by stability test
- num_noise_tests_: number of randomization tests when adding noise

[- parameter_ranges_: ranges of all strategy parameters]
[- date_i_: initial selected date to parse]
[- date_f_: final selected date to parse]
*/



// ------------------------------------------------------------------------- //
// Class for Strategy Selection and Validation

class Validation {

    int num_validated_ {0};
    BTfast &btf_;
    std::unique_ptr<DataFeed> &datafeed_;
    //const param_ranges_t &parameter_ranges_;
    const std::vector<strategy_t> &strategies_to_validate_;
    const std::string &selected_file_;
    const std::string &validated_file_;
    const std::string &fitness_metric_;
    const std::string &data_dir_;
    const std::string &data_file_oos_;
    //std::string data_file_oos_path_ {""};
    double max_variation_ { 0.3 };
    int num_noise_tests_;
    const std::string &noise_file_;

    //Date date_i_ {};
    //Date date_f_ {};
    //double OOSfraction_ {0.0};


    public:
        // constructor
        Validation( BTfast &btf,
                    std::unique_ptr<DataFeed> &datafeed,
                    const std::vector<strategy_t> &strategies_to_validate,
                    const std::string &selected_file,
                    const std::string &validated_file,
                    const std::string &fitness_metric,
                    const std::string &data_dir,
                    const std::string &data_file_oos,
                    int max_variation_pct, int num_noise_tests,
                    const std::string &noise_file );

        // full validation process
        void run_validation();

        void selection( const std::vector<strategy_t> &input_strategies,
                        std::vector<strategy_t> &output_strategies );
        void initial_generation_selection(
                        const std::vector<strategy_t> &input_strategies,
                        std::vector<strategy_t> &output_strategies );
        void selection_conditions(
                            const std::vector<strategy_t> &input_strategies,
                            std::vector<strategy_t> &output_strategies );
        void OOS_metrics_test( const std::vector<strategy_t> &input_strategies,
                               std::vector<strategy_t> &output_strategies );
        void OOS_consistency_test( const std::vector<strategy_t> &input_strategies,
                                   std::vector<strategy_t> &output_strategies );
        void profitability_test( const std::vector<strategy_t> &input_strategies,
                                 std::vector<strategy_t> &output_strategies,
                                 const std::string &optim_param_name,
                                 int start, int stop, int step );
        void stability_test( const std::vector<strategy_t> &input_strategies,
                             std::vector<strategy_t> &output_strategies );
        void noise_test( const std::vector<strategy_t> &input_strategies,
                         std::vector<strategy_t> &output_strategies );


        //void selection_conditions_OOS( const std::vector<strategy_t> &OOS_run);

        // Getters
        int num_validated() const { return( num_validated_ ); }

};






#endif
