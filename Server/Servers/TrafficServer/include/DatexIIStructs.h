/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef DATEXIISTRUCTS_H
#define DATEXIISTRUCTS_H

#include "config.h"
#include "MC2String.h"

#include "XPathAssigner.h"
#include "WGS84Coordinate.h"
#include "DeleteHelpers.h"

#include <memory>
#include <vector>



class TrafficSituation;

namespace DatexIIStructs {

typedef XMLTool::XPath::MultiExpression::NodeEvaluator Evaluator; 

using namespace XMLTool;
using namespace XPath;

/// Holds information about a TMC location
struct AlertCLocationValues {
   bool getTMCLocations(MC2String& firstLocationCode, 
                        MC2String& secondLocationCode ) const {
      // check if there is a tmc location at all for this situation
      if ( !m_specificLocation1.empty() ) {
         // Setting the country code to E instead of 14 as they send it
         // from vägverket..
         // Waiting for a answer from the supplier which wasnt sure if it was
         // supposed to be 14 or E.
         m_alertCLocCountryCode = "E";
         // create the TMC id for the first and second location
         ostringstream firstLocation, secondLocation;
         firstLocation << m_alertCLocCountryCode << m_alertCLocTableNbr << setw(5) 
            << setfill('0') << m_specificLocation1;
         firstLocationCode = firstLocation.str();
         // Get the second location if there is one
         if ( !m_specificLocation2.empty() ) {
            secondLocation << m_alertCLocCountryCode << m_alertCLocTableNbr 
               << setw(5) << setfill('0') << m_specificLocation2;
            secondLocationCode = secondLocation.str();
         }
         return true;
      } else {
         return false;
      }
   }

   mutable MC2String m_alertCLocCountryCode; // for the TMC coord first part 
   MC2String m_alertCLocTableNbr;    // for the TMC coord first part
   MC2String m_alertCLocTableVersion;// tmcVersion 
   MC2String m_alertCDirectionCoded; // direction
   MC2String m_specificLocation1;    // for the TMC coord first location
   MC2String m_specificLocation2;    // for the TMC coord second location
};

/**
 * Holds information about a situation record which can be found in any type
 * of situation record.
 */
struct SituationValues {
   MC2String m_type;
   MC2String m_sitRecID;
   MC2String m_status;
   MC2String m_sitRecCreationTime; 
   MC2String m_sitRecVersion;
   MC2String m_overallStartTime; 
   MC2String m_overallEndTime;
   MC2String m_commentSE;
   MC2String m_commentEN;
   AlertCLocationValues m_location; 
   WGS84Coordinate m_coord;
   // SRA extensions
   MC2String m_headerExtension;

