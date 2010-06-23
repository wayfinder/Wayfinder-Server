/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "QypeHandler.h"

#include "HttpHeader.h"
#include "URLFetcher.h"
#include "Properties.h"
#include "TalkerUtility.h"
#include "SearchFields.h"
#include "SearchReplyData.h"
#include "ExternalSearchRequestData.h"
#include "ExternalSearchDesc.h"
#include "STLStringUtility.h"
#include "StringUtility.h"
#include "URL.h"
#include "CoordinateTransformer.h"
#include "WGS84Coordinate.h"
#include "SearchRequestParameters.h"
#include "URLParams.h"
#include "boost/lexical_cast.hpp"
#include "XPathMultiExpression.h"
#include "ExtServices.h"
#include "LangTypes.h"
#include "ExtInfoQuery.h"
#include "Encoding.h"
#include "POIReview.h"

static const uint32 MAX_REVIEWS = 10;
static const uint32 MAX_IMAGES = 10;

namespace XMLTool {
namespace XPath {
/// node specialization for review link
template <>
void VectorAssigner< ReviewLink >::operator()( const DOMNode* node )  try {
   ReviewLink link;
   XMLTool::getAttrib( link.m_count, "count", node );
   XMLTool::getAttrib( link.m_lang, "hreflang", node );
   XMLTool::getAttrib( link.m_href, "href", node );
   m_value.push_back( link );
} catch ( const XMLTool::Exception& e ) {
   mc2dbg << warn << "[Qype] Reviews Link  node: " << e.what() << endl;
}

/// node specialization for images link
template <>
void VectorAssigner< AssetsLink >::operator()( const DOMNode* node )  try {
   AssetsLink link;
   XMLTool::getAttrib( link.m_count, "count", node );
   XMLTool::getAttrib( link.m_href, "href", node );
   m_value.push_back( link );
} catch ( const XMLTool::Exception& e ) {
   mc2dbg << warn << "[Qype] Assets Link  node: " << e.what() << endl;
}

/// node specialization for Qype link
template <>
void VectorAssigner< QypeLink >::operator()( const DOMNode* node )  try {
   QypeLink link;
   XMLTool::getAttrib( link.m_lang, "hreflang", node );
   XMLTool::getAttrib( link.m_href, "href", node );
   m_value.push_back( link );
} catch ( const XMLTool::Exception& e ) {
   mc2dbg << warn << "[Qype] Qype Link  node: " << e.what() << endl;
}

/// Specialization reviewer
template <>
inline void Assigner< Reviewer >::operator() ( const DOMNode* node ) {
   XMLTool::getAttrib(  m_value.userName, "title", node );
}

}
}

/// Struct with information about a review
struct QypeReviewInfo {
   MC2String m_rating;
   MC2String m_date;
   MC2String m_content;
   Reviewer m_reviewer;
};

/** 
 * Struct holding the image url. Needs to be a struct because we need a
 * a custom assigner.
 */
struct QypeImageInfo {
   MC2String m_url;
};

/**
 * Remove links without reviews and reviews in other languages
 * then the requested or english.
 * @param reviews Vector containing reviews. Content is changed by this
 *                function.
 * @param lang The requested language for the reviews.
 */
void
filterReviews( vector< ReviewLink >& reviews, const LangType& lang ) {
   MC2String language( LangTypes::getLanguageAsISO639( lang ) );
   vector< ReviewLink > reviewsToKeep;

   // Remove links without reviews and reviews in other languages
   // then the requested or english.   
   for ( vector< ReviewLink >::iterator it = reviews.begin();
         it != reviews.end(); ++it ) {
      if ( it->m_count >= 0 ) {
         if ( it->m_lang == language ) {
            // Insert first
            reviewsToKeep.insert( reviewsToKeep.begin(), *it );
         } else if (it->m_lang == "en" ) {
            // English is backup. Put it last in list.
            reviewsToKeep.push_back( *it );
         } 
      }
   }
   
   reviews.swap( reviewsToKeep );
}


