/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef INFOSQL_H
#define INFOSQL_H

#include "config.h"
#include "CoordinateTransformer.h"
#include "MapUpdateDBaseObject.h"
#include "MC2BoundingBox.h"
#include <vector>
#include <sstream>
#include "MySQLDriver.h"
#include "MySQLReplDriver.h"
#include "PostgreSQLDriver.h"
#include "OracleSQLDriver.h"
#include "Properties.h"
#include "DisturbanceElement.h"
#include "TrafficDataTypes.h"

class CharEncSQLConn;
class SQLQuery;

/**
 * Connection to SQL for disturbance information.
 * Company information will be added in the future.
 * Some kind of update function is needed to make sure that the
 * periodical disturbances' times are not totaly out of date.
 * Otherwise the restart will take time.
 *
 */
class InfoSQL {
   public:
      /**
       * Constructs new SQL connection.
       */
      InfoSQL();


      /**
       * Destructor, dissconnects from SQL.
       */
      ~InfoSQL();

   bool setupDatabaseConnection();
   bool checkDatabaseConnection(int maxNbrTries = 1);

      /**
       *   @name Get- and delete- functions for trafficdisturbances.
       */
      //@{
        /**
         *  Adds a number of trafficdisturbances from a vector to the
         *  database.
         *  @param distVect  The vector with disturbances.
         *  @param firstPacket True if it's the first disturbances that
         *                     is sent with this situationID.
         *  @return True if the adding was okey, otherwise false.
         */
      bool addDisturbance(DisturbanceElement* distElem);

      /**
       *  Removes the disturbances with a situationID
       *  and elementID.

       *  @param disturbanceID      The disturbance ID.
       *  @param situationReference The situationReference string.
       *  @param supplier           A string identifying a supplier.
       *  @param removeAll          Whether all disturbances from the
       *                            supplier should be removed.
       *  @param toBeKept           A list of situationReference strings
       *                            specifying which disturbances
       *                            should not be removed if removeAll
       *                            is set.
       *  @return True if the delete was OK, otherwise false.
       */
   bool deleteDisturbances(DisturbanceElement::DisturbanceID disturbanceID,
                              const MC2String& situationReference,
                              const MC2String& supplier,
                              bool removeAll,
                              vector<MC2String>& toBeKept);

      /**
       *  Updates the disturbances with a situationID and a elementID.
       *  @param type  The new disturbance type.
       *  @param severity The new severity.
       *  @param startTime The new disturbance start time.
       *  @param endTime The new disturbance end time.
       *  @param creationTime The time of the update, the new
       *                      creation time.
       *  @param comment The new disturbance comment.
       *  @param situationID The situationID of the updated disturbance.
       *  @param elementID The elementID of the updated disturbance.
       *  @return True if the updating was okey, otherwise false.
       */
      bool updateDisturbance(uint32 disturbanceID,
                             TrafficDataTypes::disturbanceType type,
                             TrafficDataTypes::severity severity,
                             uint32 startTime,
                             uint32 endTime,
                             uint32 costFactor,
                             MC2String text);

      /*
       *  Returns all disturbances within a given bounding box from
       *  the database.
       *  @param bbox  The MC2BoundingBox.
       *  @param distVect A reference to a vector, where the function
       *                  inserts all the disturbances.
       *  @return True if the loading was okey, otherwise false.
       */
      bool getDisturbancesWithinBBox(const MC2BoundingBox& bbox,
                                     vector<DisturbanceElement*> &distVect);

   /**
       *  Returns all disturbances within a given bounding box from
       *  the database.
       *  @param bbox  The MC2BoundingBox.
       *  @param distVect A reference to a vector, where the function
       *                  inserts all the disturbances.
       *  @return True if the loading was okey, otherwise false.
       */
      bool getDisturbancesWithinRadius(int32 latitude,
                                       int32 longitude,
                                       uint32 distance,
                                       vector<DisturbanceElement*> &distVect);

      /**
       *  Returns all disturbances from a specified supplier.
       *  @param distVect Output parameter. The found disturbances are
       *                  push_back:ed into the vector.
       *  @param supplier The string identifying the supplier.
       */
      void getDisturbancesForSupplier( vector<DisturbanceElement*> &distVect,
                                       const MC2String& supplier );
      /**
       *  Returns all disturbances from a specified supplier, except
       *  certain specified disturbances.
       *  @param distVect Output parameter. The found disturbances are
       *                  push_back:ed into the vector.
       *  @param supplier The string identifying the supplier.
       *  @param toBeKept A list of situationReference strings identifying
       *                  situations that should not be inserted into
       *                  <code>distVect</code>.
       */
      void getDisturbancesForSupplier( vector<DisturbanceElement*> &distVect,
                                       const MC2String& supplier,
                                       const vector<MC2String>& toBeKept );


