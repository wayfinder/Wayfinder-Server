/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef TRAFFICDATATYPES_H
#define TRAFFICDATATYPES_H

#include "config.h"

class TrafficDataTypes {
   
public:
   
   enum disturbanceType {
      Accident = 0,
      Activities = 1,
      TrafficSignalPlans = 2,
      SnowIceEquipment = 3,
      SnowOnTheRoad = 4,
      TravelTime = 5,
      WeatherData = 6,
      Wind = 7,
      ActionPlan = 8,
      AverageSpeed = 9,
      Concentration = 10,
      DelaysCancellations = 11,
      TrafficEquipmentStatus = 12,
      ExhaustPollution = 13,
      FerriesTrains = 14,
      Flow = 15,
      FogSmokeDust = 16,
      Incident = 17,
      ServiceInformation = 18,
      IndividualVehicleData = 19,
      LevelOfService = 20,
      MovingHazards = 21,
      Occupancy = 22,
      OriginDestinationMatrix = 23,
      ObstructionHazards = 24,
      OperatorActions = 25,
      Carparks = 26,
      Precipitation = 27,
      TrafficRestrictions = 28,
      RoadWorks = 29,
      Rerouting = 30,
      SkidHazards = 31,      
      Other = 32,
      Camera = 33,
      Police = 34,
      SpeedTrap = 35,
      NoType = 36,
      BlackPoint = 37,
      UserDefinedCamera = 38,

      // Added for DatexII
      // Obstruction:
      AnimalPresenceObstruction = 39,
      EnvironmentalObstruction = 40,
      EquipmentDamageObstruction = 41,
      GeneralObstruction = 42,
      VehicleObstruction = 43,
      AbnormalTraffic = 44,
      PoorRoadInfrastructure = 45,
      // Conditions:
      WeatherRelatedRoadConditions = 46,
      NonWeatherRelatedRoadConditions = 47,
      PoorEnvironmentConditions = 48,
      // Operator Action:
      ConstructionWorks = 49,
      MaintenanceWorks = 50,
      VariableMessageSignSetting = 51,
      MatrixSignSetting = 52,
      NetworkManagement = 53,
      RoadsideAssistance = 54,
      // Non road event information:
      TransitInformation = 55,
      ServiceDisruption = 56,
   };
   
