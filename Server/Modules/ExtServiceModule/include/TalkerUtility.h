/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef TALKERUTILITY_H
#define TALKERUTILITY_H

#include "config.h"

#include "MC2String.h"
#include "SearchMatch.h"
#include "LangTypes.h"
#include "ItemInfoEnums.h"
#include "StringUtility.h"
#include "ItemInfoEntry.h"
#include "XPathMultiExpression.h"
#include "XMLUtility.h"
#include "StringConvert.h"
#include "StringTable.h"
#include "XPathAssigner.h"

#include <memory>
#include <vector>

/// helper utilities for the Talkers
namespace TalkerUtility {

/**
 * Helper class to add information to an info vector.
 */
class AddInfo {
public:

   /**
    * Constructor.
    * @param lang The language
    * @param infoFilter The filter to use when adding an info.
    */
   explicit AddInfo( LangType lang, 
                     ItemInfoEnums::InfoTypeFilter infoFilter ) : 
         m_lang( lang ), m_filterLevel( infoFilter ), m_additionalInfoExists( false)  { } 

   /**
    * Adds information to the info vector if type should be included
    * in the filter level.
    *
    * Specialization for MC2String and vector< MC2String> 
    * are implemented in the cpp file.
    * @param value the value to add
    * @param typeOfInfo the type of info the value is
    */
   template <typename T>
   void add( const T& value, ItemInfoEnums::InfoType typeOfInfo );
   
   template <typename T>
   void add( const T& value, const MC2String& value2, 
             ItemInfoEnums::InfoType typeOfInfo );

   SearchMatch::ItemInfoVector& getInfoVector() { return m_infoVect; }

   const LangType& getLangType() const { return m_lang; }
   
   bool additionalInfoExits() { return m_additionalInfoExists; }
private:
   SearchMatch::ItemInfoVector m_infoVect;
   LangType m_lang;
   ItemInfoEnums::InfoTypeFilter m_filterLevel;
   bool m_additionalInfoExists;
};


/**
 * Prints information about a node when it is evaluted in MultiExpression.
 * Mostly used for debug.
 */
class NodePrinter: public XMLTool::XPath::MultiExpression::NodeEvaluator {
public:
   explicit NodePrinter(const MC2String& other = MC2String() ):
      m_info( other ) { }

   void operator() ( const DOMNode* node ) {
      cout << m_info << " node \"" << node->getNodeName() << "\""
           << "= \"" 
           << XMLUtility::getChildTextStr( *node ) 
           << "\"" << endl;
   }
private:
   MC2String m_info;
};


/**
 * Parses a xml string source and returns xml parsers instance.
 * @param url the original url
 * @param xmlSource the source to be parsed
 * @param ignoreInitialXML Whether or not to ignore the <?xml tag
 * @return pointer to DOM parser
 */
auto_ptr<XercesDOMParser> parseXMLSource( const MC2String& url, 
                                          const MC2String& xmlSource,
                                          bool ignoreInitialXML = false ); 


MC2String getIntPhoneNumber( const MC2String& number,
                             StringTable::countryCode country );

template <typename T>
T convertToIntPhoneNumber( const T& numbers, StringTable::countryCode country );
                           


}

#endif
