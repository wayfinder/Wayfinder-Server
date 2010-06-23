/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef IMAGETABLE_H
#define IMAGETABLE_H

#include "config.h"
#include "MC2String.h"

namespace ImageTable {

/// Defines the various image sets available.
enum ImageSet {
   DEFAULT = 0,
   IPHONE = 2,
   NBR_IMAGE_SETS
};

/// The images.
enum ImageCode {
   NOIMAGE = 0,
   ROUTE_ORIG = 1,
   ROUTE_DEST = 2,
   PARK_AND_WALK = 3,
   CITY_CENTRE_SQUARE = 4,
   CITY_CENTRE = 5,
   CITY_CENTRE_SMALL_RED = 6,
   CITY_CENTRE_SMALL_PINK = 7,
   CITY_CENTRE_POINT = 8,
   CITY_CENTRE_POINT_SMALL = 9,
   ATM = 10,
   GOLF_COURSE = 11,
   ICE_SKATING_RINK = 12,
   MARINA = 13,
   CAR_REPAIR = 14,
   BOWLING_CENTRE = 15,
   BANK = 16,
   CASINO = 17,
   CITYHALL = 18,
   COMMUTER_RAIL_STATION = 19,
   SUBWAY_STATION = 20,
   COURT_HOUSE = 21,
   HISTORICAL_MONUMENT = 22,
   MUSEUM = 23,
   NIGHTLIFE = 24,
   CHURCH_CHRISTIAN = 25,
   CHURCH_MOSQUE = 26,
   CHURCH_SYNAGOGUE = 27,
   HINDU_TEMPLE = 28,
   BUDDHIST_TEMPLE = 29,
   POST_OFFICE = 30,
   RECREATION_FACILITY = 31,
   RENTACAR_FACILITY = 32,
   REST_AREA = 33,
   SKI_RESORT = 34,
   SPORTS_ACTIVITY = 35,
   SHOP = 36,
   THEATRE = 37,
   TOURIST_ATTRACTION = 38,
   UNIVERSITY = 39,
   WINERY = 40,
   PARKING_GARAGE = 41,
   PARK_AND_RIDE = 42,
   OPEN_PARKING_AREA = 43,
   AMUSEMENT_PARK = 44,
   LIBRARY = 45,
   SCHOOL = 46,
   GROCERY_STORE = 47,
   PETROL_STATION = 48,
   TRAMWAY = 49,
   FERRY_TERMINAL = 50,
   CINEMA = 51,
   BUS_STATION = 52,
   RAILWAY_STATION = 53,
   AIRPORT = 54,
   RESTAURANT = 55,
   CAFE = 56,
   HOTEL = 57,
   TOURIST_OFFICE = 58,
   POLICE_STATION = 59,
   HOSPITAL = 60,
   TOLL_ROAD = 61,
   WIFI_HOTSPOT = 62,
   CHEAP_PETROL_STATION = 63,
   TURKISH_HOSPITAL = 64,
   TRAFFIC_INFORMATION = 65,
   ROAD_WORK = 66,
   SPEED_CAMERA = 67,
   USER_DEFINED_SPEED_CAMERA = 68,
   SPEED_TRAP = 69,
   COMMUTER_TRAIN = 70,
   UNKNOWN = 71,
   CAR_RIGHT_U = 72,
   CAR_RIGHT = 73,
   CAR_LEFT_U = 74,
   CAR_LEFT = 75,
   MAN = 76,
   BIKE_RIGHT = 77,
   BIKE_LEFT = 78,
   MAP_PIN_CENTERED = 79,

   CAT_AIRPORT = 80,
   CAT_ATM = 81,
   CAT_BANK = 82,
   CAT_CARDEALER = 83,
   CAT_CINEMA = 84,
   CAT_CITYCENTRE = 85,
   CAT_DOCTOR = 86,
   CAT_FERRY_TERMINAL = 87,
   CAT_GOLF_COURSE = 88,
   CAT_HOTEL = 89,
   CAT_COMMUTER_RAIL_STATION = 90,
   CAT_HISTORICAL_MONUMENT = 91,
   CAT_MUSEUM = 92,
   CAT_MUSIC_STORE = 93,
   CAT_NIGHT_LIFE = 94,
   CAT_OPEN_PARKING_AREA = 95,
   CAT_PARKING_GARAGE = 96,
   CAT_PETROL_STATION = 97,
   CAT_PHARMACY = 98,
   CAT_POLICE_STATION = 99,
   CAT_POST_OFFICE = 100,
   CAT_RENTACAR_FACILITY = 101,
   CAT_RESTAURANT = 102,
   CAT_GROCERY_STORE = 103,
   CAT_SKI_RESORT = 104,
   CAT_THEATRE = 105,
   CAT_TOURIST_OFFICE = 106,
   CAT_RAILWAY_STATION = 107,
   CAT_CAR_REPAIR = 108,
   CAT_BARS = 109,
   CAT_CAFE = 110,
   CAT_HOSPITAL = 111,
   CAT_NIGHT_CLUB = 112,
   CAT_PARKING = 113,
   CAT_SUBWAY_STATION = 114,
   CAT_SHOPPING_CENTRE = 115,
   CAT_TAXI = 116,
   CAT_WLAN = 117,
   CAT_BUS_STATION = 118,
   CAT_CASINO = 119,
   CAT_AMUSEMENTPARK = 120,
   CAT_MOSQUE = 121,
   CAT_CHURCH = 122,
   CAT_SYNAGOGUE = 123,
   CAT_TOURIST_ATTRACTION = 124,
   CAT_TRAM_STATION = 125,
   CAT_SHOP = 126,
   CAT_GENERIC = 127,

   SEARCH_HEADING_PLACES = 128,
   SEARCH_HEADING_ADDRESSES = 129,

   NBR_IMAGES
};

/**
 * Gets the image name for an image.
 *
 * If there is no specific name for the image, we will try the DEFAULT
 * imageSet.
 *
 * @param  imageCode   The image.
 * @param  imageSet    The image set.
 * @return The image name as a string, add extension to get a filename.
 */
const char* getImage( ImageCode imageCode, ImageSet imageSet = DEFAULT );

/**
 * Converts a string representation of an image set to an
 * ImageTable::ImageSet. The string representation is used in
 * navclientsettings.
 *
 * @param imageSetStr The string representation, e.g. "iphone".
 * @return The image set, e.g. DEFAULT.
 */
ImageSet getImageSetFromString( const MC2String& imageSetStr );

/**
 * Converts an ImageTable::ImageSet to a string representation which
 * can be used for debugging/logging purposes.
 *
 * @param imageSet The image set to convert.
 * @return The string representation.
 */
MC2String imageSetToString( ImageSet imageSet );

} // namespace ImageTable

#endif // IMAGETABLE_H
