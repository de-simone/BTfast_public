#include "genetic.h"

#include "utils_optim.h" // append_to_optim_results
#include "utils_random.h" // rand_generator
//#include "utils_time.h"
#include <algorithm>    // std::sort, std::shuffle, std::min_element, std::max_element
#include <functional>   // std::bind, std::placeholders
#include <iostream>     // std::cout
#include <numeric>      // std::accumulate
#include <omp.h>        // openMP


// ------------------------------------------------------------------------- //
/*! Constructor
*/
Individual::Individual(chromosome_t chromosome)
: chromosome_{chromosome}
{};


// ------------------------------------------------------------------------- //
/*! String representation of chromosomes
*/
std::string Individual::tostring()
{
    std::string result{"{"};
    for(auto gene : chromosome_ ){
        result += gene.first + ": " + std::to_string(gene.second) + ", ";
    }
    result += "}";
    return(result);
}

// ------------------------------------------------------------------------- //
/*! Overloading < operator (to use in std::sort)

bool operator<(const Individual &ind1, const Individual &ind2)
{
    return ind1.fitness() < ind2.fitness();
}
*/

// ------------------------------------------------------------------------- //
/*! Calculate fittness score of single individual
   and assign it to fitness_ member variable.
   Append performance+parameters to 'optim_results'.

   Names/Number/Order of performance metrics must be matched among:
       - utils_params::extract_parameters_from_single_strategy
       - utils_optim::append_to_optim_results
       - utils_optim::sort_by_metric
       - utils_optim::sort_by_ntrades, utils_optim::sort_by_avgtrade, etc
       - utils_fileio::write_strategies_to_file
       - Individual::compute_individual_fitness
       - Validation::intermediate_selection
       - Validation::selection_conditions
       - mode_factory_sequential (run_modes)

*/
void Individual::compute_individual_fitness(BTfast &btf,
                                        std::unique_ptr<DataFeed> &datafeed,
                                        std::string metric,
                                        std::vector<strategy_t> &optim_results )
{
    // Initialize Account
    Account account { btf.initial_balance() };

    // Run backtest (passing strategy parameters of current individual)
    btf.run_backtest( account, datafeed, this->chromosome() );

    // Initialize Performance object
    Performance performance { btf.initial_balance(), btf.day_counter(),
                              std::vector<Transaction> {} };
    // Load transaction history into performance object
    performance.set_transactions( account.transactions() );
    // Compute performance metrics
    performance.compute_metrics();

    // Set fitness according to input fitness metric
    //if( metric == "NetPL" ){
    //    fitness_ = performance.netpl();
    //}
    if( metric == "AvgTicks" ){
        fitness_ = performance.avgticks();
    }
    else if( metric == "WinPerc" ){
        fitness_ = performance.winperc();
    }
    //else if( metric == "AvgTrade" ){
    //    fitness_ = performance.avgtrade();
    //}
    else if( metric == "ProfitFactor" ){
        fitness_ = performance.profitfactor();
    }
    else if( metric == "NP/MDD" ){
        fitness_ = performance.npmdd();
    }
    else if( metric == "Expectancy" ){
        fitness_ = performance.expectancy();
    }
    else if( metric == "Z-score" ){
        fitness_ = performance.zscore();
    }
    else{   // default: AvgTicks
        fitness_ = performance.avgticks();
    }

    // Append performance metrics and parameter combination (chromosome)
    // to optimization results
    #pragma omp critical
    {
        utils_optim::append_to_optim_results(optim_results, performance,
                                             chromosome_);
    }
};


// ------------------------------------------------------------------------- //
/*! Perform single-point crossover of two parents (-> 2 offsprings)
*/
void Individual::single_crossover(Individual &parent2, double crossover_rate )
{
    Individual offspring1 {*this};
    Individual offspring2 {parent2};
    int genes_num { static_cast<int>(chromosome_.size()) };

    std::uniform_int_distribution<> rand_int {0, genes_num-1};
    std::uniform_real_distribution<> rand01 {0.0,1.0};
    double p = rand01(utils_random::rand_generator);  // random real in [0,1]

    if( p < crossover_rate ){
        // index point after which to crossover chromosomes
        int r = rand_int(utils_random::rand_generator); // random int in [0,1]

        // Swap genes after crossover point
        for(int i = r; i < genes_num; i++){
                gene_t temp = this->chromosome()[i];
                this->set_chromosome(i, offspring2.chromosome()[i] );
                parent2.set_chromosome(i, temp);
        }
    }
};

