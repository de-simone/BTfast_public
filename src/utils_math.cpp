#include "utils_math.h"

#include <algorithm>    // std::nth_element, std::sort
#include <cmath>        // std::pow, std::erfc
#include <iostream>
#include <numeric>      // std::accumulate, std::inner_product

// ------------------------------------------------------------------------- //
// Round double number to given decimal digits
double utils_math::round_double( double x , int digits )
{
    double result { ((int)(x*std::pow(10,digits))) / std::pow(10,digits) };
    return(result);
}

// ------------------------------------------------------------------------- //
// Round double to nearest integer and return an int
int utils_math::nearest_int( double x )
{
    int result { (int)(x+0.5) };
    return(result);
}

// ------------------------------------------------------------------------- //
// Modulus operator between two integers
int utils_math::modulus( int a, int b )
{
    return( ( a%b + b ) % b );
}

// ------------------------------------------------------------------------- //
// Theta-function (Heaviside step function)
int utils_math::theta( double a )
{
    if ( a > 0.0 ){
        return(1);
    }
    else{
        return(0);
    }
}

// ------------------------------------------------------------------------- //
// p-th percentile of a vector v
double utils_math::percentile( std::vector<double> &v, double p )
{
    if( p<0 || p>1 ){
        std::cout<<">>> ERROR: percentile must be in [0,1] (utils_math).\n";
        exit(1);
    }

    const size_t pctil = v.size() * p;
    // nth_element modifies the input vector
    std::nth_element(v.begin(), v.begin() + pctil, v.end());
    //std::cout << "The p-th percentile is " << v[pctil] << '\n';

    return( v[pctil] );
}

// --------------------------------------------------------------------- //
// Weighted average of a vector v with a weight vector w
double utils_math::weighted_avg( const std::vector<double> &v,
                                 const std::vector<int> &w )
{
    if( v.empty() || w.empty() ){
        return(0);
    }
    if( v.size() != w.size() ){
        std::cout<<">>> ERROR: values and weights shoudl be of same size "
                 <<"(ultis_math).\n";
        exit(1);
    }

    int sum_of_weights { std::accumulate(w.begin(), w.end(), 0) };
    double weighted_sum { std::inner_product( v.begin(), v.end(),
                                              w.begin(), 0.0 ) };
    double result { 0.0 };
    if( sum_of_weights != 0 ){
        result = weighted_sum / sum_of_weights ;
    }
    return( result );
}

// ------------------------------------------------------------------------- //
// Mean of vector
double utils_math::mean( const std::vector<double> &v )
{
    if( v.empty() ){
        return(0);
    }

    double sum { std::accumulate(v.begin(), v.end(), 0.0) };
    double mean { sum / v.size() };
    return( mean );
}




// ------------------------------------------------------------------------- //
// Standard deviation of vector (unbiased sample std, with 1/(N-1))
double utils_math::stdev( const std::vector<double> &v )
{
    if( v.empty() || v.size()<2 ){
        return(0);
    }

    double sum { std::accumulate(v.begin(), v.end(), 0.0) };
    double mean { sum / v.size() };

    double accum = 0.0;
    std::for_each( v.begin(), v.end(),
                    [&](const double d)
                    {
                        accum += (d - mean) * (d - mean);
                    }
                 );

    double stdev { sqrt( accum / (v.size()-1) ) };

    return( stdev );
}


// ------------------------------------------------------------------------- //
// Compute the ranks of each element in 'v' and store it in 'ranks'
void utils_math::ranks( const std::vector<double> &v,
                        std::vector<double> &ranks )
{
    std::vector<int> int_ranks;
    for( double el: v ){
        std::vector<double> v_copy {v};
        std::sort(v_copy.begin(), v_copy.end());
        auto iter = std::lower_bound(v_copy.begin(), v_copy.end(), el);
        int_ranks.push_back( int(iter - v_copy.begin())+1 );
    }

    for( int el: int_ranks ){
        int ties { (int) std::count(int_ranks.begin(), int_ranks.end(), el) };
        if( ties == 1 ){
            ranks.push_back(el);
        }
        else{
            int r {0};
            for( int i = 0; i< ties; i++ ){
                r += el + i;
            }
            ranks.push_back(r/(double) ties);
        }
    }
    /*
    for( auto el: ranks ){
        std::cout << "rank: " << el<<"\n";
    }
    */
}

// ------------------------------------------------------------------------- //
// Mann-Whitney U test of two samples. Returns the two-sided p-value.
double utils_math::mannwhitney( const std::vector<double> &v,
                                 const std::vector<double> &w )
{
    if( v.empty() || w.empty() ){
        return(0.0);
    }

    // pool vector merging (v,w)
    std::vector<double> pool {};
    pool.reserve( v.size() + w.size() ); // preallocate memory
    pool.insert( pool.end(), v.begin(), v.end() );
    pool.insert( pool.end(), w.begin(), w.end() );

    std::vector<double> pool_rank {};
    utils_math::ranks( pool, pool_rank );

    int n1 { (int) v.size() };
    int n2 { (int) w.size() };
    double r1 { std::accumulate(pool_rank.begin(), pool_rank.begin()+n1, 0.0) };
    double r2 { std::accumulate(pool_rank.begin()+n1, pool_rank.end(), 0.0) };
    double u1 { r1 - n1*(n1+1)*0.5 };
    double u2 { r2 - n2*(n2+1)*0.5 };
    double umin { std::min(u1,u2) };
    double mean_u { n1*n2*0.5 };
    double sigma_u { std::sqrt(n1*n2*(n1+n2+1)/12.0 ) };
    double z { (umin-mean_u)/sigma_u };
    // two-sided p-value
    double pvalue = 2*( 1 - 0.5*std::erfc( -std::abs(z)/std::sqrt(2.0)) );

    return(pvalue);
}
