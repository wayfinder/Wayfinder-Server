/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef QYPEHANDLER_H
#define QYPEHANDLER_H

#include "config.h"

class URLFetcher;
struct QypeReviewInfo;
struct QypeImageInfo;
class VanillaMatch;
class VanillaCompanyMatch;
class POIReview;

#include "ExtServiceTalker.h"
#include "NotCopyable.h"
#include <map>
#include "XMLTool.h"
#include "XPathMatchHandler.h"
#include "ItemInfoEnums.h"
#include "CategoryTreeUtils.h"

namespace XMLTool {
namespace XPath {
class MultiExpression;
class Expression;
}
}

/// information about review link
struct ReviewLink { 
   uint32 m_count; //< Nbr of reviews
   MC2String m_lang; //< The language
   MC2String m_href;   //< The link to the reviews
};

/// information about assets link
struct AssetsLink {
   uint32 m_count; //< Nbr images
   MC2String m_href; //< The link to the images
};

/// information about Qype link
struct QypeLink { 
   MC2String m_lang; //< The language
   MC2String m_href;   //< The link to the Qype page
};

/// information about a reviewer
struct Reviewer{
   MC2String userName;
};

/// data extracted from the qype interface
struct QypePlaceInfo {
   MC2String m_id;
   MC2String m_title;
   MC2String m_street;
   MC2String m_houseNumber;
   MC2String m_city;
   MC2String m_postcode;
   MC2String m_countryCode;
   MC2String m_qypeCategoryId;
   MC2String m_url;
   MC2String m_phone;
   MC2String m_openingHours;
   MC2String m_avgRating;
   vector< ReviewLink > m_reviewLinks;
   vector< AssetsLink > m_imageLinks;
   vector< QypeLink > m_qypeLinks;
   MC2String m_coord;
   MC2String m_distance;

   struct{
      MC2String m_small;
      MC2String m_medium;
      MC2String m_large;
   }image;
};

/**
 * Class for handling the Qype provider.
 */
class QypeHandler {
private:
   /// Ids for the Qype categories
   enum MC2Categories {
      // Airport
      MC2Airport = 18,
      // Bank and atm
      MC2BankAndAtm = 272, MC2ATM = 152,
      // Attractions
      MC2AmusementPark = 54, MC2ArtGalleries = 30, MC2Museum = 22, 
      MC2Stadium = 36, MC2Theatres = 88,
      // Cafe
      MC2Cafe = 86,
      // Cinemas
      MC2Cinema = 98,
      // Hotels
      MC2Hotel = 118,
      // Health and Medical
      MC2HealthAndMedical = 111, MC2Dentist = 249, MC2Hospital = 246, 
      MC2Pharmacy = 245,
      // Nightlife
      MC2Nightlife = 278, MC2Bars = 100, MC2Pubs = 232, MC2Casinos = 99,
      MC2NightClub = 90,
      // Parking
      MC2Parking = 267,
      // Petrol Stations
      MC2PetrolStations = 103,
      // Post Office
      MC2PostOffice = 107,
      // Public Transport
      MC2BusStation = 126, MC2SubwayStation = 121, MC2TramStop = 124,
      // Restaurants
      MC2Restaurants = 85, MC2Bistro = 216, MC2British = 214, MC2Chinese = 208, 
      MC2FastFood = 202, MC2French = 198, MC2German = 197, MC2Greek = 196, 
      MC2Indian = 192, MC2Italian = 190, MC2Japanese = 189, MC2Pizzeria = 175, 
      MC2Portugese = 173, MC2SeaFood = 168, MC2Spanish = 165, 
      MC2SteakHouse = 164, MC2Thai = 159, MC2Turkish = 158,
      // Shopping
      MC2Shopping = 9, MC2Bookshops = 78, MC2ClotingAndAccessories = 75,
      MC2ConvenienceStores = 74, MC2Florists = 70, MC2FoodAndDrink = 69,
      MC2GiftsAndSouvenirs = 68, MC2GroceryStore = 67, MC2JewelryAndWatches = 64,
      MC2NewsAgentsAndTobacconists = 62, MC2Opticians = 61, 
      MC2ShoppingCentre = 59, MC2SportShops = 58, MC2Toys = 56,
   };

   enum QypeCategories {
      // Airport
      Airport = 372,
      // Bank and atm
      Bank = 55, ATM = 688,
      // Attractions
      AmusementPark = 507, ArtGalleries = 170, Museum = 27, Stadium = 275,
      Theatres = 102,
      // Cafe
      Cafe = 322,
      // Cinemas
      Cinema = 24,
      // Hotels
      Hotel = 69,
      // Health and Medical
      HealthAndMedical = 32, Dentist = 36, Hospital = 171, Pharmacy = 109,
      // Nightlife
      Nightlife = 2, BarsAndPubs = 609, Bars = 22, Pubs = 21, Casinos = 178,
      NightClub = 23,
      // Parking
      Parking = 354,
      // Petrol Stations
      PetrolStations = 71,
      // Post Office
      PostOffice = 489,
      // Public Transport
      BusStation = 547, MetroStation = 751, UndergroundStation = 753, 
      TramStop = 870,
      // Restaurants
      Restaurants = 1, Bistro = 149, British = 225, Chinese = 19, FastFood = 42,
      French = 136, German = 83, Greek = 18, Indian = 122, Japanese = 49, 
      ItalianAndPizzeria = 16, Portugese = 134, SeaFood = 95, 
      Spanish = 205, SteakHouse = 101, Thai = 59, Turkish = 445,
      // Shopping
      Shopping = 3, Bookshops = 80, ClotingAndAccessories = 28,
      ConvenienceStores = 576, Florists = 77, FoodAndDrink = 350,
      GiftsAndSouvenirs = 330, GroceryStore = 878, JewelryAndWatches = 108,
      Tobacconists = 263, NewsAgents = 204, Opticians = 94, 
      ShoppingCentre = 191, SportShops = 99, Toys = 78,
   };

public:
   /**
    * Constructor for the QypeTalker class.
    */
   QypeHandler();

