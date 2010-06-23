/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "DatexUtilsII.h"

map<MC2String, TrafficDataTypes::phrase> DatexUtilsII::c_phraseMap;

map<TrafficDataTypes::phrase, TrafficDataTypes::severity> 
DatexUtilsII::c_phraseToSeverity;

map<MC2String, TrafficDataTypes::disturbanceType> DatexUtilsII::c_typeMap;

map< TrafficDataTypes::disturbanceType, uint32 > DatexUtilsII::c_estEndTimeMap;

bool 
DatexUtilsII::initializeTables()
{
   {
      typedef std::pair<const char*, TrafficDataTypes::phrase> pp;
      const pp phrase_map[] = {
         // *******************************************************************
         // AtoD, AbnormalTrafficTypeEnum
         // *******************************************************************
         pp("stationaryTraffic", TrafficDataTypes::LS1), 
         pp("slowTraffic", TrafficDataTypes::LS3), 
         pp("queueingTraffic", TrafficDataTypes::LS2),   
         pp("stopAndGo", TrafficDataTypes::stopAndGo),  

         // *******************************************************************
         // AtoD, AccidentTypeEnum
         // *******************************************************************
         pp("jackknifedArticulatedLorry", TrafficDataTypes::AJA), 
         pp("jackknifedCaravan", TrafficDataTypes::AJC),       
         pp("jackknifedTrailer", TrafficDataTypes::AJT),       
         pp("multivehicleAccident", TrafficDataTypes::ACM),    
         pp("fuelSpillageAccident", TrafficDataTypes::ACF),    
         pp("overturnedHeavyLorry", TrafficDataTypes::AOL),   
         pp("overturnedVehicle", TrafficDataTypes::AOV),       
         pp("secondaryAccident", TrafficDataTypes::ACD),       
         pp("seriousAccident", TrafficDataTypes::ACS),         
         pp("oilSpillageAccident", TrafficDataTypes::AOI),     
         pp("vehicleSpunAround", TrafficDataTypes::ASP),       
         pp("accident", TrafficDataTypes::ACI),                
         pp("accidentInvestigationWork", TrafficDataTypes::ACW),
         pp("accidentInvolvingBuses", TrafficDataTypes::ACB),  
         pp("earlierAccident", TrafficDataTypes::ACA),         
         pp("accidentInvolvingHeavyLorries", TrafficDataTypes::ACH), 
         pp("chemicalSpillageAccident", TrafficDataTypes::ACE),
         pp("accidentInvolvingHazardousMaterials", TrafficDataTypes::ACZ),
         pp("overturnedTrailer", TrafficDataTypes::overturnedTrailer),   
         pp("headOnCollision", TrafficDataTypes::headOnCollision),
         pp("headOnOrSideCollision", TrafficDataTypes::headOnOrSideCollision), 
         pp("multipleVehicleCollision", 
               TrafficDataTypes::multipleVehicleCollision), 
         pp("sideCollision", TrafficDataTypes::sideCollision), 
         pp("rearCollision", TrafficDataTypes::rearCollision), 
         pp("vehicleOffRoad", TrafficDataTypes::vehicleOffRoad), 
         pp("accidentInvolvingMopeds", 
               TrafficDataTypes::accidentInvolvingMopeds), 
         pp("accidentInvolvingBicycles", 
               TrafficDataTypes::accidentInvolvingBicycles),
         pp("accidentInvolvingMotorcycles", 
               TrafficDataTypes::accidentInvolvingMotorcycles),
         pp("accidentInvolvingTrain", TrafficDataTypes::accidentInvolvingTrain),
         pp("collision", TrafficDataTypes::collision),
         pp("collisionWithAnimal", TrafficDataTypes::collisionWithAnimal),
         pp("collisionWithObstruction", 
               TrafficDataTypes::collisionWithObstruction),
         pp("collisionWithPerson", TrafficDataTypes::collisionWithPerson),
         //pp("other", TrafficDataTypes::),                    // 

         // *******************************************************************
         // AtoD, AnimalPresenceTypeEnum
         // *******************************************************************
         pp("animalsOnTheRoad", TrafficDataTypes::ANM), 
         pp("herdOfAnimalsOnTheRoad", TrafficDataTypes::ANH),
         pp("largeAnimalsOnTheRoad", TrafficDataTypes::ANL), 

         // *******************************************************************
         // AtoD, CarParkStatusEnum
         // *******************************************************************
         pp("noMoreParkingSpacesAvailable", TrafficDataTypes::PMN),
         pp("multiStoryCarParksFull", TrafficDataTypes::PMS),  
         pp("enoughSpacesAvailable", TrafficDataTypes::PSA),   
         pp("carParkFull", TrafficDataTypes::PKO), 
         pp("allCarParksFull", TrafficDataTypes::PKO), 
         pp("noParkAndRideInformation", TrafficDataTypes::PRC),
         pp("noParkingAllowed", TrafficDataTypes::PFN),
         pp("noParkingInformationAvailable", TrafficDataTypes::PIX), 
         pp("normalParkingRestrictionsLifted", TrafficDataTypes::PRL),
         pp("onlyAFewSpacesAvailable", TrafficDataTypes::PFS), 
         pp("parkAndRideServiceNotOperating", TrafficDataTypes::PRN),
         pp("parkAndRideServiceOperating", TrafficDataTypes::PRO), 
         pp("specialParkingRestrictionsInForce", TrafficDataTypes::PSR),
         //pp("carParkFacilityFaulty", TrafficDataTypes::), 
         //pp("carParkClosed", TrafficDataTypes::),
         //pp("carParkStatusUnknown", TrafficDataTypes::),

         // *******************************************************************
         // AtoD, ConstructionWorkTypeEnum
         // *******************************************************************
         pp("constructionWork", TrafficDataTypes::RCW), 
         pp("blastingWork", TrafficDataTypes::RBL),
         pp("demolitionWork", TrafficDataTypes::demolitionWork), 

         // *******************************************************************
         // AtoD, DelaysTypeEnum
         // *******************************************************************
         pp("veryLongDelays", TrafficDataTypes::DVL),
         pp("longDelays", TrafficDataTypes::DLL), 
         pp("delaysOfUncertainDuration", TrafficDataTypes::DUD), 
         pp("delays", TrafficDataTypes::DLY),

         // *******************************************************************
         // AtoD, DisturbanceActivityTypeEnum
         // *******************************************************************
         pp("airRaid", TrafficDataTypes::AIR),
         pp("terroristIncident", TrafficDataTypes::STI), 
         pp("strike", TrafficDataTypes::EST ), 
         pp("sightseersObstructingAccess", TrafficDataTypes::ESS), 
         pp("securityAlert", TrafficDataTypes::ESA),
         pp("publicDisturbance", TrafficDataTypes::EPD), 
         pp("procession", TrafficDataTypes::EPR),
         pp("march", TrafficDataTypes::EMR), 
         pp("gunfireOnRoadway", TrafficDataTypes::GUN), 
         pp("evacuation", TrafficDataTypes::EVA),
         pp("crowd", TrafficDataTypes::ECR), 
         pp("bombAlert", TrafficDataTypes::EBA), 
         pp("demonstration", TrafficDataTypes::EVD), 
         pp("other", TrafficDataTypes::OtherPhrase), 
         pp("civilEmergency", TrafficDataTypes::SCI),
         //pp("sabotage", TrafficDataTypes::),
         //pp("theft", TrafficDataTypes::), 
         //pp("securityIncident", TrafficDataTypes::), 
         //pp("explosionHazard", TrafficDataTypes::),
         //pp("explosion", TrafficDataTypes::),
         //pp("attack", TrafficDataTypes::),  
         //pp("assault", TrafficDataTypes::),
         //pp("assetDestruction", TrafficDataTypes::), 
         //pp("filterBlockade", TrafficDataTypes::),  
         //pp("goSlowOperation", TrafficDataTypes::),
         //pp("illVehicleOccupants", TrafficDataTypes::),
         //pp("altercationOfVehicleOccupants", TrafficDataTypes::),

         // *******************************************************************
         // AtoD, DiversionAdviceEnum
         // *******************************************************************
         pp("diversionInOperation", TrafficDataTypes::DO), 
         //pp("followSpecialDiversionMarkers", TrafficDataTypes::), 
         //pp("noSuitableDiversionAvailable", TrafficDataTypes::), 
         //pp("localDriversAreRecommendedToAvoidTheArea", TrafficDataTypes::),
         //pp("other", TrafficDataTypes::), 
         //pp("heavyLorriesAreRecommendedToAvoidTheArea", TrafficDataTypes::),
         //pp("followLocalDiversion", TrafficDataTypes::), 
         //pp("diversionIsNoLongerRecommended", TrafficDataTypes::), 
         //pp("compulsoryDiversionInOperation", TrafficDataTypes::),
         //pp("followDiversionSigns", TrafficDataTypes::), 
         //pp("followSigns", TrafficDataTypes::), 
         //pp("doNotFollowDiversionSigns", TrafficDataTypes::), 

         // *******************************************************************
         // AtoD, DrivingConditionTypeEnum
         // *******************************************************************
         pp("hazardous", TrafficDataTypes::DCZ),
         pp("veryHazardous", TrafficDataTypes::DCE), 
         //pp("winterConditions", TrafficDataTypes::),
         //pp("unknown", TrafficDataTypes::),
         //pp("normal", TrafficDataTypes::), 
         //pp("passableWithCare", TrafficDataTypes::),
         //pp("other", TrafficDataTypes::),
         //pp("impossible", TrafficDataTypes::),

         // *******************************************************************
         // EtoH, EffectOnRoadLayoutEnum
         // *******************************************************************
         pp("roadLayoutUnchanged", TrafficDataTypes::RLU),
         pp("newRoadworksLayout", TrafficDataTypes::RNL),
         pp("contraflow", TrafficDataTypes::CTR),
         pp("narrowLanes", TrafficDataTypes::NLS),
         pp("temporaryTrafficLights", TrafficDataTypes::TLT),
         pp("obstacleSignalling", TrafficDataTypes::OSI),
         pp("lanesDeviated", TrafficDataTypes::lanesDeviated),
         pp("laneClosures", TrafficDataTypes::laneClosures),
         pp("carriagewayClosures", TrafficDataTypes::carriagewayClosures), 

         // *******************************************************************
         // EtoH, EnvironmentalObstructionTypeEnum
         // *******************************************************************
         pp("seriousFire", TrafficDataTypes::FIR), 
         pp("sewerOverflow", TrafficDataTypes::SEW), 
         pp("grassFire", TrafficDataTypes::FIG),
         pp("flooding", TrafficDataTypes::FLD), 
         pp("flashFloods", TrafficDataTypes::FLF), 
         pp("earthquakeDamage", TrafficDataTypes::EQD), 
         pp("avalanches", TrafficDataTypes::AVL),
         pp("stormDamage", TrafficDataTypes::STD), 
         pp("rockfalls", TrafficDataTypes::ROC), 
         pp("subsidence", TrafficDataTypes::SUB),
         pp("landslips", TrafficDataTypes::LSL), 
         pp("fallenTrees", TrafficDataTypes::FLT), 
         pp("mudSlide", TrafficDataTypes::MUD), 
         //pp("other", TrafficDataTypes::),    

         // *******************************************************************
         // EtoH, EquipmentDamageTypeEnum
         // *******************************************************************
         pp("burstWaterMain", TrafficDataTypes::BWM),
         pp("fallenPowerCables", TrafficDataTypes::FPC), 
         pp("sewerCollapse", TrafficDataTypes::SWC),
         pp("burstPipe", TrafficDataTypes::BRP), 
         pp("gasLeak", TrafficDataTypes::GAS), 
         //pp("roadInfrastructureDamage", TrafficDataTypes::), 
         //pp("other", TrafficDataTypes::),                   

         // *******************************************************************
         // ItoM, LaneUsageEnum
         // *******************************************************************
         pp("useBusLane", TrafficDataTypes::UBV),      
         pp("useHardShoulder", TrafficDataTypes::UHA), 
         pp("useHeavyVehicleLane", TrafficDataTypes::UHV), 
         //pp("keepToTheRight", TrafficDataTypes::),
         //pp("keepToTheLeft", TrafficDataTypes::), 
         //pp("heavyVehiclesUseRightLane", TrafficDataTypes::),
         //pp("useRightLane", TrafficDataTypes::), 
         //pp("heavyVehiclesUseLeftLane", TrafficDataTypes::),
         //pp("other", TrafficDataTypes::),            
         //pp("useThroughTrafficLanes", TrafficDataTypes::),
         //pp("useLocalTrafficLanes", TrafficDataTypes::),  
         //pp("useLeftHandParallelCarriageway", TrafficDataTypes::), 
         //pp("useRightHandParallelCarriageway", TrafficDataTypes::),
         //pp("useLeftLane", TrafficDataTypes::), 

         // *******************************************************************
         // NtoR, NetworkManagementTypeEnum
         // *******************************************************************
         pp("carPoolLaneInOperation", TrafficDataTypes::CAA), 
         pp("closedPermanentlyForTheWinter", TrafficDataTypes::CPW), 
         pp("contraflow", TrafficDataTypes::CTR),
         pp("convoyServiceDueToBadWeather", TrafficDataTypes::COW), 
         pp("narrowLanes", TrafficDataTypes::NLS), 
         pp("noThroughTraffic", TrafficDataTypes::RCT),
         pp("overnightClosures", TrafficDataTypes::CON),
         pp("restrictions", TrafficDataTypes::RET), 
         pp("roadClosed", TrafficDataTypes::RCD),  
         pp("singleAlternateLineTraffic", TrafficDataTypes::SAT), 
         pp("bridgeSwingInOperation", TrafficDataTypes::bridgeSwingInOperation),
         pp("intermittentClosures", TrafficDataTypes::intermittentClosures),
         pp("intermittentShortTermClosures",
               TrafficDataTypes::intermittentShortTermClosures), 
         pp("laneOrCarriagewayClosed",
               TrafficDataTypes::laneOrCarriagewayClosed),
         pp("noOvertaking", TrafficDataTypes::noOvertaking),
         pp("rushHourLaneInOperation",
               TrafficDataTypes::rushHourLaneInOperation),
         pp("tidalFlowLaneInOperation",
               TrafficDataTypes::tidalFlowLaneInOperation), 
         pp("trafficContolInOperation",
               TrafficDataTypes::trafficContolInOperation), 
         pp("trafficHeld", TrafficDataTypes::trafficHeld), 
         pp("useOfSpecifiedLaneAllowed",
               TrafficDataTypes::useOfSpecifiedLaneAllowed),
         pp("useSpecifiedLane", TrafficDataTypes::useSpecifiedLane), 
         pp("useUnderSpecifiedRestrictions",
               TrafficDataTypes::useUnderSpecifiedRestrictions), 
         //pp("other", TrafficDataTypes::OtherPhrase), 

         // *******************************************************************
         // NtoR, NonWeatherRelatedRoadConditionTypeEnum 
         // *******************************************************************
         pp("leavesOnRoad", TrafficDataTypes::RLS), 
         pp("looseChippings", TrafficDataTypes::LCI), 
         pp("looseSandOnRoad", TrafficDataTypes::LSR), 
         pp("mudOnRoad", TrafficDataTypes::RMD), 
         pp("oilOnRoad", TrafficDataTypes::FUE),
         pp("petrolOnRoadway", TrafficDataTypes::OIL),
         pp("roadSurfaceInPoorCondition", TrafficDataTypes::RPC), 
         //pp("other", TrafficDataTypes::OtherPhrase),  

         // *******************************************************************
         // NtoR, ObstructionTypeEnum
         // *******************************************************************
         pp("airCrash", TrafficDataTypes::AIC),
         pp("childrenOnRoadway", TrafficDataTypes::CHI), 
         pp("clearanceWork", TrafficDataTypes::CLW),
         pp("cyclistsOnRoadway", TrafficDataTypes::CYC), 
         pp("highSpeedChase", TrafficDataTypes::HSC), 
         pp("incident", TrafficDataTypes::INS), 
         pp("movingHazardsOnTheRoad", TrafficDataTypes::MHR),
         pp("objectOnTheRoad", TrafficDataTypes::OBR),  
         pp("objectsFallingFromMovingVehicle", TrafficDataTypes::OMV), 
         pp("obstructionOnTheRoad", TrafficDataTypes::OHX), 
         pp("peopleOnRoadway", TrafficDataTypes::PEO),   
         pp("railCrash", TrafficDataTypes::RAC), 
         pp("recklessDriver", TrafficDataTypes::RDW), 
         pp("rescueAndRecoveryWork", TrafficDataTypes::REW), 
         pp("shedLoad", TrafficDataTypes::SHL),      
         pp("spillageOccurringFromMovingVehicle", TrafficDataTypes::SMV),
         pp("spillageOnTheRoad", TrafficDataTypes::SPL), 
         //pp("craneOperating", TrafficDataTypes::),     
         //pp("damagedCrashBarrier", TrafficDataTypes::),
         //pp("fallingIce", TrafficDataTypes::),         
         //pp("fallingLightIceOrSnow", TrafficDataTypes::),
         //pp("houseFire", TrafficDataTypes::),
         //pp("industrialAccident", TrafficDataTypes::), 
         //pp("severeFrostDamagedRoadway", TrafficDataTypes::), 
         //pp("snowAndIceDebris", TrafficDataTypes::),   
         //pp("unprotectedAccidentArea", TrafficDataTypes::), 
         //pp("other", TrafficDataTypes::OtherPhrase), 

         // *******************************************************************
         // NtoR, PoorEnvironmentTypeEnum
         // *******************************************************************
         pp("blizzard", TrafficDataTypes::BLI), 
         pp("blowingDust", TrafficDataTypes::BLO),  
         pp("blowingSnow", TrafficDataTypes::BLS),
         pp("crosswinds", TrafficDataTypes::WIC), 
         pp("damagingHail", TrafficDataTypes::HAD), 
         pp("denseFog", TrafficDataTypes::FOD), 
         pp("extremeCold", TrafficDataTypes::TXC), 
         pp("extremeHeat", TrafficDataTypes::TEX), 
         pp("fog", TrafficDataTypes::FOG),   
         pp("freezingFog", TrafficDataTypes::FOF),   
         pp("frost", TrafficDataTypes::FRO),
         pp("gales", TrafficDataTypes::GAL), 
         pp("gustyWinds", TrafficDataTypes::WIG), 
         pp("hail", TrafficDataTypes::HAI), 
         pp("heavyFrost", TrafficDataTypes::FRH), 
         pp("heavyRain", TrafficDataTypes::RAH), 
         pp("heavySnowfall", TrafficDataTypes::SFH), 
         pp("hurricaneForceWinds", TrafficDataTypes::HUR),
         pp("lowSunGlare", TrafficDataTypes::LSG), 
         pp("patchyFog", TrafficDataTypes::FOP), 
         pp("precipitationInTheArea", TrafficDataTypes::PRA), 
         pp("rain", TrafficDataTypes::RAI),     
         pp("sandStorms", TrafficDataTypes::SAN), 
         pp("severeExhaustPollution", TrafficDataTypes::EXS),
         pp("severeSmog", TrafficDataTypes::SSM),  
         pp("showers", TrafficDataTypes::SWS),    
         pp("sleet", TrafficDataTypes::SLT),     
         pp("smogAlert", TrafficDataTypes::SMG),
         pp("smokeHazard", TrafficDataTypes::SMO), 
         pp("snowfall", TrafficDataTypes::SFL),   
         pp("sprayHazard", TrafficDataTypes::SPY), 
         pp("stormForceWinds", TrafficDataTypes::STM), 
         pp("strongWinds", TrafficDataTypes::WIS),    
         pp("swarmsOfInsects", TrafficDataTypes::SWA), 
         pp("temperatureFalling", TrafficDataTypes::TFA),
         pp("thunderstorms", TrafficDataTypes::THU),
         pp("tornadoes", TrafficDataTypes::TOR),    
         pp("visibilityReduced", TrafficDataTypes::VIR),  
         pp("whiteOut", TrafficDataTypes::WHI),          
         pp("winterStorm", TrafficDataTypes::WST),     
         //pp("abnormalTemperature", TrafficDataTypes::), 
         //pp("badWeather", TrafficDataTypes::CBW),      
         //pp("eclipse", TrafficDataTypes::),           
         //pp("moderateFog", TrafficDataTypes::),     
         //pp("ozonePollution", TrafficDataTypes::), 
         //pp("rainChangingToSnow", TrafficDataTypes::),  
         //pp("snowChangingToRain", TrafficDataTypes::), 
         //pp("strongGustsOfWind", TrafficDataTypes::),  
         //pp("veryStrongGustsOfWind", TrafficDataTypes::), 

         // *******************************************************************
         // NtoR, PoorRoadInfrastructureEnum
         // *******************************************************************
         pp("temporaryTrafficLightsNotWorking", TrafficDataTypes::TNW), 
         pp("levelCrossingFailure", TrafficDataTypes::LCF),
         pp("variableMessageSignsWorkingIncorrectly", TrafficDataTypes::VWI), 
         pp("tunnelVentilationNotWorking", TrafficDataTypes::TVX), 
         pp("emergencyTelephoneNumberNotWorking", TrafficDataTypes::NOO), 
         pp("rampControlSignalsWorkingIncorrectly", TrafficDataTypes::RCI), 
         pp("rampControlSignalsNotWorking", TrafficDataTypes::RNW), 
         pp("laneControlSignsNotWorking", TrafficDataTypes::SNW), 
         pp("trafficLightsNotWorking", TrafficDataTypes::TLO), 
         pp("variableMessageSignsNotWorking", TrafficDataTypes::VNW),
         pp("temporaryTrafficLightsWorkingIncorrectly", TrafficDataTypes::TWI), 
         pp("laneControlSignsWorkingIncorrectly", TrafficDataTypes::SWI), 
         pp("trafficSignalControlComputerNotWorking", TrafficDataTypes::TCC),
         pp("trafficLightsWorkingIncorrectly", TrafficDataTypes::TLI), 
         pp("trafficSignalTimingsChanged", TrafficDataTypes::TSC), 
         //pp("trafficSignalsNotWorkingProperly", TrafficDataTypes::), 
         //pp("automaticTollSystemNotWorkingPayManually", TrafficDataTypes::), 
         //pp("automaticPaymentLaneNotWorking", TrafficDataTypes::), 
         //pp("opticFiberDamage", TrafficDataTypes::), 
         //pp("safetyBarrierDamage", TrafficDataTypes::), 
         //pp("bridgeViaductDamage", TrafficDataTypes::), 
         //pp("tunnelLightsNotWorking", TrafficDataTypes::), 
         //pp("emergencyTelephonesNotWorking", TrafficDataTypes::), 
         //pp("levelCrossingNotWorkingNormally", TrafficDataTypes::), 
         //pp("powerFailure", TrafficDataTypes::), 
         //pp("galleryLightsNotWorking", TrafficDataTypes::), 
         //pp("damageToRoadInfrastructure", TrafficDataTypes::), 

         // *******************************************************************
         // NtoR, PublicEventTypeEnum
         // *******************************************************************
         pp("athleticsMeeting", TrafficDataTypes::EAM),
         pp("ballGame", TrafficDataTypes::EBG),  
         pp("bicycleRace", TrafficDataTypes::ECY), 
         pp("boxingTournament", TrafficDataTypes::EBT),   
         pp("bullFight", TrafficDataTypes::EBF),     
         pp("ceremonialEvent", TrafficDataTypes::EVC),
         pp("cricketMatch", TrafficDataTypes::ECM), 
         pp("exhibition", TrafficDataTypes::EVX), 
         pp("fair", TrafficDataTypes::EFA),
         pp("festival", TrafficDataTypes::EVF), 
         pp("footballMatch", TrafficDataTypes::EFM), 
         pp("funfair", TrafficDataTypes::FUN), 
         pp("golfTournament", TrafficDataTypes::EGT), 
         pp("internationalSportsMeeting", TrafficDataTypes::EIS),
         pp("majorEvent", TrafficDataTypes::EVM),  
         pp("marathon", TrafficDataTypes::MAR),   
         pp("market", TrafficDataTypes::MKT),    
         pp("match", TrafficDataTypes::EMT), 
         pp("parade", TrafficDataTypes::EVP), 
         pp("raceMeeting", TrafficDataTypes::ERA), 
         pp("rugbyMatch", TrafficDataTypes::RRU), 
         pp("severalMajorEvents", TrafficDataTypes::ESM), 
         pp("show", TrafficDataTypes::ESH), 
         pp("showJumping", TrafficDataTypes::ESJ), 
         pp("sportsMeeting", TrafficDataTypes::ESP),  
         pp("stateOccasion", TrafficDataTypes::ESO), 
         pp("tennisTournament", TrafficDataTypes::ETT),  
         pp("tournament", TrafficDataTypes::ETO),  
         pp("tradeFair", TrafficDataTypes::TRA),  
         pp("waterSportsMeeting", TrafficDataTypes::EWS), 
         pp("winterSportsMeeting", TrafficDataTypes::EWT), 
         //pp("baseballGame", TrafficDataTypes::),
         //pp("basketballGame", TrafficDataTypes::), 
         //pp("boatRace", TrafficDataTypes::), 
         //pp("concert", TrafficDataTypes::),  
         //pp("filmTVMaking", TrafficDataTypes::), 
         //pp("hockeyGame", TrafficDataTypes::), 
         //pp("horseRaceMeeting", TrafficDataTypes::), 
         //pp("motorSportRaceMeeting", TrafficDataTypes::),
         //pp("other", TrafficDataTypes::OtherPhrase), 

         // *******************************************************************
         // NtoR, RoadMaintenanceTypeEnum
         // *******************************************************************
         pp("clearanceWorkOnCarriageway", TrafficDataTypes::CLW), 
         pp("snowploughsInUse", TrafficDataTypes::SN), 
         pp("saltingInProgress", TrafficDataTypes::SAL), 
         pp("clearingAction", TrafficDataTypes::CLE), 
         pp("maintenanceWork", TrafficDataTypes::RMK),
         pp("roadMarkingWork", TrafficDataTypes::RMW), 
         pp("roadworks", TrafficDataTypes::RWK),      
         pp("resurfacingWork", TrafficDataTypes::RWR), 
         pp("roadworkClearanceInProgress", TrafficDataTypes::RWX),
         pp("treeAndVegetationCuttingWork",
               TrafficDataTypes::treeAndVegetationCuttingWork),
         pp("grassCuttingWork", TrafficDataTypes::grassCuttingWork),
         pp("overrunningRoadworks", TrafficDataTypes::overrunningRoadworks), 
         pp("repairWork", TrafficDataTypes::repairWork),
         pp("roadsideWork", TrafficDataTypes::roadsideWork),
         pp("rockFallPreventativeMaintenance",
               TrafficDataTypes::rockFallPreventativeMaintenance), 
         pp("installationWork", TrafficDataTypes::installationWork), 

         // *******************************************************************
         // NtoR, RoadworksDurationEnum
         // *******************************************************************
         pp("longTerm", TrafficDataTypes::RWL), 
         //pp("mediumTerm", TrafficDataTypes::), 
         //pp("shortTerm", TrafficDataTypes::), 

         // *******************************************************************
         // NtoR, RoadworksScaleEnum
         // *******************************************************************
         pp("major", TrafficDataTypes::RWM), 
         //pp("medium", TrafficDataTypes::),
         //pp("minor", TrafficDataTypes::), 

         // *******************************************************************
         // StoZ, TransitServiceInformationEnum
         // *******************************************************************
         pp("delayedUntilFurtherNotice", TrafficDataTypes::DEU), 
         pp("serviceDelaysOfUncertainDuration", TrafficDataTypes::DUD),
         pp("railServiceNotOperating", TrafficDataTypes::RSN), 
         pp("freeShuttleServiceOperating", TrafficDataTypes::SSF),
         pp("cancellations", TrafficDataTypes::CAC), 
         pp("serviceSuspended", TrafficDataTypes::DPN),  
         pp("ferryServiceNotOperating", TrafficDataTypes::FSN), 
         pp("serviceWithdrawn", TrafficDataTypes::SCN),  
         pp("serviceNotOperatingSubstituteServiceAvailable", 
               TrafficDataTypes::SNS),
         pp("shuttleServiceOperating", TrafficDataTypes::SSO), 
         pp("railServicesIrregularDelays", TrafficDataTypes::RSI),
         //pp("delayDueToRepairs", TrafficDataTypes::), 
         //pp("delayDueToBadWeather", TrafficDataTypes::), 
         //pp("ferryDelayed", TrafficDataTypes::),
         //pp("serviceDelays", TrafficDataTypes::),  
         //pp("other", TrafficDataTypes::),         
         //pp("departureOnSchedule", TrafficDataTypes::), 
         //pp("rapidTransitInformationServiceNotAvailable", TrafficDataTypes::), 
         //pp("railInformationServiceNotAvailable", TrafficDataTypes::), 
         //pp("ferryLoadCapacityChanged", TrafficDataTypes::), 
         //pp("temporaryChangesToTimetables", TrafficDataTypes::),
         //pp("ferryReplacedByIceRoad", TrafficDataTypes::), 
         //pp("ferryServicesIrregularDelays", TrafficDataTypes::), 
         //pp("delaysDueToFlotsum", TrafficDataTypes::),    
         //pp("restrictionsForLongerVehicles", TrafficDataTypes::), 

         // *******************************************************************
         // StoZ, TrafficTrendTypeEnum
         // *******************************************************************
         pp( "trafficEasing", TrafficDataTypes::TEA ), 
         pp( "trafficBuildingUp", TrafficDataTypes::TBU ), 
         pp( "trafficStable", TrafficDataTypes::trafficStable ), 

         // *******************************************************************
         // StoZ, VehicleObstructionTypeEnum
         // *******************************************************************
         pp("abnormalLoad", TrafficDataTypes::ABL),
         pp("brokenDownBus", TrafficDataTypes::BDB), 
         pp("brokenDownVehicle", TrafficDataTypes::BKD), 
         pp("convoy", TrafficDataTypes::CVY), 
         pp("dangerousSlowMovingVehicle", TrafficDataTypes::VSM), 
         pp("emergencyVehicle", TrafficDataTypes::EMV), 
         pp("highSpeedEmergencyVehicle", TrafficDataTypes::EMH),
         pp("longLoad", TrafficDataTypes::LLD),          
         pp("militaryConvoy", TrafficDataTypes::MIL),  
         pp("overheightVehicle", TrafficDataTypes::OHV), 
         pp("prohibitedVehicleOnTheRoadway", TrafficDataTypes::PRV), 
         pp("slowMovingMaintenanceVehicle", TrafficDataTypes::RMV),  
         pp("slowVehicle", TrafficDataTypes::SVH), 
         pp("trackLayingVehicle", TrafficDataTypes::TLV),
         pp("vehicleOnFire", TrafficDataTypes::VFR),    
         pp("vehicleCarryingHazardousMaterials", TrafficDataTypes::HAZ),
         pp("vehicleOnWrongCarriageway", TrafficDataTypes::VWC),  
         pp("vehiclesSlowingToLookAtAccidents", TrafficDataTypes::VSA), 
         pp("vehicleWithOverheightLoad", TrafficDataTypes::VOD),
         //pp("abandonedVehicle", TrafficDataTypes::),  
         //pp("brokenDownHeavyLorry", TrafficDataTypes::HDB) 
         //pp("damagedVehicle", TrafficDataTypes::),  
         //pp("saltingOrGrittingVehicleInUse", TrafficDataTypes::), 
         //pp("snowplough", TrafficDataTypes::),     
         //pp("unlitVehicleOnTheRoad", TrafficDataTypes::),
         //pp("vehicleStuck", TrafficDataTypes::),       
         //pp("vehicleStuckUnderBridge", TrafficDataTypes::), 
         //pp("vehicleWithOverwideLoad", TrafficDataTypes::), 
         //pp("other", TrafficDataTypes::OtherPhrase),    

         // *******************************************************************
         // StoZ, WeatherRelatedRoadConditionTypeEnum
         // *******************************************************************
         pp("freezingRain", TrafficDataTypes::RAF),
         pp("slushStrings", TrafficDataTypes::SLU), 
         pp("blackIce", TrafficDataTypes::RBI),    
         pp("ice",TrafficDataTypes::RIC),         
         pp("icyPatches", TrafficDataTypes::ICP),
         pp("iceBuildUp", TrafficDataTypes::IBU), 
         pp("snowOnTheRoad", TrafficDataTypes::SRO), 
         pp("snowDrifts", TrafficDataTypes::SNR),   
         pp("packedSnow", TrafficDataTypes::SNP),  
         pp("wetAndIcyRoad", TrafficDataTypes::RWI),  
         pp("freshSnow", TrafficDataTypes::SNF),     
         pp("deepSnow", TrafficDataTypes::SND), 
         pp("slushOnRoad", TrafficDataTypes::SLU), 
         //pp("damp", TrafficDataTypes::),            
         //pp("freezingOfWetRoads", TrafficDataTypes::), 
         //pp("wetOrDamp", TrafficDataTypes::),  
         //pp("looseSnow", TrafficDataTypes::), 
         //pp("iceWithWheelBarTracks", TrafficDataTypes::),
         //pp("wetIcyPavement", TrafficDataTypes::), 
         //pp("roadSurfaceMelting", TrafficDataTypes::),
         //pp("freezingPavements", TrafficDataTypes::),
         //pp("dry", TrafficDataTypes::), 
         //pp("snowOnPavement", TrafficDataTypes::),  
         //pp("normalWinterConditionsForPedestrians", TrafficDataTypes::), 
         //pp("other", TrafficDataTypes::),          
         //pp("wet", TrafficDataTypes::),          

         // *******************************************************************
         // StoZ, WinterEquipmentAdviceEnum
         // *******************************************************************
         pp("snowChainsRecommended", TrafficDataTypes::SR), 
         pp("snowTyresRecommended", TrafficDataTypes::TR), 
         pp("studTyresMayBeUsed", TrafficDataTypes::STU), 
         pp("winterEquipmentRecommended", TrafficDataTypes::WR), 
         //pp("snowChainsOrTyresRecommended", TrafficDataTypes::),
         //pp("other", TrafficDataTypes::OtherPhrase),     

         // *******************************************************************
         // StoZ, WinterEquipmentRequirementEnum
         // *******************************************************************
         pp("snowChainsMandatory", TrafficDataTypes::SM), 
         pp("snowTyresMandatory", TrafficDataTypes::TM),  
         pp("studTyresAreNotAllowed", TrafficDataTypes::STF), 
         pp("winterEquipmentMandatory", TrafficDataTypes::WRM),
         pp("other", TrafficDataTypes::OtherPhrase)        
         //pp("snowChainsOrTyresMandatory", TrafficDataTypes::),

      };
      c_phraseMap.insert(phrase_map, 
                         phrase_map + sizeof(phrase_map)/sizeof(*phrase_map));
   }

   // Translates from phrase to severity
   {
      typedef map<TrafficDataTypes::phrase, 
         TrafficDataTypes::severity>::value_type ps;
      const ps phrase_severity_map[] = {
#if 1
         // Accidents types, set to Severe
         ps( TrafficDataTypes::AJA, TrafficDataTypes::QueuingTraffic ),
         ps( TrafficDataTypes::AJC, TrafficDataTypes::QueuingTraffic ),
         ps( TrafficDataTypes::AJT, TrafficDataTypes::QueuingTraffic ),
         ps( TrafficDataTypes::ACM, TrafficDataTypes::QueuingTraffic ),
         ps( TrafficDataTypes::ACF, TrafficDataTypes::TrafficCongestion ),
         ps( TrafficDataTypes::AOL, TrafficDataTypes::QueuingTraffic ),
         ps( TrafficDataTypes::AOV, TrafficDataTypes::QueuingTraffic ),
         ps( TrafficDataTypes::ACD, TrafficDataTypes::StationaryTraffic ),
         ps( TrafficDataTypes::ACS, TrafficDataTypes::StationaryTraffic ),
         ps( TrafficDataTypes::AOI, TrafficDataTypes::QueuingTraffic ),
         ps( TrafficDataTypes::ASP, TrafficDataTypes::QueuingTraffic ),
         ps( TrafficDataTypes::ACI, TrafficDataTypes::TrafficCongestion ),
         ps( TrafficDataTypes::ACW, TrafficDataTypes::TrafficCongestion ),
         ps( TrafficDataTypes::ACB, TrafficDataTypes::TrafficCongestion ),
         ps( TrafficDataTypes::ACA, TrafficDataTypes::QueuingTraffic ),
         ps( TrafficDataTypes::ACH, TrafficDataTypes::TrafficCongestion ),
         ps( TrafficDataTypes::ACE, TrafficDataTypes::QueuingTraffic ),
         ps( TrafficDataTypes::ACZ, TrafficDataTypes::Closed ),
         ps( TrafficDataTypes::overturnedTrailer,
               TrafficDataTypes::StationaryTraffic ),
         ps( TrafficDataTypes::headOnCollision, 
               TrafficDataTypes::QueuingTraffic ),
         ps( TrafficDataTypes::headOnOrSideCollision,
               TrafficDataTypes::QueuingTraffic ),
         ps( TrafficDataTypes::multipleVehicleCollision,
               TrafficDataTypes::StationaryTraffic ), 
         ps( TrafficDataTypes::sideCollision,
               TrafficDataTypes::TrafficCongestion ), 
         ps( TrafficDataTypes::rearCollision,
               TrafficDataTypes::TrafficCongestion ), 
         ps( TrafficDataTypes::vehicleOffRoad,
               TrafficDataTypes::TrafficCongestion ), 
         ps( TrafficDataTypes::accidentInvolvingMopeds,
               TrafficDataTypes::TrafficCongestion ), 
         ps( TrafficDataTypes::accidentInvolvingBicycles,
               TrafficDataTypes::TrafficCongestion ),
         ps( TrafficDataTypes::accidentInvolvingMotorcycles,
               TrafficDataTypes::QueuingTraffic ),
         ps( TrafficDataTypes::accidentInvolvingTrain,
               TrafficDataTypes::StationaryTraffic ), 
         ps( TrafficDataTypes::collision,
               TrafficDataTypes::QueuingTraffic ), 
         ps( TrafficDataTypes::collisionWithAnimal,
               TrafficDataTypes::TrafficCongestion ), 
         ps( TrafficDataTypes::collisionWithObstruction,
               TrafficDataTypes::TrafficCongestion ),
         ps( TrafficDataTypes::collisionWithPerson,
               TrafficDataTypes::SlowTraffic ),     

         // Abnormal and traffic trends types
         ps( TrafficDataTypes::LS1, TrafficDataTypes::StationaryTraffic ), 
         ps( TrafficDataTypes::LS2, TrafficDataTypes::QueuingTraffic ),
         ps( TrafficDataTypes::LS3, TrafficDataTypes::SlowTraffic ),
         ps( TrafficDataTypes::TBU, TrafficDataTypes::TrafficBuildingUp ), 
         ps( TrafficDataTypes::TEA, TrafficDataTypes::TrafficEasing ), 
         ps( TrafficDataTypes::stopAndGo,
               TrafficDataTypes::AlternatingContraflow ),
         ps( TrafficDataTypes::trafficStable, TrafficDataTypes::NoSeverity ), 

         // Network managment types
         ps( TrafficDataTypes::CAA, TrafficDataTypes::NoSeverity ),
         ps( TrafficDataTypes::CPW, TrafficDataTypes::Closed ), 
         ps( TrafficDataTypes::CTR, TrafficDataTypes::AlternatingContraflow ),
         ps( TrafficDataTypes::COW, TrafficDataTypes::TrafficBuildingUp ),
         ps( TrafficDataTypes::NLS, TrafficDataTypes::NoSeverity ), 
         ps( TrafficDataTypes::RCT, TrafficDataTypes::Closed ), 
         ps( TrafficDataTypes::CON, TrafficDataTypes::NoSeverity ), 
         ps( TrafficDataTypes::RET, TrafficDataTypes::NoSeverity), 
         ps( TrafficDataTypes::RCD, TrafficDataTypes::Closed ), 
         ps( TrafficDataTypes::SAT, TrafficDataTypes::AlternatingContraflow ), 
         ps( TrafficDataTypes::bridgeSwingInOperation,
               TrafficDataTypes::NoSeverity ),
         ps( TrafficDataTypes::intermittentClosures,
               TrafficDataTypes::AlternatingContraflow ),
         ps( TrafficDataTypes::intermittentShortTermClosures,
               TrafficDataTypes::AlternatingContraflow ), 
         ps( TrafficDataTypes::laneOrCarriagewayClosed,
               TrafficDataTypes::TrafficBuildingUp ),  
         ps( TrafficDataTypes::noOvertaking, TrafficDataTypes::NoSeverity ),
         ps( TrafficDataTypes::rushHourLaneInOperation,
               TrafficDataTypes::NoSeverity ),  
         ps( TrafficDataTypes::tidalFlowLaneInOperation,
               TrafficDataTypes::NoSeverity ),  
         ps( TrafficDataTypes::trafficContolInOperation,
               TrafficDataTypes::NoSeverity ), 
         ps( TrafficDataTypes::trafficHeld, TrafficDataTypes::HeavyTraffic ), 
         ps( TrafficDataTypes::useOfSpecifiedLaneAllowed,
               TrafficDataTypes::NoSeverity ),
         ps( TrafficDataTypes::useSpecifiedLane, TrafficDataTypes::NoSeverity ), 
         ps( TrafficDataTypes::useUnderSpecifiedRestrictions,
               TrafficDataTypes::NoSeverity ), 

         // Construction work types
         ps( TrafficDataTypes::RCW, TrafficDataTypes::MiscRoadworks ), 
         ps( TrafficDataTypes::RBL, TrafficDataTypes::MiscRoadworks ), 
         ps( TrafficDataTypes::demolitionWork, 
               TrafficDataTypes::MiscRoadworks ), 

         // Road maintenance types
         ps( TrafficDataTypes::CLW, TrafficDataTypes::MiscRoadworks ), 
         ps( TrafficDataTypes::SN, TrafficDataTypes::MiscRoadworks ), 
         ps( TrafficDataTypes::SAL, TrafficDataTypes::MiscRoadworks ),
         ps( TrafficDataTypes::CLE, TrafficDataTypes::MiscRoadworks ), 
         ps( TrafficDataTypes::RMK, TrafficDataTypes::MiscRoadworks ),
         ps( TrafficDataTypes::RMW, TrafficDataTypes::MiscRoadworks ), 
         ps( TrafficDataTypes::RWK, TrafficDataTypes::MiscRoadworks ),
         ps( TrafficDataTypes::RWR, TrafficDataTypes::MiscRoadworks ), 
         ps( TrafficDataTypes::RWX, TrafficDataTypes::MiscRoadworks ),
         ps( TrafficDataTypes::treeAndVegetationCuttingWork,
               TrafficDataTypes::MiscRoadworks ),
         ps( TrafficDataTypes::grassCuttingWork, 
               TrafficDataTypes::MiscRoadworks ), 
         ps( TrafficDataTypes::overrunningRoadworks, 
               TrafficDataTypes::MiscRoadworks ), 
         ps( TrafficDataTypes::repairWork, TrafficDataTypes::MiscRoadworks ),
         ps( TrafficDataTypes::roadsideWork, TrafficDataTypes::MiscRoadworks ),
         ps( TrafficDataTypes::rockFallPreventativeMaintenance, 
               TrafficDataTypes::MiscRoadworks ), 
         ps( TrafficDataTypes::installationWork, 
               TrafficDataTypes::MiscRoadworks ),
#endif 
      };
      const size_t ps_size = 
         sizeof(phrase_severity_map)/sizeof(*phrase_severity_map);
      c_phraseToSeverity.insert( phrase_severity_map, 
                                 phrase_severity_map + ps_size );
   }

   {
      typedef std::pair<const char*, TrafficDataTypes::disturbanceType> dm;
      const dm type_map[] = { 
#if 1
         dm( "AnimalPresenceObstruction", 
               TrafficDataTypes::AnimalPresenceObstruction ),
         dm( "EnvironmentalObstruction", 
               TrafficDataTypes::EnvironmentalObstruction ),
         dm( "EquipmentDamageObstruction", 
               TrafficDataTypes::EquipmentDamageObstruction ),
         dm( "GeneralObstruction", TrafficDataTypes::GeneralObstruction ),
         dm( "VehicleObstruction", TrafficDataTypes::VehicleObstruction ),
         dm( "AbnormalTraffic", TrafficDataTypes::AbnormalTraffic ),
         dm( "Accident", TrafficDataTypes::Accident ),
         dm( "PoorRoadInfrastructure", 
               TrafficDataTypes::PoorRoadInfrastructure ),
         dm( "Activities", TrafficDataTypes::Activities ),
         dm( "WeatherRelatedRoadConditions", 
               TrafficDataTypes::WeatherRelatedRoadConditions ),
         dm( "NonWeatherRelatedRoadConditions",
               TrafficDataTypes::NonWeatherRelatedRoadConditions ),
         dm( "PoorEnvironmentConditions", 
               TrafficDataTypes::PoorEnvironmentConditions ),
         dm( "ConstructionWorks", TrafficDataTypes::ConstructionWorks ),
         dm( "MaintenanceWorks", TrafficDataTypes::MaintenanceWorks ),
         dm( "VariableMessageSignSetting",
               TrafficDataTypes::VariableMessageSignSetting ),
         dm( "MatrixSignSetting", TrafficDataTypes::MatrixSignSetting ),
         dm( "NetworkManagement", TrafficDataTypes::NetworkManagement ),
         dm( "RoadsideAssistance", TrafficDataTypes::RoadsideAssistance ),
         dm( "TransitInformation", TrafficDataTypes::TransitInformation ),
         dm( "ServiceDisruption", TrafficDataTypes::ServiceDisruption ),
         dm( "Carparks", TrafficDataTypes::Carparks ),
#endif
      };
      c_typeMap.insert( type_map, 
            type_map + sizeof(type_map)/sizeof(*type_map) );
   }

   {
      typedef std::pair< TrafficDataTypes::disturbanceType, uint32 > tm;
      const uint32 DAY = 60 * 60 * 24;
      const uint32 WEEK = DAY * 7;
      const tm estTime_map[] = { 
#if 1
         tm( TrafficDataTypes::AnimalPresenceObstruction, DAY ),
         tm( TrafficDataTypes::EnvironmentalObstruction, DAY ),
         tm( TrafficDataTypes::EquipmentDamageObstruction, DAY ),
         tm( TrafficDataTypes::GeneralObstruction, DAY ),
         tm( TrafficDataTypes::VehicleObstruction, DAY ),
         tm( TrafficDataTypes::AbnormalTraffic, DAY ),
         tm( TrafficDataTypes::Accident, DAY ),
         tm( TrafficDataTypes::PoorRoadInfrastructure, DAY ),
         tm( TrafficDataTypes::Activities, DAY ),
         tm( TrafficDataTypes::WeatherRelatedRoadConditions, DAY ),
         tm( TrafficDataTypes::NonWeatherRelatedRoadConditions, DAY ),
         tm( TrafficDataTypes::PoorEnvironmentConditions, DAY ),
         tm( TrafficDataTypes::ConstructionWorks, WEEK ),
         tm( TrafficDataTypes::MaintenanceWorks, WEEK ),
         tm( TrafficDataTypes::VariableMessageSignSetting, DAY ),
         tm( TrafficDataTypes::MatrixSignSetting, DAY ),
         tm( TrafficDataTypes::NetworkManagement, WEEK ),
         tm( TrafficDataTypes::RoadsideAssistance, DAY ),
         tm( TrafficDataTypes::TransitInformation, DAY ),
         tm( TrafficDataTypes::ServiceDisruption, DAY ),
         tm( TrafficDataTypes::Carparks, DAY ),
         tm( TrafficDataTypes::NoType, DAY ),
#endif
      };
      c_estEndTimeMap.insert( estTime_map, 
            estTime_map + sizeof(estTime_map)/sizeof(*estTime_map) );
   }

   return true;
}
bool
DatexUtilsII::c_initialized = initializeTables();

