#include "account.h"

#include "utils_fileio.h"      // copy_file_to_file

#include <iomanip>     // std::setw
#include <iostream>     // std::cout
#include <fstream>     // std::ofstream


// ------------------------------------------------------------------------- //
/*! Constructors
*/
Account::Account(double initial_balance)
: initial_balance_{initial_balance}, balance_{initial_balance}
{}


//-------------------------------------------------------------------------- //
/*! Add new trade to transaction history
*/
void Account::add_transaction_to_history(Transaction new_trade)
{
    transactions_.push_back(new_trade);
}


//-------------------------------------------------------------------------- //
/*! Print transaction history on stdout
*/
void Account::print_transaction_history() const
{
    std::string header{""};

    header += "\n# ------------------- ";
    header += ">>> Transaction List <<< ------------------- #\n";
    header += "# Trade   Side  Qty      TimeStamp       ";
    header += "Price    NetPL/Cumulative";

    std::cout << header << "\n";

    // Iterate over transaction vector
    int i {1};
    for( Transaction tr : transactions_ ) {
        printf("%7d %s\n", i, tr.tostring().c_str());
        i++;
    }
}


//-------------------------------------------------------------------------- //
/*! Write transaction history to file
*/
void Account::write_transaction_history_to_file( std::string fname,
                        std::string paramfile,
                        std::string strategy_name, std::string symbol_name,
                        std::string timeframe, Date date_i, Date date_f ) const
{

    DateTime now{};
    now.set_current();

    std::ofstream outfile;
    outfile.open(fname);

    outfile << "# TimeStamp   : " << now.tostring() << ":" <<
                std::setw(2) << now.second() << "\n";
    outfile << "# Strategy    : " << strategy_name << "\n";
    outfile << "# Symbol      : " << symbol_name << "\n";
    outfile << "# TimeFrame   : " << timeframe << "\n";
    outfile << "# Date Range  : " << date_i.tostring() << " --> "
                                  << date_f.tostring() << "\n";
    outfile << "#\n";
    outfile.close();

    // Copy XML strategy file
    utils_fileio::copy_file_to_file(paramfile, fname);

    outfile.open(fname, std::fstream::app);
    std::string header{""};
    header += "# ------------------- ";
    header += ">>> Transaction List <<< ------------------- #\n";
    header += "# Trade   Side  Qty      TimeStamp       ";
    header += "Price    NetPL/Cumulative";

    outfile << header << "\n";

    // Iterate over transaction vector
    int i {1};
    for( Transaction tr : transactions_ ) {
        outfile << std::setw(7) << i << " " << tr.tostring() <<"\n";
        i++;
    }

    outfile.close();
    std::cout<< "\nTransaction history written on file: " << fname << "\n";
}

//-------------------------------------------------------------------------- //
/*! Write to file only profit/loss of each trade in transaction history
*/
void Account::write_transaction_history_pl_to_file( std::string fname ) const
{
    std::ofstream outfile;
    outfile.open(fname);
    for( Transaction tr : transactions_ ) {
        outfile<< tr.net_pl()<<"\n";
    }
    outfile.close();
    std::cout<< "\nProfits/Losses written on file: " << fname << "\n";
}

//-------------------------------------------------------------------------- //
/*! Write trade number and cumulative profit/loss to file
*/
void Account::write_equity_to_file( std::string fname ) const
{
    std::ofstream outfile;
    outfile.open(fname);
    outfile << "# Trade    Entry Date    DOW     Exit Date    Qty    Ticks    PL    Equity\n";
    outfile << "     0,    0000-00-00,     0,   0000-00-00,    0,    0,    0,    0,\n";
    //outfile << "# Trade      Balance\n"<< ",        " << initial_balance_ << "\n";

    // Iterate over transaction vector
    int i {1};
    for( Transaction tr : transactions_ ) {
        outfile << "     " << i
                << ",    " << tr.entry_time().date().tostring()
                << ",    " << tr.entry_time().date().weekday()
                << ",    " << tr.exit_time().date().tostring()
                << ",    " << tr.quantity()
                << ",    " << tr.ticks()
                << ",    " << tr.net_pl()
                << ",    " << tr.cumul_pl() << ",\n";
                // << ",        " <<(initial_balance_ + tr.cumul_pl()) << "\n";
        i++;
    }

    outfile.close();
    std::cout<< "\nCumulative profits written on file: " << fname << "\n";
}

//-------------------------------------------------------------------------- //
/*! Compute  largest losing trade in transactions (negative value)
    Useful for position sizing.
*/
double Account::largest_loss() const
{
    if( transactions_.empty() ){
        return(0.0);
    }
    double max_loss {0.0};

    for( Transaction tr: transactions_ ){
        if( tr.net_pl() < max_loss ){
            max_loss = tr.net_pl();
        }
    }
    return(max_loss);
}

//-------------------------------------------------------------------------- //
/*! Reset all memeber variables
*/
void Account::reset( double initial_balance )
{
    initial_balance_ = initial_balance;
    balance_ = initial_balance;
    transactions_ = std::vector<Transaction> {};
}
