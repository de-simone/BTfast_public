#include "performance.h"

#include "utils_fileio.h"   // copy_file_to_file
#include "utils_math.h"     // nearest_int, weighted_avg

#include <algorithm>        // std::transform
#include <cmath>            // std::abs, std::pow
#include <cstdio>           // fprintf
#include <functional>       // std::placeholders
#include <iostream>         // std::cout
#include <iterator>         // std::back_inserter
#include <numeric>          // std::accumulate, std::inner_product

using std::pow;
using std::abs;
using std::sqrt;

// ------------------------------------------------------------------------- //
/*! Constructors
*/
Performance::Performance( double initial_balance, int ndays,
                          std::vector<Transaction> transactions)
 : initial_balance_{initial_balance},
   ndays_ {ndays},
   transactions_{transactions}
 {}



//-------------------------------------------------------------------------- //
/*! Compute performance metrics from transaction history in 'transactions_'
    for all/long/short trades,
    and store them into maps: metrics_all_, metrics_long_, metrics_short_.
*/
void Performance::compute_metrics()
{
    // Select long/short transactions
    std::vector<Transaction> transactions_long {};
    std::vector<Transaction> transactions_short {};

    for( Transaction tr: transactions_ ){
        if( tr.side() == "LONG" ){
            transactions_long.push_back(tr);
        }
        else if( tr.side() == "SHORT" ){
            transactions_short.push_back(tr);
        }
    }

    // Compute metrics for all/long/short trades
    // and store them into corresponding maps

    compute_metrics( transactions_, metrics_all_ );
    compute_metrics( transactions_long, metrics_long_ );
    compute_metrics( transactions_short, metrics_short_ );

}