// ------------------------------------------------------------------------- //
/*! Perform uniform crossover of two parents (-> 1 offspring (this) )
*/
void Individual::uniform_crossover(Individual &parent2)
{
    std::uniform_int_distribution<> rand_int {0,1};

    for(int i = 0; i < chromosome_.size(); i++){

        int r { rand_int(utils_random::rand_generator) };// random int in [0,1]
        if( r == 1 ){
            // insert gene from parent 2
            this->set_chromosome(i, parent2.chromosome()[i] );
        }
        // otherwise keep gene from parent 1 (this)
    }
};


// ------------------------------------------------------------------------- //
/*! Mutate a random gene in this individual
*/
void Individual::mutate( const std::vector<chromosome_t> &search_space )
{
    int genes_num  { static_cast<int>(chromosome_.size()) };
    int param_size { static_cast<int>(search_space.size()) };

    std::uniform_int_distribution<> rand_int_1 (0, genes_num-1);
    std::uniform_int_distribution<> rand_int_2 (0, param_size-1);

    // random integer in [0, chromosome.size-1 )
    int r1 { rand_int_1(utils_random::rand_generator) };
    // random integer in [0, search_space.size-1 )
    int r2 { rand_int_2(utils_random::rand_generator) };

    int mutation_trials {0};
    // enforce that the gene mutates to different value
    // (at most 2*genes_num trials)
    while( chromosome_[r1] == search_space[r2][r1]
            && mutation_trials < 2*genes_num ){
        r1 = rand_int_1(utils_random::rand_generator);
        r2 = rand_int_2(utils_random::rand_generator);
        mutation_trials++;
    }
    chromosome_[r1] = search_space[r2][r1];
    //chromosome_.at(r1) = search_space.at(r2).at(r1);

}




// ------------------------------------------------------------------------- //
// ------------------------------------------------------------------------- //
// ------------------------------------------------------------------------- //



// ------------------------------------------------------------------------- //
/*! Constructor
*/
Population::Population( int population_size, std::string fitness_metric )
: population_size_{population_size}, fitness_metric_{fitness_metric}
{}


// ------------------------------------------------------------------------- //
/*! Fill 'population_' vector by random sampling 'population_size_' elements
  (without replacement) from whole parameter space 'search_space'
*/
void Population::initialize_population(std::vector<chromosome_t> &search_space)
{
    total_fitness_ = 0;

    // Shuffle the search space
    std::shuffle( search_space.begin(), search_space.end(),
                  utils_random::rand_generator );

    // Get the first 'population_size_' values
    std::vector<chromosome_t> population_chromosomes{
                                            search_space.begin(),
                                            search_space.begin() + population_size_};

    /* // Random sample with replacement
    std::vector<chromosome_t> population_chromosomes;
    std::sample(search_space.begin(), search_space.end(),
                std::back_inserter(population_chromosomes),
                population_size_, utils_random::rand_generator);  */

    // Fill population_ with individuals having population_chromosomes
    population_ = std::vector<Individual> {};
    for( auto chromosome: population_chromosomes ){
        population_.push_back( Individual{chromosome} );
    }
}

// ------------------------------------------------------------------------- //
/*! Print name and value of all chromosome in population's individuals.
*/
void Population::print_population()
{
    for(Individual indiv: population_){
        std::cout<< "Fitness = " << indiv.fitness()
                 << ", Prob = "<< indiv.probability()
                 << " " << indiv.tostring() <<"\n";
    }
}

// ------------------------------------------------------------------------- //
/*! Binary function for sorting in descending order of fitness
*/
bool Population::sort_by_fitness(const Individual& a, const Individual& b)
{
    return( a.fitness() > b.fitness() );
}

// ------------------------------------------------------------------------- //
/*! Sort the population in decreasing order of fitness score
*/
void Population::sort()
{
    std::sort(population_.begin(), population_.end(),
               std::bind(&Population::sort_by_fitness, this,
                         std::placeholders::_1, std::placeholders::_2) );
}

// ------------------------------------------------------------------------- //
/*! Sum the fitness of all individuals in population
*/
void Population::set_total_fitness()
{
    total_fitness_ = 0.0;
    for( Individual indiv: population_ ){
        total_fitness_ += indiv.fitness();
    }

    if( total_fitness_ == 0 ){
        std::cout<<">>>ERROR: population fitness = 0. No trades generated."
                 <<" (genetic).\n";
        exit(1);
    }
}

