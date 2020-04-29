#ifndef GENETIC_H
#define GENETIC_H

#include "btfast.h"


/*!
    Define classes in use for genetic optimization: Individual, Population
*/


// Type aliases (renamed from btfast.h, with meaning for GA)
// Gene: single parameter combination, e.g. ("p1", 2)
using gene_t = single_param_t;
// Chromosome: vector of genes, e.g. [ ("p1", 2), ("p2", 7), ... ]
using chromosome_t = parameters_t;

// ------------------------------------------------------------------------- //
/*! Class representing single individual in population

Member Variables
- chromosome_: vector of genes
- fitness_: fitness value of individual
- probability_: fitness value normalized to sum to 1 over entire population

*/

class Individual {

    chromosome_t chromosome_;
    double fitness_{0.0};
    double probability_{0.0};

    public:
        Individual(chromosome_t chromosome = {});
        std::string tostring();

        void compute_individual_fitness(BTfast &btf,
                                        std::unique_ptr<DataFeed> &datafeed,
                                        std::string metric,
                                        std::vector<strategy_t> &optim_results);
        void single_crossover(Individual &parent2, double crossover_rate );
        void uniform_crossover(Individual &parent2);
        void mutate(const std::vector<chromosome_t> &search_space);
        void set_probability( double p ) { probability_ = p; };
        void set_chromosome(int i, gene_t new_gene){chromosome_[i] = new_gene;}

        chromosome_t chromosome() const { return(chromosome_); }
        double fitness() const { return(fitness_); }
        double probability() const { return(probability_); }
};


// ------------------------------------------------------------------------- //
/*! Class representing collection of individuals

Member Variables
- population_size_: number of individuals in population
- fitness_metrc_: performance metric to maximize
- population_: collection (vector) of individuals
- total_fitness_: sum of fitness of all individuals in population

*/

class Population {

    int population_size_{100};
    std::string fitness_metric_ {"AvgTrade"};
    std::vector<Individual> population_;
    double total_fitness_{0};

    public:
        Population( int population_size, std::string fitness_metric );

        void initialize_population(std::vector<chromosome_t> &search_space);
        void print_population();
        bool sort_by_fitness(const Individual& a, const Individual& b);
        void sort();
        void set_total_fitness();
        void set_probabilities();

        void compute_population_fitness(BTfast &btf,
                                        std::unique_ptr<DataFeed> &datafeed,
                                        std::vector<strategy_t> &optim_results);
        Individual select();
        void mutate( const std::vector<chromosome_t> &search_space,
                     double mutation_rate, int exclude_first = 2 );

        void insert_individual(Individual &ind){ population_.push_back(ind); }
        double total_fitness() const { return(total_fitness_); }
        std::vector<Individual> population() const { return(population_); }

};




#endif