   virtual ~SituationValues() {
   }
};

/// For a vector containing SituationValues pointers
typedef std::vector< SituationValues* > SitValsCont;

auto_ptr< TrafficSituation > 
createTrafficSit( const MC2String& provider,
                  const SitValsCont& sre,
                  const MC2String& sitID );

/// Holds information uniqe to a NetworkManagement situation record
struct MaintenanceWorksValues : public SituationValues {
   MC2String m_effectOnRoadLayout;
   MC2String m_roadMaintenanceType;
};

/// Holds information uniqe to a NetworkManagement situation record
struct NetworkManagementValues : public SituationValues {
   MC2String m_networkManagementType;
   MC2String m_temporarySpeedLimit;
   MC2String m_heigthComparisonType;
   MC2String m_vehicleHeight;
   MC2String m_widthComparisonType;
   MC2String m_vehicleWidth;
};

/// Holds information uniqe to a Accident situation record
struct AccidentValues : public SituationValues {
   MC2String m_accidentType;        // type of accident
};

/// Holds information uniqe to a AbnormalTraffic situation record
struct AbnormalTrafficValues : public SituationValues {
   MC2String m_abnormalTrafficType;
   MC2String m_trafficTrend;
};

/// Holds information uniqe to a ConstructionWorks situation record
struct ConstructionWorksValues : public SituationValues {
   MC2String m_effectOnRoadLayout;
   MC2String m_constructionWorkType;
};

/// Holds information uniqe to a Activities situation record
struct ActivitiesValues : public SituationValues {
   MC2String m_disturbanceActivityType;
   MC2String m_publicEventType;
};

/// Holds information uniqe to a AnimalPresenceObstruction situation record
struct AnimalPresenceObstructionValues : public SituationValues {
   MC2String m_animalPresenceType;
};

/// Holds information uniqe to a GeneralObstruction situation record
struct GeneralObstructionValues : public SituationValues {
   MC2String m_obstructionType;
};

/// Holds information uniqe to a NonWeatherRelatedRoadConditions situation record
struct NonWeatherRelatedRoadConditionsValues : public SituationValues {
   MC2String m_nonWeatherRelatedConditionType;
};

/// Holds information uniqe to a VehicleObstruction situation record
struct VehicleObstructionValues : public SituationValues {
   MC2String m_vehicleObstructionType;
};

/**
 * Gets the AlertC locations 
 */
class AlertCMethod2Expression : public Evaluator {
public:
   explicit AlertCMethod2Expression( AlertCLocationValues& alertCVals ) :
      m_alertCLocExp( NULL ),
      m_alertCLocValues( alertCVals ) {
         MultiExpression::NodeDescription desc[] = {
            { "alertCLocationCountryCode",
               makeAssigner( m_alertCLocValues.m_alertCLocCountryCode ) },
            { "alertCLocationTableNumber",
               makeAssigner( m_alertCLocValues.m_alertCLocTableNbr ) },
            { "alertCLocationTableVersion",
               makeAssigner( m_alertCLocValues.m_alertCLocTableVersion ) },
            { "alertCDirection/alertCDirectionCoded",
               makeAssigner( m_alertCLocValues.m_alertCDirectionCoded ) },
            { "alertCMethod2PrimaryPointLocation/alertCLocation/specificLocation",
               makeAssigner( m_alertCLocValues.m_specificLocation1 ) },
            { "alertCMethod2SecondaryPointLocation/alertCLocation/specificLocation",
               makeAssigner( m_alertCLocValues.m_specificLocation2 ) }
         };

         uint32 descSize = sizeof( desc ) / sizeof( desc[ 0 ] );
         m_alertCLocExp = new MultiExpression( MultiExpression::
               Description( desc, desc + descSize ) );
      };

   ~AlertCMethod2Expression() {
      delete m_alertCLocExp;
   }

   void operator() ( const DOMNode* node ) {
      m_alertCLocExp->evaluate( node );
   }

private:
   MultiExpression* m_alertCLocExp;
   AlertCLocationValues& m_alertCLocValues;
};

/**
 * Gets the AlertC locations
 */
class AlertCMethod4Expression : public Evaluator {
public:
   explicit AlertCMethod4Expression( AlertCLocationValues& alertCVals ) :
      m_alertCLocExp( NULL ),
      m_alertCLocValues( alertCVals ) {
         MultiExpression::NodeDescription desc[] = {
            { "alertCLocationCountryCode",
               makeAssigner( m_alertCLocValues.m_alertCLocCountryCode ) },
            { "alertCLocationTableNumber",
               makeAssigner( m_alertCLocValues.m_alertCLocTableNbr ) },
            { "alertCLocationTableVersion",
               makeAssigner( m_alertCLocValues.m_alertCLocTableVersion ) },
            { "alertCDirection/alertCDirectionCoded",
               makeAssigner( m_alertCLocValues.m_alertCDirectionCoded ) },
            { "alertCMethod4PrimaryPointLocation/alertCLocation/specificLocation",
               makeAssigner( m_alertCLocValues.m_specificLocation1 ) },
            { "alertCMethod4SecondaryPointLocation/alertCLocation/specificLocation",
               makeAssigner( m_alertCLocValues.m_specificLocation2 ) }
         };

         uint32 descSize = sizeof( desc ) / sizeof( desc[ 0 ] );
         m_alertCLocExp = new MultiExpression( MultiExpression::
               Description( desc, desc + descSize ) );
      };

   ~AlertCMethod4Expression() {
      delete m_alertCLocExp;
   }