   enum phrase {
      ABL = 0,   // Abnormal load(s)
      ABX = 1,   // Exceptional load warning cleared
      ACA = 2,   // Earlier accident(s)
      ACB = 3,   // Accident involving bus(es)
      ACD = 4,   // Secondary accident(s)
      ACE = 5,   // Chemical spillage accident(s)
      ACF = 6,   // Fuel spillage accident(s)
      ACH = 7,   // Accident involving heavy lorr(y/ies)
      ACI = 8,   // Accident(s)
      ACL = 9,   // Accident clearance
      ACM = 10,  // Multi-vehicle accident
      ACS = 11,  // Serious accident(s)
      ACW = 12,  // Accident investigation work
      ACX = 13,  // Accident cleared
      ACZ = 14,  // Accident(s) involving hazardous materials
      AIC = 15,  // Air crash
      AIR = 16,  // Air raid
      AJA = 17,  // Jack-knifed articulated lorr(y/ies)
      AJC = 18,  // Jack-knifed caravan(s)
      AJT = 19,  // Jack-knifed trailer(s)
      ALA = 20,  // Alarm call: important new information on this
                 // frequency follows now
      ALC = 21,  // Alarm call: important new information will be
                 // broadcast between these times
      ALL = 22,  // All accidents cleared
      ALR = 23,  // Additional local information is provided by another
                 // TMC service
      ANH = 24,  // Herd of animals on the road
      ANL = 25,  // Large animals on the road
      ANM = 26,  // Animals on the road
      AOI = 27,  // Oil spillage accident(s)
      AOL = 28,  // Overturned heavy lorr(y/ies)
      AOV = 29,  // Overturned vehicle(s)
      APF = 30,  // All carparks full
      APT = 31,  // Additional public transport information is provided
                 // by another TMC service
      AQD = 32,  // Danger of aquaplaning
      ASP = 33,  // Vehicle(s) spun around
      ATR = 34,  // Additional regional information is provided by
                 // another TMC service
      AVL = 35,  // Avalanches
      AVS = 36,  // Average speed
      BCL = 37,  // Broken-down vehicle clearance
      BDB = 38,  // Broken down bus(es)
      BDX = 39,  // Breakdown cleared
      BKD = 40,  // Broken down vehicle(s)
      BLI = 41,  // Blizzard
      BLO = 42,  // Blowing dust
      BLS = 43,  // Blowing snow
      BRB = 44,  // Bridge blocked
      BRC = 45,  // Bridge closed
      BRP = 46,  // Burst pipe
      BSI = 47,  // Bus services irregular. Delays
      BUN = 48,  // Bus services not operating
      BWM = 49,  // Burst water main
      CAA = 50,  // Carpool lane in operation
      CAC = 51,  // Cancellations
      CAL = 52,  // Message cancelled
      CBW = 53,  // Closures due to bad weather
      CCB = 54,  // Connecting carriageway blocked
      CCC = 55,  // Connecting carriageway closed
      CHI = 56,  // Children on roadway
      CLE = 57,  // Clearing action
      CLW = 58,  // Clearance work
      CON = 59,  // Overnight closures
      COO = 60,  // Emergancy telephones not working
      COW = 61,  // Convoy service due to bad weather
      CPW = 62,  // Roads permanently closed for the winter
      CRC = 63,  // Carpool restrictions changed
      CRX = 64,  // Carpool restrictions lifted
      CTR = 65,  // Contraflow
      CTT = 66,  // Concentration
      CTX = 67,  // Contraflow removed
      CV1 = 68,  // Closed for vehicles with only one occupant
      CV3 = 69,  // Closed for vehicles with less than three occupants
      CVX = 70,  // Convoy cleared
      CVY = 71,  // Convoy(s)
      CWC = 72,  // All carriageways cleared
      CYC = 73,  // Cyclists on roadway
      DCD = 74,  // Difficult driving conditions
      DCE = 75,  // Extremely hazardous driving conditions
      DCL = 76,  // Air raid warning cancelled
      DCN = 77,  // Driving conditions improved
      DCZ = 78,  // Hazardous driving conditions
      DEI = 79,  // Detailed information will be given on audio
                 // program broadcast
      DEU = 80,  // Delayed until further notice
      DEX = 81,  // Delays cleared
      DIP = 82,  // Detailed information is provided by another TMC
                 // provider
      DLL = 83,  // Long delays
      DLY = 84,  // Delays
      DO  = 85,  // Diversion in operation
      DPN = 86,  // Service suspended
      DUD = 87,  // Delays of uncertain duration
      DVL = 88,  // Very long delays
      DXX = 89,  // Delays expected to be cleared
      EAM = 90,  // Athletics meeting
      EBA = 91,  // Bomb alert
      EBF = 92,  // Bull fight
      EBG = 93,  // Ball game
      EBT = 94,  // Boxing tournament
      ECL = 95,  // Give way to emergency vehicles on the carpool lane
      ECM = 96,  // Cricket match
      ECR = 97,  // Crowd
      ECY = 98,  // Cycle race
      EFA = 99,  // Fair
      EFM = 100, // Football match
      EFR = 101, // Emergancy call facilities restored
      EGT = 102, // Golf tournament
      EHL = 103, // Give way to emergency vehicles on the heavy vehicle lane
      EIS = 104, // International sports meeting
      EMH = 105, // High-speed emergency vehicle(s)
      EMR = 106, // March
      EMT = 107, // Match
      EMV = 108, // Emergency vehicle(s)
      EMX = 109, // Emergency vehicle warning cleared
      EPD = 110, // Public disturbance
      EPR = 111, // Procession
      EQD = 112, // Earthquake damage
      ERA = 113, // Race meeting
      ESA = 114, // Security alert
      ESH = 115, // Show
      ESI = 116, // Securitu incident
      ESJ = 117, // Show jumping
      ESM = 118, // Several major events
      ESO = 119, // State occasion
      ESP = 120, // Sports meeting
      ESS = 121, // Sightseers obstruction access
      EST = 122, // Strike
      ESX = 123, // Sports traffic cleared
      ESY = 124, // Security alert cleared
      ETN = 125, // Traffic has returned to normal
      ETO = 126, // Tournament
      ETT = 127, // Tennis tournament
      EVA = 128, // Evacuation
      EVC = 129, // Ceremonial event
      EVD = 130, // Demonstration
      EVF = 131, // Festival
      EVM = 132, // Major event
      EVP = 133, // Parade
      EVX = 134, // Exhibition
      EWS = 135, // Water sports meeting
      EWT = 136, // Winter sports meeting
      EXB = 137, // Express lanes blocked
      EXC = 138, // Express lanes closed
      EXS = 139, // Severe exhaust pollution
      FFX = 140, // Fog forecast withdrawn
      FIG = 141, // Grass fire
      FIR = 142, // Serious fire
      FLD = 143, // Flooding
      FLF = 144, // Flash floods
      FLO = 145, // Flow
      FLT = 146, // Fallen trees
      FOD = 147, // Dense fog
      FOF = 148, // Freezing fog
      FOG = 149, // Fog
      FOP = 150, // Patchy fog
      FOX = 151, // Fog cleared
      FPC = 152, // Fallen power cables
      FRH = 153, // Heavy frost
      FRO = 154, // Frost
      FSN = 155, // Ferry service not operating
      FUE = 156, // Oil on road
      FUN = 157, // Funfair
      GAL = 158, // Gales
      GAS = 159, // Gas leak
      GMW = 160, // Gas main work
      GP  = 161, // Gritting vehicle in use
      GUN = 162, // Gunfire on roadway
      HAD = 163, // Damaging hail
      HAI = 164, // Hail
      HAX = 165, // Hazardous load warning cleared
      HAZ = 166, // Vehicle(s) carrying hazardous materials
      HBD = 167, // Broken down heavy lorr(y/ies)
      HLL = 168, // Temporary height limit lifted
      HLT = 169, // Temporary height limit
      HSC = 170, // High-speed chase
      HUR = 171, // Hurricane force winds
      IAV = 172, // Information available
      IBU = 173, // Ice build-up
      ICP = 174, // Icy patches
      IMA = 175, // Almost impassable
      IMP = 176, // Impassable
      INS = 177, // Incident(s)
      IVD = 178, // Individual vehicle data
      LAP = 179, // Carpool lane closed
      LB1 = 180, // One lane blocked
      LB2 = 181, // Two lanes blocked
      LB3 = 182, // Three lanes blocked
      LBA = 183, // Crawler lane blocked
      LBB = 184, // Bus lane blocked
      LBC = 185, // Centre lane(s) blocked
      LBD = 186, // Slow vehicle lane blocked
      LBE = 187, // Emergency lane blocked
      LBH = 188, // Hard shoulder blocked
      LBK = 189, // Lane(s) blocked
      LBL = 190, // Left lane(s) blocked
      LBP = 191, // Carpool lane blocked
      LBR = 192, // Right lane(s) blocked
      LBT = 193, // Turning lane blocked
      LBV = 194, // Heavy vehicle lane blocked
      LBX = 195, // Lane blockages cleared
      LC1 = 196, // One lane closed
      LC2 = 197, // Two lanes closed
      LC3 = 198, // Three lanes closed
      LCA = 199, // Crawler lane closed
      LCB = 200, // Bus lane closed
      LCC = 201, // Centre lane(s) closed
      LCD = 202, // Slow vehicle lane closed
      LCE = 203, // Emergency lane closed
      LCF = 204, // Level crossing failure
      LCH = 205, // Hard shoulder closed
      LCI = 206, // Loose chippings
      LCL = 207, // Left lane(s) closed
      LCP = 208, // Overtaking lane(s) closed
      LCR = 209, // Right lane(s) closed
      LCS = 210, // Lane(s) closed
      LCT = 211, // Turning lane closed
      LCV = 212, // Heavy vehicle lane closed
      LCW = 213, // Level crossing now working normally
      LLB = 214, // Local lanes blocked
      LLC = 215, // Local lanes closed
      LLD = 216, // Long load(s)
      LLL = 217, // Temporary length limit lifted
      LLT = 218, // Temporary length limit
      LO1 = 219, // Carriageway reduced to one lane
      LO2 = 220, // Carriageway reduced to two lanes
      LO3 = 221, // Carriageway reduced to three lanes
      LPB = 222, // Left-hand parallel carriageway blocked
      LPC = 223, // Left-hand parallel carriageway closed
      LPX = 224, // No problems to report
      LRR = 225, // Normal lane regulations restored
      LRS = 226, // Lane closures removed
      LRX = 227, // Lane restrictions lifted
      LS1 = 228, // Stationary traffic
      LS2 = 229, // Queuing traffic
      LS3 = 230, // Slow traffic
      LS4 = 231, // Heavy traffic
      LS5 = 232, // Traffic flowing freely
      LS6 = 233, // Long queues
      LSG = 234, // Low sun glare
      LSL = 235, // Landslips
      LSO = 236, // Traffic problem
      LSR = 237, // Loose sand on road
      LTH = 238, // Traffic heavier than normal
      LTM = 239, // Traffic very much heavier than normal
      LVB = 240, // Overtaking lane(s) blocked
      MAR = 241, // Marathon
      MHR = 242, // Moving hazards on the road
      MIL = 243, // Military convoy(s)
      MKT = 244, // Market
      MSR = 245, // Electronic signs repaired
      MUD = 246, // Mud slide
      MVL = 247, // Motor vehicle restrictions lifted
      NAR = 248, // Next arrival
      NCR = 249, // Normal services resumed
      NDT = 250, // Next departure
      NIA = 251, // No information available
      NLS = 252, // Narrow lanes
      NML = 253, // Null message - completely silent
      NOO = 254, // Emergency telephone number not working
      NRL = 255, // New road layout
      NUL = 256, // Null message with location
      OBR = 257, // X object(s) on roadway
      OCC = 258, // Occupancy
      OCL = 259, // Obstacle clearance
      OHV = 260, // Overheight vehicle(s)
      OHW = 261, // Overheight warning system triggered
      OHX = 262, // Obstruction(s) on the road
      OIL = 263, // Petrol on roadway
      OMV = 264, // Objects falling from moving vehicle
      OPE = 265, // Open
      OSI = 266, // Obstacle signalling
      OWW = 267, // Obstruction warning withdrawn
      PAC = 268, // Passable with care
      PCB = 269, // Parallel carriageway blocked
      PCC = 270, // Parallel carriageway closed
      PEO = 271, // People on roadway
      PFN = 272, // No parking allowed
      PFS = 273, // Only a few spaces available
      PIX = 274, // No parking information available
      PKO = 275, // Car park full
      PKT = 276, // Park and ride trip time
      PMN = 277, // No more parking spaces available
      PMS = 278, // Multi story car parks full
      PRA = 279, // Precipitation on the area
      PRC = 280, // No park and ride information to report
      PRI = 281, // Park and ride information available again
      PRL = 282, // Normal parking restrictions lifted
      PRN = 283, // Park and ride service not operating
      PRO = 284, // Park and ride service operating
      PRS = 285, // Special public transport services operating
      PRV = 286, // Prohibited vehicle(s) on the roadway
      PRX = 287, // No park and ride information
      PSA = 288, // X parking spaces available enough spaces available
      PSL = 289, // Special parking restrictions lifted
      PSN = 290, // Less than X parking spaces available
      PSR = 291, // Special parking restrictions in force
      PSS = 292, // Passable
      PTB = 293, // Police directing traffic via the bus lane
      PTC = 294, // Police directing traffic via the carpool lane
      PTH = 295, // Police directing traffic via the heavy vehicle lane
      PTN = 296, // Public transport services not operating
      PTS = 297, // Public transport strike
      RAC = 298, // Rail crash
      RAD = 299, // Rerouting adviced
      RAF = 300, // Freezing rain
      RAH = 301, // Heavy rain
      RAI = 302, // Rain
      RBA = 303, // Blocked ahead
      RBD = 304, // Bridge demolition work
      RBE = 305, // Entry blocked
      RBI = 306, // Black ice
      RBL = 307, // Blasting work
      RBM = 308, // Bridge maintenance work
      RBX = 309, // Exit blocked
      RC2 = 310, // Road cleared
      RCA = 311, // Closed ahead
      RCB = 312, // Blocked
      RCD = 313, // Closed
      RCE = 314, // Entry slip road(s) closed
      RCI = 315, // Ramp control signals working incorrectly
      RCL = 316, // Road free again
      RCP = 317, // Right-hand parallel carriageway closed
      RCR = 318, // Reopened
      RCT = 319, // No through traffic
      RCW = 320, // Construction work
      RCX = 321, // Road conditions improved
      RDW = 322, // Reckless driver(s)
      REB = 323, // Entry slip road blocked
      REC = 324, // Entry slip road closed
      REL = 325, // Restrictions lifted
      REO = 326, // Entry reopened
      RET = 327, // Restrictions
      REW = 328, // Rescue and recovery work
      REX = 329, // Slip roads closed
      RIC = 330, // Ice
      RIN = 331, // Road closed intermittently
      RIS = 332, // Tram service not operating
      RLS = 333, // Leaves on road
      RLU = 334, // Road layout unchanged
      RMD = 335, // Mud on road
      RMK = 336, // Maintenance work
      RMM = 337, // Maintenance vehicles merging into traffic flow
      RMP = 338, // No motor vehicles
      RMV = 339, // Slow moving maintenance vehicle(s)
      RMW = 340, // Road marking work
      RMX = 341, // Maintenance work cleared, slow moving maintenance
                 // vehicle warnings withdrawn
      RNL = 342, // New roadworks layout
      RNW = 343, // Ramp control signals not working
      ROA = 344, // All carriageways reopened
      ROC = 345, // Rockfalls
      RPB = 346, // Right-hand parallel carriageway blocked
      RPC = 347, // Road surface in poor condition
      RRI = 348, // Intermittent rerouting
      RRU = 349, // Rugby match
      RRW = 350, // Central reservation work
      RSB = 351, // Slip roads blocked
      RSI = 352, // Rail services irregular, delays
      RSL = 353, // Slippery road
      RSN = 354, // Rail services not operating
      RSO = 355, // Slip roads reopened
      RSR = 356, // Slip road restrictions
      RTO = 357, // Public transport services resumed
      RWC = 358, // Roadworks cleared
      RWI = 359, // Wet and icy roads
      RWK = 360, // Roadworks
      RWL = 361, // Long-term roadworks
      RWM = 362, // Major roadworks
      RWR = 363, // Resurfacing work
      RWX = 364, // Roadwork clearance in progress
      RXB = 365, // Exit slip road blocked
      RXC = 366, // Exit slip road closed
      RXD = 367, // Exit slip road(s) closed
      RXO = 368, // Exit reopened
      SAB = 369, // Service area busy
      SAL = 370, // Salting in progress
      SAN = 371, // Sandstorms
      SAO = 372, // Service area overcrowded, drive to another service area
      SAT = 373, // Single alternative line traffic
      SCI = 374, // Civil emergency
      SCN = 375, // Service withdrawn
      SEW = 376, // Sewer overflow
      SEX = 377, // Civil emergency cancelled
      SFB = 378, // Service(s) fully blocked
      SFC = 379, // Service area, fuel station closed
      SFH = 380, // Heavy snowfall
      SFL = 381, // Snowfall
      SFO = 382, // Fuel station reopened
      SHL = 383, // Shed load(s)
      SHX = 384, // Skid hazard reduced
      SLT = 385, // Sleet
      SLU = 386, // Slush
      SM  = 387, // Snow chains mandatory
      SMC = 388, // Closed due to smog alert
      SMG = 389, // Smog alert
      SMO = 390, // Smoke hazard
      SMV = 391, // Spillage occurring from moving vehicle
      SMX = 392, // Smog alert ended
      SN  = 393, // Snowploughs in use
      SND = 393, // Deep snow
      SNF = 394, // Fresh snow
      SNP = 395, // Packed snow
      SNR = 396, // Snow drifts
      SNS = 397, // Service not operating, substitute service available
      SNW = 398, // Lane control signs not working
      SNX = 399, // Snow cleared
      SPC = 400, // Police checkpoint
      SPL = 401, // Spillage on the road
      SPY = 402, // Spray hazard
      SR  = 403, // Snow chains recommended
      SRC = 404, // Service area, restaurant closed
      SRO = 405, // Snow on the road
      SRX = 406, // Restaurant reopened
      SSF = 407, // Free shuttle service operating
      SSM = 408, // Severe smog
      SSO = 409, // Shuttle service operating
      STD = 410, // Storm damage
      STF = 411, // Stud tyres are not authorised
      STI = 412, // Terrorist incident
      STM = 413, // Storm force winds
      STU = 414, // Stud tyres may be used
      SUB = 415, // Subsidence
      SVH = 416, // Slow vehicle(s)
      SWA = 417, // Swarms of insects
      SWC = 418, // Sewer collapse
      SWH = 419, // Surface water hazard
      SWI = 420, // Lane control signs working incorrectly
      SWN = 421, // Lane control signs operating
      SWS = 422, // Showers
      SWT = 423, // Switch your car radio to XX
      TAL = 424, // Temporary axle load limit
      TAX = 425, // Temporary axle weight limit lifted
      TBU = 426, // Traffic building up
      TCC = 427, // Traffic signal control computer not working
      TCN = 428, // Traffic congestion
      TCX = 429, // Traffic problem congestion cleared
      TDX = 430, // Traffic disruption cleared
      TEA = 431, // Traffic easing
      TEX = 432, // Extreme heat
      TFA = 433, // Temperature falling
      TGL = 434, // Temporary weight limit lifted
      TGW = 435, // Temporary gross weight limit
      THU = 436, // Thunderstorms
      TIA = 437, // National traffic
      TIR = 438, // Only major road information provided by this service
      TLI = 439, // Traffic lights working incorrectly
      TLN = 440, // Traffic lighter than normal
      TLO = 441, // Traffic lights not working
      TLT = 442, // Temporary traffic lights
      TLV = 443, // Track-laying vehicle(s)
      TLX = 444, // Less extreme temperatures expected
      TM  = 445, // Snow tyres mandatory
      TMO = 446, // This message is for test purposes only. Please ignore
      TMP = 447, // Current temperature
      TNL = 448, // Normal traffic expected
      TNW = 449, // Temporary traffic lights not working
      TOR = 450, // Tornadoes
      TOX = 451, // Restrictions for high-sided vehicles lifted
      TR  = 452, // Snow tyres recommended
      TRA = 453, // Trade fair
      TRC = 454, // Traffic regulations have been changed
      TRT = 455, // Trip time
      TRX = 456, // Tornado warning ended
      TSC = 457, // Traffic signal timings changed
      TSR = 458, // Traffic signals repaired
      TSX = 459, // No TMC service available
      TTB = 460, // Through traffic lanes blocked
      TTL = 461, // Through traffic lanes closed
      TTM = 462, // Travel time
      TUB = 463, // Tunnel blocked
      TUC = 464, // Tunnel closed
      TVX = 465, // Tunnel ventilation not working
      TWI = 466, // Temportary traffic lights working incorrectly
      TXA = 467, // Information restricted to this area
      TXB = 468, // Active TMC service on this frequency to be resumed
      TXC = 469, // Extreme cold
      TXD = 470, // No detailed regional information provided by this service
      TXL = 471, // Only local information provided by this service
      TXN = 472, // No new traffic information available
      TXO = 473, // No cross-border information provided by this service
      TXP = 474, // No public transport information available
      TXR = 475, // Only regional information provided by this service
      TXS = 476, // TMC broadcast on this frequency will stop
      TXV = 477, // Information about major events is no longer valid
      TXW = 478, // Previous announcement about this or other TMC services
                 // no longer valid
      TXX = 479, // No detailed local information provided by this service
      TXY = 480, // Reference to audio programmes no longer valid
      TXZ = 481, // Reference to other TMC services no longer valid
      UBA = 482, // Use of bus lane allowed under carpool restriction
      UBV = 483, // Use of bus lane allowed for all vehicles
      UCA = 484, // Use of carpool lane allowed for all vehicles
      UGI = 485, // Urgent information will be given on normal program
                 // broadcast
      UHA = 486, // Use of hard shoulder allowed
      UHV = 487, // Use of heavy vehicle lane allowed for all vehicles
      USI = 488, // Underground services irregular
      USN = 489, // Underground service not operating
      VFR = 490, // Vehicle fire(s)
      VIR = 491, // Visibility reduced
      VNW = 492, // Variable message signs not working
      VOD = 493, // Vehicle with overheight load, danger
      VSA = 494, // Vehicles slowing to look at accident(s)
      VSM = 495, // Dangerous slow moving vehicle in the roadway
      VSX = 496, // Visibility improved
      VWC = 497, // Vehicle(s) on wrong carriageway
      VWI = 498, // Variable message signs working incorrectly
      VWN = 499, // Variable message signs operating
      VWX = 500, // Dangerous vehicle warning cleared
      WBC = 501, // Work on buried cables
      WBS = 502, // Work on buried services
      WHI = 503, // White out
      WIC = 504, // Crosswinds
      WIG = 505, // Gusty winds
      WIS = 506, // Strong winds
      WIX = 507, // Strong wind have eased
      WLD = 508, // Wide load(s)
      WLL = 509, // Temporary width limit lifted
      WLT = 510, // Temporary width limit
      WMW = 511, // Water main work
      WR  = 512, // Winter equipment recommended
      WRM = 513, // Winter equipment mandatory
      WST = 514, // Winter storm
      WSX = 515, // Weather situation improved
      WSZ = 516, // Weather expected to improve
      EFX = 517,
      CAS = 518,
      LPR = 519,
      WBW = 520,
      OtherPhrase = 521,
      NoPhrase = 522,