//-------------------------------------------------------------------------- //
/*! Compute performance metrics from input 'transactions' history, and
    store them into 'metrics' unordered map (name, value)
*/
void Performance::compute_metrics(
                        const std::vector<Transaction> &transactions,
                        std::unordered_map<std::string,double> &metrics )
{
    //-- Initialize entries of unordered map
    metrics["ntrades"] = 0;
    metrics["nwins"] = 0;
    metrics["net_pl"] = 0;
    metrics["net_pl_pct"] = 0;
    metrics["avg_trade"] = 0;
    metrics["std_trade"] = 0;
    metrics["avg_ticks"] = 0;
    metrics["std_ticks"] = 0;
    metrics["avg_profit"] = 0;
    metrics["avg_loss"] = 0;
    metrics["max_profit"] = 0;
    metrics["max_loss"] = 0;
    metrics["win_perc"] = 0;
    metrics["bars_in_win"] = 0;
    metrics["bars_in_loss"] = 0;
    metrics["gross_profit"] = 0;
    metrics["gross_loss"] = 0;
    metrics["profit_factor"] = 0;
    metrics["expectancy"] = 0;
    metrics["sharpe"] = 0;
    metrics["cagr"] = 0;
    metrics["mar"] = 0;         // MAR = CAGR / MaxDD(%)
    metrics["max_dd"] = 0;
    metrics["max_dd_pct"] = 0;
    metrics["avg_dd"] = 0;
    metrics["avg_dd_pct"] = 0;
    metrics["avg_dd_duration"] = 0; // avg num of days between equity peaks
    metrics["netpl_maxdd"] = 0;
    metrics["tsindex"] = 0;
    metrics["max_consec_win"] = 0;
    metrics["max_consec_loss"] = 0;
    metrics["rsquared"] = 0;
    metrics["zscore"] = 0;
    metrics["min_capital"] = 0;
    //--

    if( !transactions.empty() ){

        std::vector<double> profits {};     // vector of profit/loss
        std::vector<int> lots {};           // vector of contract sizes
        std::vector<Date> dates {};   // vector of trade exit dates
        double margin { transactions.front().symbol().margin() };
        double tick_value { transactions.front().symbol().tick_value() };

        // Loop over transaction objects
        for( Transaction tr : transactions ) {

            if( tr.quantity() > 0 ){
                // fill vector of profits/losses from transactions
                profits.push_back( tr.net_pl() );
                // fill vector of contract sizes from transactions
                lots.push_back( tr.quantity() );
                // fill vector of trade exit dates
                dates.push_back( tr.exit_time().date() );
            }
            else{
                continue; // discard transaction with 0 contracts
            }
            // Sum number of bars in winning trades
            if( tr.net_pl() > 0 ){
                metrics["bars_in_win"] += tr.bars_in_trade();
            }
            // Sum number of bars in losing trades
            else if( tr.net_pl() < 0 ){
                metrics["bars_in_loss"] += tr.bars_in_trade();
            }
        }

        // Number of trades
        metrics["ntrades"] = profits.size();
        if( metrics["ntrades"] <= 0 ){
            return;
        }

        // Total net profit/loss
        metrics["net_pl"] = std::accumulate(profits.begin(),profits.end(),0.0);
        // NetPL % over initial balance
        metrics["net_pl_pct"] = ( metrics["net_pl"] / initial_balance_ ) * 100.0;
        // Avg Trade (mean of profits)
        metrics["avg_trade"]  = utils_math::mean( profits );
        // Standard Deviation of Profits
        metrics["std_trade"] = utils_math::stdev( profits );
        // DrawDowns, Nwins, GrossProfit, GrossLoss, MaxProfit, MaxLoss
        drawdown( profits, dates, metrics );
        // Average number of bars in winning/losing trade, Average Profit, Loss
        if( metrics["nwins"] > 0 ){
            metrics["bars_in_win"]  /= metrics["nwins"];
            metrics["avg_profit"] = metrics["gross_profit"] / metrics["nwins"];
        }
        if( metrics["ntrades"] > metrics["nwins"] ){
            metrics["bars_in_loss"] /= (metrics["ntrades"]-metrics["nwins"]);
            metrics["avg_loss"] = metrics["gross_loss"]/(metrics["ntrades"]-metrics["nwins"]);
        }
        // Average Ticks per trade
        avgticks( profits, lots, tick_value, metrics );
        // Win Percent (Percent Profitable)
        metrics["win_perc"]   = metrics["nwins"]/ metrics["ntrades"] * 100.0;
        // Z-score
        zscore( profits, metrics );
        // Profit Factor
        if( metrics["gross_loss"] != 0.0 ){
            metrics["profit_factor"] = metrics["gross_profit"]/ abs(metrics["gross_loss"]);
        }
        // Expectancy
        if( metrics["avg_loss"] < 0.0 ){
            metrics["expectancy"] = ( metrics["avg_profit"]*metrics["win_perc"]/100.0
                            - abs(metrics["avg_loss"])*(1-metrics["win_perc"]/100.0) )
                                / abs(metrics["avg_loss"]);
        }
        // Net Profit / Max DD
        if( metrics["max_dd"] != 0.0 ){
            metrics["netpl_maxdd"] = metrics["net_pl"] / abs(metrics["max_dd"]);
        }
        // TSindex = (Net Profit / Max DD) * Nwins
        metrics["tsindex"] = metrics["netpl_maxdd"] * metrics["nwins"];
        // Max consecutive winning/losing trades
        max_consec_win_loss( profits, metrics );
        // CAGR
        //int nyears { transactions.back().entry_time().year() - transactions.front().entry_time().year() + 1 };
        int nyears = utils_math::nearest_int( ndays_ / 252.0 );
        cagr( profits, metrics, nyears );
        // MAR
        metrics["mar"] = metrics["cagr"]/abs(metrics["max_dd_pct"]);
        // R^2
        rsquared( profits, metrics );
        // Minimum required capital
        metrics["min_capital"] = margin + 1.5 * abs( metrics["max_dd"] );
    }
}