   void operator() ( const DOMNode* node ) {
      m_alertCLocExp->evaluate( node );
   }

private:
   MultiExpression* m_alertCLocExp;
   AlertCLocationValues& m_alertCLocValues;
};

/**
 * Gets the information that is common for all situation records.
 * Acts as a super class for the specific types.
 */
class SituationRecordExpression : public Evaluator {
public:
   explicit SituationRecordExpression( SituationValues& sitVals ) :
      m_sitExp( NULL ),
      m_sitValues( sitVals ) {
         MultiExpression::NodeDescription desc[] = {
            { "@id", makeAssigner( m_sitValues.m_sitRecID ) },
            { "situationRecordCreationTime", makeAssigner( m_sitValues.m_sitRecCreationTime ) },
            { "situationRecordVersion", makeAssigner( m_sitValues.m_sitRecVersion ) },
            { "validity/validityStatus", makeAssigner( m_sitValues.m_status ) },
            { "validity/validityTimeSpecification/overallStartTime",
               makeAssigner( m_sitValues.m_overallStartTime ) },
            { "validity/validityTimeSpecification/overallEndTime",
               makeAssigner( m_sitValues.m_overallEndTime ) },
            // TODO: Rewrite so it covers all languages
            { "situationRecordExtension/HeaderExtension/header/value/@lang=se",
               makeAssigner( m_sitValues.m_headerExtension ) },
            { "generalPublicComment/comment/value*/@lang=se",
               makeAssigner( m_sitValues.m_commentSE ) },
            { "generalPublicComment/comment/value/@lang=en",
               makeAssigner( m_sitValues.m_commentEN ) },
            // One of the tmc locations will be chosen
            { "groupOfLocations/locationContainedInGroup*/alertCPoint/@xsi:type=AlertCMethod4Point",
               new AlertCMethod4Expression( m_sitValues.m_location ) },
            { "groupOfLocations/locationContainedInGroup*/alertCPoint/@xsi:type=AlertCMethod2Point",
               new AlertCMethod2Expression( m_sitValues.m_location ) },
            { "groupOfLocations/locationContainedInGroup*/alertCLinear/@xsi:type=AlertCMethod4Linear",
               new AlertCMethod4Expression( m_sitValues.m_location ) },
            { "groupOfLocations/locationContainedInGroup*/alertCLinear/@xsi:type=AlertCMethod2Linear",
               new AlertCMethod2Expression( m_sitValues.m_location ) },
            // In case we dont have a TMC location get the coordinates
            { "groupOfLocations/locationContainedInGroup/pointByCoordinates/pointCoordinates/latitude",
               makeAssigner( m_sitValues.m_coord.lat ) },
            { "groupOfLocations/locationContainedInGroup/pointByCoordinates/pointCoordinates/longitude",
               makeAssigner( m_sitValues.m_coord.lon ) }
         };

         uint32 descSize = sizeof( desc ) / sizeof( desc[ 0 ] );
         m_sitExp = new MultiExpression( MultiExpression::
               Description( desc, desc + descSize ) );
      }

   ~SituationRecordExpression(){
      delete m_sitExp;
   }

   void operator() ( const DOMNode* node ) {
      m_sitExp->evaluate( node );
   }

private:
   MultiExpression* m_sitExp;
   SituationValues& m_sitValues;
};


/**
 * Gets the information that is common for a situation record.
 * And specific information that is unique for an Accident record.
 */
class AccidentExpression : public SituationRecordExpression {
public:
   explicit AccidentExpression( SitValsCont& sre ) :
      SituationRecordExpression( m_accValues ),
      m_accExp( NULL ),
      m_sre( sre ) {
         MultiExpression::NodeDescription desc[] = {
            // XXX: doesnt handle multiple accident nodes for now, defined as
            // [1..*] in DATEX II standard
            { "accidentType*", makeAssigner( m_accValues.m_accidentType ) },
         };

         uint32 descSize = sizeof( desc ) / sizeof( desc[ 0 ] );
         m_accExp = new MultiExpression( MultiExpression::
               Description( desc, desc + descSize ) );
      }

   ~AccidentExpression() {
      delete m_accExp;
   }