/*
"AccidentCauseEnum","poorMergeEntryOrExitJudgement",
"AccidentCauseEnum","driverDistraction",
"AccidentCauseEnum","driverDrugAbuse",
"AccidentCauseEnum","driverIllness",
"AccidentCauseEnum","exceedingSpeedsLimits",
"AccidentCauseEnum","excessAlcohol",
"AccidentCauseEnum","excessiveDriverTiredness",
"AccidentCauseEnum","limitedVisibility",
"AccidentCauseEnum","onTheWrongSideOfTheRoad",
"AccidentCauseEnum","other",
"AccidentCauseEnum","avoidanceOfObstacles",
"AccidentCauseEnum","poorLaneAdherence",
"AccidentCauseEnum","impermissibleManoeuvre",
"AccidentCauseEnum","poorRoadSurfaceCondition",
"AccidentCauseEnum","poorSurfaceAdherence",
"AccidentCauseEnum","undisclosed",
"AccidentCauseEnum","unknown",
"AccidentCauseEnum","vehicleFailure",
"AccidentCauseEnum","pedestrianInRoad",
"AccidentCauseEnum","notKeepingASafeDistance",

"AlertCDirectionEnum","unknown","okänd"
"AlertCDirectionEnum","negative","negativ"
"AlertCDirectionEnum","both","båda"
"AlertCDirectionEnum","positive","positive"

"AreaOfInterestEnum","national","nationell"
"AreaOfInterestEnum","neighbouringCountries","angränsande länder"
"AreaOfInterestEnum","notSpecified","ej angett"
"AreaOfInterestEnum","regional","regionalt"
"AreaOfInterestEnum","continentWide","kontinentalt"

"AuthorityOperationTypeEnum","other",
"AuthorityOperationTypeEnum","survey","undersökning"
"AuthorityOperationTypeEnum","policeInvestigation","polisutredning"
"AuthorityOperationTypeEnum","policeCheckPoint","poliskontroll"
"AuthorityOperationTypeEnum","vehicleInspectionCheckPoint","fordonskontroll"

"CarParkConfigurationEnum","underground","under jord"
"CarParkConfigurationEnum","multiStorey","flervånings"
"CarParkConfigurationEnum","singleLevel","envånings"
"CarParkConfigurationEnum","parkAndRide","infartsparkering"

"CarriagewayEnum","oppositeCarriageway","mötande körbana"
"CarriagewayEnum","slipRoads","påfarter"
"CarriagewayEnum","flyover",
"CarriagewayEnum","underpass",
"CarriagewayEnum","serviceRoad","serviceväg"
"CarriagewayEnum","entrySlipRoad","påfart"
"CarriagewayEnum","parallelCarriageway","parallell körbana"
"CarriagewayEnum","leftHandFeederRoad",
"CarriagewayEnum","rightHandFeederRoad",
"CarriagewayEnum","connectingCarriageway","anslutande körbana"
"CarriagewayEnum","mainCarriageway","huvudkörbana"
"CarriagewayEnum","leftHandParallelCarriageway",
"CarriagewayEnum","exitSlipRoad","avfart"
"CarriagewayEnum","rightHandParallelCarriageway",

"CauseTypeEnum","vandalism","vandaliserat"
"CauseTypeEnum","problemsAtCustomPost",
"CauseTypeEnum","problemsAtBorderPost","problem vid gränsstation"
"CauseTypeEnum","problemsOnLocalRoads","problem på lokalavägar"
"CauseTypeEnum","roadsideEvent","händelse vid sidan av vägen"
"CauseTypeEnum","terrorism","terrorism"
"CauseTypeEnum","poorWeather","dåligt väder"
"CauseTypeEnum","securityIncident","säkerhets incident"
"CauseTypeEnum","infrastructureFailure","fel på infrastruktur"
"CauseTypeEnum","equipmentFailure","fel på utrustning"
"CauseTypeEnum","congestion","trafikstockning"
"CauseTypeEnum","accident","olycka"
"CauseTypeEnum","obstruction","hinder"

"ChangedFlagEnum","catalogue",
"ChangedFlagEnum","filter",

"ComparisonOperatorEnum","greaterThan",
"ComparisonOperatorEnum","equalTo",
"ComparisonOperatorEnum","lessThan",

"CompressionMethodEnum","gzip",
"CompressionMethodEnum","deflate",

"ComputationMethodEnum","medianOfSamplesInATimePeriod",
"ComputationMethodEnum","movingAverageOfSamples",
"ComputationMethodEnum","harmonicAverageOfSamplesInATimePeriod",
"ComputationMethodEnum","arithmeticAverageOfSamplesInATimePeriod",
"ComputationMethodEnum","arithmeticAverageOfSamplesBasedOnAFixedNumberOfSam",

"ConfidentialityValueEnum","restrictedToAuthoritiesAndTrafficOperators",
"ConfidentialityValueEnum","noRestriction",
"ConfidentialityValueEnum","internalUse",
"ConfidentialityValueEnum","restrictedToAuthoritiesTrafficOperatorsAndPublishe",
"ConfidentialityValueEnum","restrictedToAuthorities",

"DangerousGoodRegulationsEnum","imoImdg",
"DangerousGoodRegulationsEnum","railroadDangerousGoodsBook",
"DangerousGoodRegulationsEnum","iataIcao",
"DangerousGoodRegulationsEnum","ADR",

"DataClassEnum","poorDrivingConditions",
"DataClassEnum","poorRoadInfrastructure",
"DataClassEnum","trafficMeasurement",
"DataClassEnum","trafficManagement",
"DataClassEnum","trafficElement",
"DataClassEnum","situation",
"DataClassEnum","roadsideAssistance",
"DataClassEnum","abnormalTraffic",
"DataClassEnum","weatherMeasurement",
"DataClassEnum","elaboratedData",
"DataClassEnum","roadMaintenance",
"DataClassEnum","planActivation",
"DataClassEnum","activity",
"DataClassEnum","elaboratedTravelTime",
"DataClassEnum","information",
"DataClassEnum","massData",
"DataClassEnum","obstruction",
"DataClassEnum","operatorAction",
"DataClassEnum","accident",

"DatexPictogramEnum","crossWind",
"DatexPictogramEnum","trafficCongestion",
"DatexPictogramEnum","noEntryForGoodsVehicles",
"DatexPictogramEnum","noEntry",
"DatexPictogramEnum","overtakingProhibited",
"DatexPictogramEnum","overtakingByGoodsVehiclesProhibited",
"DatexPictogramEnum","roadClosedAhead",
"DatexPictogramEnum","roadworks",
"DatexPictogramEnum","slipperyRoad",
"DatexPictogramEnum","noEntryForVehiclesHavingAMassExceedingXTonnesOnOne",
"DatexPictogramEnum","keepASafeDistance",
"DatexPictogramEnum","snow",
"DatexPictogramEnum","blankVoid",
"DatexPictogramEnum","noEntryForVehiclesCarryingDangerousGoods",
"DatexPictogramEnum","chainsOrSnowTyresRecommended",
"DatexPictogramEnum","drivingOfVehiclesLessThanXMetresApartProhibited",
"DatexPictogramEnum","noEntryForVehiclesExceedingXTonnesLadenMass",
"DatexPictogramEnum","maximumSpeedLimit",
"DatexPictogramEnum","advisorySpeed",
"DatexPictogramEnum","noEntryForVehiclesHavingAnOverallLengthExceedingXM",
"DatexPictogramEnum","snowTyresCompulsory",
"DatexPictogramEnum","otherDanger",
"DatexPictogramEnum","endOfAdvisorySpeed",
"DatexPictogramEnum","endOfSpeedLimit",
"DatexPictogramEnum","noEntryForVehiclesHavingAnOverallHeightExceedingXM",
"DatexPictogramEnum","fog",
"DatexPictogramEnum","exitClosed",
"DatexPictogramEnum","endOfProhibitionOfOvertaking",
"DatexPictogramEnum","endOfProhibitionOfOvertakingForGoodsVehicles",

"DayEnum","wednesday","onsdag"
"DayEnum","sunday","söndag"
"DayEnum","tuesday","tisdag"
"DayEnum","thursday","torsdag"
"DayEnum","friday","fredag"
"DayEnum","saturday","lördag"
"DayEnum","monday","mondag"

"DayGroupEnum","mondayToFriday","måndag till fredag"
"DayGroupEnum","allDaysOfWeek","alla dagar i veckan"
"DayGroupEnum","weekend","weekend"
"DayGroupEnum","sundayToFriday","söndag till fredag"
"DayGroupEnum","mondayToSaturday","måndag till lördag"

"DelayCodeEnum","negligible","mycket liten"
"DelayCodeEnum","delayLongerThanSixHours","försening mer än sex timmar"
"DelayCodeEnum","delayLessThanThirtyMinutes","försening mindre än trettio minuter"
"DelayCodeEnum","delayBetweenThreeHoursAndSixHours","försening mellan tre-till sex timmar"
"DelayCodeEnum","delayBetweenOneHourAndThreeHours","försening mellan en-till tre timmar"
"DelayCodeEnum","delayBetweenThirtyMinutesAndOneHour","försening mellan trettio minuter-och en timme"

"DenyReasonEnum","wrongCatalogue","fel katalog"
"DenyReasonEnum","wrongPartner","fel partner"
"DenyReasonEnum","wrongFilter","fel filter"
"DenyReasonEnum","unknownReason","okänt skäl"
"DenyReasonEnum","wrongOrder","fel ordning"

"DirectionCompassEnum","northNorthWest","nordnordväst"
"DirectionCompassEnum","eastSouthEast","östsydöst"
"DirectionCompassEnum","southWest","sydväst"
"DirectionCompassEnum","east","öst"
"DirectionCompassEnum","southSouthEast","sydöst"
"DirectionCompassEnum","northNorthEast","nordnordöst"
"DirectionCompassEnum","north","nord"
"DirectionCompassEnum","eastNorthEast","nordöst"
"DirectionCompassEnum","southEast","sydöst"
"DirectionCompassEnum","northWest","nordväst"
"DirectionCompassEnum","westNorthWest","nordväst"
"DirectionCompassEnum","west","väst"
"DirectionCompassEnum","westSouthWest","sydväst"
"DirectionCompassEnum","southSouthWest","sydsydväst"
"DirectionCompassEnum","south","syd"
"DirectionCompassEnum","northEast","nordöst"

"DirectionEnum","northWestBound","gränsar nordväst"
"DirectionEnum","northBound","gränsar nord"
"DirectionEnum","anticlockwise","motsols"
"DirectionEnum","northEastBound","gränsar nordöst"
"DirectionEnum","westBound","gränsar väst"
"DirectionEnum","southWestBound","gränsar sydväst"
"DirectionEnum","southBound","gränsar syd"
"DirectionEnum","southEastBound","gränsar sydöst"
"DirectionEnum","eastBound","gränsar öst"
"DirectionEnum","clockwise","medsols"

"FuelTypeEnum","lPG",
"FuelTypeEnum","petrol",
"FuelTypeEnum","liquidGas",
"FuelTypeEnum","methane",
"FuelTypeEnum","diesel",

"InformationStatusEnum","securityExercise","säkertsövning"
"InformationStatusEnum","real","verklig"
"InformationStatusEnum","test","test"
"InformationStatusEnum","technicalExercise","tekniskövning"

"InformationUsageEnum","internal","inre"
"InformationUsageEnum","broadcast","sända"
"InformationUsageEnum","internet","internet"
"InformationUsageEnum","variableMessageSign","variabla hastighetsskyltar"

"InjuryStatusTypeEnum","slightInjury","obetydlig skada"
"InjuryStatusTypeEnum","uninjured","oskadd"
"InjuryStatusTypeEnum","seriouslyInjured","allvarligt skadad"
"InjuryStatusTypeEnum","dead","död"
"InjuryStatusTypeEnum","unknown","okänd"

"InstructionsEnum","allowEmergencyVehiclesToPass","låt utryckningdsfordon passera"
"InstructionsEnum","useFogLights","använd dimmljus"
"InstructionsEnum","useHardShoulderAsLane",
"InstructionsEnum","stopAtNextSafePlace","stanna vid nästa säkra avfart"
"InstructionsEnum","switchOffMobilePhonesAndTwoWayRadios","slå av mobiltelefonerna"
"InstructionsEnum","stopAtNextServiceArea","stanna vid nästa service plats"
"InstructionsEnum","onlyTravelIfAbsolutelyNecessary","endast resa vid absolut nödvändighet"
"InstructionsEnum","inEmergencyWaitForPolicePatrol","vid nödsituation
"InstructionsEnum","useHeadlights","använd helljus"
"InstructionsEnum","doNotAllowUnnecessaryGaps","tillåt inte onödiga luckor"
"InstructionsEnum","doNotLeaveYourVehicle","lämna inte ditt fordon"
"InstructionsEnum","overtakeWithCare","kör om försiktigt"
"InstructionsEnum","pullOverToTheEdgeOfTheRoadway","kör in till vägkanten"
"InstructionsEnum","pleaseUseUndergroundService","var snäll och använd tunnelbana"
"InstructionsEnum","leaveYourVehicleProceedToNextSafePlace","lämna fordon och fortsätt till nästa säkra ställe"
"InstructionsEnum","useHazardWarningLights","använd varningslampor"
"InstructionsEnum","switchOffEngine","stäng av motorn"
"InstructionsEnum","doNotDriveOnTheHardShoulder",
"InstructionsEnum","other","annat"
"InstructionsEnum","clearALaneForEmergencyVehicles","lämna en vägbana för räddningsfordon"
"InstructionsEnum","waitForEscortVehicle","vänta på eskortfordon"
"InstructionsEnum","driveCarefully","kör försiktigt"
"InstructionsEnum","driveWithExtremeCaution","kör med stor försiktighet"
"InstructionsEnum","approachWithCare","närma sig med försiktighet"
"InstructionsEnum","keepYourDistance","håll avstånd"
"InstructionsEnum","increaseNormalFollowingDistance","öka normalt avstånd"
"InstructionsEnum","noOvertaking","omkörning förbjuden"
"InstructionsEnum","closeAllWindowsTurnOffHeaterAndVents","stäng alla fönster och stäng av värmen och ventiler"
"InstructionsEnum","clearALaneForSnowploughsAndGrittingVehicles","gör en fri fil för snöplogar och sandfordon"
"InstructionsEnum","crossJunctionWithCare","passera järnvägskorsning med försiktighet"
"InstructionsEnum","observeSignals","iaktta signaler"
"InstructionsEnum","doNotSlowdownUnnecessarily","sakta inte ner onödigtvis"
"InstructionsEnum","followTheVehicleInFrontSmoothly","följ fordonet längst fram "
"InstructionsEnum","testYourBrakes","provbromsa"
"InstructionsEnum","pleaseUseBusService","var god använd busstrafik"
"InstructionsEnum","pleaseUseRailService","var god använd tågtrafik"
"InstructionsEnum","pleaseUseTramService","var god använd spårvagnstrafik"
"InstructionsEnum","noSmoking","rökning förbjuden"
"InstructionsEnum","noNakedFlames","inga öppen eld"
"InstructionsEnum","observeSigns","observera signaler"
"InstructionsEnum","doNotThrowOutAnyBurningObjects","kasta inte ut brinnande föremål"

"InvolvementRolesEnum","vehicleOccupant","fordonsinnehavare"
"InvolvementRolesEnum","pedestrian","fotgängare"
"InvolvementRolesEnum","vehicleDriver","fordonsförare"
"InvolvementRolesEnum","vehiclePassenger","fordonspassagerare"
"InvolvementRolesEnum","cyclist","cyklist"
"InvolvementRolesEnum","witness","vittne"
"InvolvementRolesEnum","unknown","okänd"

"LanesEnum","busStop","busshållpats"
"LanesEnum","leftHandTurningLane",
"LanesEnum","rightHandTurningLane",
"LanesEnum","busLane","bussfält"
"LanesEnum","layBy","parkeringsplats invid väg"
"LanesEnum","escapeLane","utrymmningsfält"
"LanesEnum","turningLane",
"LanesEnum","slowVehicleLane","körfält för långsamma fordon"
"LanesEnum","throughTrafficLane","genomfartsfält"
"LanesEnum","setDownArea","avslastningsområde"
"LanesEnum","crawlerLane","kryp körfält"
"LanesEnum","carPoolLane","bilpools körfält"
"LanesEnum","expressLane","snabb körfält"
"LanesEnum","leftLane","vänster körfält"
"LanesEnum","rightLane","höger körfält"
"LanesEnum","middleLane","mittersta körfältet"
"LanesEnum","emergencyLane","utryckningskörfält"
"LanesEnum","lane7","körfält 7"
"LanesEnum","verge","vägren"
"LanesEnum","allLanesCompleteCarriageway","alla körfält"
"LanesEnum","lane3","körfält 3"
"LanesEnum","lane4","körfält 4"
"LanesEnum","lane5","körfält 5"
"LanesEnum","overtakingLane","omörningsfält"
"LanesEnum","localTrafficLane",
"LanesEnum","centralReservation",
"LanesEnum","opposingLanes",
"LanesEnum","lane8","körfält 8"
"LanesEnum","lane9","körfält 9"
"LanesEnum","lane2","körfält 2"
"LanesEnum","hardShoulder",
"LanesEnum","lane1","körfält 1"
"LanesEnum","heavyVehicleLane","körfält för tunga fordon"
"LanesEnum","lane6","körfält 6"
"LanesEnum","tidalFlowLane",
"LanesEnum","rushHourLane",

"LoadTypeEnum","debris",
"LoadTypeEnum","perishableProducts",
"LoadTypeEnum","toxicMaterials",
"LoadTypeEnum","other",
"LoadTypeEnum","chemicals",
"LoadTypeEnum","combustibleMaterials",
"LoadTypeEnum","corrosiveMaterials",
"LoadTypeEnum","petrol",
"LoadTypeEnum","fuel","olja på vägen"
"LoadTypeEnum","ammunition",
"LoadTypeEnum","vehicles",
"LoadTypeEnum","refuse",
"LoadTypeEnum","hazardousMaterials","fordon som transporterar farligt gods"
"LoadTypeEnum","livestock",
"LoadTypeEnum","explosiveMaterials",
"LoadTypeEnum","materialsDangerousForPeople",
"LoadTypeEnum","glass",
"LoadTypeEnum","abnormalLoad",
"LoadTypeEnum","extraWideLoad",
"LoadTypeEnum","extraLongLoad",
"LoadTypeEnum","materialsDangerousForTheEnvironment",
"LoadTypeEnum","extraHighLoad",
"LoadTypeEnum","oil","bensin på vägen"
"LoadTypeEnum","ordinary",
"LoadTypeEnum","pharmaceuticalMaterials",
"LoadTypeEnum","radioactiveMaterials","omdirigering rekommenderas"
"LoadTypeEnum","materials",

"LocationDescriptorEnum","onTheLeft","till vänster"
"LocationDescriptorEnum","atRestArea","på rastplats"
"LocationDescriptorEnum","atServiceArea","på serviceområde"
"LocationDescriptorEnum","atTollPlaza","vid betalstation"
"LocationDescriptorEnum","withinJunction","inom trafikplats"
"LocationDescriptorEnum","atMotorwayInterchange","på motorvägstrafikplats"
"LocationDescriptorEnum","inTheOppositeDirection","i motsattkörriktning"
"LocationDescriptorEnum","onIceRoad","på isväg"
"LocationDescriptorEnum","onLevelCrossing","i korsning"
"LocationDescriptorEnum","onBridge","på bro"
"LocationDescriptorEnum","onPass","på bergspass"
"LocationDescriptorEnum","inTunnel","i tunnel"
"LocationDescriptorEnum","inGallery","i galleria"
"LocationDescriptorEnum","inTheCentre","i centrum"
"LocationDescriptorEnum","onTheRoadway","på vägbanan"
"LocationDescriptorEnum","onBorder","på gränsnen"
"LocationDescriptorEnum","onTheRight","till höger"
"LocationDescriptorEnum","atTunnelEntryOrExit","i tunnel mynning eller utgång"
"LocationDescriptorEnum","onRoundabout","på cirkulationsplats"
"LocationDescriptorEnum","onLinkRoad","på ihopkopplad väg"
"LocationDescriptorEnum","overCrestOfHill","över toppen av backe"
"LocationDescriptorEnum","onConnector","sammankoppling"
"LocationDescriptorEnum","aroundABendInRoad","runt en kurva på väg"

"LogicalOperatorEnum","or","eller"
"LogicalOperatorEnum","and","och"

         // *******************************************************************
         // ItoM, MaintenanceVehicleActionsEnum
         // *******************************************************************
         //pp("snowClearing", TrafficDataTypes::), // snöröjning
         //pp("stoppingToServiceEquipments", TrafficDataTypes::), // stanna för tjänsteutrustning
         //pp("maintenanceVehiclesMergingIntoTrafficFlow", TrafficDataTypes::), // arbetsfordon på vägen
         //pp("slowMoving", TrafficDataTypes::), // långsamtgående
         //pp("saltAndGritSpreading", TrafficDataTypes::), // saltning

"MatrixFaultEnum","communicationsFailure",
"MatrixFaultEnum","unknown",
"MatrixFaultEnum","unableToClearDown",
"MatrixFaultEnum","powerFailure",
"MatrixFaultEnum","incorrectAspectDisplayed",
"MatrixFaultEnum","other",

"MeasuredOrDerivedDataTypeEnum","trafficStatusInformation",
"MeasuredOrDerivedDataTypeEnum","individualVehicleMeasurements",
"MeasuredOrDerivedDataTypeEnum","visibilityInformation",
"MeasuredOrDerivedDataTypeEnum","pollutionInformation",
"MeasuredOrDerivedDataTypeEnum","windInformation",
"MeasuredOrDerivedDataTypeEnum","precipitationInformation",
"MeasuredOrDerivedDataTypeEnum","pressureInformation",
"MeasuredOrDerivedDataTypeEnum","radiationInformation",
"MeasuredOrDerivedDataTypeEnum","roadSurfaceConditionInformation",
"MeasuredOrDerivedDataTypeEnum","trafficFlow",
"MeasuredOrDerivedDataTypeEnum","trafficHeadway",
"MeasuredOrDerivedDataTypeEnum","trafficSpeed",
"MeasuredOrDerivedDataTypeEnum","travelTimeInformation",
"MeasuredOrDerivedDataTypeEnum","humidityInformation",
"MeasuredOrDerivedDataTypeEnum","temperatureInformation",
"MeasuredOrDerivedDataTypeEnum","trafficConcentration",

"MobilityEnum","unknown","okänt"
"MobilityEnum","mobile","mobilt"
"MobilityEnum","stationary","stationärt"

"MonthOfYearEnum","march",
"MonthOfYearEnum","december",
"MonthOfYearEnum","november",
"MonthOfYearEnum","february",
"MonthOfYearEnum","september",
"MonthOfYearEnum","october",
"MonthOfYearEnum","may",
"MonthOfYearEnum","june",
"MonthOfYearEnum","july",
"MonthOfYearEnum","august",
"MonthOfYearEnum","april",
"MonthOfYearEnum","january",

"OperatingModeEnum","operatingMode3",
"OperatingModeEnum","operatingMode2",
"OperatingModeEnum","operatingMode1",
"OperatingModeEnum","operatingMode0",

         // *******************************************************************
         // NtoR, OperatorActionOriginEnum
         // *******************************************************************
         //pp("external", TrafficDataTypes::), 
         //pp("internal", TrafficDataTypes::), 

"OperatorActionStatusEnum","approved",
"OperatorActionStatusEnum","inProgress",
"OperatorActionStatusEnum","implemented",
"OperatorActionStatusEnum","over",
"OperatorActionStatusEnum","requested",

"OverallImpactEnum","highest","mycket stor"
"OverallImpactEnum","high","stor"
"OverallImpactEnum","normal","normalt"
"OverallImpactEnum","lowest","mycket liten"
"OverallImpactEnum","low","liten"

"PeriodByPhaseOfDayEnum","duringDayLightHours",
"PeriodByPhaseOfDayEnum","duringDarknessHours",
"PeriodByPhaseOfDayEnum","duringEveningBeforeSunset",
"PeriodByPhaseOfDayEnum","fromSunsetToMidnight",
"PeriodByPhaseOfDayEnum","duringMorningBeforeSunrise",
"PeriodByPhaseOfDayEnum","duringEveningRushHour",
"PeriodByPhaseOfDayEnum","fromMidnightToSunrise",
"PeriodByPhaseOfDayEnum","duringMorningRushHour",

"PersonCategoryEnum","medicalStaff",
"PersonCategoryEnum","emergencyServicesPerson",
"PersonCategoryEnum","infant",
"PersonCategoryEnum","publicTransportPassenger",
"PersonCategoryEnum","trafficPatrolOfficer",
"PersonCategoryEnum","police",
"PersonCategoryEnum","politician",
"PersonCategoryEnum","veryImportantPerson",
"PersonCategoryEnum","adult",
"PersonCategoryEnum","child","barn på vägbanan"
"PersonCategoryEnum","sickPerson",

"PlacesAdviceEnum","inLowLyingAreas","i lågt liggande områden"
"PlacesAdviceEnum","onEnteringOrLeavingTunnels","träda in eller lämna tunnlar"
"PlacesAdviceEnum","inTunnels","i tunell"
"PlacesAdviceEnum","onBridges","på broar"
"PlacesAdviceEnum","other","annat"
"PlacesAdviceEnum","onSlipRoads","på hala vägar"
"PlacesAdviceEnum","inRoadworksAreas","på vägarbetsområden"
"PlacesAdviceEnum","inShadedAreas","i skyddsområden"
"PlacesAdviceEnum","atHighAltitudes","på höjder"
"PlacesAdviceEnum","inGallaries","i gallerier"
"PlacesAdviceEnum","aroundBendsInTheRoad","omkring kurvor på vägen"
"PlacesAdviceEnum","overTheCrestOfHills","över krönet på berget"
"PlacesAdviceEnum","atCustomsPosts",
"PlacesAdviceEnum","atTollPlazas","i tullhus"
"PlacesAdviceEnum","onMotorways","på motorvägar"
"PlacesAdviceEnum","onRoundabouts","i rondeller"
"PlacesAdviceEnum","inTheInnerCityAreas","i innerstans områden"
"PlacesAdviceEnum","inTheCityCentre","i centrum"
"PlacesAdviceEnum","onUndergroundSections","i tunnelbanesektioner på vägen "

"PollutantTypeEnum","particulates10","mikrometer"
"PollutantTypeEnum","polycyclicAromaticHydrocarbons",
"PollutantTypeEnum","totalHydrocarbons",
"PollutantTypeEnum","lead","led"
"PollutantTypeEnum","primaryParticulate",
"PollutantTypeEnum","benzeneTolueneXylene",
"PollutantTypeEnum","ozone","ozon"
"PollutantTypeEnum","nitrogenMonoxide",
"PollutantTypeEnum","nonMethaneHydrocarbons",
"PollutantTypeEnum","nitrogenDioxide",
"PollutantTypeEnum","carbonMonoxide",
"PollutantTypeEnum","methane","metan"
"PollutantTypeEnum","nitrogenOxides",
"PollutantTypeEnum","nitricOxide",
"PollutantTypeEnum","sulphurDioxide",

"PrecipitationTypeEnum","hail","hagel"
"PrecipitationTypeEnum","sleet","snöblandat regn"
"PrecipitationTypeEnum","snow","snö"
"PrecipitationTypeEnum","rain","regn"
"PrecipitationTypeEnum","drizzle","duggregn"

"ProbabilityOfOccurrenceEnum","probable","sannolik"
"ProbabilityOfOccurrenceEnum","improbable","osannolik"
"ProbabilityOfOccurrenceEnum","riskOf","risk för"
"ProbabilityOfOccurrenceEnum","certain","viss"

"ReferencePointDirectionEnum","both","båda"
"ReferencePointDirectionEnum","positive","positiv"
"ReferencePointDirectionEnum","negative","negativ"
"ReferencePointDirectionEnum","unknown","okänd"

"RelativeTrafficFlowEnum","trafficLighterThanNormal","mindre trafik än vanligt"
"RelativeTrafficFlowEnum","trafficVeryMuchHeavierThanNormal","trafiken mycket intensivare än normalt"
"RelativeTrafficFlowEnum","trafficHeavierThanNormal","trafiken intensivare än normalt"

"RequestTypeEnum","requestHistoricalData","begära historisk data"
"RequestTypeEnum","catalogue","katalog"
"RequestTypeEnum","filter","filter"
"RequestTypeEnum","subscription","prenumeration"
"RequestTypeEnum","requestData","begära data"

"ReroutingTypeEnum","reroutingUseEntry","omledning använd ingång"
"ReroutingTypeEnum","reroutingUseExit","omledning använd utgång"
"ReroutingTypeEnum","reroutingUseIntersectionOrJunction","omledning använd gatukorsning eller järnvägsföbindelse"

"ResponseEnum","filterRequestDenied","filter begäran avslagen"
"ResponseEnum","catalogueRequestDenied","katalog begäran avslagen"
"ResponseEnum","subscriptionRequestDenied","prenumerations begäran avslagen"
"ResponseEnum","acknowledge","bekräftan"
"ResponseEnum","requestDenied","begäran avslagen"

"RoadsideAssistanceTypeEnum","helicopterRescue","helikopterräddning"
"RoadsideAssistanceTypeEnum","other","annat"
"RoadsideAssistanceTypeEnum","vehicleRecovery","fordonsbättring"
"RoadsideAssistanceTypeEnum","emergencyServices","nödsituationstjänst"
"RoadsideAssistanceTypeEnum","airAmbulance","flygambulans"
"RoadsideAssistanceTypeEnum","vehicleRepair","fordonsreparering"
"RoadsideAssistanceTypeEnum","firstAid","första hjälpen"
"RoadsideAssistanceTypeEnum","foodDelivery","matleverans"
"RoadsideAssistanceTypeEnum","busPassengerAssistance","busspassagerar assistans"

"ServiceDisruptionTypeEnum","someCommercialServicesClosed","föräljningsställen stängda"
"ServiceDisruptionTypeEnum","restAreaBusy","rastplats upptagen"
"ServiceDisruptionTypeEnum","methaneShortage","metangasläcka"
"ServiceDisruptionTypeEnum","noToiletFacilities","inga toalettinrättningar"
"ServiceDisruptionTypeEnum","noPublicTelephones","inga publika telefoner"
"ServiceDisruptionTypeEnum","noVehicleRepairFacilities","inga fordonsreparationsinrättningar"
"ServiceDisruptionTypeEnum","barClosed","bar stängd"
"ServiceDisruptionTypeEnum","noDieselForHeavyVehicles","ingen diesel för tunga fordon"
"ServiceDisruptionTypeEnum","restAreaOvercrowdedDriveToAnotherRestArea","rastplats överfull var god kör till annan"
"ServiceDisruptionTypeEnum","petrolShortage","ont om bensin"
"ServiceDisruptionTypeEnum","serviceAreaRestaurantClosed","serviceområdets restaurang stängd"
"ServiceDisruptionTypeEnum","noDieselForLightVehicles","ingen diesel för lätta fordon"
"ServiceDisruptionTypeEnum","fuelShortage","ont om bränsle"
"ServiceDisruptionTypeEnum","serviceAreaOvercrowdedDriveToAnotherServiceArea","serviceområdet fullt
"ServiceDisruptionTypeEnum","serviceAreaBusy","serviceområdet upptaget"
"ServiceDisruptionTypeEnum","serviceAreaFuelStationClosed","serviceområdets bensinstation stängd"
"ServiceDisruptionTypeEnum","serviceAreaClosed","serviceområde stängt"
"ServiceDisruptionTypeEnum","restAreaClosed","rastplats stängd"
"ServiceDisruptionTypeEnum","lPGShortage","ont om flytande petroleum gas"
"ServiceDisruptionTypeEnum","dieselShortage","ont om diesel"
"ServiceDisruptionTypeEnum","waterShortage","ont om vatten"

"SourceTypeEnum","mobileTelephoneCaller","mobiltelefon användare"
"SourceTypeEnum","policePatrol","polispatrull"
"SourceTypeEnum","otherOfficialVehicle","andra officiella fordon"
"SourceTypeEnum","nonPoliceEmergencyServicePatrol","idrottsevenemang"
"SourceTypeEnum","registeredMotoristObserver","registrerad motorist observatör"
"SourceTypeEnum","microwaveMonitoringStation",
"SourceTypeEnum","infraredMonitoringStation",
"SourceTypeEnum","inductionLoopMonitoringStation",
"SourceTypeEnum","freightVehicleOperator",
"SourceTypeEnum","cameraObservation","kameraobservation"
"SourceTypeEnum","otherInformation","annan information"
"SourceTypeEnum","transitOperator",
"SourceTypeEnum","spotterAircraft","flygräd"
"SourceTypeEnum","automobileClubPatrol","patrullerande automobil klubb"
"SourceTypeEnum","videoProcessingMonitoringStation","övervakningsstation för videobearbetning"
"SourceTypeEnum","privateBreakdownService","privat maskinhaveri tjänst"
"SourceTypeEnum","vehicleProbeMeasurement","fordonsundersökningsutrustning"
"SourceTypeEnum","publicAndPrivateUtilities","publik eller privat samhällsservice"
"SourceTypeEnum","roadOperatorPatrol","vägoperatörspatrull"
"SourceTypeEnum","trafficMonitoringStation","trafikövervakningstation"
"SourceTypeEnum","roadsideTelephoneCaller","normal kollektivtrafik åter igång"
"SourceTypeEnum","roadAuthorities","vägauktoritet"

"SpeedAdviceEnum","speedLimitInForceForHeavyVehicles","hastighetsbegränsning i kraft för tunga fordon"
"SpeedAdviceEnum","mandatorySpeedLimitInForce",
"SpeedAdviceEnum","policeSpeedChecksInOperation","hastighetskontroller i drift"
"SpeedAdviceEnum","other","annat"
"SpeedAdviceEnum","observeRecommendedSpeed","iaktta rekommenderad hastighet"
"SpeedAdviceEnum","observeSpeedLimits","iaktta hastighetsbegränsning"
"SpeedAdviceEnum","reduceYourSpeed","sänk hastigheten"

"SubjectTypeOfWorksEnum","roadSigns","vägskyltar"
"SubjectTypeOfWorksEnum","gallery","galleri"
"SubjectTypeOfWorksEnum","bridge","broarbete"
"SubjectTypeOfWorksEnum","levelCrossing","övergångsställe"
"SubjectTypeOfWorksEnum","lightingSystem","belysningssystem"
"SubjectTypeOfWorksEnum","interchange","utbyte"
"SubjectTypeOfWorksEnum","junction","knutpunkt"
"SubjectTypeOfWorksEnum","roundabout","rondell"
"SubjectTypeOfWorksEnum","buriedServices","arbete med nedgrävda ledningar"
"SubjectTypeOfWorksEnum","buriedCables","arbete med nedgrävda kablar"
"SubjectTypeOfWorksEnum","waterMain","arbete på vattenledning"
"SubjectTypeOfWorksEnum","measurementEquipment","mätutrustning"
"SubjectTypeOfWorksEnum","gasMainWork","arbete på huvudgasledning"
"SubjectTypeOfWorksEnum","other","annat "
"SubjectTypeOfWorksEnum","road","väg"
"SubjectTypeOfWorksEnum","lane","fil"
"SubjectTypeOfWorksEnum","centralReservation","arbete på mittremsan"
"SubjectTypeOfWorksEnum","tunnel","tunnel"
"SubjectTypeOfWorksEnum","carriageway","körbana"
"SubjectTypeOfWorksEnum","roadsideEquipment","väggrensutrustning"
"SubjectTypeOfWorksEnum","crashBarrier","avspärrning"

"SubscriptionStateEnum","suspended","suspenderad"
"SubscriptionStateEnum","active","aktiv"

"TPEGLoc01AreaLocationSubtypeEnum","other","annat"
"TPEGLoc01AreaLocationSubtypeEnum","largeArea","stort område"
"TPEGLoc01FramedPointLocationSubtypeEnum","framedPoint","inramad punkt"
"TPEGLoc01LinearLocationSubtypeEnum","segment","segment"
"TPEGLoc01SimplePointLocationSubtypeEnum","nonLinkedPoint","ej länkad punkt"
"TPEGLoc01SimplePointLocationSubtypeEnum","intersection","vägkorsning"
"TPEGLoc02DirectionTypeEnum","southEastBound","gränsar sydöst"
"TPEGLoc02DirectionTypeEnum","other","annat"
"TPEGLoc02DirectionTypeEnum","innerRing","innerring"
"TPEGLoc02DirectionTypeEnum","westBound","gränsar väst"
"TPEGLoc02DirectionTypeEnum","southBound","gränsar syd"
"TPEGLoc02DirectionTypeEnum","unknown","okänd"
"TPEGLoc02DirectionTypeEnum","eastBound","gränsar öst"
"TPEGLoc02DirectionTypeEnum","northEastBound","gränsar nordöst"
"TPEGLoc02DirectionTypeEnum","northBound","gränsar nord"
"TPEGLoc02DirectionTypeEnum","bothWays","båda riktningar"
"TPEGLoc02DirectionTypeEnum","outerRing","ytterring"
"TPEGLoc02DirectionTypeEnum","anticlockwise","moturs"
"TPEGLoc02DirectionTypeEnum","clockwise","medurs"
"TPEGLoc02DirectionTypeEnum","northWestBound","gränsar nordväst"
"TPEGLoc02DirectionTypeEnum","opposite","motsats"
"TPEGLoc02DirectionTypeEnum","southWestBound","gränsar sydväst"
"TPEGLoc02DirectionTypeEnum","allDirections","alla riktiningar"
"TPEGLoc03AreaDescriptorSubtypeEnum","townName","stadsnamn"
"TPEGLoc03AreaDescriptorSubtypeEnum","lakeName","insjönamn"
"TPEGLoc03AreaDescriptorSubtypeEnum","areaName","områdesnamn"
"TPEGLoc03AreaDescriptorSubtypeEnum","seaName","sjönamn"
"TPEGLoc03AreaDescriptorSubtypeEnum","policeForceControlAreaName","poliskontrollsområdesnamn"
"TPEGLoc03AreaDescriptorSubtypeEnum","administrativeReferenceName","adminstrativreferensnamn"
"TPEGLoc03AreaDescriptorSubtypeEnum","nationName","nationsnamn"
"TPEGLoc03AreaDescriptorSubtypeEnum","other","annat"
"TPEGLoc03AreaDescriptorSubtypeEnum","regionName","regionsnamn"
"TPEGLoc03AreaDescriptorSubtypeEnum","countyName","länsnamn"
"TPEGLoc03AreaDescriptorSubtypeEnum","administrativeAreaName","administrativområdesnamn"
"TPEGLoc03ILCPointDescriptorSubtypeEnum","tpegILCName3",
"TPEGLoc03ILCPointDescriptorSubtypeEnum","tpegILCName1",
"TPEGLoc03ILCPointDescriptorSubtypeEnum","tpegILCName2",
"TPEGLoc03JunctionPointDescriptorSubtypeEnum","junctionName",
"TPEGLoc03OtherPointDescriptorSubtypeEnum","seaName",
"TPEGLoc03OtherPointDescriptorSubtypeEnum","other",
"TPEGLoc03OtherPointDescriptorSubtypeEnum","airportName",
"TPEGLoc03OtherPointDescriptorSubtypeEnum","serviceAreaName",
"TPEGLoc03OtherPointDescriptorSubtypeEnum","parkingFacilityName",
"TPEGLoc03OtherPointDescriptorSubtypeEnum","pointOfInterestName",
"TPEGLoc03OtherPointDescriptorSubtypeEnum","townName",
"TPEGLoc03OtherPointDescriptorSubtypeEnum","buildingName",
"TPEGLoc03OtherPointDescriptorSubtypeEnum","busStopIdentifier",
"TPEGLoc03OtherPointDescriptorSubtypeEnum","busStopName",
"TPEGLoc03OtherPointDescriptorSubtypeEnum","nonLinkedPointName",
"TPEGLoc03OtherPointDescriptorSubtypeEnum","lakeName",
"TPEGLoc03OtherPointDescriptorSubtypeEnum","canalName",
"TPEGLoc03OtherPointDescriptorSubtypeEnum","riverName",
"TPEGLoc03OtherPointDescriptorSubtypeEnum","linkName",
"TPEGLoc03OtherPointDescriptorSubtypeEnum","ferryPortName",
"TPEGLoc03OtherPointDescriptorSubtypeEnum","railwayStation",
"TPEGLoc03OtherPointDescriptorSubtypeEnum","tidalRiverName",
"TPEGLoc03OtherPointDescriptorSubtypeEnum","localLinkName",
"TPEGLoc03OtherPointDescriptorSubtypeEnum","metroStationName",
"TPEGLoc03OtherPointDescriptorSubtypeEnum","intersectionName",
"TPEGLoc03OtherPointDescriptorSubtypeEnum","pointName",

"TPEGLoc04HeightTypeEnum","at",
"TPEGLoc04HeightTypeEnum","atSeaLevel",
"TPEGLoc04HeightTypeEnum","other",
"TPEGLoc04HeightTypeEnum","below",
"TPEGLoc04HeightTypeEnum","unknown",
"TPEGLoc04HeightTypeEnum","above",
"TPEGLoc04HeightTypeEnum","undefined",
"TPEGLoc04HeightTypeEnum","aboveSeaLevel",
"TPEGLoc04HeightTypeEnum","belowSeaLevel",
"TPEGLoc04HeightTypeEnum","aboveStreetLevel",
"TPEGLoc04HeightTypeEnum","belowStreetLevel",
"TPEGLoc04HeightTypeEnum","atStreetLevel",

"TrafficControlEquipmentTypeEnum","laneControlSigns","trafikljussignaler"
"TrafficControlEquipmentTypeEnum","rampControls","uppfartsväg"
"TrafficControlEquipmentTypeEnum","speedControlSigns","hastighetskontrollssignaler"
"TrafficControlEquipmentTypeEnum","tollGates","tullbom"
"TrafficControlEquipmentTypeEnum","trafficLightSets","sammanhängande trafikljus"

"TrafficControlOptionsEnum","mandatory","frivilligt"
"TrafficControlOptionsEnum","advisory","rådande"

"TrafficControlTypeEnum","operatorImposedLimitOrRestriction","operatör införde begränsning eller restriktion"
"TrafficControlTypeEnum","hardShoulderRunning",
"TrafficControlTypeEnum","vehicleStorageActive","fordonsförvaring"
"TrafficControlTypeEnum","tollGatesOpen","tullgrind öppen"
"TrafficControlTypeEnum","rerouting","omledning"
"TrafficControlTypeEnum","activeSpeedControl","verksam hastighetskontroll"
"TrafficControlTypeEnum","rampMeteringActive","verksam uppfartsväg"

"TrafficDestinationTypeEnum","destinedForFerryService","ämnad för färjetjänster"
"TrafficDestinationTypeEnum","throughTraffic","genomtrafik"
"TrafficDestinationTypeEnum","destinedForRailService","ämnad för järnvägstjänster"
"TrafficDestinationTypeEnum","destinedForAirport","ämnad för flygplats"
"TrafficRestrictionTypeEnum","laneDeviated","filavvikelse"
"TrafficRestrictionTypeEnum","roadPartiallyObstructed","väg till viss del blockerad"
"TrafficRestrictionTypeEnum","carriagewayBlocked","körbana blockerad"
"TrafficRestrictionTypeEnum","carriagewayPartiallyObstructed","körbana till viss del blockerad"
"TrafficRestrictionTypeEnum","lanesPartiallyObstructed","filer till viss del blockerade"
"TrafficRestrictionTypeEnum","lanesBlocked","körfält blockerad"
"TrafficRestrictionTypeEnum","roadBlocked","väg blockerad"

"TrafficStatusEnum","freeFlow","fritt flöde"
"TrafficStatusEnum","heavy","tungt"
"TrafficStatusEnum","congested","stoppa upp"
"TrafficStatusEnum","impossible","omöjligt"
"TrafficStatusEnum","unknown","okänd"

"TransitServiceTypeEnum","ferry","färja"
"TransitServiceTypeEnum","bus","buss"
"TransitServiceTypeEnum","undergroundMetro","tunnelbana"
"TransitServiceTypeEnum","tram","spårvagn"
"TransitServiceTypeEnum","air","flyg"
"TransitServiceTypeEnum","hydrofoil",
"TransitServiceTypeEnum","rail","tåg",

"TravelTimeTrendTypeEnum","stable",
"TravelTimeTrendTypeEnum","decreasing",
"TravelTimeTrendTypeEnum","increasing",

"TravelTimeTypeEnum","instantaneous",
"TravelTimeTypeEnum","best",
"TravelTimeTypeEnum","reconstituted",
"TravelTimeTypeEnum","estimated",

"UpdateMethodEnum","snapshot",
"UpdateMethodEnum","singleElementUpdate",
"UpdateMethodEnum","allElementUpdate",

"UrgencyEnum","urgent",
"UrgencyEnum","normalUrgency",
"UrgencyEnum","extremelyUrgent",

"ValidityStatusEnum","suspended","inaktivt"
"ValidityStatusEnum","definedByValidityTimeSpec","defineras av giltighetstider"
"ValidityStatusEnum","active","aktivt"

"ValueCompareOperatorEnum","greaterEqual",
"ValueCompareOperatorEnum","unequal",
"ValueCompareOperatorEnum","notContainedIn",
"ValueCompareOperatorEnum","equal",
"ValueCompareOperatorEnum","containedIn",
"ValueCompareOperatorEnum","less",
"ValueCompareOperatorEnum","greater",
"ValueCompareOperatorEnum","contains",
"ValueCompareOperatorEnum","containsNot",
"ValueCompareOperatorEnum","lessEqual",

"VehicleEquipmentEnum","snowChainsInUse",
"VehicleEquipmentEnum","withoutSnowTyresOrChainsOnBoard",
"VehicleEquipmentEnum","notUsingSnowChainsOrTyres",
"VehicleEquipmentEnum","notUsingSnowChains",
"VehicleEquipmentEnum","snowChainsOrTyresInUse",
"VehicleEquipmentEnum","snowTyresInUse",

"VehicleStatusEnum","burntOut",
"VehicleStatusEnum","damaged",
"VehicleStatusEnum","brokenDown",
"VehicleStatusEnum","onFire",
"VehicleStatusEnum","abandoned",
"VehicleStatusEnum","damagedAndImmobililized",

"VehicleTypeEnum","vehicleWithCatalyticConverter",
"VehicleTypeEnum","vehicleWithCaravan",
"VehicleTypeEnum","carOrLightVehicle",
"VehicleTypeEnum","bus",
"VehicleTypeEnum","van",
"VehicleTypeEnum","motorcycle",
"VehicleTypeEnum","lorry",
"VehicleTypeEnum","car",
"VehicleTypeEnum","heavyLorry",
"VehicleTypeEnum","twoWheeledVehicle",
"VehicleTypeEnum","heavyVehicle",
"VehicleTypeEnum","lightVehicle",
"VehicleTypeEnum","goodsVehicle",
"VehicleTypeEnum","articulatedVehicle",
"VehicleTypeEnum","vehicleWithTrailer",
"VehicleTypeEnum","carWithCaravan",
"VehicleTypeEnum","fourWheelDrive",
"VehicleTypeEnum","withEvenNumberedRegistrationPlates",
"VehicleTypeEnum","withOddNumberedRegistrationPlates",
"VehicleTypeEnum","other",
"VehicleTypeEnum","carWithTrailer",
"VehicleTypeEnum","anyVehicle",
"VehicleTypeEnum","vehicleWithoutCatalyticConverter",
"VehicleTypeEnum","highSidedVehicle",

"VehicleUsageEnum","roadOperator",
"VehicleUsageEnum","patrol",
"VehicleUsageEnum","military",
"VehicleUsageEnum","emergencyServices",
"VehicleUsageEnum","taxi",
"VehicleUsageEnum","recoveryServices",
"VehicleUsageEnum","agricultural",
"VehicleUsageEnum","nonCommercial",
"VehicleUsageEnum","commercial",
"VehicleUsageEnum","roadMaintenanceOrConstruction",

"VMSFaultEnum","unableToClearDown",
"VMSFaultEnum","incorrectPictogramDisplayed",
"VMSFaultEnum","incorrectMessageDisplayed",
"VMSFaultEnum","powerFailure",
"VMSFaultEnum","unknown",
"VMSFaultEnum","other",
"VMSFaultEnum","communicationsFailure",

"VMSTypeEnum","continuousSign",
"VMSTypeEnum","other",
"VMSTypeEnum","colourGraphic",
"VMSTypeEnum","monochromeGraphic",

"WarningAdviceEnum","trafficBeingDirectedAroundAccidentArea","trafiken omleds runt olycksområde"
"WarningAdviceEnum","dangerOfExplosion","explosionsrisk"
"WarningAdviceEnum","policeInAttendance","polis är närvarande"
"WarningAdviceEnum","aquaplaningRisk","risk för vattenplaning"
"WarningAdviceEnum","emergencyVehiclesAtScene","utryckningsfordon vid olycksplatsen"
"WarningAdviceEnum","toxicLeak","giftläckage"
"WarningAdviceEnum","trafficWardensDirectingTraffic","trafikvakt dirigerar trafiken"
"WarningAdviceEnum","radiationHazard","strålningsfara"
"WarningAdviceEnum","danger","fara"
"WarningAdviceEnum","dangerOfFire","fara för brand"
"WarningAdviceEnum","extraPolicePatrolsInOperation","extra polispatruller i arbete"
"WarningAdviceEnum","rescueAndRecoveryWorkInProgress","räddnings och bättringsarbete pågår"
"WarningAdviceEnum","severalAccidentsHaveTakenPlace","flera olyckor har skett"
"WarningAdviceEnum","helicopterRescueInProgress","helikopterräddning pågår"
"WarningAdviceEnum","skidRisk","halt väglag"
"WarningAdviceEnum","policeDirectingTraffic","polis dirigerar trafik"
"WarningAdviceEnum","pilotCarInOperation",
"WarningAdviceEnum","firemenDirectingTraffic","brandmän dirigerar trafiken"
"WarningAdviceEnum","surfaceWaterHazard","varning för våt vägbana"
"WarningAdviceEnum","repairsInProgress","reparationer pågår"
"WarningAdviceEnum","lookOutForFlagman","håll utsikt efter flaggman"
"WarningAdviceEnum","other","annat"
"WarningAdviceEnum","slipperyPavements","hala trottoarer"
"WarningAdviceEnum","increasedRiskOfAccident","ökad risk för olycka"

"WeekOfMonthEnum","fifthWeekOfMonth",
"WeekOfMonthEnum","fouthWeekOfMonth",
"WeekOfMonthEnum","thirdWeekOfMonth",
"WeekOfMonthEnum","secondWeekOfMonth",
"WeekOfMonthEnum","firstWeekOfMonth",
*/
