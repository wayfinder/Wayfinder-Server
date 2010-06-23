/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "SQLTrafficElementDatabase.h"


#include "MC2String.h"
#include "PropertyHelper.h"
#include "TrafficDataTypes.h"

#include "SQLTrafficElementStrings.h"
#include "SQLCreateConnection.h"
#include "CharEncSQLConn.h"
#include "SQLTransaction.h"
#include "SQLException.h"
#include "SQLStringStream.h"

#include "DisturbanceChangeset.h"
#include "DisturbanceElement.h"

#include "boost/lexical_cast.hpp"

struct SQLTrafficElementDatabase::Impl {
   auto_ptr<CharEncSQLConn> m_connection;
};

SQLTrafficElementDatabase::SQLTrafficElementDatabase():
   m_impl( new Impl() ) {
   // initialize SQL connection
   m_impl->m_connection.reset( SQL::createInfoConnectionFromProperties() );
}

SQLTrafficElementDatabase::~SQLTrafficElementDatabase() {
   delete m_impl;
}

namespace {

/// Queries the database.
/// Throws QueryException exception on error.
/// @param sqlQuery Database query.
/// @param query The query string.
void doQuery( CharEncSQLQuery& sqlQuery, const MC2String& query )
   throw (SQL::QueryException) {

   if ( ! sqlQuery.prepare( query.c_str() ) ) {
      throw SQL::QueryException( MC2String( "Failed prepare() on \" " ) +
                                 query + "\"" +
                                 sqlQuery.getErrorString() );
   }

   if ( ! sqlQuery.execute() && sqlQuery.getError() > 0 ) {
      throw SQL::QueryException( MC2String( "Failed execute() on \"" ) +
                                 query + "\"" +
                                 sqlQuery.getErrorString() );
   }
}


/// Delete \c elements from \c table.
/// @param connection Database connection.
/// @param table A specific table.
/// @param removeElements Elements to be removed from \c table.
void deleteFromTable( CharEncSQLConn& connection,
                      const MC2String& table,
                      const TrafficElementDatabase::
                      TrafficElements& removeElements )
   throw (SQL::QueryException) {

   auto_ptr<CharEncSQLQuery> sqlQuery( connection.newQuery() );
   doQuery( *sqlQuery, SQLTrafficElementStrings::
            createDeleteQuery( table, removeElements ) );
}

/// Tests connection
bool checkConnection( auto_ptr<CharEncSQLConn>& connection ) {
   if ( connection.get() && connection->ping() ) {
      return true;
   }

   // try to connect to database 3 times.
   connection.reset( SQL::createInfoConnectionFromProperties() );
   for ( uint32 i = 0; i < 2 && connection.get() == 0; ++i ) {
      connection.reset( SQL::createInfoConnectionFromProperties() );
   }

   return connection.get() != NULL;
}

DisturbanceElement*
getDisturbance( CharEncSQLQuery& sqlQuery, uint32 disturbanceID ) {
   MC2String situationReference = sqlQuery.getColumn( 1 );
   TrafficDataTypes::disturbanceType type =
      TrafficDataTypes::disturbanceType( atoi( sqlQuery.getColumn( 2 ) ) );
   TrafficDataTypes::phrase phrase =
      TrafficDataTypes::phrase( atoi( sqlQuery.getColumn( 3 ) ) );
   uint32 eventCode = uint32( atoi( sqlQuery.getColumn( 4 ) ) );
   uint32 startTime = uint32( atoi( sqlQuery.getColumn( 5 ) ) );
   uint32 endTime = uint32( atoi( sqlQuery.getColumn( 6 ) ) );
   uint32 creationTime = uint32( atoi( sqlQuery.getColumn( 7 ) ) );
   TrafficDataTypes::severity severity =
      TrafficDataTypes::severity( atoi( sqlQuery.getColumn( 8 ) ) );
   TrafficDataTypes::direction direction =
      TrafficDataTypes::direction( atoi( sqlQuery.getColumn( 9 ) ) );
   MC2String text = sqlQuery.getColumn( 10 );
   bool deleted = bool( atoi( sqlQuery.getColumn( 11 ) ) );
   MC2String firstLocation = sqlQuery.getColumn( 12 );
   MC2String secondLocation = sqlQuery.getColumn( 13 );
   uint32 extent = uint32( atoi( sqlQuery.getColumn( 14 ) ) );
   uint32 costFactor = uint32( atoi( sqlQuery.getColumn( 15 ) ) );
   uint32 queueLength = uint32( atoi( sqlQuery.getColumn( 16 ) ) );

   DisturbanceElement* distElem =
      new DisturbanceElement( disturbanceID,
                              situationReference,
                              type,
                              phrase,
                              eventCode,
                              startTime,
                              endTime,
                              creationTime,
                              severity,
                              direction,
                              firstLocation,
                              secondLocation,
                              extent,
                              costFactor,
                              text,
                              queueLength);
   distElem->setDeleted( deleted );
   return distElem;
}

void getDisturbances( CharEncSQLQuery& query,
                      TrafficElementDatabase::
                      TrafficElements& disturbances ) {
   uint32 prevDisturbanceID = MAX_UINT32;
   DisturbanceElement* currentElement = NULL;
   while ( query.nextRow() ) {
      uint32 disturbanceID = uint32( atoi( query.getColumn( 0 ) ) );
      if ( disturbanceID != prevDisturbanceID ) {
         // save current element and create a new
         if ( currentElement != NULL ) {
            disturbances.push_back( currentElement );
         }
         currentElement = getDisturbance( query, disturbanceID );
         prevDisturbanceID = disturbanceID;
      }
      int32 lat = int32( atoi( query.getColumn( 19 ) ) );
      int32 lon = int32( atoi( query.getColumn( 20 ) ) );
      uint32 angle = uint32( atoi( query.getColumn( 21 ) ) );
      uint32 routeIndex = uint32( atoi( query.getColumn( 22 ) ) );

      currentElement->addCoordinate( lat, lon, angle, routeIndex );
   }
   if ( currentElement != NULL ) {
      disturbances.push_back( currentElement );
   }
}

} // anonymous