   void operator() ( const DOMNode* node ) {
      SituationRecordExpression::operator() ( node );
      m_accExp->evaluate( node );
      m_accValues.m_type = "Accident";
      AccidentValues* copy = new AccidentValues( m_accValues );
      m_sre.push_back( copy );
      m_accValues = AccidentValues();
   }

private:
   MultiExpression* m_accExp;
   AccidentValues m_accValues;
   SitValsCont& m_sre;
};

/**
 * Gets the information that is common for a situation record.
 * And specific information that is unique for an NetworkManagement record.
 */
class NetworkManagementExpression : public SituationRecordExpression {
public:
   explicit NetworkManagementExpression( SitValsCont& sre ) :
      SituationRecordExpression( m_networkManValues ),
      m_networkManExp( NULL ),
      m_sre( sre ) {
         MultiExpression::NodeDescription desc[] = {
            { "networkManagementType", 
               makeAssigner( m_networkManValues.m_networkManagementType ) },
            { "trafficControl/temporarySpeedLimit",
               makeAssigner( m_networkManValues.m_temporarySpeedLimit ) },
            // For the height limit
            { "forVehicleWithCharacteristicsOf/heightCharacteristic/comparisonOperator",
               makeAssigner( m_networkManValues.m_heigthComparisonType ) },
            { "forVehicleWithCharacteristicsOf/heightCharacteristic/vehicleHeight",
               makeAssigner( m_networkManValues.m_vehicleHeight ) },
            // For the width limit
            { "forVehicleWithCharacteristicsOf/widthCharacteristic/comparisonOperator",
               makeAssigner( m_networkManValues.m_widthComparisonType ) },
            { "forVehicleWithCharacteristicsOf/widthCharacteristic/vehicleWidth",
               makeAssigner( m_networkManValues.m_vehicleWidth ) },

         };

         uint32 descSize = sizeof( desc ) / sizeof( desc[ 0 ] );
         m_networkManExp = new MultiExpression( MultiExpression::
               Description( desc, desc + descSize ) );
      }

   ~NetworkManagementExpression() {
      delete m_networkManExp;
   }

   void operator() ( const DOMNode* node ) {
      SituationRecordExpression::operator() ( node );
      m_networkManExp->evaluate( node );
      m_networkManValues.m_type = "NetworkManagement";
      NetworkManagementValues* copy = 
         new NetworkManagementValues( m_networkManValues );
      m_sre.push_back( copy );
      m_networkManValues = NetworkManagementValues();
   }

private:
   MultiExpression* m_networkManExp;
   NetworkManagementValues m_networkManValues;
   SitValsCont& m_sre;
};

/**
 * Gets the information that is common for a situation record.
 * And specific information that is unique for an NetworkManagement record.
 */
class MaintenanceWorksExpression : public SituationRecordExpression {
public:
   explicit MaintenanceWorksExpression( SitValsCont& sre ) :
      SituationRecordExpression( m_maintenanceWorksValues ),
      m_maintenanceWorksExp( NULL ),
      m_sre( sre ) {
         MultiExpression::NodeDescription desc[] = {
            // XXX: dosent handle multiple effectOnRoadLayout and
            // roadMaintenanceType for now, defined as
            // [1..*] in DATEX II standard
            { "effectOnRoadLayout*", 
               makeAssigner( m_maintenanceWorksValues.m_effectOnRoadLayout ) },
            { "roadMaintenanceType*", 
               makeAssigner( m_maintenanceWorksValues.m_roadMaintenanceType ) }
         };

         uint32 descSize = sizeof( desc ) / sizeof( desc[ 0 ] );
         m_maintenanceWorksExp = new MultiExpression( MultiExpression::
               Description( desc, desc + descSize ) );
      }

   ~MaintenanceWorksExpression() {
      delete m_maintenanceWorksExp;
   }

   void operator() ( const DOMNode* node ) {
      SituationRecordExpression::operator() ( node );
      m_maintenanceWorksExp->evaluate( node );
      m_maintenanceWorksValues.m_type = "MaintenanceWorks";
      MaintenanceWorksValues* copy = 
         new MaintenanceWorksValues( m_maintenanceWorksValues );
      m_sre.push_back( copy );
      m_maintenanceWorksValues = MaintenanceWorksValues();
   }

private:
   MultiExpression* m_maintenanceWorksExp;
   MaintenanceWorksValues m_maintenanceWorksValues;
   SitValsCont& m_sre;
};

/**
 * Gets the information that is common for a situation record.
 * And specific information that is unique for an ConstructionWorks record.
 */
class ConstructionWorksExpression : public SituationRecordExpression {
public:
   explicit ConstructionWorksExpression( SitValsCont& sre ) :
      SituationRecordExpression( m_conWorksValues ),
      m_conWorksExp( NULL ),
      m_sre( sre ) {
         MultiExpression::NodeDescription desc[] = {
            // XXX: dosent handle multiple effectOnRoadLayout for now, defined 
            // as [1..*] in DATEX II standard
            { "effectOnRoadLayout*", 
               makeAssigner( m_conWorksValues.m_effectOnRoadLayout ) },
            { "constructionWorkType",
               makeAssigner( m_conWorksValues.m_constructionWorkType ) }
         };

         uint32 descSize = sizeof( desc ) / sizeof( desc[ 0 ] );
         m_conWorksExp = new MultiExpression( MultiExpression::
               Description( desc, desc + descSize ) );
      }