   /**
    * Destructor for the QypeTalker class.
    */
   virtual ~QypeHandler();

   /**
    * Implementation of the doQuery function in the ionterface.
    */
   int doQueryImpl( SearchReplyData& reply, 
                    const ExternalSearchRequestData& searchData,
                    int nbrRetries = 3 );
   
   /**
    * Implementation of the doInfoQuery function in the ionterface.
    */
   int doInfoQueryImpl( ExtInfoQuery& info );
   
   /**
    * Send the request to the Qype API
    *
    * @param qype_xml Response from Qype.
    * @param url Request sent to Qype.
    */
   int sendRequest( MC2String& qype_xml,
                    const MC2String& url );


   /**
    * Converts qype category id to mc2 category id.
    *
    * @param categoryId Qype category id.
    * @return MC2 category id.
    */
   static CategoryTreeUtils::CategoryID QypeCategoryToMC2Category( 
      CategoryTreeUtils::CategoryID categoryId );

    /**
    * Creates a url request for the qype API.
    *
    * @param searchData Data for the search
    * @return A qype request url.
    */
   MC2String createURL( const ExternalSearchRequestData& searchData );

    /**
    * Parses XML result data
    *
    * @param reply Reply sent back to the caller.
    * @param qype_xml The XML buffer to parse.
    * @param lang The language used by the user.
    * @param url The Qype query URL used to produce the XML buffer.
    * @param filterLevel Filter level of the call.
    * @param poiInfoRequest true if a poi info request, else false.
    */
   int parseResult( SearchReplyData& reply,
                    const MC2String& qype_xml,
                    const LangType& lang,
                    const MC2String& url,
                    ItemInfoEnums::InfoTypeFilter filterLevel,
                    bool poiInfoRequest = false);

   /**
    * Creates vanilla matches from the parsed result.
    * @param finalMatch The final result.
    * @param matches The parsed info from XML file.
    * @param lang The language used by the user.
    * @param id Id to be set in the result.
    * @param filterLevel Filter level of the call.
    * @param poiInfoRequest true if a poi info request, else false.
    */
   void createSearchMatches( vector<VanillaMatch*>& finalMatches,
                             const vector<QypePlaceInfo>& matches,
                             const LangType& lang,
                             const MC2String& id,
                             ItemInfoEnums::InfoTypeFilter filterLevel,
                             bool poiInfoRequest );

   /**
    * Returns the consumer key for Qype.
    * @return The consumer key.
    */
   const MC2String& getConsumerKey() { return m_consumerKey; }

   /**
    * Returns the Qype base url.
    * @param The Qype url.
    */
   const MC2String& getQypeUrl() { return m_qypeUrl; }

   /**
    * Cache for the locator ids.
    * Public due to needed by the unit test code.
    */
   std::map< MC2String, MC2String> m_locatorCache;
private:
  
   /**
    * Converts mc2 category id to qype category id.
    *
    * @param categoryId MC2 category id.
    * @return Qype category id.
    */
   static CategoryTreeUtils::CategoryID MC2CategoryToQypeCategory( 
      CategoryTreeUtils::CategoryID categoryId );

   /**
    * Returns the locator id in Qype for a country.
    *
    * @param Name of the country.
    * @param areaName The area.
    * @return An id or empty if not found.
    */
   MC2String getLocatorId( const MC2String& countryName,
                           const MC2String& areaName);
   
   /**
    * Parses the reviews
    * @param xml String containing the XML to parse
    * @param url The url to the XML document
    * @param nbrToFetch The macimum number of reviews to fetch
    * @param reviews Vector with the parsed reviews
    **/
   int parseReviews( const MC2String& xml, MC2String url, uint32& nbrToFetch,
                     vector<POIReview>& reviews );

   /**
    * Parses the images
    * @param xml String containing the XML to parse
    * @param url The url to the XML document
    * @param nbrToFetch The macimum number of images to fetch
    * @param reviews Vector with the image urls
    **/
   int parseImages( const MC2String& xml, MC2String url, uint32& nbrToFetch,
                    vector< MC2String >& images );

private:
   /// Constant for the consumer key
   const MC2String m_consumerKey;
   /// Constant for the Qype url
   const MC2String m_qypeUrl;

   auto_ptr<URLFetcher> m_urlFetcher; ///< fetching query urls

   /// XPath expressions for main XML
   auto_ptr<XMLTool::XPath::MultiExpression> m_expression; 
   uint32 m_totalMatches; ///< total number of matches
   typedef XMLTool::XPath::MatchHandler< ::QypePlaceInfo > Matches;
   Matches* m_matches; ///< is deleted by expression

   /// XPath expressions for Reviews XML
   auto_ptr<XMLTool::XPath::MultiExpression> m_expressionReviews; 
   typedef XMLTool::XPath::MatchHandler< ::QypeReviewInfo > ReviewMatches;
   ReviewMatches* m_reviews; ///< is deleted by expressionReviews

   /// XPath expressions for Images XML
   auto_ptr<XMLTool::XPath::MultiExpression> m_expressionImages; 
   typedef XMLTool::XPath::MatchHandler< ::QypeImageInfo > ImageMatches;
   ImageMatches* m_images; ///< is deleted by expressionImages
};
#endif