/**
 * Get link in requested language or english if requested language does 
 * not exists. If english does not exsists either the first link is 
 * returned.
 * @param links Vector with QypeLinks.
 * @param lang The requested language.
 * @return The QypeLink
 **/
MC2String
getPOIUrl(const vector< QypeLink >& links, const LangType& lang ) {
   MC2String language( LangTypes::getLanguageAsISO639( lang ) );
   MC2String defaultString;

   for ( vector< QypeLink >::const_iterator it = links.begin();
         it != links.end(); ++it ) {      
      if ( it->m_lang == language ) {
         return it->m_href;
      } else if (it->m_lang == "en" ) {
         defaultString = it->m_href;
      }       
   }

   if ( defaultString.empty() && !links.empty() ) {
      // Page does not exist in requested language or in english
      // just get the first link.
      defaultString = links.front().m_href;
   }
   
   return defaultString;
}


QypeHandler::QypeHandler() :
      m_locatorCache(),
      m_consumerKey( Properties::getProperty( "QYPE_CONSUMER_KEY",
                                              "" ) ),
      m_qypeUrl( Properties::getProperty( "QYPE_URL",
                                          "http://api.qype.com/v1/" ) ),
      m_urlFetcher( new URLFetcher() ),
      m_matches( new QypeHandler::Matches() ),
      m_reviews( new QypeHandler::ReviewMatches() ),
      m_images( new QypeHandler::ImageMatches() ) {

   ::QypePlaceInfo& match = m_matches->getCurrMatch();
   ::QypeReviewInfo& review = m_reviews->getCurrMatch();
   ::QypeImageInfo& image = m_images->getCurrMatch();

   using XMLTool::XPath::MultiExpression;
   using XMLTool::XPath::makeAssigner;

   MultiExpression::NodeDescription desc[] = {
      { "/place*", 
        m_matches },
      { "/place/id",
        makeAssigner( match.m_id ) },
      { "/place/title", 
        makeAssigner( match.m_title ) },
      { "/place/address/street", 
        makeAssigner( match.m_street ) },
      { "/place/address/housenumber", 
        makeAssigner( match.m_houseNumber ) },
      { "/place/address/city", 
        makeAssigner( match.m_city ) },
      { "/place/address/postcode", 
        makeAssigner( match.m_postcode ) },
      { "/place/address/country_code", 
        makeAssigner( match.m_countryCode ) },
      { "/place/category/id", 
        makeAssigner( match.m_qypeCategoryId ) },
      { "/place/url",
        makeAssigner( match.m_url ) },
      { "/place/phone",
        makeAssigner( match.m_phone ) },
      { "/place/opening_hours",
        makeAssigner( match.m_openingHours ) },
      { "/place/average_rating",
        makeAssigner( match.m_avgRating ) },
      { "/place/link*/@rel=\"http://schemas.qype.com/reviews\"",
        makeAssigner( match.m_reviewLinks ) },
      { "/place/link/@rel=\"http://schemas.qype.com/assets\"",
        makeAssigner( match.m_imageLinks ) },
      { "/place/link/@rel=alternate",
        makeAssigner( match.m_qypeLinks ) },
      { "/place/point",
        makeAssigner( match.m_coord ) },
      { "/place/distance",
        makeAssigner( match.m_distance ) },
      { "/place/image/@small",
        makeAssigner( match.image.m_small ) },
      { "/place/image/@medium",
        makeAssigner( match.image.m_medium ) },
      { "/place/image/@large",
        makeAssigner( match.image.m_large ) }
   };
   
   uint32 size = sizeof ( desc ) / sizeof ( desc[ 0 ] );
   m_expression.reset( new MultiExpression( MultiExpression::
                                       Description( desc, desc + size ) ) );

   // Reviews
   MultiExpression::NodeDescription reviewDesc[] = {
      { "/review*", 
        m_reviews },
      { "/review/rating",
        makeAssigner( review.m_rating ) },
      { "/review/updated", 
        makeAssigner( review.m_date ) },
      { "/review/content*/@type=text", 
        makeAssigner( review.m_content ) },
      { "/review/link*/@rel=\"http://schemas.qype.com/user\"", 
        makeAssigner( review.m_reviewer ) }
   };
   
   uint32 sizeRev = sizeof ( reviewDesc ) / sizeof ( reviewDesc[ 0 ] );
   m_expressionReviews.reset(  
      new MultiExpression( MultiExpression::
                           Description( reviewDesc, reviewDesc + sizeRev ) ) );

   // Images
   MultiExpression::NodeDescription imageDesc[] = {
      { "/asset*", 
        m_images },
      { "/asset/image/@medium",
        makeAssigner( image.m_url ) }
   };
   
   uint32 sizeImg = sizeof ( imageDesc ) / sizeof ( imageDesc[ 0 ] );
   m_expressionImages.reset( 
      new MultiExpression( MultiExpression::
                           Description( imageDesc, imageDesc + sizeImg ) ) );

}