//-------------------------------------------------------------------------- //
/*! Max and average drawdown (cash and %), avg drawdown duration,
    Nwins, GrossProfit, GrossLoss, MaxProfit, MaxLoss,

    Store result into metrics.
*/
void Performance::drawdown( const std::vector<double> &profits,
                            const std::vector<Date>& dates_vec,
                            std::unordered_map<std::string,double> &metrics )
{
    double cumul_pl {0.0};          // cumulative P/L
    double max_cumul_pl {0.0};      // max cumulative P/L
    double drawdown {0.0};          // drawdown after each trade
    double drawdown_pct {0.0};      // drawdown % after each trade
    double drawdown_sum {0.0};      // sum of drawdowns
    double drawdown_sum_pct {0.0};  // sum of drawdowns %
    int days_delay {0};             // time delay between equity highs (days)
    std::vector<double> days_delay_vec {}; // vector storing days_delays
    Date curr_date {};              // Date of trade in loop
    Date peak_date {};              // Date of trade at equity high

    int counter {0};                // loop counter
    for( double pl : profits ) {

        cumul_pl += pl;
        if( pl > 0 ){
            metrics["nwins"] += 1;
            metrics["gross_profit"] += pl;
        }
        else{
            metrics["gross_loss"] += pl;
        }

        if( pl > metrics["max_profit"] ){
            metrics["max_profit"]  = pl;
        }
        if( pl < metrics["max_loss"] ){
            metrics["max_loss"] = pl;
        }

        //- Compute DrawDown
        if( cumul_pl > max_cumul_pl ){          // new equity high

            max_cumul_pl = cumul_pl;
            peak_date = dates_vec.at(counter);
            if( days_delay != 0 ){
                days_delay_vec.push_back((double) days_delay );
            }
            days_delay = 0;
        }
        else{                                   // no new equity high
            curr_date = dates_vec.at(counter);
            days_delay = curr_date.DaysDiff(peak_date);
        }

        drawdown = cumul_pl - max_cumul_pl;
        drawdown_pct = ( (initial_balance_ + cumul_pl)
                         / (initial_balance_+ max_cumul_pl) - 1)*100;
        drawdown_sum += drawdown;
        drawdown_sum_pct += drawdown_pct;

        if( drawdown < metrics["max_dd"] ){
            metrics["max_dd"] = drawdown;
        }
        if( drawdown_pct < metrics["max_dd_pct"] ){
            metrics["max_dd_pct"] = drawdown_pct;
        }
        //-
        counter++;
    }

    // Avg DrawDown
    metrics["avg_dd"] = drawdown_sum / metrics["ntrades"] ;
    metrics["avg_dd_pct"] = drawdown_sum_pct / metrics["ntrades"] ;
    // Avg DrawDown Duration (in days)
    metrics["avg_dd_duration"] = utils_math::mean( days_delay_vec );
}


//-------------------------------------------------------------------------- //
/*! Mean and stddev of number of ticks per trade
            avg_ticks = ( 1/(Ntrades*tick_value) ) * sum_i (PL_i/Nlots_i).
                      = mean( PL/(Nlots*tick_value) )
            std_ticks = stdev( PL/(Nlots*tick_value) )

    Store results into metrics.
*/
void Performance::avgticks( const std::vector<double> &profits,
                            const std::vector<int> &lots,
                            double tick_value,
                            std::unordered_map<std::string,double> &metrics )
{
    // vector of inverse lots 1/Nlots
    std::vector<double> inv_lots( lots.begin(), lots.end() );
    std::transform( inv_lots.begin(), inv_lots.end(), inv_lots.begin(),
                std::bind(std::divides<double>(), 1.0, std::placeholders::_1) );

    std::vector<double> ticks_vec {};
    ticks_vec.reserve( profits.size() );
    // Fill ticks_vec with PL_i/Nlots_i
    std::transform( profits.begin(), profits.end(), inv_lots.begin(),
                    std::back_inserter(ticks_vec), std::multiplies<double>());
    // Divide by tick_value
    std::transform( ticks_vec.begin(), ticks_vec.end(), ticks_vec.begin(),
        std::bind(std::divides<double>(), std::placeholders::_1, tick_value));
    metrics["avg_ticks"] = utils_math::mean( ticks_vec );
    metrics["std_ticks"] = utils_math::stdev( ticks_vec );

    /* // Sum of PL_i / Nlots_i
    double weighted_sum { std::inner_product( profits.begin(), profits.end(),
                                              inv_lots.begin(), 0.0 ) };
    if( metrics["ntrades"]*tick_value > 0 ){
        metrics["avg_ticks"] = weighted_sum / (metrics["ntrades"]*tick_value);
    }
    */
}