      // Added for DatexII
      // Accidents
      overturnedTrailer = 523,
      headOnCollision = 524,
      headOnOrSideCollision = 525,
      multipleVehicleCollision = 526,
      sideCollision = 527,
      rearCollision = 528,
      vehicleOffRoad = 529,
      accidentInvolvingMopeds = 530,
      accidentInvolvingBicycles = 531,
      accidentInvolvingMotorcycles = 532,
      accidentInvolvingTrain = 533,
      collision = 534,
      collisionWithAnimal = 535,
      collisionWithObstruction = 536,
      collisionWithPerson = 537,
      // Road maintenance
      treeAndVegetationCuttingWork = 538,
      grassCuttingWork = 539,
      overrunningRoadworks = 540,
      repairWork = 541,
      roadsideWork = 542,
      rockFallPreventativeMaintenance = 543,
      installationWork = 544,
      // Network management
      bridgeSwingInOperation = 545,
      intermittentClosures = 546,
      intermittentShortTermClosures = 547,
      laneOrCarriagewayClosed = 548,
      noOvertaking = 549,
      rushHourLaneInOperation = 550,
      tidalFlowLaneInOperation = 551,
      trafficContolInOperation = 552,
      trafficHeld = 553,
      useOfSpecifiedLaneAllowed = 554,
      useSpecifiedLane = 555,
      useUnderSpecifiedRestrictions = 556,
      // Construction work
      demolitionWork = 557,
      // Effect on road layout
      lanesDeviated = 558,
      laneClosures = 559,
      carriagewayClosures = 560,
      // Abnormal traffic
      stopAndGo = 561,
      // Traffic trend
      trafficStable = 562
   };

#if 1
   enum rtm {
      rtm00_1   = 1000,
      rtm00_2   = 1001,
      rtm00_3   = 1002,
      rtm00_4   = 1003,
      rtm00_5   = 1004,
      rtm00_6   = 1005,
      rtm00_7   = 1006,
      rtm00_8   = 1007,
      rtm00_9   = 1008,
      rtm00_10  = 1009,
      rtm00_11  = 1010,
      rtm00_12  = 1011,
      rtm00_13  = 1012,
      rtm01_0   = 1013,
      rtm01_1   = 1014,
      rtm01_2   = 1015,
      rtm01_3   = 1016,
      rtm01_4   = 1017,
      rtm01_5   = 1018,
      rtm01_6   = 1019,
      rtm01_7   = 1020,
      rtm01_8   = 1021,
      rtm01_9   = 1022,
      rtm01_10  = 1023,
      rtm01_11  = 1024,
      rtm01_12  = 1025,
      rtm01_13  = 1026,
      rtm01_14  = 1027,
      rtm01_15  = 1028,
      rtm01_16  = 1029,
      rtm01_17  = 1030,
      rtm01_18  = 1031,
      rtm01_19  = 1032,
      rtm01_20  = 1033,                    
      rtm01_255 = 1034,
      rtm02_0   = 1035,
      rtm02_1   = 1036,
      rtm02_2   = 1037,
      rtm02_3   = 1038,
      rtm02_4   = 1039,
      rtm02_5   = 1040,
      rtm02_6   = 1041,
      rtm02_7   = 1042,
      rtm02_8   = 1043,
      rtm02_9   = 1044,
      rtm02_10  = 1045,
      rtm02_11  = 1046,
      rtm02_12  = 1047,
      rtm02_13  = 1048,
      rtm02_14  = 1049,
      rtm02_15  = 1050,
      rtm02_16  = 1051,
      rtm02_17  = 1052,
      rtm02_18  = 1053,
      rtm02_19  = 1054,
      rtm02_20  = 1055,
      rtm02_21  = 1056,
      rtm02_22  = 1057,
      rtm02_23  = 1058,
      rtm02_255 = 1059,
      rtm03_0   = 1060,
      rtm03_1   = 1061,
      rtm03_2   = 1062,
      rtm03_3   = 1063,
      rtm03_4   = 1064,
      rtm03_5   = 1065,
      rtm03_6   = 1066,
      rtm03_7   = 1067,
      rtm03_8   = 1068,
      rtm03_9   = 1069,
      rtm03_10  = 1070,
      rtm03_11  = 1071,
      rtm03_12  = 1072,
      rtm03_13  = 1073,
      rtm03_14  = 1074,
      rtm03_15  = 1075,
      rtm03_16  = 1076,
      rtm03_17  = 1077,
      rtm03_18  = 1078,
      rtm03_19  = 1079,
      rtm03_20  = 1080,
      rtm03_21  = 1081,
      rtm03_22  = 1082,
      rtm03_255 = 1083,
      rtm04_0   = 1084,
      rtm04_1   = 1085,
      rtm04_2   = 1086,
      rtm04_3   = 1087,
      rtm04_4   = 1088,
      rtm04_5   = 1089,
      rtm04_255 = 1090,
      rtm05_0   = 1091,
      rtm05_1   = 1092,
      rtm05_2   = 1093,
      rtm05_3   = 1094,
      rtm05_4   = 1095,
      rtm05_5   = 1096,
      rtm05_6   = 1097,
      rtm05_7   = 1098,
      rtm05_255 = 1099,
      rtm06_0   = 1100,
      rtm06_1   = 1101,
      rtm06_2   = 1102,
      rtm06_3   = 1103,
      rtm06_4   = 1104,
      rtm06_5   = 1105,
      rtm06_6   = 1106,
      rtm06_7   = 1107,
      rtm06_8   = 1108,
      rtm06_9   = 1109,
      rtm06_10  = 1110,
      rtm06_255 = 1111,
      rtm07_0   = 1112,
      rtm07_1   = 1113,
      rtm07_2   = 1114,
      rtm07_3   = 1115,
      rtm07_4   = 1116,
      rtm07_5   = 1117,
      rtm07_6   = 1118,
      rtm07_7   = 1119,
      rtm07_255 = 1120,
      rtm08_0   = 1121,
      rtm08_1   = 1122,
      rtm08_2   = 1123,
      rtm08_3   = 1124,
      rtm08_4   = 1125,
      rtm08_5   = 1126,
      rtm08_6   = 1127,
      rtm08_7   = 1128,
      rtm08_255 = 1129,
      rtm09_0   = 1130,
      rtm09_1   = 1131,
      rtm09_2   = 1132,
      rtm09_3   = 1133,
      rtm09_4   = 1134,
      rtm09_5   = 1135,
      rtm09_6   = 1136,
      rtm09_255 = 1137,
      rtm10_0   = 1138,
      rtm10_1   = 1139,
      rtm10_2   = 1140,
      rtm10_3   = 1141,
      rtm10_4   = 1142,
      rtm10_5   = 1143,
      rtm10_6   = 1144,
      rtm10_7   = 1145,
      rtm10_8   = 1146,
      rtm10_9   = 1147,
      rtm10_10  = 1148,
      rtm10_11  = 1149,
      rtm10_12  = 1150,
      rtm10_13  = 1151,
      rtm10_14  = 1152,
      rtm10_15  = 1153,
      rtm10_16  = 1154,
      rtm10_17  = 1155,
      rtm10_18  = 1156,
      rtm10_19  = 1157,
      rtm10_20  = 1158,
      rtm10_21  = 1159,
      rtm10_22  = 1160,
      rtm10_23  = 1161,
      rtm10_24  = 1162,
      rtm10_25  = 1163,
      rtm10_26  = 1164,
      rtm10_27  = 1165,
      rtm10_28  = 1166,
      rtm10_29  = 1167,
      rtm10_30  = 1168,
      rtm10_31  = 1169,
      rtm10_32  = 1170,
      rtm10_33  = 1171,
      rtm10_34  = 1172,
      rtm10_35  = 1173,
      rtm10_36  = 1174,
      rtm10_37  = 1175,
      rtm10_38  = 1176,
      rtm10_39  = 1177,
      rtm10_40  = 1178,
      rtm10_41  = 1179,
      rtm10_42  = 1180,
      rtm10_43  = 1181,
      rtm10_44  = 1182,
      rtm10_45  = 1183,
      rtm10_46  = 1184,
      rtm10_47  = 1185,
      rtm10_48  = 1186,
      rtm10_49  = 1187,
      rtm10_50  = 1188,
      rtm10_51  = 1189,
      rtm10_52  = 1190,
      rtm10_53  = 1191,
      rtm10_54  = 1192,
      rtm10_55  = 1193,
      rtm10_56  = 1194,
      rtm10_57  = 1195,
      rtm10_58  = 1196,
      rtm10_59  = 1197,
      rtm10_60  = 1198,
      rtm10_61  = 1199,
      rtm10_62  = 1200,
      rtm10_63  = 1201,
      rtm10_64  = 1202,
      rtm10_65  = 1203,
      rtm10_66  = 1204,
      rtm10_67  = 1205,
      rtm10_68  = 1206,
      rtm10_69  = 1207,
      rtm10_70  = 1208,
      rtm10_71  = 1209,
      rtm10_72  = 1210,
      rtm10_73  = 1211,
      rtm10_74  = 1212,
      rtm10_75  = 1213,
      rtm10_76  = 1214,
      rtm10_77  = 1215,
      rtm10_78  = 1216,
      rtm10_79  = 1217,
      rtm10_80  = 1218,
      rtm10_81  = 1219,
      rtm10_82  = 1220,
      rtm10_83  = 1221,
      rtm10_84  = 1222,
      rtm10_85  = 1223,
      rtm10_86  = 1224,
      rtm10_87  = 1225,
      rtm10_88  = 1226,
      rtm10_89  = 1227,
      rtm10_90  = 1228,
      rtm10_91  = 1229,
      rtm10_92  = 1230,
      rtm10_93  = 1231,
      rtm10_94  = 1232,
      rtm10_95  = 1233,
      rtm10_96  = 1234,
      rtm10_97  = 1235,
      rtm10_98  = 1236,
      rtm10_99  = 1237,
      rtm10_100 = 1238,
      rtm10_101 = 1239,
      rtm10_102 = 1240,
      rtm10_103 = 1241,
      rtm10_104 = 1242,
      rtm10_105 = 1243,
      rtm10_106 = 1244,
      rtm10_255 = 1245,
      rtm11_0   = 1246,
      rtm11_1   = 1247,
      rtm11_2   = 1248,
      rtm11_3   = 1249,
      rtm11_4   = 1250,
      rtm11_5   = 1251,
      rtm11_6   = 1252,
      rtm11_7   = 1253,
      rtm11_8   = 1254,
      rtm11_9   = 1255,
      rtm11_10  = 1256,
      rtm11_11  = 1257,
      rtm11_255 = 1258,
      rtm12_0   = 1259,
      rtm12_1   = 1260,
      rtm12_2   = 1261,
      rtm12_3   = 1262,
      rtm12_4   = 1263,
      rtm12_5   = 1264,
      rtm12_6   = 1265,
      rtm12_7   = 1266,
      rtm12_8   = 1267,
      rtm12_9   = 1268,
      rtm12_10  = 1269,
      rtm12_11  = 1270,
      rtm12_12  = 1271,
      rtm12_13  = 1272,
      rtm12_14  = 1273,
      rtm12_15  = 1274,
      rtm12_16  = 1275,
      rtm12_17  = 1276,
      rtm12_18  = 1277,
      rtm12_255 = 1278,
      rtm13_0   = 1279,
      rtm13_1   = 1280,
      rtm13_2   = 1281,
      rtm13_255 = 1282,
      rtm14_0   = 1283,
      rtm14_1   = 1284,
      rtm14_2   = 1285,
      rtm14_3   = 1286,
      rtm14_4   = 1287,
      rtm14_255 = 1288,
      rtm15_0   = 1289,
      rtm15_1   = 1290,
      rtm15_2   = 1291,
      rtm15_3   = 1292,
      rtm15_4   = 1293,
      rtm15_5   = 1294,
      rtm15_6   = 1295,
      rtm15_7   = 1296,
      rtm15_255 = 1297,
      rtm16_0   = 1298,
      rtm16_1   = 1299,
      rtm16_2   = 1300,
      rtm16_3   = 1301,
      rtm16_4   = 1302,
      rtm16_5   = 1303,
      rtm16_6   = 1304,
      rtm16_7   = 1305,
      rtm16_255 = 1306,
      rtm17_0   = 1307,
      rtm17_1   = 1308,
      rtm17_2   = 1309,
      rtm17_3   = 1310,
      rtm17_4   = 1311,
      rtm17_5   = 1312,
      rtm17_6   = 1313,
      rtm17_7   = 1314,
      rtm17_8   = 1315,
      rtm17_9   = 1316,
      rtm17_10  = 1317,
      rtm17_11  = 1318,
      rtm17_255 = 1319,
      rtm18_0   = 1320,
      rtm18_1   = 1321,
      rtm18_2   = 1322,
      rtm18_3   = 1323,
      rtm18_4   = 1324,
      rtm18_5   = 1325,
      rtm18_6   = 1326,
      rtm18_7   = 1327,
      rtm18_8   = 1328,
      rtm18_9   = 1329,
      rtm18_10  = 1330,
      rtm18_11  = 1331,
      rtm18_255 = 1332,
      rtm19_0   = 1333,
      rtm19_1   = 1334,
      rtm19_2   = 1335,
      rtm19_3   = 1336,
      rtm19_4   = 1337,
      rtm19_5   = 1338,
      rtm19_6   = 1339,
      rtm19_7   = 1340,
      rtm19_8   = 1341,
      rtm19_9   = 1342,
      rtm19_255 = 1343,
      rtm20_0   = 1344,
      rtm20_1   = 1345,
      rtm20_2   = 1346,
      rtm20_3   = 1347,
      rtm20_4   = 1348,
      rtm20_5   = 1349,
      rtm20_6   = 1350,
      rtm20_7   = 1351,
      rtm20_8   = 1352,
      rtm20_9   = 1353,
      rtm20_10  = 1354,
      rtm20_11  = 1355,
      rtm20_12  = 1356,
      rtm20_13  = 1357,
      rtm20_14  = 1358,
      rtm20_255 = 1359,
      rtm21_0   = 1360,
      rtm21_1   = 1361,
      rtm21_2   = 1362,
      rtm21_3   = 1363,
      rtm21_4   = 1364,
      rtm21_5   = 1365,
      rtm21_6   = 1366,
      rtm21_7   = 1367,
      rtm21_8   = 1368,
      rtm21_9   = 1369,
      rtm21_10  = 1370,
      rtm21_11  = 1371,
      rtm21_12  = 1372,
      rtm21_13  = 1373,
      rtm21_14  = 1374,
      rtm21_255 = 1375,
      rtm22_0   = 1376,
      rtm22_1   = 1377,
      rtm22_2   = 1378,
      rtm22_3   = 1379,
      rtm22_4   = 1380,
      rtm22_255 = 1381,
      rtm23_0   = 1382,
      rtm23_1   = 1383,
      rtm23_2   = 1384,
      rtm23_3   = 1385,
      rtm23_4   = 1386,
      rtm23_5   = 1387,
      rtm23_6   = 1388,
      rtm23_7   = 1389,
      rtm23_8   = 1390,
      rtm23_9   = 1391,
      rtm23_255 = 1392,
      rtm24_0   = 1393,
      rtm24_1   = 1394,
      rtm24_2   = 1395,
      rtm24_3   = 1396,
      rtm24_4   = 1397,
      rtm24_5   = 1398,
      rtm24_6   = 1399,
      rtm24_255 = 1400,
      rtm25_0   = 1401,
      rtm25_1   = 1402,
      rtm25_2   = 1403,
      rtm25_3   = 1404,
      rtm25_4   = 1405,
      rtm25_5   = 1406,
      rtm25_6   = 1407,
      rtm25_7   = 1408,
      rtm25_8   = 1409,
      rtm25_255 = 1410,
      rtm26_0   = 1411,
      rtm26_1   = 1412,
      rtm26_2   = 1413,
      rtm26_3   = 1414,
      rtm26_4   = 1415,
      rtm26_5   = 1416,
      rtm26_6   = 1417,
      rtm26_7   = 1418,
      rtm26_255 = 1419,
      rtm27_0   = 1420,
      rtm27_1   = 1421,
      rtm27_2   = 1422,
      rtm27_3   = 1423,
      rtm27_255 = 1424,
      rtm28_0   = 1425,
      rtm28_1   = 1426,
      rtm28_2   = 1427,
      rtm28_3   = 1428,
      rtm28_4   = 1429,
      rtm28_5   = 1430,
      rtm28_6   = 1431,
      rtm28_7   = 1432,
      rtm28_8   = 1433,
      rtm28_9   = 1434,
      rtm28_10  = 1435,
      rtm28_255 = 1436,
      rtm29_0   = 1437,
      rtm29_1   = 1438,
      rtm29_2   = 1439,
      rtm29_3   = 1440,
      rtm29_4   = 1441,
      rtm29_5   = 1442,
      rtm29_255 = 1443,
      rtm30_0   = 1444,
      rtm30_1   = 1445,
      rtm30_2   = 1446,
      rtm30_3   = 1447,
      rtm30_4   = 1448,
      rtm30_255 = 1449,
      rtm31_0   = 1450,
      rtm31_1   = 1451,
      rtm31_2   = 1452,
      rtm31_3   = 1453,
      rtm31_4   = 1454,
      rtm31_5   = 1455,
      rtm31_255 = 1456,
      rtm32_0   = 1457,
      rtm32_1   = 1458,
      rtm32_2   = 1459,
      rtm32_3   = 1460,
      rtm32_4   = 1461,
      rtm32_5   = 1462,
      rtm32_255 = 1463,
      rtm33_0   = 1464,
      rtm33_1   = 1465,
      rtm33_2   = 1466,
      rtm33_3   = 1467,
      rtm33_4   = 1468,
      rtm33_5   = 1469,
      rtm33_6   = 1470,
      rtm33_255 = 1471,
      rtm34_0   = 1472,
      rtm34_1   = 1473,
      rtm34_2   = 1474,
      rtm34_3   = 1475,
      rtm34_4   = 1476,
      rtm34_5   = 1477,
      rtm34_6   = 1478,
      rtm34_255 = 1479,
      rtm35_0   = 1480,
      rtm35_1   = 1481,
      rtm35_2   = 1482,
      rtm35_3   = 1483,
      rtm35_4   = 1484,
      rtm35_5   = 1485,
      rtm35_6   = 1486,
      rtm35_7   = 1487,
      rtm35_8   = 1488,
      rtm35_255 = 1489,
      rtm36_0   = 1490,
      rtm36_1   = 1491,
      rtm36_2   = 1492,
      rtm36_3   = 1493,
      rtm36_4   = 1494,
      rtm36_5   = 1495,
      rtm36_6   = 1496,
      rtm36_7   = 1497,
      rtm36_8   = 1498,
      rtm36_9   = 1499,
      rtm36_10  = 1500,
      rtm36_11  = 1501,
      rtm36_12  = 1502,
      rtm36_13  = 1503,
      rtm36_14  = 1504,
      rtm36_15  = 1505,
      rtm36_16  = 1506,
      rtm36_17  = 1507,
      rtm36_18  = 1508,
      rtm36_255 = 1509,
      rtm37_0   = 1510,
      rtm37_1   = 1511,
      rtm37_2   = 1512,
      rtm37_3   = 1513,
      rtm37_4   = 1514,
      rtm37_5   = 1515,
      rtm37_6   = 1516,
      rtm37_7   = 1517,
      rtm37_8   = 1518,
      rtm37_9   = 1519,
      rtm37_10  = 1520,
      rtm37_11  = 1521,
      rtm37_12  = 1522,
      rtm37_13  = 1523,
      rtm37_14  = 1524,
      rtm37_15  = 1525,
      rtm37_16  = 1526,
      rtm37_17  = 1527,
      rtm37_18  = 1528,
      rtm37_19  = 1529,
      rtm37_20  = 1530,
      rtm37_21  = 1531,
      rtm37_22  = 1532,
      rtm37_255 = 1533,
      rtm38_0   = 1534,
      rtm38_1   = 1535,
      rtm38_2   = 1536,
      rtm38_3   = 1537,
      rtm38_4   = 1538,
      rtm38_5   = 1539,
      rtm38_6   = 1540,
      rtm38_7   = 1541,
      rtm38_8   = 1542,
      rtm38_9   = 1543,
      rtm38_10  = 1544,
      rtm38_11  = 1545,
      rtm38_12  = 1546,
      rtm38_255 = 1547,
      rtm39_0   = 1548,
      rtm39_1   = 1549,
      rtm39_2   = 1550,
      rtm39_3   = 1551,
      rtm39_4   = 1552,
      rtm39_5   = 1553,
      rtm39_6   = 1554,
      rtm39_7   = 1555,
      rtm39_8   = 1556,
      rtm39_9   = 1557,
      rtm39_10  = 1558,
      rtm39_11  = 1559,
      rtm39_12  = 1560,
      rtm39_13  = 1561,
      rtm39_14  = 1562,
      rtm39_15  = 1563,
      rtm39_16  = 1564,
      rtm39_17  = 1565,
      rtm39_18  = 1566,
      rtm39_19  = 1567,
      rtm39_20  = 1568,
      rtm39_21  = 1569,
      rtm39_22  = 1570,
      rtm39_23  = 1571,
      rtm39_255 = 1572,
      rtm40_0   = 1573,
      rtm40_1   = 1574,
      rtm40_2   = 1575,
      rtm40_3   = 1576,
      rtm40_4   = 1577,
      rtm40_5   = 1578,
      rtm40_6   = 1579,
      rtm40_7   = 1580,
      rtm40_8   = 1581,
      rtm40_9   = 1582,
      rtm40_10  = 1583,
      rtm40_11  = 1584,
      rtm40_12  = 1585,
      rtm40_13  = 1586,
      rtm40_14  = 1587,
      rtm40_15  = 1588,
      rtm40_16  = 1589,
      rtm40_17  = 1590,
      rtm40_18  = 1591,
      rtm40_19  = 1592,
      rtm40_20  = 1593,
      rtm40_255 = 1594,
      rtm41_0   = 1595,
      rtm41_1   = 1596,
      rtm41_2   = 1597,
      rtm41_3   = 1598,
      rtm41_4   = 1599,
      rtm41_5   = 1600,
      rtm41_6   = 1601,
      rtm41_7   = 1602,
      rtm41_8   = 1603,
      rtm41_9   = 1604,
      rtm41_10  = 1605,
      rtm41_11  = 1606,
      rtm41_12  = 1607,
      rtm41_13  = 1608,
      rtm41_14  = 1609,
      rtm41_15  = 1610,
      rtm41_16  = 1611,
      rtm41_17  = 1612,
      rtm41_18  = 1613,
      rtm41_255 = 1614,
      rtm42_0   = 1615,
      rtm42_1   = 1616,
      rtm42_2   = 1617,
      rtm42_3   = 1618,
      rtm42_4   = 1619,
      rtm42_5   = 1620,
      rtm42_6   = 1621,
      rtm42_7   = 1622,
      rtm42_8   = 1623,
      rtm42_9   = 1624,
      rtm42_10  = 1625,
      rtm42_11  = 1626,
      rtm42_12  = 1627,
      rtm42_13  = 1628,
      rtm42_14  = 1629,
      rtm42_255 = 1630,
      rtm43_0   = 1631,
      rtm43_1   = 1632,
      rtm43_2   = 1633,
      rtm43_3   = 1634,
      rtm43_4   = 1635,
      rtm43_5   = 1636,
      rtm43_6   = 1637,
      rtm43_7   = 1638,
      rtm43_8   = 1639,
      rtm43_9   = 1640,
      rtm43_10  = 1641,
      rtm43_11  = 1642,
      rtm43_12  = 1643,
      rtm43_255 = 1644,
      rtm44_0   = 1645,
      rtm44_1   = 1646,
      rtm44_2   = 1647,
      rtm44_3   = 1648,
      rtm44_4   = 1649,
      rtm44_5   = 1650,
      rtm44_6   = 1651,
      rtm44_7   = 1652,
      rtm44_8   = 1653,
      rtm44_9   = 1654,
      rtm44_10  = 1655,
      rtm44_11  = 1656,
      rtm44_12  = 1657,
      rtm44_13  = 1658,
      rtm44_14  = 1659,
      rtm44_15  = 1660,
      rtm44_16  = 1661,
      rtm44_255 = 1662,
      rtm45_0   = 1663,
      rtm45_1   = 1664,
      rtm45_2   = 1665,
      rtm45_3   = 1666,
      rtm45_4   = 1667,
      rtm45_5   = 1668,
      rtm45_6   = 1669,
      rtm45_7   = 1670,
      rtm45_8   = 1671,
      rtm45_9   = 1672,
      rtm45_10  = 1673,
      rtm45_11  = 1674,
      rtm45_12  = 1675,
      rtm45_13  = 1676,
      rtm45_14  = 1677,
      rtm45_15  = 1678,
      rtm45_16  = 1679,
      rtm45_17  = 1680,
      rtm45_18  = 1681,
      rtm45_19  = 1682,
      rtm45_20  = 1683,
      rtm45_21  = 1684,
      rtm45_22  = 1685,
      rtm45_23  = 1686,
      rtm45_24  = 1687,
      rtm45_25  = 1688,
      rtm45_26  = 1689,
      rtm45_27  = 1690,
      rtm45_28  = 1691,
      rtm45_29  = 1692,
      rtm45_30  = 1693,
      rtm45_31  = 1694,
      rtm45_32  = 1695,
      rtm45_33  = 1696,
      rtm45_34  = 1697,
      rtm45_35  = 1698,
      rtm45_36  = 1699,
      rtm45_37  = 1700,
      rtm45_38  = 1701,
      rtm45_39  = 1702,
      rtm45_40  = 1703,
      rtm45_255 = 1704,
      rtm46_0   = 1705,
      rtm46_1   = 1706,
      rtm46_255 = 1707,
      rtm47_0   = 1708,
      rtm47_1   = 1709,
      rtm47_2   = 1710,
      rtm47_3   = 1711,
      rtm47_4   = 1712,
      rtm47_5   = 1713,
      rtm47_6   = 1714,
      rtm47_7   = 1715,
      rtm47_8   = 1716,
      rtm47_9   = 1717,
      rtm47_10  = 1718,
      rtm47_11  = 1719,
      rtm47_12  = 1720,
      rtm47_13  = 1721,
      rtm47_14  = 1722,
      rtm47_15  = 1723,
      rtm47_16  = 1724,
      rtm47_17  = 1725,
      rtm47_18  = 1726,
      rtm47_255 = 1727,
      rtm48_0   = 1728,
      rtm48_1   = 1729,
      rtm48_2   = 1730,
      rtm48_3   = 1731,
      rtm48_4   = 1732,
      rtm48_5   = 1733,
      rtm48_6   = 1734,
      rtm48_7   = 1735,
      rtm48_8   = 1736,
      rtm48_9   = 1737,
      rtm48_255 = 1738,
      rtm49_0   = 1739,
      rtm49_1   = 1740,
      rtm49_2   = 1741,
      rtm49_3   = 1742,
      rtm49_4   = 1743,
      rtm49_5   = 1744,
      rtm49_6   = 1745,
      rtm49_7   = 1746,
      rtm49_8   = 1747,
      rtm49_9   = 1748,
      rtm49_10  = 1749,
      rtm49_11  = 1750,
      rtm49_12  = 1751,
      rtm49_255 = 1752,
      rtm50_0   = 1753,
      rtm50_1   = 1754,
      rtm50_2   = 1755,
      rtm50_3   = 1756,
      rtm50_4   = 1757,
      rtm50_5   = 1758,
      rtm50_6   = 1759,
      rtm50_7   = 1760,
      rtm50_8   = 1761,
      rtm50_9   = 1762,
      rtm50_10  = 1763,
      rtm50_11  = 1764,
      rtm50_12  = 1765,
      rtm50_13  = 1766,
      rtm50_14  = 1767,
      rtm50_15  = 1768,
      rtm50_16  = 1769,
      rtm50_17  = 1770,
      rtm50_18  = 1771,
      rtm50_19  = 1772,
      rtm50_255 = 1773,
   };
#endif
      