   ~ConstructionWorksExpression() {
      delete m_conWorksExp;
   }

   void operator() ( const DOMNode* node ) {
      SituationRecordExpression::operator() ( node );
      m_conWorksExp->evaluate( node );
      m_conWorksValues.m_type = "ConstructionWorks";
      ConstructionWorksValues* copy = 
         new ConstructionWorksValues( m_conWorksValues );
      m_sre.push_back( copy );
      m_conWorksValues = ConstructionWorksValues();
   }

private:
   MultiExpression* m_conWorksExp;
   ConstructionWorksValues m_conWorksValues;
   SitValsCont& m_sre;
};

/**
 * Gets the information that is common for a situation record.
 * And specific information that is unique for an AbnormalTraffic record.
 */
class AbnormalTrafficExpression : public SituationRecordExpression {
public:
   explicit AbnormalTrafficExpression( SitValsCont& sre ) :
      SituationRecordExpression( m_abnormalTrafficValues ),
      m_abnormalTrafficExp( NULL ),
      m_sre( sre ) {
         MultiExpression::NodeDescription desc[] = {
            { "abnormalTrafficType", 
               makeAssigner( m_abnormalTrafficValues.m_abnormalTrafficType ) },
            { "trafficTrendType",
               makeAssigner( m_abnormalTrafficValues.m_trafficTrend ) }
         };

         uint32 descSize = sizeof( desc ) / sizeof( desc[ 0 ] );
         m_abnormalTrafficExp = new MultiExpression( MultiExpression::
               Description( desc, desc + descSize ) );
      }

   ~AbnormalTrafficExpression() {
      delete m_abnormalTrafficExp;
   }

   void operator() ( const DOMNode* node ) {
      SituationRecordExpression::operator() ( node );
      m_abnormalTrafficExp->evaluate( node );
      m_abnormalTrafficValues.m_type = "AbnormalTraffic";
      AbnormalTrafficValues* copy = 
         new AbnormalTrafficValues( m_abnormalTrafficValues );
      m_sre.push_back( copy );
      m_abnormalTrafficValues = AbnormalTrafficValues();
   }

private:
   MultiExpression* m_abnormalTrafficExp;
   AbnormalTrafficValues m_abnormalTrafficValues;
   SitValsCont& m_sre;
};

/**
 * Gets the information that is common for a situation record.
 * And specific information that is unique for an Activities record.
 */
class ActivitiesExpression : public SituationRecordExpression {
public:
   explicit ActivitiesExpression( SitValsCont& sre ) :
      SituationRecordExpression( m_activitiesValues ),
      m_activitiesExp( NULL ),
      m_sre( sre ) {
         MultiExpression::NodeDescription desc[] = {
            { "disturbanceActivityType", 
               makeAssigner( m_activitiesValues.m_disturbanceActivityType ) },
            { "publicEventType",
               makeAssigner( m_activitiesValues.m_publicEventType ) }
         };

         uint32 descSize = sizeof( desc ) / sizeof( desc[ 0 ] );
         m_activitiesExp = new MultiExpression( MultiExpression::
               Description( desc, desc + descSize ) );
      }

   ~ActivitiesExpression() {
      delete m_activitiesExp;
   }
   
   void operator() ( const DOMNode* node ) {
      SituationRecordExpression::operator() ( node );
      m_activitiesExp->evaluate( node );
      m_activitiesValues.m_type = "Activities";
      ActivitiesValues* copy = new ActivitiesValues( m_activitiesValues );
      m_sre.push_back( copy );
      m_activitiesValues = ActivitiesValues();
   }

private:
   MultiExpression* m_activitiesExp;
   ActivitiesValues m_activitiesValues;
   SitValsCont& m_sre;
};

/**
 * Gets the information that is common for a situation record.
 * And specific information that is unique for an AnimalPresenceObstruction 
 * record.
 */
class AnimalPresenceObstructionExpression : public SituationRecordExpression {
public:
   explicit AnimalPresenceObstructionExpression( SitValsCont& sre ) :
      SituationRecordExpression( m_animalPresenceObstrValues ),
      m_animalPresenceObstrExp( NULL ),
      m_sre( sre ) {
         MultiExpression::NodeDescription desc[] = {
            { "animalPresenceType", 
              makeAssigner( m_animalPresenceObstrValues.m_animalPresenceType ) }
         };

         uint32 descSize = sizeof( desc ) / sizeof( desc[ 0 ] );
         m_animalPresenceObstrExp = new MultiExpression( MultiExpression::
               Description( desc, desc + descSize ) );
      }

