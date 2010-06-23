/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef PROPERTYHELPER_H
#define PROPERTYHELPER_H

#include "MC2String.h"
#include "Properties.h"
#include "StringConvert.h"
#include <boost/lexical_cast.hpp>

/// exception thrown when there is a problem with properties
class PropertyException: public std::exception {
public:
   explicit PropertyException( const MC2String& str) throw():
      m_str( str ) { }
   ~PropertyException() throw() { }
   const char* what() const throw() { return m_str.c_str(); }
private:
   MC2String m_str;
};

/**
 * Helper for properties.
 *
 */
namespace PropertyHelper {

/**
 * Sets up and finlizes class Properties in a scope.
 * Makes the code exception safe. Instanciate one instance of this class
 * in the beginning of the program.
 */
class PropertyInit { 
public:
   PropertyInit() { }
   ~PropertyInit() {
      Properties::Destroy();
   }
};

/**
 * Obsolete, use StringConvert::convert instead.
 * Tries to set specified property from string,
 * throws MC2String with a description of the error on failure.
 * This must be specialized.
 * @return copy of property
 */
template <typename T>
T getFromString( const char* str ) throw (PropertyException);

/**
 * Gets property, if it fails it will throw MC2String with
 * a description of the error.
 * @return copy of property value
 */
template <typename T>   
T get( const char *propStr ) throw (PropertyException) try { 

   const char *str = Properties::getProperty( propStr );
   if ( str == NULL ) {
      throw PropertyException("Could not find property. ");
   }

   return StringConvert::convert<T>( str );

} catch ( PropertyException str ) {
   throw PropertyException( MC2String( str.what() ) + 
                            " Property: " + ( propStr ? propStr : "" ));

} catch ( const StringConvert::ConvertException& e ) {
   throw PropertyException( MC2String( e.what() ) +
                            " Property: " + ( propStr ? propStr : "" ) );
}

/**
 * Gets a property, if it doesn't exist (or is invalid somehow)
 * a defaultValue is returned.
 */
template<typename T>
T get( const MC2String& propStr, T defaultValue );

/**
 *  Gets the value for a property by trying different variations of
 *  the property name. This is used so a property file can include for
 *  instance properties like "MODULE_MAX_MEM", "MAP_MODULE_MAX_MEM",
 *  "MODULE_MAX_MEM_0" etc, where the more specialized names take
 *  precedence.
 *
 *  @param propertyName   The base name of the property.
 *  @param modulePrefix   The name of the module, for instance "MAP".
 *  @param mapSet         The map set, is added as a suffix.
 *  @param defaultValue   The default value to use if the property does
 *                        not exist or is invalid somehow.
 *  @return The value of the most specialized property name.
 */
template<typename T>
T getMostSpecializedProperty( const MC2String& propertyName,
                              const MC2String& modulePrefix,
                              uint32 mapSet,
                              T defaultValue ) {
   MC2String prefix = modulePrefix + "_";
   MC2String suffix = MC2String("_") + boost::lexical_cast<MC2String>( mapSet );

   T result = get( propertyName, defaultValue );
   result   = get( propertyName + suffix, result );
   result   = get( prefix + propertyName, result );
   result   = get( prefix + propertyName + suffix, result );

   return result;
}

};

#endif 