QypeHandler::~QypeHandler() {
}

int 
QypeHandler::doQueryImpl( SearchReplyData& reply, 
                          const ExternalSearchRequestData& searchData,
                          int nbrRetries ) {
   // Create the request url.
   MC2String url( createURL( searchData ) );
   
   int res = -1;
   bool firstRun = true;
   while ( nbrRetries-- ) {
      if ( ! firstRun ) {
         mc2log << warn << "[QypeTalker]: Retrying" << endl;
      }
      firstRun = false;
      MC2String xml_result;
      
      res = sendRequest( xml_result, url );
      if ( res != 200 ) {
         // Do another round.
         continue;
      }

      res = parseResult( reply, xml_result, 
                         searchData.getLang(), url, 
                         searchData.getInfoFilterLevel() );
      
      if ( res < 0 ) {
         // Error - retry (Sometimes the server returns empty answer)
         continue;
      } else {
         // Ok - stop running.
         return 0;
      }
   }
   
   return res;
}

int 
QypeHandler::doInfoQueryImpl( ExtInfoQuery& info ) {
   MC2String poiId = Encoding::base64Decode( info.getExternalID() );
   MC2String xml_result;
   MC2String url( getQypeUrl() );
   int res = 0;
   try {
      poiId = poiId.substr( poiId.rfind('/') );
      url += "places";
      url += poiId;
      URLParams params;
      params.add("consumer_key", getConsumerKey() );
      url += params;
      
      res = sendRequest( xml_result, url );   
   } catch ( const std::out_of_range& ) {
      mc2log << error << "[QypeHandler] Exception thrown when extracting "
         "id." << endl;
   }

   res = parseResult( info.getReply(),
                      xml_result, info.getLang(),
                      url, ItemInfoEnums::All, true);
   return res;
}

int
QypeHandler::sendRequest( MC2String& qype_xml,
                         const MC2String& url ) {

   mc2dbg << "[QypeHandler]: URL request " << MC2CITE( url ) << endl;

   HttpHeader urlHeader;
   int res = m_urlFetcher->get( qype_xml, URL( url ), 
                                Properties::getUint32Property( "QYPE_TIMEOUT",
                                                               5000 ),
                                &urlHeader );
   return res;
}

int 
QypeHandler::parseResult( SearchReplyData& reply,
                         const MC2String& qype_xml,
                         const LangType& lang,
                         const MC2String& url,
                         ItemInfoEnums::InfoTypeFilter filterLevel,
                         bool poiInfoRequest){

   // parse document
   auto_ptr<XercesDOMParser> xmlParser( TalkerUtility::
                                        parseXMLSource( url, qype_xml ) );
   if ( xmlParser.get() == NULL ) {
      mc2log << error
             << "[QypeHandler] Failed to parse xml document." << endl;
      return -1;
   }

   DOMNode* document = xmlParser->getDocument();
   if ( ! document ) {
      mc2dbg << warn << "[QypeHandler] Failed to parse result retrieved from: " 
             <<  url << endl;
      return -1;
   }

   m_matches->reset();
   DOMNode* root = document;
   if ( ! poiInfoRequest ) {
      root = XMLTool::findNode( document, "places" );
   }
   
   m_expression->evaluate( root );
   m_matches->handleNewMatch();
   m_totalMatches = m_matches->getMatches().size();
   mc2dbg << "[QypeHandler] #" << m_totalMatches << " matches." << endl;

   MC2String qypeId = StringUtility::replaceString(url.c_str(),
                                                   m_qypeUrl.c_str(), "");
   qypeId = StringUtility::replaceString(qypeId.c_str(),
                                         m_consumerKey.c_str(), "#");

   // setup reply
   createSearchMatches( reply.getMatchVector(), m_matches->getMatches(), 
                        lang, qypeId, filterLevel, poiInfoRequest );

   m_matches->reset();

   return 0;
}