   enum severity {
      Blocked = 0,                           // MAX_UINT32
      Closed = 1,                            // MAX_UINT32
      StationaryTraffic = 2,                 //  20000
      QueuingTraffic = 3,                    //   5000
      SlowTraffic = 4,                       //   3000
      HeavyTraffic = 5,                      //   1333
      TrafficFlowingFreely = 6,              //   1150
      LongQueues = 7,                        //  10000     ?
      TrafficCongestion = 8,                 //   2005
      TrafficBuildingUp = 9,                 //   1300     ?
      TrafficHeavierThanNormal = 10,         //   1334     ?
      TrafficVeryMuchHeavierThanNormal = 11, //   2000
      TrafficEasing = 12,                    //   1115     ?
      TrafficLighterThanNormal = 13,         //   1015
      Reroute = 14,                          // MAX_UINT32 ?
      NotProvided = 15,                      //   1010
      NoSeverity = 16,                       //   1001
      AlmostBlocked = 17,                    // 100000 
      // Newly added severity
      AlternatingContraflow = 19,            //   4000
      MiscRoadworks = 20,                    //   1025
   };

   enum severity_factor {
      Unknown     = 0,
      VerySlight  = 1,                        //   ??
      Slight      = 2,                        //   ??
      Medium      = 3,                        //   ??
      Severe      = 4,                        //   ?? 
      VerySevere  = 5,                        //   ?? 
      Unspecified = 6,
   };

