#ifndef UTILS_RANDOM_H
#define UTILS_RANDOM_H

#include "events.h"

#include <random>           // std::mt19937

// Set of Utility functions for random numbers


namespace utils_random {

    /*! Standard mersenne twister random generator (global)
    */
    extern std::mt19937 rand_generator;

    // --------------------------------------------------------------------- //
    /*! Add gaussian noise to bar
    */
    void add_gaussian_noise( Event &bar );
}


#endif