//-------------------------------------------------------------------------- //
/*! Number of Max consecutive winning/losing trades.
    Store result into metrics.
*/
void Performance::max_consec_win_loss( const std::vector<double> &profits,
                            std::unordered_map<std::string,double> &metrics )
{
    int consec_win = 1;
    int consec_loss = 1;
    for( size_t i = 1; i < profits.size(); i++ ){

        if( profits[i] > 0 ){
            if( profits[i-1] > 0 ){
                consec_win += 1;
            }
            else{
                if( consec_win > metrics["max_consec_win"] ){
                    metrics["max_consec_win"] = (double) consec_win;
                }
                consec_win = 1;
            }
        }
        else if( profits[i] < 0 ){
            if( profits[i-1] < 0 ){
                consec_loss += 1;
            }
            else{
                if( consec_loss > metrics["max_consec_loss"] ){
                    metrics["max_consec_loss"] = (double) consec_loss;
                }
                consec_loss = 1;
            }
        }
    }
    if( consec_win > metrics["max_consec_win"] ){
        metrics["max_consec_win"] = (double) consec_win;
    }
    if( consec_loss > metrics["max_consec_loss"] ){
        metrics["max_consec_loss"] = (double) consec_loss;
    }
}

//-------------------------------------------------------------------------- //
/*! Z-score: mean(PL)/[sigma(PL)/sqrt(Ntrades)].
    Valid for Ntrades >= 30, otherwise 0.
    Used to compare with null hypothesis: mean(PL)=0
    Store result into metrics.
*/
void Performance::zscore( const std::vector<double> &profits,
                          std::unordered_map<std::string,double> &metrics )
{
    if( metrics["std_trade"] > 0.0 && metrics["ntrades"] >= 30 ){
        metrics["zscore"] = sqrt(metrics["ntrades"])
                            * metrics["avg_trade"] / metrics["std_trade"];
    }
}

//-------------------------------------------------------------------------- //
/*! Compute the Compound Annual Growth Rate of profits, given
    the number of days of backtest.
*/
void Performance::cagr( const std::vector<double> &profits,
                        std::unordered_map<std::string,double> &metrics,
                        int nyears )
{
    double final_balance = initial_balance_ + metrics["net_pl"];

    if( nyears > 0 && final_balance > 0 ){
        metrics["cagr"] = ( pow(final_balance / initial_balance_, 1.0/nyears)
                             - 1 ) * 100;
    }
}

//-------------------------------------------------------------------------- //
/*! Compute R^2, a.k.a. Coefficient of determination
    (squared of Pearson coefficient).
    Store result into metrics.
*/
void Performance::rsquared( const std::vector<double> &profits,
                            std::unordered_map<std::string,double> &metrics )
{

    int n = (int) profits.size();
    double sumx  {0};
    double sumy  {0};
    double sumx2 {0};
    double sumy2 {0};
    double sumxy {0};
    double balance {0};

    for( int j = 0; j < n; j++ ){
        balance += profits[j];
        sumx  += j;
        sumx2 += pow(j,2);
        sumxy += (j)*balance;
        sumy  += balance;
        sumy2 += pow(balance,2);

    }

    if( n * sumxy - sumx * sumy > 0 && n * sumx2 - pow(sumx,2) > 0
        && n * sumy2 - pow(sumy,2) > 0 ){

        metrics["rsquared"] = pow( n * sumxy - sumx * sumy, 2 )/
                ( ( n * sumx2 - pow(sumx,2) ) * ( n * sumy2 - pow(sumy,2) ) );
    }
    //std::cout<<"R2 = "<< metrics["rsquared"]<<"\n\n";
}