void 
QypeHandler::createSearchMatches( vector<VanillaMatch*>& finalMatches,
                                 const vector<QypePlaceInfo>& matches,
                                 const LangType& lang,
                                 const MC2String& id,
                                 ItemInfoEnums::InfoTypeFilter filterLevel,
                                 bool poiInfoRequest ) {
   
   for ( vector<QypePlaceInfo>::size_type i = 0; i < matches.size(); ++i ) {
      
      const QypePlaceInfo& match = matches[ i ];

      auto_ptr<VanillaCompanyMatch> searchMatch (
         static_cast<VanillaCompanyMatch*>( 
            SearchMatch::createMatch( SEARCH_COMPANIES ) ) );
     
      MC2String qypeId;
      MC2String poiUrl;
      try {
         qypeId = match.m_id.substr( match.m_id.rfind('/') );
         poiUrl = getPOIUrl( match.m_qypeLinks, lang );
         qypeId = id + qypeId;
      } catch ( const std::out_of_range& ) {
         mc2log << "[QypeHandler] Failed to extract id." << endl;
      }

      searchMatch->setExtSourceAndID( ExtService::qype, 
                                      StringUtility::base64Encode( qypeId ) );
      searchMatch->setName( match.m_title.c_str() );
      searchMatch->setLocationName( match.m_title.c_str() );
      
      // The coordinate is a single string "[wgs84-lat],[wgs84-lon]" so 
      // we need to split and convert it. The regex is a simple 
      // lookahead/lookbehind for comma separator.
      try {
         vector<MC2String> latlon;
         if ( ! match.m_coord.empty() ) {
            int pos = match.m_coord.find( ',' );
            
            
            latlon.push_back( match.m_coord.substr( 0, pos ) );
            latlon.push_back( match.m_coord.substr( pos+1 ) );
            
            WGS84Coordinate wgs84_coord(
               boost::lexical_cast<float64>(latlon[0]),  
               boost::lexical_cast<float64>(latlon[1]) );
            
            if( !wgs84_coord.isValid() ){
               mc2log << error << "[QypeHandler] Invalid coordinate." << endl;
            } else {
               
               MC2Coordinate mc2_coord = 
                  CoordinateTransformer::transformToMC2( wgs84_coord );
               searchMatch->setCoords( mc2_coord );
            }
         }
         
         // Translate Qype category to MC2 category
         if( ! match.m_qypeCategoryId.empty() ){
            
            vector<MC2String> qcatid;
            bool ok = StringUtility::regexp( "/places/categories/(.+)", 
                                             match.m_qypeCategoryId, 1, qcatid );
            
            if( ! ok ){
               mc2log << error << "[QypeHandler] Failed to extract category id." 
                      << endl;
            } else {
               vector<int16> mc2cats;
               const uint32 size = qcatid.size();
               for ( uint32 i=0 ; i < size ; ++i ) {
                  CategoryTreeUtils::CategoryID qype_cat = 
                     boost::lexical_cast< CategoryTreeUtils::CategoryID >( qcatid[i] );
                  CategoryTreeUtils::CategoryID mc2_cat = QypeHandler::QypeCategoryToMC2Category( qype_cat );
                  if ( CategoryTreeUtils::INVALID_CATEGORY != mc2_cat ) {
                     mc2cats.push_back( mc2_cat );
                     break;
                  }
               }               
               searchMatch->setCategories( mc2cats );
            }
         }
      } catch (const boost::bad_lexical_cast& e) {
         mc2log << error << "[QypeHandler] Exception thorown." << e.what() 
                << endl;
      } catch ( const std::out_of_range& ) {
         mc2log << error << "[QypeHandler] Exception thrown when extracting "
            "coordinates." << endl;
      }


      // Add additional info to search match
      TalkerUtility::AddInfo info( lang, filterLevel );

      info.add( match.m_street, ItemInfoEnums::vis_address );
      info.add( match.m_houseNumber, ItemInfoEnums::vis_house_nbr );
      info.add( match.m_postcode, ItemInfoEnums::vis_zip_code );
      info.add( match.m_city, ItemInfoEnums::Vis_zip_area );
      info.add( match.m_phone, ItemInfoEnums::phone_number );
      info.add( match.m_url, ItemInfoEnums::url );
      info.add( match.m_openingHours, ItemInfoEnums::open_hours );
      info.add( match.m_avgRating, ItemInfoEnums::average_rating );
      info.add( MC2String( "Qype;http://www.qype.com" ), 
                ItemInfoEnums::provider_info );
      info.add( poiUrl, MC2String( "Qype" ), ItemInfoEnums::poi_url );
      // small image by default. Larger can be provided in some other way?
      info.add( match.image.m_small, ItemInfoEnums::poi_thumb );

      searchMatch->swapItemInfos( info.getInfoVector() );
      searchMatch->setAdditionalInfoExists( info.additionalInfoExits() );

      if ( poiInfoRequest ) {
         // Check if we have any reviews or photos to parse
         URLParams params;
         params.add("consumer_key", m_consumerKey );

         // Get the reviews
         vector<ReviewLink> reviewLinks = match.m_reviewLinks;
         filterReviews( reviewLinks, lang );
         if ( ! reviewLinks.empty() ) {                     
            MC2String xml_result;
            vector< POIReview > reviews;
            uint32 reviewsToFetch = MAX_REVIEWS;
            for ( vector< ReviewLink >::const_iterator it = reviewLinks.begin(); 
                  it != reviewLinks.end() && reviewsToFetch > 0; ++it ) {
               if ( it->m_count > 0 ) {
                  MC2String url = it->m_href;
                  url += params;                  
                  int res = sendRequest( xml_result, url ); 
                  if ( res != 200 ) {
                     mc2log << warn << "[QypeHandler] Failed to fetch review url: " << url << endl;
                  } else {
                     parseReviews( xml_result, url, reviewsToFetch, reviews );
                  }
               }
            }

            mc2dbg << "[QypeHandler] #" << reviews.size() << " reviews." << endl;
            searchMatch->swapReviews( reviews );
         }
         
         // Get the images
         vector<AssetsLink> images = match.m_imageLinks;
         if( ! images.empty() ) {
            MC2String xml_result;
            vector< MC2String > imageURLs;
            uint32 imagesToFetch = MAX_IMAGES;
            for ( vector< AssetsLink >::const_iterator it = images.begin(); 
                  it != images.end() && imagesToFetch > 0; ++it ) {
               if ( it->m_count > 0 ) {
                  MC2String url = it->m_href;
                  url += params;                  
                  int res = sendRequest( xml_result, url ); 
                  if ( res != 200 ) {
                     mc2log << warn << "[QypeHandler] Failed to fetch images url: " << url << endl;
                  } else {
                     parseImages( xml_result, url, imagesToFetch, imageURLs );
                  }
               }
            }       
            mc2dbg << "[QypeHandler] #" << imageURLs.size() << " imagess." << endl;            
            searchMatch->swapImageURLs( imageURLs );            
         }         
      }
      
      finalMatches.push_back( static_cast<VanillaMatch*>
                              ( searchMatch.release() ) );
   }
}