      /**
       * Removes disturbances with end time less than time.
       * @return true if operation successfull, false otherwise.
       */
      bool deleteOldDisturbances();


      /**
       *   Get the disturbance with the given ID.
       *   @return The found disturbance or NULL if not found.
       */
      DisturbanceElement* getDisturbance(uint32 disturbanceID);

      /**
       *   Get the disturbance with the given situationReference
       *   @return The found disturbance or NULL if not found.
       */
      DisturbanceElement* getDisturbance(const MC2String& situationReference);

private:
      /**
       *   Get the disturbance with the given situationReference and
       *   disturbanceID. If disturbanceID is equal to MAX_UINT32 it
       *   is not considered.  If situationReference is equal to the
       *   empty string it is not considered.
       *   @return The found disturbance or NULL if not found.
       */
   DisturbanceElement* getDisturbance(DisturbanceElement::DisturbanceID disturbanceID,
                                      const MC2String& situationReference);
public:

      /**
       *  Returns the coordinates of one or two TMC points in the database.
       *  @param firstLocationCode The primary TMC point.
       *  @param secondLocationCode The secondary TMC-point.
       *  @param extent The extent.
       *  @param direction The direction.
       *  @param country The country
       *  @param latitude The vector with lats.
       *  @param longitude The vector with longs.
       *  @return True if the point(s) were successfully found.
       */
      bool getTMCCoords(MC2String firstLocation,
                        MC2String secondLocation,
                        int32 extent,
                        TrafficDataTypes::direction direction,
                        vector< pair<int32, int32> > &firstCoords,
                        vector< pair<int32, int32> > &secondCoords);

   private:
   /**
    * Extract the first DisturbanceElement from a SQL answer.
    * @param sqlQuery A SQL query already containing the answer.
    *                 When the function returns it will have advanced
    *                 to the first line that does not belong with the
    *                 disturbance.
    * @param disturbances Output parameter for the extracted disturbances.
    */
   void extractDisturbances(SQLQuery& sqlQuery,
                           map<uint32,DisturbanceElement*>& disturbances);

      /**
       *   Do a SQL query. Copied from UserModule/UserProcessor
       *   @param sqlQuery Pointer to the SQLQuery to use
       *   @param query The query string
       *   @param whereTag Set this to a constant string which
       *                   identifies the callee
       *   @return True if successful, false otherwise
       */
      //@{
      bool doQuery(SQLQuery* sqlQuery,
                   const std::ostringstream& stream,
                   const char* whereTag);

      bool doQuery(SQLQuery* sqlQuery, const MC2String& str,
                   const char* whereTag);

      bool doQuery(SQLQuery* sqlQuery, const char* query,
                   const char* whereTag);
      //@}

      /**
       *   Adds a table to the internal table vectors. Copied from
       *   UserModule/UserProcessor
       *   @param name The name of the table
       *   @param createQuery The SQL query that creates the query
       *   @param extraQuery Any additional query to run after creation of
       *                     table (such as index creation)
       */
      void addTable(const MC2String& name, const MC2String& createQuery,
                    const MC2String& extraQuery = "");

      /**
       *   Initializes the table vectors. Copied from UserModule/UserProcessor
       *   @return The number of tables, 0 if something went wrong
       */
      size_t initTables();

      /**
       *   Checks database for required tables. Tries to create tables.
       *   @return true if successfull
       */
      bool initialCheckDatabase();

      /**
       * Returns an unused id in a table.
       * @param tableName The name of the table
       * @param colName   The name of the id column
       * @return Returns an unused id. 0 if error.
       */
      uint32 getNewUniqueID(const char* tableName, const char* colName);

      /**
       *   The database connection
       */
      CharEncSQLConn* m_sqlConnection;

   /**
    * Struct that encapsulates the data stored by calls to addTable.
    */
   struct TableData {
      /**
       * Constructor.
       * @param name The name of the table.
       * @param createQuery The query used to create the table if it
       *                    doesn't exist.
       * @param extraQuery Extra query run after a successful create
       *                   query.
       */
      TableData(const MC2String& name,
                const MC2String& createQuery,
                const MC2String& extraQuery) :
         name(name), createQuery(createQuery), extraQuery(extraQuery)
      {}
      /** The table name. */
      MC2String name;
      /** The query to create the table. */
      MC2String createQuery;
      /** Any extra query. */
      MC2String extraQuery;
   };
   /** Typedef of container to store TableData structs in. */
   typedef std::vector<TableData> tableCont_t;
   /** Data about the needed tables. */
   tableCont_t m_tableData;
};

#endif // INFOSQL_H