//-------------------------------------------------------------------------- //
/*! Print performance report on stdout
*/
void Performance::print_performance_report()
{
    printf("\n   # ==================  Performance Report ================== #\n");
    printf("                        |     All    |    Long    |    Short   |\n");
    printf("%4s %-18s : %10.2f\n", "","Initial Balance",
                                initial_balance_);
    printf("%4s %-18s : %7d %12d %12d\n", "","N. of Trades",
                                    (int) metrics_all_["ntrades"],
                                    (int) metrics_long_["ntrades"],
                                    (int) metrics_short_["ntrades"] );
    printf("%4s %-18s : %10.2f %12.2f %12.2f\n", "", "Net Profit ",
                                    metrics_all_["net_pl"],
                                    metrics_long_["net_pl"],
                                    metrics_short_["net_pl"]);
    printf("%4s %-18s : %9.1f%% %11.1f%% %11.1f%%\n", "", "Net Profit %",
                                    metrics_all_["net_pl_pct"],
                                    metrics_long_["net_pl_pct"],
                                    metrics_short_["net_pl_pct"] );
    printf("%4s %-18s : %9.1f%% %11.1f%% %11.1f%%\n","", "CAGR",
                                    metrics_all_["cagr"],
                                    metrics_long_["cagr"],
                                    metrics_short_["cagr"] );
    printf("%4s %-18s : %10.2f %12.2f %12.2f\n", "", "Avg Trade",
                                    metrics_all_["avg_trade"],
                                    metrics_long_["avg_trade"],
                                    metrics_short_["avg_trade"] );
    printf("%4s %-18s : %10.2f %12.2f %12.2f\n", "", "Avg Ticks",
                                    metrics_all_["avg_ticks"],
                                    metrics_long_["avg_ticks"],
                                    metrics_short_["avg_ticks"] );
    printf("%4s %-18s : %10.2f %12.2f %12.2f\n", "", "Profit Factor",
                                    metrics_all_["profit_factor"],
                                    metrics_long_["profit_factor"],
                                    metrics_short_["profit_factor"] );
    printf("%4s %-18s : %9.1f%% %11.1f%% %11.1f%%\n", "", "Win Percent   ",
                                    metrics_all_["win_perc"],
                                    metrics_long_["win_perc"],
                                    metrics_short_["win_perc"] );
    printf("%4s %-18s : %10.2f %12.2f %12.2f\n", "", "Avg DrawDown",
                                    metrics_all_["avg_dd"],
                                    metrics_long_["avg_dd"],
                                    metrics_short_["avg_dd"] );
    printf("%4s %-18s : %9.1f%% %11.1f%% %11.1f%%\n", "", "Avg DrawDown %",
                                    metrics_all_["avg_dd_pct"],
                                    metrics_long_["avg_dd_pct"],
                                    metrics_short_["avg_dd_pct"] );
    printf("%4s %-18s : %10.2f %12.2f %12.2f\n", "", "Max DrawDown",
                                    metrics_all_["max_dd"],
                                    metrics_long_["max_dd"],
                                    metrics_short_["max_dd"] );
    printf("%4s %-18s : %9.1f%% %11.1f%% %11.1f%%\n", "", "Max DrawDown %",
                                    metrics_all_["max_dd_pct"],
                                    metrics_long_["max_dd_pct"],
                                    metrics_short_["max_dd_pct"] );
    printf("%4s %-18s : %10.2f %12.2f %12.2f\n", "", "Avg DD Duration",
                                    metrics_all_["avg_dd_duration"],
                                    metrics_long_["avg_dd_duration"],
                                    metrics_short_["avg_dd_duration"] );
    printf("%4s %-18s : %10.2f %12.2f %12.2f\n", "", "Net PL/Max DD",
                                    metrics_all_["netpl_maxdd"],
                                    metrics_long_["netpl_maxdd"],
                                    metrics_short_["netpl_maxdd"] );
    printf("%4s %-18s : %10.2f %12.2f %12.2f\n", "", "CAGR/Max DD %",
                                    metrics_all_["mar"],
                                    metrics_long_["mar"],
                                    metrics_short_["mar"] );
    printf("%4s %-18s : %10.2f %12.2f %12.2f\n","", "Expectancy",
                                    metrics_all_["expectancy"],
                                    metrics_long_["expectancy"],
                                    metrics_short_["expectancy"] );
    printf("%4s %-18s : %10.2f %12.2f %12.2f\n", "", "Z-score",
                                    metrics_all_["zscore"],
                                    metrics_long_["zscore"],
                                    metrics_short_["zscore"] );
    printf("   # - - - - - - - - - - - - - - - - - - - - - - - - - - - - - #\n");
    printf("%4s %-18s : %10.4f %12.4f %12.4f\n","", "R^2",
                                    metrics_all_["rsquared"],
                                    metrics_long_["rsquared"],
                                    metrics_short_["rsquared"]);
    printf("%4s %-18s : %10.2f %12.2f %12.2f\n", "", "Std.Dev. of P/L",
                                    metrics_all_["std_trade"],
                                    metrics_long_["std_trade"],
                                    metrics_short_["std_trade"] );
    printf("%4s %-18s : %10.2f %12.2f %12.2f\n", "", "Gross Profit",
                                    metrics_all_["gross_profit"],
                                    metrics_long_["gross_profit"],
                                    metrics_short_["gross_profit"] );
    printf("%4s %-18s : %10.2f %12.2f %12.2f\n", "", "Gross Loss",
                                    metrics_all_["gross_loss"],
                                    metrics_long_["gross_loss"],
                                    metrics_short_["gross_loss"] );
    printf("%4s %-18s : %10.2f %12.2f %12.2f\n", "", "Avg Profit",
                                    metrics_all_["avg_profit"],
                                    metrics_long_["avg_profit"],
                                    metrics_short_["avg_profit"] );
    printf("%4s %-18s : %10.2f %12.2f %12.2f\n", "", "Avg Loss",
                                    metrics_all_["avg_loss"],
                                    metrics_long_["avg_loss"],
                                    metrics_short_["avg_loss"] );
    printf("%4s %-18s : %10.2f %12.2f %12.2f\n", "", "Max Profit",
                                    metrics_all_["max_profit"],
                                    metrics_long_["max_profit"],
                                    metrics_short_["max_profit"] );
    printf("%4s %-18s : %10.2f %12.2f %12.2f\n", "", "Max Loss",
                                    metrics_all_["max_loss"],
                                    metrics_long_["max_loss"],
                                    metrics_short_["max_loss"] );
    printf("%4s %-18s : %10.2f %12.2f %12.2f\n", "", "Avg Bars in Win",
                                    metrics_all_["bars_in_win"],
                                    metrics_long_["bars_in_win"],
                                    metrics_short_["bars_in_win"] );
    printf("%4s %-18s : %10.2f %12.2f %12.2f\n", "", "Avg Bars in Loss",
                                    metrics_all_["bars_in_loss"],
                                    metrics_long_["bars_in_loss"],
                                    metrics_short_["bars_in_loss"] );
    printf("%4s %-18s : %7d %12d %12d\n", "", "Max Consec. Wins",
                                    (int) metrics_all_["max_consec_win"],
                                    (int) metrics_long_["max_consec_win"],
                                    (int) metrics_short_["max_consec_win"] );
    printf("%4s %-18s : %7d %12d %12d\n", "", "Max Consec. Losses",
                                    (int) metrics_all_["max_consec_loss"],
                                    (int) metrics_long_["max_consec_loss"],
                                    (int) metrics_short_["max_consec_loss"] );
    printf("%4s %-18s : %10.2f %12.2f %12.2f\n", "", "Minimum Capital",
                                    metrics_all_["min_capital"],
                                    metrics_long_["min_capital"],
                                    metrics_short_["min_capital"] );
    printf("   # ========================================================= #\n");
}