   ~AnimalPresenceObstructionExpression() {
      delete m_animalPresenceObstrExp;
   }

   void operator() ( const DOMNode* node ) {
      SituationRecordExpression::operator() ( node );
      m_animalPresenceObstrExp->evaluate( node );
      m_animalPresenceObstrValues.m_type = "AnimalPresenceObstruction";
      AnimalPresenceObstructionValues* copy = 
         new AnimalPresenceObstructionValues( m_animalPresenceObstrValues );
      m_sre.push_back( copy );
      m_animalPresenceObstrValues = AnimalPresenceObstructionValues();
   }

private:
   MultiExpression* m_animalPresenceObstrExp;
   AnimalPresenceObstructionValues m_animalPresenceObstrValues;
   SitValsCont& m_sre;
};

/**
 * Gets the information that is common for a situation record.
 * And specific information that is unique for an GeneralObstruction record.
 */
class GeneralObstructionExpression : public SituationRecordExpression {
public:
   explicit GeneralObstructionExpression( SitValsCont& sre ) :
      SituationRecordExpression( m_generalObstrValues ),
      m_generalObstrExp( NULL ),
      m_sre( sre ) {
         MultiExpression::NodeDescription desc[] = {
            { "obstructionType", 
               makeAssigner( m_generalObstrValues.m_obstructionType ) }
         };

         uint32 descSize = sizeof( desc ) / sizeof( desc[ 0 ] );
         m_generalObstrExp = new MultiExpression( MultiExpression::
               Description( desc, desc + descSize ) );
      }

   ~GeneralObstructionExpression() {
      delete m_generalObstrExp;
   }

   void operator() ( const DOMNode* node ) {
      SituationRecordExpression::operator() ( node );
      m_generalObstrExp->evaluate( node );
      m_generalObstrValues.m_type = "GeneralObstruction";
      GeneralObstructionValues* copy = 
         new GeneralObstructionValues( m_generalObstrValues );
      m_sre.push_back( copy );
      m_generalObstrValues = GeneralObstructionValues();
   }

private:
   MultiExpression* m_generalObstrExp;
   GeneralObstructionValues m_generalObstrValues;
   SitValsCont& m_sre;
};

/**
 * Gets the information that is common for a situation record.
 * And specific information that is unique for an
 * NonWeatherRelatedRoadConditions record.
 */
class NonWeatherRelatedRoadConditionsExpression : 
   public SituationRecordExpression {
public:
   explicit NonWeatherRelatedRoadConditionsExpression( SitValsCont& sre ) :
      SituationRecordExpression( m_nonWeatherRRCValues ),
      m_nonWeatherRelRoadConExp( NULL ),
      m_sre( sre ) {
         MultiExpression::NodeDescription desc[] = {
            { "nonWeatherRelatedRoadConditionType",
               makeAssigner( 
                     m_nonWeatherRRCValues.m_nonWeatherRelatedConditionType ) }
         };

         uint32 descSize = sizeof( desc ) / sizeof( desc[ 0 ] );
         m_nonWeatherRelRoadConExp = new MultiExpression( MultiExpression::
               Description( desc, desc + descSize ) );
      }

   ~NonWeatherRelatedRoadConditionsExpression() {
      delete m_nonWeatherRelRoadConExp;
   }

   void operator() ( const DOMNode* node ) {
      SituationRecordExpression::operator() ( node );
      m_nonWeatherRelRoadConExp->evaluate( node );
      m_nonWeatherRRCValues.m_type = "NonWeatherRelatedRoadConditions";
      NonWeatherRelatedRoadConditionsValues* copy =
         new NonWeatherRelatedRoadConditionsValues( m_nonWeatherRRCValues );
      m_sre.push_back( copy );
      m_nonWeatherRRCValues = NonWeatherRelatedRoadConditionsValues();
   }

private:
   MultiExpression* m_nonWeatherRelRoadConExp;
   NonWeatherRelatedRoadConditionsValues m_nonWeatherRRCValues;
   SitValsCont& m_sre;
};

/**
 * Gets the information that is common for a situation record.
 * And specific information that is unique for an VehicleObstruction record.
 */
class VehicleObstructionExpression : public SituationRecordExpression {
public:
   explicit VehicleObstructionExpression( SitValsCont& sre ) :
      SituationRecordExpression( m_vehicleObstrValues ),
      m_vehicleObstrExp( NULL ),
      m_sre( sre ) {
         MultiExpression::NodeDescription desc[] = {
            { "vehicleObstructionType", 
               makeAssigner( m_vehicleObstrValues.m_vehicleObstructionType ) }
         };

         uint32 descSize = sizeof( desc ) / sizeof( desc[ 0 ] );
         m_vehicleObstrExp = new MultiExpression( MultiExpression::
               Description( desc, desc + descSize ) );
      }