bool SQLTrafficElementDatabase::
fetchAllDisturbances( const MC2String& providerID,
                      TrafficElements& disturbances ) {
   // Make sure we have a valid connection
   if ( ! ::checkConnection( m_impl->m_connection ) ) {
      mc2log << error << "[SQLTrafficElementDatabase] "
             << "No connection to database!" << endl;
      return false;
   }

   SQL::StringStream stream;
   stream << "SELECT * FROM ISABDisturbance, ISABDisturbanceCoords"
          << " WHERE ISABDisturbanceCoords.situationReference LIKE "
          << ( providerID + ":%" )
          << " AND "
          << " ISABDisturbance.disturbanceID = "
          << "ISABDisturbanceCoords.disturbanceID";
   try {
      auto_ptr<CharEncSQLQuery> sqlQuery( m_impl->m_connection->newQuery() );
      doQuery( *sqlQuery, stream.str() );
      ::getDisturbances( *sqlQuery, disturbances );
   } catch ( const SQL::QueryException& e ) {
      mc2log << error << "[SQLTrafficElementDatabase] fetch all dist: "
             << e.what() << endl;
      return false;
   }
   return true;
}
namespace {
DisturbanceElement::DisturbanceID
getDisturbanceIDForSituationReferenceID( CharEncSQLConn& connection,
                                         const DisturbanceElement::
                                         SituationReference& refID ) try {
   SQL::StringStream queryStream;
   queryStream << "SELECT disturbanceID FROM ISABDisturbance"
               << " WHERE situationReference = "
               << refID;

   auto_ptr<CharEncSQLQuery> sqlQuery( connection.newQuery() );
   doQuery( *sqlQuery, queryStream.str() );
   if ( sqlQuery->nextRow() ) {
      return boost::lexical_cast<DisturbanceElement::DisturbanceID>
         ( sqlQuery->getColumn( 0 ) );
   }

   return DisturbanceElement::INVALID_DISTURBANCE_ID;   

} catch ( const SQL::QueryException& queryErr ) {
   mc2log << warn << "[SQLTrafficElementDatabase] Failed to query database."
          << " In getDisturbanceIDForSituationReferenceID()."
          << queryErr.what() << endl;
   return DisturbanceElement::INVALID_DISTURBANCE_ID; 
} catch ( const boost::bad_lexical_cast& castErr ) {
   mc2log << warn << "[SQLTrafficElementDatabase]"
          << "Failed to convert disturbance id. " 
          << castErr.what() << endl;
   return DisturbanceElement::INVALID_DISTURBANCE_ID;
}

                                         
DisturbanceElement::DisturbanceID
getUniqueID( CharEncSQLConn& connection,
             const char* tableName, const char* colName)
try {
   std::ostringstream queryBaseStream;
   //Generate the constant part of the query
   queryBaseStream << "SELECT " << colName << " FROM " << tableName
                   << " WHERE " << colName << " = ";
   const MC2String queryBase = queryBaseStream.str(); //store in string
   //we need a query object
   auto_ptr<CharEncSQLQuery> sqlQuery( connection.newQuery() );
   while ( true ) {
      //generate random value
      DisturbanceElement::DisturbanceID id =
         1+static_cast<DisturbanceElement::DisturbanceID>
         ((float64)MAX_INT32*rand()/(RAND_MAX + 1.0));

      ostringstream queryStream;
      queryStream << queryBase << id;

      mc2dbg << "[SQLTrafficElementDatabase]" << " unique ID: " 
             << id << " stream: " << queryStream.str() << endl;

      doQuery( *sqlQuery, queryStream.str() ); 

      if( ! sqlQuery->nextRow() ) {
         //the query returned no rows, use this id.
         return id;
      }
      //the id was already used, try another one.
   }

   return DisturbanceElement::INVALID_DISTURBANCE_ID;

} catch ( const SQL::QueryException& e ) {
   // The query failed...
   mc2log << error << e.what() << ": getNewUniqueID() giving up!" << endl;
   return DisturbanceElement::INVALID_DISTURBANCE_ID;
}

/** 
 * Create unque disturbance ids for all the elements that does not yet have a valid
 * disturbance id.
 * @param connection SQL connection.
 * @param elements The elements to fetch and set unique disturbance ID for.
 */
void createUniqueIDs( CharEncSQLConn& connection,
                      DisturbanceChangeset::Elements& elements ) {
   
   // for convinience
   typedef DisturbanceElement::DisturbanceID DisturbanceID;

   // for keeping track of the new DisturbanceID's
   typedef std::set< DisturbanceID > DisturbanceSet;
   DisturbanceSet newDisturbanceIDs;

   typedef MC2String SituationReference;
   // for keeping track of a situations unique id
   typedef std::map< SituationReference, DisturbanceID > SituationToDisturnbace;
   SituationToDisturnbace sitRefToDistID;

   for ( DisturbanceChangeset::Elements::iterator it = elements.begin();
         it != elements.end(); ++it ) {
      // only set those disturbances that does not yet have a valid id.
      if ( (*it)->getDisturbanceID() !=
           DisturbanceElement::INVALID_DISTURBANCE_ID ) {
         continue;
      }

      // check if the situation already has a disturbance id
      SituationToDisturnbace::iterator mapIt = 
         sitRefToDistID.find( (*it)->getSituationReference() );
      if ( mapIt != sitRefToDistID.end() ) {
         // this situation reference already gotten a disturbace id, use it..
         (*it)->setDisturbanceID( mapIt->second );
      } else {
         // Lets see if there is a situation reference in the database already
         // and reuse the disturbance id for that one.
         // This is the case when there is a disturbance that spans more than
         // one map.
         {
            DisturbanceElement::DisturbanceID id = 
               getDisturbanceIDForSituationReferenceID( connection,
                                                        (*it)->getSituationReference() );
            if ( id != DisturbanceElement::INVALID_DISTURBANCE_ID ) {
               (*it)->setDisturbanceID( id );
               // add it so we can track when a disturbance id is chosen for a
               // situaiton reference.
               sitRefToDistID.
                  insert( make_pair( (*it)->getSituationReference(), id ) );
               continue;
            }
         }
         
         // no disturbace id for this situation reference, get one.
         while ( true ) {
            // get a unique id that is not in the SQL database
            DisturbanceElement::DisturbanceID id =
               getUniqueID( connection, "ISABDisturbance", "disturbanceID" );
            // insert to keep track of the new ids not inserted in the data base
            pair< DisturbanceSet::iterator, bool > ret = 
               newDisturbanceIDs.insert( id );
            // keep on going until we get a new unique id, in other words.. until
            // the insert succeeds.
            if ( id != DisturbanceElement::INVALID_DISTURBANCE_ID && 
                 ret.second ) {
               // set the new id for the disturbance element
               (*it)->setDisturbanceID( id );
               // add it so we can track when a disturbance id is chosen for a
               // situaiton reference.
               sitRefToDistID.insert( 
                     make_pair( (*it)->getSituationReference(), id ) );
               break;
            }
         }
      }
   }
}

}

