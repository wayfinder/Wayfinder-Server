/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "DatexIIStructs.h"

#include "TrafficSituation.h"
#include "TrafficDataTypes.h"
#include "DatexUtilsII.h"
#include "ISO8601TimeParser.h"
#include "TimeUtility.h"
#include "CoordinateTransformer.h"
#include "STLStringUtility.h"

#define DIITP "[DIITP:" << __LINE__ << "] "

namespace DatexIIStructs {

typedef TrafficSituationElement::CoordCont CoordCont;

time_t getEstimatedEndTime( TrafficDataTypes::disturbanceType type ) {
   map< TrafficDataTypes::disturbanceType, uint32 >::const_iterator it;
   it = DatexUtilsII::c_estEndTimeMap.find( type );
   if ( it != DatexUtilsII::c_estEndTimeMap.end() ) {
      return it->second + TimeUtility::getRealTime();
   } else {
      // Should not happen that the type is not in the map
      // but just in case we set the end time to one day
      return 60*60*24 + TimeUtility::getRealTime();
   }
}

bool getLocation( MC2String& firstLocation, 
                  MC2String& secondLocation,
                  MC2String& tmcVersion,
                  TrafficDataTypes::direction& direction,
                  TrafficSituationElement::CoordCont& coords,
                  const SituationValues& sitRec ) {
   // Get the TMC locations if none are present use coordinates instead
   if ( sitRec.m_location.getTMCLocations( firstLocation, secondLocation ) ) {
      // Get the direction traffic data type
      MC2String tmcDirection = sitRec.m_location.m_alertCDirectionCoded;
      if ( tmcDirection == "positive" ) {
         direction = TrafficDataTypes::Positive;
      } else if ( tmcDirection == "negative" ) {
         direction = TrafficDataTypes::Negative;
      } else if ( tmcDirection == "both" ) {
         direction = TrafficDataTypes::BothDirections;
      }

      // Get the tmc version
      tmcVersion = sitRec.m_location.m_alertCLocTableVersion;
      return true;
   } else {

      if ( sitRec.m_coord.isValid() ) {
         // Get coordinates instead
         // Create MC2 coordinates
         MC2Coordinate mc2coord = 
            CoordinateTransformer::transformToMC2( sitRec.m_coord );

         if ( mc2coord.isValid() ) { 
            coords.push_back( TrafficSituationElement::CoordCont::
                              value_type( mc2coord.lat, mc2coord.lon ) );
            return true;
         }
      }

      mc2dbg4 << error << "No cordinates and no TMC location!" << endl;
      return false;
   }
}

bool vehicleWithinLimits( const NetworkManagementValues& nmv ) {
   // These are just initial values might be changed
   const float64 CAR_MIN_WIDTH = 1.0;
   const float64 CAR_MAX_WIDTH = 2.5;
   const float64 CAR_MIN_HEIGHT = 1.0;
   const float64 CAR_MAX_HEIGHT = 2.5;
   bool withinLimits = true;

   // See if there is any width characteristics limits to this situation
   float64 width;
   if ( !nmv.m_widthComparisonType.empty() &&
        STLStringUtility::strtod( nmv.m_vehicleWidth, width ) ) {
      // Check that its within the width limits
      if ( nmv.m_widthComparisonType == "greaterThan" ) {
         // If its greater then the limit this situation record applies for it
         withinLimits = CAR_MAX_WIDTH > width;
      } else if ( nmv.m_widthComparisonType == "lessThan" ) {
         // If its less then the limit this situation record applies for it
         withinLimits = CAR_MIN_WIDTH < width;
      } else if ( nmv.m_widthComparisonType == "equalTo" ) {
         // have a hard time seeing this being used, but it could so...
         withinLimits = CAR_MAX_WIDTH == width;
      }
   }

   // See if there is any heigth characteristics limits to this situation
   float64 heigth;
   if ( !nmv.m_heigthComparisonType.empty() &&
        STLStringUtility::strtod( nmv.m_vehicleHeight, heigth ) ) {
      // Check that its within the heigth limits
      if ( nmv.m_heigthComparisonType == "greaterThan" ) {
         // If its greater then the limit this situation record applies for it
         withinLimits = CAR_MAX_HEIGHT > heigth;
      } else if ( nmv.m_heigthComparisonType == "lessThan" ) {
         // If its less then the limit this situation record applies for it
         withinLimits = CAR_MIN_HEIGHT < heigth;
      } else if ( nmv.m_heigthComparisonType == "equalTo" ) {
         // have a hard time seeing this being used, but it could so...
         withinLimits = CAR_MAX_HEIGHT == heigth;
      }   
   }
   return withinLimits;
}

TrafficDataTypes::disturbanceType
getDisturbanceType( const SituationValues& sitRec ) {
   // Find the disturbance type
   map< MC2String, TrafficDataTypes::disturbanceType >::const_iterator typeIt;
   typeIt = DatexUtilsII::c_typeMap.find( sitRec.m_type );
   if ( typeIt != DatexUtilsII::c_typeMap.end() ) {
      return typeIt->second;
   } else {
      return TrafficDataTypes::NoType;
   }
}

bool getPhraseCode( MC2String& phraseCode, const SituationValues& sitRec ) {
   bool gotPhrase = true;
   
   switch ( getDisturbanceType( sitRec ) ) {
      case TrafficDataTypes::Accident:
         {
            const AccidentValues& tmp = 
               static_cast<const AccidentValues&>( sitRec );
            phraseCode = tmp.m_accidentType;
            
            mc2dbg4 << "[Accident type]: " << tmp.m_accidentType << endl;
         }
         break;
      case TrafficDataTypes::MaintenanceWorks:
         {
            const MaintenanceWorksValues& tmp = 
               static_cast<const MaintenanceWorksValues&>( sitRec );
            phraseCode = tmp.m_roadMaintenanceType;

            mc2dbg4 << "[Effect on road layout]: " 
                   << tmp.m_effectOnRoadLayout << endl;
            mc2dbg4 << "[Road maintenance type]: " 
                   << tmp.m_roadMaintenanceType << endl;
         }
         break;
      case TrafficDataTypes::NetworkManagement:
         {
            const NetworkManagementValues& tmp = 
               static_cast<const NetworkManagementValues&>( sitRec );
            // Check if its has vehicle restrictions
            gotPhrase = vehicleWithinLimits( tmp );
            phraseCode = tmp.m_networkManagementType;

            mc2dbg4 << "[Network management type]: " 
                   << tmp.m_networkManagementType << endl;
            mc2dbg4 << "[Temporary Speed Limit]: " 
                   << tmp.m_temporarySpeedLimit << endl;
         }
         break;
      case TrafficDataTypes::ConstructionWorks:
         {
            const ConstructionWorksValues& tmp =
               static_cast<const ConstructionWorksValues&>( sitRec );
            phraseCode = tmp.m_constructionWorkType;

            mc2dbg4 << "[Construction work type]: "
                   << tmp.m_constructionWorkType << endl;
            mc2dbg4 << "[Effect on road layout]: " 
                   << tmp.m_effectOnRoadLayout << endl;
         }
         break;
      case TrafficDataTypes::AbnormalTraffic:
         {
            const AbnormalTrafficValues& tmp = 
               static_cast<const AbnormalTrafficValues&>( sitRec );
            phraseCode = tmp.m_abnormalTrafficType;
            
            mc2dbg4 << "[Abnormal traffic type]: " 
                   << tmp.m_abnormalTrafficType << endl;
            mc2dbg4 << "[Traffic trend]: " << tmp.m_trafficTrend << endl;
         }
         break;
      case TrafficDataTypes::Activities:
         {
            const ActivitiesValues& tmp = 
               static_cast<const ActivitiesValues&>( sitRec );
            if ( !tmp.m_disturbanceActivityType.empty() ) {
               phraseCode = tmp.m_disturbanceActivityType;
            } else {
               phraseCode = tmp.m_publicEventType;
            }
         }
         break;
      case TrafficDataTypes::AnimalPresenceObstruction:
         {
            const AnimalPresenceObstructionValues& tmp = 
               static_cast<const AnimalPresenceObstructionValues&>( sitRec );
            phraseCode = tmp.m_animalPresenceType;
         }
         break;
      case TrafficDataTypes::GeneralObstruction:
         {
            const GeneralObstructionValues& tmp = 
               static_cast<const GeneralObstructionValues&>( sitRec );
            phraseCode = tmp.m_obstructionType;
         }
         break;
      case TrafficDataTypes::NonWeatherRelatedRoadConditions:
         {
            const NonWeatherRelatedRoadConditionsValues& tmp = 
               static_cast
               <const NonWeatherRelatedRoadConditionsValues&>( sitRec );
            phraseCode = tmp.m_nonWeatherRelatedConditionType;
         }
         break;
      case TrafficDataTypes::VehicleObstruction:
         {
            const VehicleObstructionValues& tmp = 
               static_cast<const VehicleObstructionValues&>( sitRec );
            phraseCode = tmp.m_vehicleObstructionType;
         }
         break;
      default:
         gotPhrase = false;
         mc2dbg4 << warn << "In the default case!" << endl;
         break;
   }

   return gotPhrase;
}

TrafficDataTypes::phrase getPhrase( const MC2String& phraseCode ) {
   // Mapping the phraseCode to a TrafficDataTypes::phrase type
   map< MC2String, TrafficDataTypes::phrase >::const_iterator it;
   it = DatexUtilsII::c_phraseMap.find( phraseCode );
   if ( it != DatexUtilsII::c_phraseMap.end() ) {
      return it->second;
   } else {
      return TrafficDataTypes::NoPhrase;
   }
}

TrafficDataTypes::severity getSeverity( const TrafficDataTypes::phrase phrase ) {
   // Finding severity
   map< TrafficDataTypes::phrase, TrafficDataTypes::severity >::
      const_iterator sevIt;
   sevIt = DatexUtilsII::c_phraseToSeverity.find( phrase );
   if( sevIt != DatexUtilsII::c_phraseToSeverity.end() ) {
      return sevIt->second;
   } else {
      return TrafficDataTypes::NoSeverity;
   }
}


auto_ptr< TrafficSituation > 
createTrafficSit( const MC2String& provider,
                  const SitValsCont& sre, 
                  const MC2String& sitID ) {
   mc2dbg4 << info << sre.size() << " situation records to go through." << endl;
   // check if there are any situation records to parse
   if ( sre.empty() ) {
      mc2dbg4 << warn << "situation id: " << sitID << endl;
      mc2dbg4 << warn << "Does not contain any situation records we can parse" 
              << endl;
      // return NULL, skip this situation..
      return auto_ptr< TrafficSituation >();
   }
   
   
   MC2String elementReference;
   time_t isoStartTime( 0 );
   time_t isoStopTime( 0 );
   time_t isoCreationTime( 0 );
   TrafficDataTypes::disturbanceType distType( TrafficDataTypes::NoType );
   TrafficDataTypes::phrase phrase( TrafficDataTypes::NoPhrase );
   TrafficDataTypes::severity severity( TrafficDataTypes::NoSeverity );
   MC2String comment;
   MC2String firstLocation, secondLocation;
   TrafficDataTypes::direction direction(TrafficDataTypes::NoDirection);
   CoordCont coords;
   MC2String tmcVersion = "";
   uint32 costFactor = 0;
   MC2String phraseCode;

   bool oneSitRecDone = false;
   SitValsCont::const_iterator sitRec;
   for ( sitRec = sre.begin(); sitRec != sre.end(); ++sitRec ) {

      // check if the validity is set to suspended and then skip it
      if ( (*sitRec)->m_status == "suspended" ) {
         mc2dbg4 << info << "Situation record: [" << (*sitRec)->m_sitRecID
                << "] suspended. Skipping it." << endl;
         // Skip this situation record
         continue;
      }

      if ( !oneSitRecDone ) {
         elementReference = (*sitRec)->m_sitRecID;
         // Find the disturbance type
         distType = getDisturbanceType( *(*sitRec) );
         mc2dbg4 << info << "[dist type] " << (*sitRec)->m_type << endl;

         // Set the start and creation time
         if ( (*sitRec)->m_overallStartTime.empty() || 
              (*sitRec)->m_sitRecCreationTime.empty() ) {
            mc2dbg4 << warn << DIITP << "Situation record: " 
               << (*sitRec)->m_sitRecID << " has no start or creation time!"
               << " Skipping situation record." << endl;
            // Skip this situation record
            continue;
         } else {
            isoStartTime = ISO8601Time::getTime( (*sitRec)->m_overallStartTime );
            isoCreationTime = ISO8601Time::getTime( (*sitRec)->m_sitRecCreationTime );
         }

         // check if the situation has a end time since it's optional
         if ( (*sitRec)->m_overallEndTime.empty() ) {
            isoStopTime = getEstimatedEndTime( distType );
         } else {
            // TODO: fix ISO8601Time::getTime() it dosent work corectly without
            // timezones
            isoStopTime = ISO8601Time::getTime( (*sitRec)->m_overallEndTime );
         }

         // check if there is a comment in english first, if not chose the 
         // country language. If none exists say so.
         comment += (*sitRec)->m_headerExtension;
         if ( !(*sitRec)->m_commentEN.empty() ) {
            comment += " - ";
            comment += (*sitRec)->m_commentEN;
         } else if ( !(*sitRec)->m_commentSE.empty() ) {
            comment += " - ";
            comment += (*sitRec)->m_commentSE;
         } else {
            comment += " - No comment.";
         }

         // Get the location
         if ( !getLocation( firstLocation, secondLocation, tmcVersion, direction,
                  coords, *(*sitRec) ) ) {
            // No valid location that we can handle in the situation record.
            // Skip this situation record
            continue;
         }

         // Get phrase, severity and costfactor so we can see which one to keep.
         if ( !getPhraseCode( phraseCode, *(*sitRec) ) ) {
            mc2dbg4 << warn << " getPhraseCode false !!!" << endl;
            // Skip this situation record
            continue;
         }
         phrase = getPhrase( phraseCode );
         severity = getSeverity( phrase );
         costFactor = TrafficDataTypes::getCostFactorFromSeverity( severity );

         // if all ok
         oneSitRecDone = true;
      } else {
         // TODO: Use this to save the severity that has the worst cost and create
         // one TrafficSituationElement instead of all since it dosent seem like
         // it goes through them all in DATEXRequest. Instead it just uses the
         // first one in the vector.

         MC2String tmpPhraseCode;
         if ( !getPhraseCode( tmpPhraseCode, *(*sitRec) ) ) {
            mc2dbg4 << warn << " getPhraseCode false !!!" << endl;
            // Skip this situation record
            continue;
         }
         TrafficDataTypes::phrase tmpPhrase =
            getPhrase( tmpPhraseCode );
         TrafficDataTypes::severity tmpSeverity = getSeverity( tmpPhrase );

         // If the new cost factor is bigger then use the new data
         if ( costFactor < 
               TrafficDataTypes::getCostFactorFromSeverity( tmpSeverity ) ) {
            costFactor = 
               TrafficDataTypes::getCostFactorFromSeverity( tmpSeverity );
            phrase = tmpPhrase;
            severity = tmpSeverity;
         }
      }
   }

   // TODO: Done with all situation records in a situation, now its time to
   // create a TrafficSituationElement and add it in to the TrafficSituation.
   // However we need to check that there is any data to create a
   // TrafficSituationElement since there might be a case where we only have
   // one element and its suspended or that all elements are suspended.
   if ( !oneSitRecDone ) {
      mc2dbg4 << warn << DIITP << "Situation record: [" << sitID 
         << "] no elements to parse, all suspended or incorrect data." << endl;
      // return NULL
      return auto_ptr< TrafficSituation >();
   }

   TrafficSituationElement* situationElement =
      new TrafficSituationElement( elementReference,
                                   isoStartTime,
                                   isoStopTime,
                                   isoCreationTime,
                                   distType, 
                                   phrase,
                                   severity,
                                   comment,
                                   firstLocation,
                                   secondLocation,
                                   MAX_UINT16,
                                   0, // extent
                                   direction,
                                   0,  // Queue length
                                   coords,
                                   tmcVersion ); //optional


   auto_ptr< TrafficSituation > trafficSit( new TrafficSituation() );
   trafficSit->addSituationElement( situationElement );
   trafficSit->setLocationTable( tmcVersion );
   MC2String sitRef = provider + ":" + sitID;
   trafficSit->setSituationReference( sitRef );

   return trafficSit;
}

} // DatexIIStructs