MC2String 
QypeHandler::createURL( const ExternalSearchRequestData& searchData ) {
   MC2String url( m_qypeUrl );
   const MC2String search_word = TalkerUtility::getWhatField(
      searchData, TalkerUtility::FIELD_YELLOWPAGES_WITH_CATEGORY );
   const MC2String& search_area =
      searchData.getVal( ExternalSearchDesc::city_or_area );
   const MC2String& categoryId =
      searchData.getVal( ExternalSearchDesc::category_id );
   const MC2String& country_name =
      searchData.getVal( ExternalSearchDesc::country_name );
   
   const MC2String locator_id = getLocatorId( country_name, search_area );
   MC2Coordinate coordMc2 = searchData.getCoordinate();
   if ( coordMc2.isValid() ) {
      WGS84Coordinate coord = CoordinateTransformer::transformToWGS84( 
         searchData.getCoordinate() );
    
      try {
         url += "positions/";
         url += boost::lexical_cast< MC2String >( coord.lat );
         url += ",";
         url += boost::lexical_cast< MC2String >( coord.lon );
         url += "/";
      } catch (const boost::bad_lexical_cast& e) {
         mc2log << error << "[QypeHandler] Exception thorown." << e.what() 
                << endl;
         return "";
      }
   }

   
   url += "places";

   URLParams params; 
   bool categoryParamAdded = false;
   if ( ! categoryId.empty() ) {
      CategoryTreeUtils::CategoryID catId = MC2CategoryToQypeCategory( 
         boost::lexical_cast< CategoryTreeUtils::CategoryID >( categoryId ) );
      if ( catId != CategoryTreeUtils::INVALID_CATEGORY ) {
         params.add( "in_category", catId );
         categoryParamAdded = true;
      }
   }

   if ( ( ! categoryParamAdded ) && 
        ( ! search_word.empty() ) ) {
      params.add( "show", search_word );
   }

   if ( ! coordMc2.isValid() ) {
      if ( ! locator_id.empty() ) {
         params.add( "in_locator", locator_id );
      } else if ( ! search_area.empty() ) {
         params.add( "in", search_area );
      }
   } else {
      params.add( "order", "distance" );
      if ( ( searchData.getDistance()/1000 ) > 0 ) {
         params.add( "radius", searchData.getDistance() / 1000 );
      }
   }
   
   params.add("per_page", searchData.getNbrWantedHits() );
   params.add("consumer_key", m_consumerKey );
   
   url += params;
   return url;
}


