#include "btfast.h"

#include "genetic.h"    // gene_t, chromosome_t type aliases
#include "utils_fileio.h"      // write_strategies_to_file
#include "utils_optim.h"      //  sort_by_metric
#include "utils_time.h"     // current_datetime_str

#include <iostream>     // std::cout


//-------------------------------------------------------------------------- //
/*! Run Parallelized Genetic Optimizationover over 'search_space'
    combinations.
    Results stored into 'optim_results' and written to 'optim_file'.

    search_space: combination of parameters to run optimization over (input)
            (not const because modified in Population::initialize_population)
    optim_results: vector where storing optimization resus (metrics + params)
    paramfile: XML file with strategy parameter ranges/value.
    optim_file: file where optimization results are written
    fitness_metric: used to sort optimization results in descending order
    datafeed: smart pointer to DataFeed object
    population_size: size of population
    generations: number of generations

*/

void BTfast::run_genetic_optimization( std::vector<parameters_t> &search_space,
                                       std::vector<strategy_t> &optim_results,
                                       const std::string &optim_file,
                                       const std::string &paramfile,
                                       const std::string &fitness_metric,
                                       std::unique_ptr<DataFeed> &datafeed,
                                       int population_size, int generations )
{
    // disable printing number of bars parsed
    print_progress_ = false;

    // Set up probabilities for genetic operations
    double crossover_rate { 0.9 };
    double mutation_rate { 0.1 };
    // Number of consecutive generations with same total fitness causing
    // early exit of GA (0 to disable)
    int early_exit_generations {0};
    // number of elite individuals passing unchanged
    int elite_num {2};


    if( population_size > search_space.size() ){
        std::cout << ">>> ERROR: population size must be smaller than "
                  << "search space dimension " << search_space.size() <<"\n";
        exit(1);
    }
    if( population_size*generations > search_space.size() ){
        std::cout << ">>> ERROR: number of genetic runs "
                  << population_size*generations
                  << " must be smaller than exhaustive runs "
                  << search_space.size()
                  << ".\nChange RUN_MODE to exhaustive optimization.\n";
        exit(1);
    }

    // Number of consecutive generations with same total fitness
    int consec_gens_same_fitness {1};

    //-- Initial population (1st generation)
    std::cout << utils_time::current_datetime_str() + " | "
              << "Start Generation 1 / " << generations << "\n";

    Population population {population_size, fitness_metric};
    population.initialize_population(search_space);
    population.compute_population_fitness(*this, datafeed, optim_results);
    //population.print_population();

    // fitness of best individual in previous and current generations
    double best_fitness_prev { population.population()[0].fitness() };
    double best_fitness_curr { population.population()[0].fitness() };

    std::cout << utils_time::current_datetime_str() + " | "
              << "End   Generation 1 / "
              << generations << "\t"
              << "Best Fitness = " << best_fitness_curr << "\n\n";
    //--

    //--- Loop over successive generations
    for( int generation = 2; generation <= generations; generation++ ){

        std::cout << utils_time::current_datetime_str() + " | "
                  << "Start Generation " << generation
                  << " / " << generations << "\n";

        Population new_population {population_size, fitness_metric};

        //--- Create new population for next generation
        // Elitism
        // (the 'elite_num' fittest individuals pass unchanged to new generation)
        for( int j = 0; j < elite_num; j++){
            new_population.insert_individual( population.population()[j] );
        }

        while( new_population.population().size() < population_size ){

            // Selection
            Individual offspring1 { population.select() };
            Individual offspring2 { population.select() };
            int selection_trials {0};
            // enforce that the parents are different (max population_size trials)
            while( offspring2.chromosome() == offspring1.chromosome()
                   && selection_trials < population_size ){
                offspring2 = population.select();
                selection_trials++;
            }

            // Crossover
            /*
            //- single crossover
            offspring1.single_crossover( offspring2, crossover_rate );
            new_population.insert_individual( offspring1 );
            new_population.insert_individual( offspring2 );
            //-
            */
            //- uniform crossover
            crossover_rate = 0.0; //dummy
            offspring1.uniform_crossover( offspring2 );
            new_population.insert_individual( offspring1 );
            //-
        }
        //---

        // Mutation
        new_population.mutate( search_space, mutation_rate, elite_num );

        new_population.compute_population_fitness(*this, datafeed, optim_results);
        //new_population.print_population();

        best_fitness_prev = population.population()[0].fitness() ;
        best_fitness_curr = new_population.population()[0].fitness() ;

        // Current population is the new one
        population = new_population;

        std::cout << utils_time::current_datetime_str() + " | "
                  << "End   Generation " << generation
                  << " / " << generations << "\t"
                  << "Best Fitness = " << best_fitness_curr << "\n\n";

        //-- Check early exit (if no improvement in fitness)
        if( best_fitness_curr == best_fitness_prev ){
            consec_gens_same_fitness += 1;
        }
        else{
            consec_gens_same_fitness = 1;
        }
        if( early_exit_generations != 0
            && consec_gens_same_fitness >= early_exit_generations ){
            std::cout << "Early exit after " << early_exit_generations
                      << " generations without improvement.\n";
            break;
        }
        //--
    }
    //--- End loop over generations

    // Sort in descending order of fitness_metric
    utils_optim::sort_by_metric( optim_results, fitness_metric );

    // Write optimization results to file 'optim_file'
    int control = utils_fileio::write_strategies_to_file(
                                            optim_file, paramfile,
                                            optim_results, strategy_name_,
                                            symbol_.name(), timeframe_,
                                            first_date_parsed_,
                                            last_date_parsed_, true );
    if( control == 1 ){
        std::cout << "\nOptimization results written on file: "
                  << optim_file <<"\n";
    }
}