   ~VehicleObstructionExpression() {
      delete m_vehicleObstrExp;
   }

   void operator() ( const DOMNode* node ) {
      SituationRecordExpression::operator() ( node );
      m_vehicleObstrExp->evaluate( node );
      m_vehicleObstrValues.m_type = "VehicleObstruction";
      VehicleObstructionValues* copy = 
         new VehicleObstructionValues( m_vehicleObstrValues );
      m_sre.push_back( copy );
      m_vehicleObstrValues = VehicleObstructionValues();
   }

private:
   MultiExpression* m_vehicleObstrExp;
   VehicleObstructionValues m_vehicleObstrValues;
   SitValsCont& m_sre;
};

/// Parses for situation records and creates a TrafficSituation
class SituationExpression : public Evaluator {
public:
   explicit SituationExpression( vector<TrafficSituation*>& trafficSituations,
                                 const MC2String& provider ):
      m_situationExp( NULL ),
      m_trafficSituations( trafficSituations ),
      m_provider( provider ),
      m_sitID(),
      m_sre() {
         /*
            Conditions
            EnvironmentalObstruction
            PoorRoadInfrastructure
            */
         MultiExpression::NodeDescription desc[] = {
            { "/@id", makeAssigner( m_sitID ) },
            { "situationRecord*/@xsi:type=Accident", 
               new AccidentExpression( m_sre ) },
            { "situationRecord/@xsi:type=NetworkManagement", 
               new NetworkManagementExpression( m_sre ) },
            { "situationRecord/@xsi:type=MaintenanceWorks",
               new MaintenanceWorksExpression( m_sre ) },
            { "situationRecord/@xsi:type=ConstructionWorks", 
               new ConstructionWorksExpression( m_sre ) },
            { "situationRecord/@xsi:type=AbnormalTraffic", 
               new AbnormalTrafficExpression( m_sre ) },
            { "situationRecord/@xsi:type=Activities",
               new ActivitiesExpression( m_sre ) },
            { "situationRecord/@xsi:type=AnimalPresenceObstruction",
               new AnimalPresenceObstructionExpression( m_sre ) },
            { "situationRecord/@xsi:type=GeneralObstruction",
               new GeneralObstructionExpression( m_sre ) },
            { "situationRecord/@xsi:type=NonWeatherRelatedRoadConditions",
               new NonWeatherRelatedRoadConditionsExpression( m_sre ) },
            { "situationRecord/@xsi:type=VehicleObstruction",
               new VehicleObstructionExpression( m_sre ) }
         };
         uint32 descSize = sizeof( desc ) / sizeof( desc[ 0 ] );
         m_situationExp = new MultiExpression( MultiExpression::
               Description( desc, desc + descSize ) );
      }

   ~SituationExpression() {
      delete m_situationExp;
   }

   void operator() ( const DOMNode* node ) {
      m_situationExp->evaluate( node );
      // creates a situation by going through the situationRecords in it
      auto_ptr< TrafficSituation > trafficSit = 
         createTrafficSit( m_provider, m_sre, m_sitID );
      if ( trafficSit.get() ) {
         //trafficSit->setSituationReference( m_sitID );
         m_trafficSituations.push_back( trafficSit.release() );
      } else {
         mc2dbg4 << info << "Situation: [" << m_sitID << "] has no situation " 
                << "records. Not creating a disturbance." << endl;
      }
      STLUtility::deleteValues( m_sre );
   }

private:
   MultiExpression* m_situationExp;
   vector< TrafficSituation* >& m_trafficSituations;
   MC2String m_provider;
   MC2String m_sitID;
   SitValsCont m_sre;
};

} // DatexIIStructs

#endif // DATEXIISTRUCTS_H