MC2String 
QypeHandler::getLocatorId( const MC2String& countryName,
                          const MC2String& areaName ) {
   if ( countryName.empty() || areaName.empty() ) {
      return "";
   }

   MC2String locatorName = areaName + " " + countryName;
   MC2String locator = m_locatorCache[ locatorName ];
   if ( locator.empty() ) {
      URLParams param;
      MC2String xml_result;
      MC2String url;

      url = m_qypeUrl + "locators";
      param.add( "show", locatorName );
      param.add("consumer_key", m_consumerKey );
      url += param;
 
      sendRequest( xml_result, url );

      vector< MC2String > idvec;
      StringUtility::regexp( "<domain_id>(.[^<]+)",
                             xml_result, 1, idvec );
      
      if ( ! idvec.empty() ) {
         m_locatorCache[ locatorName ] = idvec[0];
         mc2log << "[QypeHandler] Cacheing locator id: " << locatorName
                << ", (" << idvec[0] << ") in " << this << endl;
      }

      if ( idvec.empty() ) {
         mc2log << error << "[QypeHandler] Failed to add locator id to cache for "
            "country " << locatorName << endl;
      }
   }

   return locator;
}


int
QypeHandler::parseReviews( const MC2String& xml, MC2String url,
                          uint32& nbrToFetch,
                          vector< POIReview >& reviews ) {
   // parse document
   auto_ptr<XercesDOMParser> xmlParser( TalkerUtility::
                                        parseXMLSource( url, xml ) );
   if ( xmlParser.get() == NULL ) {
      mc2log << error
             << "[QypeHandler] Failed to parse reviews xml document." << endl;
      return -1;
   }

   DOMNode* document = xmlParser->getDocument();
   if ( ! document ) {
      mc2dbg << warn << "[QypeHandler] Failed to parse review result retrieved from: " 
             <<  url << endl;
      return -1;
   }

   m_reviews->reset();
   DOMNode* root = XMLTool::findNode( document, "reviews" );
   
   m_expressionReviews->evaluate( root );
   m_reviews->handleNewMatch();

   const vector<QypeReviewInfo> infos = m_reviews->getMatches();
   for( vector<QypeReviewInfo>::const_iterator it = infos.begin();
        it != infos.end() && nbrToFetch > 0; ++it ) {
      reviews.push_back( POIReview( STLStringUtility::strtol( it->m_rating ), 
                                    it->m_reviewer.userName, it->m_date, 
                                    it->m_content ) );
      nbrToFetch--;
   }

   return 0;
}

