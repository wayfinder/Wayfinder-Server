/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef SQLTRAFFICELEMENTSTRINGS_H
#define SQLTRAFFICELEMENTSTRINGS_H

#include "MC2String.h"
#include "TrafficElementDatabase.h"

/// @namespace Holds convenience function for creating query strings.
namespace SQLTrafficElementStrings {

/// Create a query string for elements to delete from a table.
/// @param table Name of the table to delete elements from.
/// @param removeElements Elements to removed.
/// @return A query string for remove in table.
MC2String createDeleteQuery( const MC2String& table,
                             const TrafficElementDatabase::
                             TrafficElements& removeElements );

/// Create add query string for disturbance elements to be added to a MySQL
/// database table \c ISABDisturbance
/// @param begin The iterator where to start creating the query from.
/// @param end The iterator where to end creating the queries.
/// @param queryString The query string which can be used with the SQL database.
/// @param queryMaxSize The max size of the query string.
/// @return An iterator pointing at the next element from where it stoped.
///         It could either be the end or when the query is to big.
TrafficElementDatabase::TrafficElements::const_iterator 
createAddQueryDisturbance( 
      TrafficElementDatabase::TrafficElements::const_iterator begin,
      TrafficElementDatabase::TrafficElements::const_iterator end,
      MC2String& queryString,
      const size_t queryMaxSize );

/// Create query string to add to coordinates to \c ISABDisturbanceCoords
/// table.
/// @param begin The iterator where to start creating the query from.
/// @param end The iterator where to end creating the queries.
/// @param queryString The query string which can be used with the SQL database.
/// @param queryMaxSize The max size of the query string.
/// @return An iterator pointing at the next element from where it stoped.
///         It could either be the end or when the query is to big.
TrafficElementDatabase::TrafficElements::const_iterator 
createAddQueryCoords( 
      TrafficElementDatabase::TrafficElements::const_iterator begin,
      const TrafficElementDatabase::TrafficElements::const_iterator end,
      MC2String& queryString,
      const size_t queryMaxSize );


} // SQLTrafficElementStrings

#endif // SQLTRAFFICELEMENTSTRINGS_H