//-------------------------------------------------------------------------- //
/*! Write performance report to file
*/
void Performance::write_performance_to_file( std::string fname,
                            std::string paramfile,
                            std::string strategy_name, std::string symbol_name,
                            std::string timeframe, Date date_i, Date date_f )
{
    if( fname != "" ){

        DateTime now{};
        now.set_current();

        FILE *outfile = fopen(fname.c_str(), "w");
        fprintf(outfile, "# TimeStamp   : %s:%02d\n",
                now.tostring().c_str(),  (int) now.second() );
        fprintf(outfile, "# Strategy    : %s\n", strategy_name.c_str());
        fprintf(outfile, "# Symbol      : %s\n", symbol_name.c_str());
        fprintf(outfile, "# TimeFrame   : %s\n", timeframe.c_str());
        fprintf(outfile, "# Date Range  : %s --> %s\n", date_i.tostring().c_str(),
                                                        date_f.tostring().c_str());
        fprintf(outfile,"#\n");
        fclose(outfile);

        // Copy XML strategy file
        utils_fileio::copy_file_to_file(paramfile, fname);

        fopen(fname.c_str(), "a");
        fprintf(outfile, "\n   # ==================  Performance Report ================== #\n");
        fprintf(outfile, "                        |     All    |    Long    |    Short   |\n");
        fprintf(outfile, "%4s %-18s : %10.2f\n", "","Initial Balance",
                                    initial_balance_);
        fprintf(outfile, "%4s %-18s : %7d %12d %12d\n", "","N. of Trades",
                                        (int) metrics_all_["ntrades"],
                                        (int) metrics_long_["ntrades"],
                                        (int) metrics_short_["ntrades"] );
        fprintf(outfile, "%4s %-18s : %10.2f %12.2f %12.2f\n", "", "Net Profit ",
                                        metrics_all_["net_pl"],
                                        metrics_long_["net_pl"],
                                        metrics_short_["net_pl"]);
        fprintf(outfile, "%4s %-18s : %9.1f%% %11.1f%% %11.1f%%\n", "", "Net Profit %",
                                        metrics_all_["net_pl_pct"],
                                        metrics_long_["net_pl_pct"],
                                        metrics_short_["net_pl_pct"] );
        fprintf(outfile, "%4s %-18s : %9.1f%% %11.1f%% %11.1f%%\n","", "CAGR",
                                        metrics_all_["cagr"],
                                        metrics_long_["cagr"],
                                        metrics_short_["cagr"] );
        fprintf(outfile, "%4s %-18s : %10.2f %12.2f %12.2f\n", "", "Avg Trade",
                                        metrics_all_["avg_trade"],
                                        metrics_long_["avg_trade"],
                                        metrics_short_["avg_trade"] );
        fprintf(outfile, "%4s %-18s : %10.2f %12.2f %12.2f\n", "", "Avg Ticks",
                                        metrics_all_["avg_ticks"],
                                        metrics_long_["avg_ticks"],
                                        metrics_short_["avg_ticks"] );
        fprintf(outfile, "%4s %-18s : %10.2f %12.2f %12.2f\n", "", "Profit Factor",
                                        metrics_all_["profit_factor"],
                                        metrics_long_["profit_factor"],
                                        metrics_short_["profit_factor"] );
        fprintf(outfile, "%4s %-18s : %9.1f%% %11.1f%% %11.1f%%\n", "", "Win Percent   ",
                                        metrics_all_["win_perc"],
                                        metrics_long_["win_perc"],
                                        metrics_short_["win_perc"] );
        fprintf(outfile, "%4s %-18s : %10.2f %12.2f %12.2f\n", "", "Avg DrawDown",
                                        metrics_all_["avg_dd"],
                                        metrics_long_["avg_dd"],
                                        metrics_short_["avg_dd"] );
        fprintf(outfile, "%4s %-18s : %9.1f%% %11.1f%% %11.1f%%\n", "", "Avg DrawDown %",
                                        metrics_all_["avg_dd_pct"],
                                        metrics_long_["avg_dd_pct"],
                                        metrics_short_["avg_dd_pct"] );
        fprintf(outfile, "%4s %-18s : %10.2f %12.2f %12.2f\n", "", "Max DrawDown",
                                        metrics_all_["max_dd"],
                                        metrics_long_["max_dd"],
                                        metrics_short_["max_dd"] );
        fprintf(outfile, "%4s %-18s : %9.1f%% %11.1f%% %11.1f%%\n", "", "Max DrawDown %",
                                        metrics_all_["max_dd_pct"],
                                        metrics_long_["max_dd_pct"],
                                        metrics_short_["max_dd_pct"] );
        fprintf(outfile, "%4s %-18s : %10.2f %12.2f %12.2f\n", "", "Avg DD Duration",
                                        metrics_all_["avg_dd_duration"],
                                        metrics_long_["avg_dd_duration"],
                                        metrics_short_["avg_dd_duration"] );
        fprintf(outfile, "%4s %-18s : %10.2f %12.2f %12.2f\n", "", "Net PL/Max DD",
                                        metrics_all_["netpl_maxdd"],
                                        metrics_long_["netpl_maxdd"],
                                        metrics_short_["netpl_maxdd"] );
        //fprintf(outfile, "%4s %-18s : %10.2f\n", "", "Sharpe",
        //                            metrics_all_["sharpe"] );
        fprintf(outfile, "%4s %-18s : %10.2f %12.2f %12.2f\n", "", "CAGR/Max DD %",
                                        metrics_all_["mar"],
                                        metrics_long_["mar"],
                                        metrics_short_["mar"] );
        fprintf(outfile, "%4s %-18s : %10.2f %12.2f %12.2f\n","", "Expectancy",
                                        metrics_all_["expectancy"],
                                        metrics_long_["expectancy"],
                                        metrics_short_["expectancy"] );
        fprintf(outfile, "%4s %-18s : %10.2f %12.2f %12.2f\n", "", "Z-score",
                                        metrics_all_["zscore"],
                                        metrics_long_["zscore"],
                                        metrics_short_["zscore"] );
        fprintf(outfile, "   # - - - - - - - - - - - - - - - - - - - - - - - - - - - - - #\n");
        fprintf(outfile, "%4s %-18s : %10.4f %12.4f %12.4f\n","", "R^2",
                                        metrics_all_["rsquared"],
                                        metrics_long_["rsquared"],
                                        metrics_short_["rsquared"]);
        fprintf(outfile, "%4s %-18s : %10.2f %12.2f %12.2f\n", "", "Std.Dev. of P/L",
                                        metrics_all_["std_trade"],
                                        metrics_long_["std_trade"],
                                        metrics_short_["std_trade"] );
        fprintf(outfile, "%4s %-18s : %10.2f %12.2f %12.2f\n", "", "Gross Profit",
                                        metrics_all_["gross_profit"],
                                        metrics_long_["gross_profit"],
                                        metrics_short_["gross_profit"] );
        fprintf(outfile, "%4s %-18s : %10.2f %12.2f %12.2f\n", "", "Gross Loss",
                                        metrics_all_["gross_loss"],
                                        metrics_long_["gross_loss"],
                                        metrics_short_["gross_loss"] );
        fprintf(outfile, "%4s %-18s : %10.2f %12.2f %12.2f\n", "", "Avg Profit",
                                        metrics_all_["avg_profit"],
                                        metrics_long_["avg_profit"],
                                        metrics_short_["avg_profit"] );
        fprintf(outfile, "%4s %-18s : %10.2f %12.2f %12.2f\n", "", "Avg Loss",
                                        metrics_all_["avg_loss"],
                                        metrics_long_["avg_loss"],
                                        metrics_short_["avg_loss"] );
        fprintf(outfile, "%4s %-18s : %10.2f %12.2f %12.2f\n", "", "Max Profit",
                                        metrics_all_["max_profit"],
                                        metrics_long_["max_profit"],
                                        metrics_short_["max_profit"] );
        fprintf(outfile, "%4s %-18s : %10.2f %12.2f %12.2f\n", "", "Max Loss",
                                        metrics_all_["max_loss"],
                                        metrics_long_["max_loss"],
                                        metrics_short_["max_loss"] );
        fprintf(outfile, "%4s %-18s : %10.2f %12.2f %12.2f\n", "", "Avg Bars in Win",
                                        metrics_all_["bars_in_win"],
                                        metrics_long_["bars_in_win"],
                                        metrics_short_["bars_in_win"] );
        fprintf(outfile, "%4s %-18s : %10.2f %12.2f %12.2f\n", "", "Avg Bars in Loss",
                                        metrics_all_["bars_in_loss"],
                                        metrics_long_["bars_in_loss"],
                                        metrics_short_["bars_in_loss"] );
        fprintf(outfile, "%4s %-18s : %7d %12d %12d\n", "", "Max Consec. Wins",
                                        (int) metrics_all_["max_consec_win"],
                                        (int) metrics_long_["max_consec_win"],
                                        (int) metrics_short_["max_consec_win"] );
        fprintf(outfile, "%4s %-18s : %7d %12d %12d\n", "", "Max Consec. Losses",
                                        (int) metrics_all_["max_consec_loss"],
                                        (int) metrics_long_["max_consec_loss"],
                                        (int) metrics_short_["max_consec_loss"] );
        fprintf(outfile, "%4s %-18s : %10.2f %12.2f %12.2f\n", "", "Minimum Capital",
                                        metrics_all_["min_capital"],
                                        metrics_long_["min_capital"],
                                        metrics_short_["min_capital"] );
        fprintf(outfile, "   # ========================================================= #\n");

        fclose(outfile);
        std::cout<< "\nPerformance report written on file: " << fname << "\n";
    }
}



//-------------------------------------------------------------------------- //
/*! Compute performance metrics from transaction history
    and print performance report on stdout
*/
void Performance::compute_and_print_performance()
{
    if( !transactions_.empty() ){
        compute_metrics();
        print_performance_report();
    }
    else{
        printf("No trades generated.\n");
    }
}