int
QypeHandler::parseImages( const MC2String& xml, MC2String url,
                         uint32& nbrToFetch,
                         vector< MC2String >& images ) {
   // parse document
   auto_ptr<XercesDOMParser> xmlParser( TalkerUtility::
                                        parseXMLSource( url, xml ) );
   if ( xmlParser.get() == NULL ) {
      mc2log << error
             << "[QypeHandler] Failed to parse images xml document." << endl;
      return -1;
   }

   DOMNode* document = xmlParser->getDocument();
   if ( ! document ) {
      mc2dbg << warn << "[QypeHandler] Failed to parse images result retrieved from: " 
             <<  url << endl;
      return -1;
   }

   m_images->reset();
   DOMNode* root = XMLTool::findNode( document, "assets" );
   
   m_expressionImages->evaluate( root );
   m_images->handleNewMatch();

   const vector<QypeImageInfo> infos = m_images->getMatches();
   for( vector<QypeImageInfo>::const_iterator it = infos.begin();
        it != infos.end() && nbrToFetch > 0; ++it ) {
      // Convert to internal "cache links"
      MC2String cacheLink = it->m_url;
      STLStringUtility::replaceString( cacheLink, "://", "://cache_img/" );
      images.push_back( cacheLink );
      nbrToFetch--;  
   }

   return 0;
}

CategoryTreeUtils::CategoryID
QypeHandler::QypeCategoryToMC2Category( 
   CategoryTreeUtils::CategoryID categoryId ) {
   switch ( categoryId ) {
      case Airport :
         return MC2Airport;
      case ATM :
         return MC2ATM;
      case AmusementPark :
         return MC2AmusementPark;
      case ArtGalleries :
         return MC2ArtGalleries;
      case Museum :
         return MC2Museum;
      case Stadium :
         return MC2Stadium;
      case Theatres :
         return MC2Theatres ;
      case Cafe :
         return MC2Cafe;
      case Cinema :
         return MC2Cinema;
      case Hotel :
         return MC2Hotel;
      case HealthAndMedical :
         return MC2HealthAndMedical;
      case Dentist :
         return MC2Dentist;
      case Hospital :
         return MC2Hospital;
      case Pharmacy :
         return MC2Pharmacy;
      case Nightlife :
         return MC2Nightlife;
      case Bars:
         return MC2Bars;
      case Pubs :
         return MC2Pubs;
      case Casinos :
         return MC2Casinos;
      case NightClub :
         return MC2NightClub;
      case Parking :
         return MC2Parking;
      case PetrolStations :
         return MC2PetrolStations;
      case PostOffice :
         return MC2PostOffice;
      case BusStation :
         return MC2BusStation;
      case MetroStation :
         return MC2SubwayStation;
      case UndergroundStation :
         return MC2SubwayStation;
      case TramStop :
         return MC2TramStop;
      case Restaurants :
         return MC2Restaurants;
      case Bistro :
         return MC2Bistro;
      case British :
         return MC2British;
      case Chinese :
         return MC2Chinese;
      case FastFood :
         return MC2FastFood;
      case French :
         return MC2French;
      case German :
         return MC2German;
      case Greek :
         return MC2Greek;
      case Indian :
         return MC2Indian;
       case Japanese :
         return MC2Japanese;
      case Portugese :
         return MC2Portugese;
      case SeaFood :
         return MC2SeaFood;
      case Spanish :
         return MC2Spanish;
      case SteakHouse :
         return MC2SteakHouse;
      case Thai :
         return MC2Thai;
      case Turkish :
         return MC2Turkish;
      case Shopping :
         return MC2Shopping;
      case Bookshops :
         return MC2Bookshops;
      case ClotingAndAccessories :
         return MC2ClotingAndAccessories;
      case ConvenienceStores :
         return MC2ConvenienceStores;
      case Florists :
         return MC2Florists;
      case FoodAndDrink :
         return MC2FoodAndDrink;
      case GiftsAndSouvenirs :
         return MC2GiftsAndSouvenirs;
      case GroceryStore :
         return MC2GroceryStore;
      case JewelryAndWatches :
         return MC2JewelryAndWatches;
      case NewsAgents :
         return MC2NewsAgentsAndTobacconists;
      case Tobacconists :
         return MC2NewsAgentsAndTobacconists;
      case Opticians :
         return MC2Opticians;
      case ShoppingCentre :
         return MC2ShoppingCentre;
      case SportShops :
         return MC2SportShops;
      case Toys :
         return MC2Toys;
   }
   return CategoryTreeUtils::INVALID_CATEGORY;
}


