#ifndef UTILS_MATH_H
#define UTILS_MATH_H

#include <vector>   // std::vector
// Set of Math Utility functions


namespace utils_math {

    // --------------------------------------------------------------------- //
    /*! Round double number to given decimal digits
    */
    double round_double( double x , int digits );

    // --------------------------------------------------------------------- //
    /*! Round double to nearest integer and return an int
    */
    int nearest_int( double x );

    // --------------------------------------------------------------------- //
    /*! Modulus operator between two integers, handling also negative numbers.
        Reproduces the second output of python divmod() function.
    */
    int modulus( int a, int b );

    // --------------------------------------------------------------------- //
    /*! Theta-function (Heaviside step function)
    */
    int theta( double a );

    // --------------------------------------------------------------------- //
    /*! p-th percentile  of a vector v
    */
    double percentile( std::vector<double> &v, double p );

    // --------------------------------------------------------------------- //
    /*! Weighted average of a vector v with a weight vector w
    */
    double weighted_avg( const std::vector<double> &v,
                         const std::vector<int> &w );

    // --------------------------------------------------------------------- //
    /*! Mean of vector
    */
    double mean( const std::vector<double> &v );

    // --------------------------------------------------------------------- //
    /*! Standard deviation of vector (unbiased sample std, with 1/(N-1))
    */
    double stdev( const std::vector<double> &v );

    // ------------------------------------------------------------------------- //
    /*! Compute the ranks of each element in 'v' and store it in 'ranks'
    */
    void ranks( const std::vector<double> &v, std::vector<double> &ranks );

    // --------------------------------------------------------------------- //
    /*! Mann-Whitney U test of two samples. Returns the two-sided p-value.
    */
    double mannwhitney( const std::vector<double> &v,
                         const std::vector<double> &w );
}


#endif
