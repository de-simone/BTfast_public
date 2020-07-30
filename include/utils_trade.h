#ifndef UTILS_TRADE_H
#define UTILS_TRADE_H

#include "position_handler.h"
#include "price_collection.h"

// Set of Utility functions for strategies

namespace utils_trade {

    // --------------------------------------------------------------------- //
    /*! Return Market position of current position: 0=flat, 1=long, -1=short
    */
    int MarketPosition( const PositionHandler& position_handler );

    // --------------------------------------------------------------------- //
    /*! Find the position with side "LONG" or "SHORT" and corresponding to
        strategy 'strat_name', inside the vector of open positions
    */
    Position find_pos_to_close( const std::string& side,
                                const std::string& strat_name,
                                const std::vector<Position>& open_positions );

    // --------------------------------------------------------------------- //
    /*! Compute the number of entries during current session, as sum of
        positions open and closed + positions open and not closed.
        'timestamp' is the current bar timestamp.
        (deprecated because slow)

    int EntriesInSession( const DateTime &timestamp, const Instrument &symbol,
                          const PositionHandler &position_handler);
    */

    // --------------------------------------------------------------------- //
    /*! Extract features from price_collection from recent history before
        trade entry, and write them on file 'fname'
    */
    void FeaturesExtraction( const PriceCollection &price_collection,
                             std::string fname );




}


#endif