template < typename Functor >
void createQueries( CharEncSQLConn& connection,
                      const TrafficElementDatabase::TrafficElements& elements,
                      Functor func ) {
   // max size of the query set to make sure we dont exeed the allowed
   // packet size set in the database
   const size_t QUERY_MAX_SIZE = 800000;
   TrafficElementDatabase::TrafficElements::const_iterator startIt = 
      elements.begin();
   MC2String query;
   query.reserve( QUERY_MAX_SIZE );
   auto_ptr<CharEncSQLQuery> sqlQuery( connection.newQuery() );
   while ( startIt != elements.end() ) {
      // create the query string
      startIt = func( startIt, elements.end(), 
                      query, 
                      QUERY_MAX_SIZE );
      if ( !query.empty() ) {
         // create a new SQL query
         doQuery( *sqlQuery, query );
         query.clear();
      }
   }
}

int
SQLTrafficElementDatabase::
updateChangeset( const DisturbanceChangeset& changes ) {

   // Make sure we have a valid connection
   if ( ! ::checkConnection( m_impl->m_connection ) ) {
      mc2log << error << "[SQLTrafficElementDatabase] "
             << "No connection to database!" << endl;
      return
         TrafficElementDatabase::REMOVE_FAILED |
         TrafficElementDatabase::UPDATE_FAILED;
   }

   int status = TrafficElementDatabase::OK;

   // Remove old elements
   if ( ! changes.getRemoveSet().empty() ) {
      try {
         // make exception safe queries
         SQL::Transaction transaction( *m_impl->m_connection );
         // remove disturbances
         ::deleteFromTable( *m_impl->m_connection,
                            "ISABDisturbance", changes.getRemoveSet() );
         // remove coordinates for the disturbances
         ::deleteFromTable( *m_impl->m_connection,
                            "ISABDisturbanceCoords", changes.getRemoveSet() );

         transaction.commit();

      } catch ( const SQL::Exception& err ) {
         status |= TrafficElementDatabase::REMOVE_FAILED;

         mc2log << error << "Failed to erase infos. "
                << err.what() << endl;
      }
   }
   // setup unique ids
   // do a litle const-hack...we are just suppose to set the ids
   DisturbanceChangeset::Elements& newElements = 
      const_cast< DisturbanceChangeset::Elements& >( changes.getUpdateSet() );
                                                    
   // Add new elements
   if ( ! newElements.empty() ) {
      // make sure all elements have unique IDs in the database.
      ::createUniqueIDs( *m_impl->m_connection, newElements );

      try {
         // make exception safe queries
         SQL::Transaction transaction( *m_impl->m_connection );

         using namespace SQLTrafficElementStrings;

         // update disturbance table
         createQueries( *m_impl->m_connection, 
                        newElements, 
                        createAddQueryDisturbance );
         // update disturbace coords table
         createQueries( *m_impl->m_connection, 
                        newElements, 
                        createAddQueryCoords );

         transaction.commit();

      } catch ( const SQL::Exception& err ) {
         status |= TrafficElementDatabase::UPDATE_FAILED;

         mc2log << error << "Failed to insert/update infos. "
                << err.what() << endl;
      }
   }

   return status;
}