CategoryTreeUtils::CategoryID
QypeHandler::MC2CategoryToQypeCategory( 
   CategoryTreeUtils::CategoryID categoryId ) {
   switch ( categoryId ) {
      case MC2Airport :
         return Airport;
      case MC2ATM :
         return ATM;
      case MC2AmusementPark :
         return AmusementPark;
      case MC2ArtGalleries :
         return ArtGalleries;
      case MC2Museum :
         return Museum;
      case MC2Stadium :
         return Stadium;
      case MC2Theatres :
         return Theatres ;
      case MC2Cafe :
         return Cafe;
      case MC2Cinema :
         return Cinema;
      case MC2Hotel :
         return Hotel;
      case MC2HealthAndMedical :
         return HealthAndMedical;
      case MC2Dentist :
         return Dentist;
      case MC2Hospital :
         return Hospital;
      case MC2Pharmacy :
         return Pharmacy;
      case MC2Nightlife :
         return Nightlife;
      case MC2Bars:
         return Bars;
      case MC2Pubs :
         return Pubs;
      case MC2Casinos :
         return Casinos;
      case MC2NightClub :
         return NightClub;
      case MC2Parking :
         return Parking;
      case MC2PetrolStations :
         return PetrolStations;
      case MC2PostOffice :
         return PostOffice;
      case MC2BusStation :
         return BusStation;
      case MC2TramStop :
         return TramStop;
      case MC2Restaurants :
         return Restaurants;
      case MC2Bistro :
         return Bistro;
      case MC2British :
         return British;
      case MC2Chinese :
         return Chinese;
      case MC2FastFood :
         return FastFood;
      case MC2French :
         return French;
      case MC2German :
         return German;
      case MC2Greek :
         return Greek;
      case MC2Indian :
         return Indian;
      case MC2Italian :
         return ItalianAndPizzeria;
      case MC2Japanese :
         return Japanese;
      case MC2Pizzeria :
         return ItalianAndPizzeria;
      case MC2Portugese :
         return Portugese;
      case MC2SeaFood :
         return SeaFood;
      case MC2Spanish :
         return Spanish;
      case MC2SteakHouse :
         return SteakHouse;
      case MC2Thai :
         return Thai;
      case MC2Turkish :
         return Turkish;
      case MC2Shopping :
         return Shopping;
      case MC2Bookshops :
         return Bookshops;
      case MC2ClotingAndAccessories :
         return ClotingAndAccessories;
      case MC2ConvenienceStores :
         return ConvenienceStores;
      case MC2Florists :
         return Florists;
      case MC2FoodAndDrink :
         return FoodAndDrink;
      case MC2GiftsAndSouvenirs :
         return GiftsAndSouvenirs;
      case MC2GroceryStore :
         return GroceryStore;
      case MC2JewelryAndWatches :
         return JewelryAndWatches;
      case MC2Opticians :
         return Opticians;
      case MC2ShoppingCentre :
         return ShoppingCentre;
      case MC2SportShops :
         return SportShops;
      case MC2Toys :
         return Toys;
   }
   return CategoryTreeUtils::INVALID_CATEGORY;
}


