#include "utils_random.h"


namespace utils_random {

    std::random_device rd{};            // seed for the random number engine
    std::mt19937 rand_generator{rd()};  // initialize random generator with seed
}


// ------------------------------------------------------------------------- //
// Add gaussian noise to bar. Uniform 25% chance to change OHLC.
// Add Gaussian noise with zero-mean, and std=(H-L)/3
void utils_random::add_gaussian_noise( Event &bar )
{
    //std::uniform_int_distribution<> rand_int_01 {0,1};
    //int p { rand_int_01(utils_random::rand_generator) };// random int in [0,1]

    // Gaussian distribution with zero-mean, and std = (H-L)/3
    std::normal_distribution<> gaussian {0, (bar.high()-bar.low())*0.33 };
    double noise { gaussian(utils_random::rand_generator) };

    // Random integer in [1,4]
    std::uniform_int_distribution<> rand_int_14 {1,4};
    int r { rand_int_14(utils_random::rand_generator) };

    // Initialize new bar values to those prior to change
    double new_open { bar.open() };
    double new_high { bar.high() };
    double new_low { bar.low() };
    double new_close { bar.close() };

    // Decide which bar value to change
    switch( r ){
        case 1:
            new_open += noise;
            break;
        case 2:
            new_high += noise;
            break;
        case 3:
            new_low += noise;
            break;
        case 4:
            new_close += noise;
            break;
    }

    // Re-establish order of high/low as max/min prices
    bar.reorder_OHLC(new_open, new_high, new_low, new_close);

}