// ------------------------------------------------------------------------- //
/*! Set the probabilities of each individual in population
    Sum of probabilites over population = 1
*/
void Population::set_probabilities()
{
    // store fitness of each individual into vector
    std::vector<double> fitness_vec {};
    for( Individual &indiv: population_ ){
        fitness_vec.push_back( indiv.fitness() );
    }
    if( fitness_vec.empty() ){
        std::cout<<">>>ERROR: Empty population (genetic). \n";
        exit(1);
    }

    double min_fitness { *std::min_element( fitness_vec.begin(),
                                            fitness_vec.end()) };
    double max_fitness { *std::max_element( fitness_vec.begin(),
                                            fitness_vec.end()) };
    if( (max_fitness-min_fitness) == 0.0 ){
        std::cout<<">>>ERROR: max fitness = min fitness (genetic). \n";
        exit(1);
    }

    // Map fitness values f_i into [0,1]
    // vec_i = (f_i - min(f))/(max(f)-min(f))
    std::vector<double> vec {};
    for( double f: fitness_vec ){
            vec.push_back( (f-min_fitness)/(max_fitness-min_fitness) );
    }
    double sum_vec { std::accumulate( vec.begin(), vec.end(), 0.0 ) };

    if( sum_vec == 0.0 ){
        std::cout<<">>>ERROR: sum_vec = 0 (genetic). \n";
        exit(1);
    }

    // Normalize fitness of individual by the total population fitness
    // prob_i = v_i/sum_i v_i
    std::vector<double> prob {};
    for( double v: vec ){
            prob.push_back( v / sum_vec );
    }

    // Set probability for each individual in population
    int indiv_count {0};
    for( Individual &indiv: population_ ){
        indiv.set_probability( prob.at(indiv_count) );
        indiv_count++;
    }
}

// ------------------------------------------------------------------------- //
/*! Compute fitness for all individuals in population,
    set probability for all individuals in population,
    sort population in decreasing order of fitness of its individuals.
    Append performance+parameters to 'optim_results'.
*/
void Population::compute_population_fitness(BTfast &btf,
                                        std::unique_ptr<DataFeed> &datafeed,
                                        std::vector<strategy_t> &optim_results)
{
    //int indiv_count {0};

    // Compute fitness of each individual in population
    #pragma omp parallel for
    for(auto indiv = population_.begin(); indiv < population_.end(); ++indiv){

        // Make a copy of DataFeed object and wrap it into a new unique_ptr
        std::unique_ptr<DataFeed> datafeed_copy = datafeed.get()->clone();

        indiv->compute_individual_fitness( btf, datafeed_copy,
                                           fitness_metric_, optim_results );

        /*
        #pragma omp critical
        {
            indiv_count++;        // Increment count on individuals
            std::cout << utils_time::current_datetime_str() + " | "
                      << "(Parallel) Running backtest on individual "
                      << indiv_count << " / " << population_.size()
                      << "\t Fitness = " << indiv->fitness() << "\n";
        }
        */
    }

    // Compute total fitness of population
    set_total_fitness();
    // Assign probabilities to each individual in population
    set_probabilities();

    // Sort individuals in population in decreasing order of fitness
    sort();
}


// ------------------------------------------------------------------------- //
/*! Select an individual in population using fitness-proportionate selection
    (aka roulette wheel selection)
*/
Individual Population::select()
{
    std::uniform_real_distribution<> rand01 {0.0,1.0};
    double p = rand01(utils_random::rand_generator); // random real in [0,1]
    double offset {0.0};
    Individual pick{};

    // Select individual according to its probability
    for( Individual indiv: population_ ){
        offset += indiv.probability();
        if( p < offset ){
            pick = indiv;
            break;
        }
    }
    return(pick);
}


// ------------------------------------------------------------------------- //
/*! Mutate a random gene in each individual of the population
    ( besides the first  'exclude_first' individuals ),
    with probability given by mutation_rate
*/
void Population::mutate( const std::vector<chromosome_t> &search_space,
                         double mutation_rate, int exclude_first )
{
    std::uniform_real_distribution<> rand01 {0.0,1.0};

    // loop over individuals in population
    // (excluding first 'exclude_first' elements)
    for( auto indiv = population_.begin() + exclude_first;
              indiv != population_.end(); ++indiv){

        double p { rand01(utils_random::rand_generator) }; // random real in [0,1]
        if( p < mutation_rate ){
            indiv->mutate(search_space);
        }
    }
}