   enum direction {
      Positive = 0,
      Negative = 1,
      BothDirections = 2,
      NoDirection = 3
   };

   enum country {
      Sweden = 0,
      Germany = 1,
      Netherlands = 2,
      Belgium = 3,
      Spain = 4,
      Switzerland = 5,
      Austria = 6,
      NoCountry = 7
   };
   static const char* getPhraseText( TrafficDataTypes::phrase phrase );
   inline static uint32 getCostFactorFromSeverity(severity sev);

   inline static float getCostFactorFromSeverityFactor(severity_factor sevFact);   
};
   
inline uint32
TrafficDataTypes::getCostFactorFromSeverity(TrafficDataTypes::severity sev){
   uint32 costFactor = 1010;
   switch( sev ) {
      case TrafficDataTypes::Blocked : {
         costFactor = MAX_UINT32;
         break;
      }
      case TrafficDataTypes::Closed : {
         costFactor = MAX_UINT32;
         break;
      }
      case TrafficDataTypes::StationaryTraffic : {
         costFactor = 20000;
         break;
      }
      case TrafficDataTypes::QueuingTraffic : {
         costFactor = 5000;
         break;
      }
      case TrafficDataTypes::SlowTraffic : {
         costFactor = 3000;
         break;
      }
      case TrafficDataTypes::HeavyTraffic : {
         costFactor = 1333;
         break;
      }
      case TrafficDataTypes::TrafficFlowingFreely : {
         costFactor = 1150;
         break;
      }
      case TrafficDataTypes::LongQueues : {
         costFactor = 10000;
         break;
      }
      case TrafficDataTypes::TrafficCongestion : {
         costFactor = 2005;
         break;
      }
      case TrafficDataTypes::TrafficBuildingUp : {
         costFactor = 1300;
         break;
      }
      case TrafficDataTypes::TrafficHeavierThanNormal : {
         costFactor = 1334;
         break;
      }
      case TrafficDataTypes::TrafficVeryMuchHeavierThanNormal : {
         costFactor = 2000;
         break;
      }
      case TrafficDataTypes::TrafficEasing : {
         costFactor = 1115;
         break;
      }
      case TrafficDataTypes::TrafficLighterThanNormal : {
         costFactor = 1015;
         break;
      }
      case TrafficDataTypes::Reroute : {
         costFactor = MAX_UINT32;
         break;
      }
      case TrafficDataTypes::NotProvided : {
         costFactor = 1010;
         break;
      }
      case TrafficDataTypes::NoSeverity : {
         costFactor = 1001;
         break;
      }
      case TrafficDataTypes::AlmostBlocked : {
         costFactor = 100000;
         break;
      }
      case TrafficDataTypes::AlternatingContraflow : {
         costFactor = 4000;
         break;
      }
      case TrafficDataTypes::MiscRoadworks : {
         costFactor = 1025;
         break;
      }
      default : {
         costFactor = 1012;
         break;
      }
   }
   return costFactor;
   
}

inline float 
TrafficDataTypes::getCostFactorFromSeverityFactor(TrafficDataTypes::severity_factor sevFact){
   float costFactor = 1.0;
   switch(sevFact) {
      case TrafficDataTypes::VerySlight : {
         costFactor = 1.0;
         break;
      }
      case TrafficDataTypes::Slight : {
         costFactor = 1.1;
         break;
      }
      case TrafficDataTypes::Medium : {
         costFactor = 1.2;
         break;
      }
      case TrafficDataTypes::Severe : {
         costFactor = 1.3;
         break;
      }              
      case TrafficDataTypes::VerySevere : {   
         costFactor = 1.4;
         break;
      }
      case TrafficDataTypes::Unknown :
      case TrafficDataTypes::Unspecified: {
         costFactor = 1.0;
         break;
      }
   }
   return costFactor;
}


#endif
