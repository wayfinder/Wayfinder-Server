/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "config.h"

#include "Properties.h"

#include "Utility.h"
#include "StringUtility.h"
#include "NewStrDup.h"
#include "Property.h"
#include "FilePtr.h"
#include "ScopedArray.h"

#include <fstream>
#include <stdlib.h>

Property**  Properties::hashTable      = NULL;  // init hashTable;
Property**  Properties::oldHashTable   = NULL;  // setPropertyFileName
bool        Properties::initialized    = false;
uint32      Properties::m_mapSet       = MAX_UINT32;

//=================================================================
// TODO: These values should be changed when the correct behavior
//       is implemented in all applications. The filename should
//       be initialized as "" and the bool value as false, in order
//       to force applications to set the property filename.
//=================================================================
MC2String      Properties::filename       = "mc2.prop";
bool        Properties::propertyFileNameSet = true;
//-----------------------------------------------------------------

MC2String      Properties::cohProperties;


Properties::~Properties() 
{
   Destroy();
}

void Properties::Destroy()
{
   if (initialized) {
      for (uint32 i=0; i<43; i++) {
         Property* prop = (hashTable[i]);
         while (prop != NULL) {
            Property* tmp = (Property *)prop->getNextInBucket();
            delete prop;
            prop = tmp;
         }
      }
      delete [] hashTable;
      hashTable = NULL;
      initialized = false;
   }
   if ( oldHashTable != NULL ) {
      for ( uint32 i = 0 ; i < 43 ; ++i ) {
         Property* prop = oldHashTable[ i ];
         while ( prop != NULL ) {
            Property* tmp = static_cast<Property *>( 
               prop->getNextInBucket() );
            delete prop;
            prop = tmp;
         }
      }
      delete [] oldHashTable;
      oldHashTable = NULL;
   }
}

bool
Properties::setPropertyFileName(const char* fileName)
{
   filename = fileName;
   ifstream test(filename.c_str());
   if(test) {
      propertyFileNameSet = true;
      oldHashTable = hashTable;
      initialized = false;
      return true;
   }

   return false;
}

const char*
Properties::getPropertyFileName()
{
   return filename.c_str();
}
 
const char *
Properties::getProperty(const char *key, const char* defVal )
{
   if (!initialized) initialize();
   if (m_mapSet != MAX_UINT32) {
      char mapSetKey[255];
      if (strcmp(key, "MAP_PATH") == 0) {
         sprintf(mapSetKey, "MAP_PATH_%u", m_mapSet);
         return getProperty(mapSetKey);
      }
      if (strcmp(key, "MAP_PATH_URL") == 0) {
         sprintf(mapSetKey, "MAP_PATH_URL_%u", m_mapSet);
         return getProperty(mapSetKey);
      }
      if (strcmp(key, "MODULE_CACHE_PATH") == 0) {
         sprintf(mapSetKey, "MODULE_CACHE_PATH_%u", m_mapSet);
         return getProperty(mapSetKey);
      }
   }
   uint16 hashCode = Utility::hash(key) % 43;
   Property *prop = hashTable[hashCode];

   while (prop && ( strcmp(key, prop->getKey()) != 0 ) ){
      prop = (Property *)prop->getNextInBucket();
   }

   return (prop != NULL) ? prop->getValue() : const_cast<char*>(defVal);
}

const char*
Properties::getProperty(const MC2String& key)
{
   return getProperty(key.c_str());
}

uint32
Properties::getUint32Property(const char* key,
                              uint32 defVal)
{
   uint64 result = getUint64Property( key, defVal );
   // does the property file contain a uint32 setting with a
   // value which is too large?
   MC2_ASSERT( result <= MAX_UINT32 );
   return static_cast<uint32>( result );
}

uint32
Properties::getUint32Property(const MC2String& key,
                              uint32 defVal)
{
   return getUint32Property(key.c_str(), defVal);
}

uint64
Properties::getUint64Property(const char* key,
                              uint64 defVal)
{
   const char* stringVal = getProperty(key);
   if ( stringVal == NULL || stringVal[0] == '\0' ) {
      return defVal;
   }

   char* endPtr = NULL;
   errno = 0;
   uint64 value = strtoull(stringVal, &endPtr, 0);

   if ( endPtr == stringVal || errno != 0 ) {
      // No digits.
      return defVal;
   }
   return value;
}

uint64
Properties::getUint64Property(const MC2String& key,
                              uint64 defVal)
{
   return getUint64Property(key.c_str(), defVal);
}

bool
Properties::getBoolProperty( const char* key, bool defVal ) {
   const char* stringVal = getProperty( key );
   if ( stringVal == NULL || stringVal[ 0 ] == '\0' ) {
      return defVal;
   }

   return StringUtility::checkBoolean( stringVal, defVal );
}

bool
Properties::getBoolProperty( const MC2String& key, bool defVal ) {
   return getBoolProperty( key.c_str(), defVal );
}

int
Properties::getListProperty( vector<MC2String>& outlist,
                             const MC2String& key,
                             const char* defVal,
                             char separator )
{
   const char* propVal = getProperty( key.c_str(), defVal );
   ScopedArray<char> propCopy( NewStrDup::newStrDup( propVal  ) );
   StringUtility::tokenListToVector( outlist,
                                     propCopy.get(),
                                     separator );
   return outlist.size();
}

int
Properties::getUint32ListProperty( vector<uint32>& outlist,
                                   const MC2String& key,
                                   const char* defVal,
                                   char separator )
{
   vector<MC2String> stringList;
   int nbr = getListProperty( stringList,
                              key,
                              defVal,
                              separator );
   
   outlist.reserve( stringList.size() );
   for ( vector<MC2String>::const_iterator it = stringList.begin();
         it != stringList.end();
         ++it ) {
      outlist.push_back( strtoul( it->c_str(), NULL, 0 ) );
   }
   return nbr;
}


void
Properties::insertProperty( const char* name, const char* value ) {
   if (!initialized) initialize();
   insert( new Property( name, value ) );
}


void
Properties::insertProperty( const MC2String& name, 
                            const MC2String& value )
{
   insertProperty( name.c_str(), value.c_str() );
}
      

void
Properties::setCohProperties( const MC2String& s ) {
   cohProperties = s;
}


bool
Properties::initialize() {
   if(!initialized && propertyFileNameSet) {
      
      if (!(hashTable = new (Property *[43]))) {
         PANIC("Properties: ", "Can't allocate hashtable");
      }
      for (uint16 i = 0; i < 43; i++) 
         hashTable[i] = NULL;
      
      FileUtils::FilePtr inFile( fopen( filename.c_str(), "r") );
      if (inFile.get() == NULL) {
         perror("Error: Could not open mc2.prop in current directory.\n"
                "System error is " );
         exit(1);
      }

      // Keys
      vector< char* > target;
      // Values
      vector< char* > value;

      // Start by inserting the username ( it can be overwritten )
      {
         char* userName = getenv("LOGNAME");
         // It is called USERNAME on my WINNT-install
         // Don't know if it's always like that.
         if ( userName != NULL ) {
            target.push_back(  StringUtility::newStrDup("USER") );
            value.push_back( StringUtility::newStrDup(userName) );
         } else {
            userName = getenv("USERNAME");
            if ( userName != NULL ) {
               target.push_back(  StringUtility::newStrDup("USER") );
               value.push_back( StringUtility::newStrDup(userName) );
            } else {
               // User is administrator in cygwin, which is
               // not as good as the login-name.
               userName = getenv("USER"); 
               if ( userName != NULL ) {
                  target.push_back(  StringUtility::newStrDup("USER") );
                  value.push_back( StringUtility::newStrDup(userName) );
               }
            }
         }
      }
      
      char ch;

      MC2String tmpTarget;
      MC2String tmpValue;
      
      bool val = false;
      bool storews = false;

      // Read rules from file and store in target, value
      while( (ch=fgetc(inFile.get())) != EOF ) {
         switch ( ch ) {
            case '=' : 
               if ( val ) {
                  tmpValue += ch;
               } else {
                  val = true;
               }
               break;            
            case 10 : // endl (LF)
               val = false;
               storews = false;
               if( ! tmpTarget.empty() ) {
                  target.push_back( 
                     StringUtility::newStrDup( tmpTarget.c_str()) );
                  value.push_back( 
                     StringUtility::newStrDup( tmpValue.c_str()) );
               }               
               tmpTarget.clear();
               tmpValue.clear();
               break;            
            case '#' :
               while( ( (ch=fgetc(inFile.get())) != EOF ) && ch != 10 );  // get passed comment line
               val = false;
               storews = false;
               if( ! tmpTarget.empty() ) {
                  target.push_back( 
                     StringUtility::newStrDup( tmpTarget.c_str()) );
                  value.push_back( 
                     StringUtility::newStrDup( tmpValue.c_str()) );
                  tmpTarget.clear();
                  tmpValue.clear();
               }       
               break;
            case '"' :
               storews = !storews;  
               break;
            
            case 32 : // space
            case 9 :  // tab
               if(storews) {
                  if(!val) {
                     tmpTarget += ch;
                  } else {
                     tmpValue += ch;
                  }
               }
               break;            
            default :
               if(!val) {
                  tmpTarget += ch;
               } else {
                  tmpValue += ch;
               }
               break;
         }
      }

      // The properties from command line.
      // Parse and add properties
      vector<MC2String> strings;
      StringUtility::tokenListToVector( strings, cohProperties, ',' );
      for ( uint32 i = 0 ; i < strings.size() ; ++i ) {
         // TEST=42
         MC2String::size_type findPos = strings[ i ].find( '=' );
         if ( findPos != MC2String::npos ) {
            // Split and add
            target.push_back( StringUtility::newStrDup( 
               strings[ i ].substr( 0, findPos).c_str() ) );
            value.push_back( StringUtility::newStrDup( 
               strings[ i ].substr( findPos + 1 ).c_str() ) );
         } else {
            cerr   << warn << "[Properties] In Command line properties "
                   << "Failed "
                   << "to find = in property: \"" << strings[ i ] << "\""
                   << endl;
         }
      }

      // variable expansion
      for ( uint32 i = 0 ; i < target.size() ; i++ ) {
         DEBUG4(cerr << "[Properties] Looking at " << target[i]
                     << " = " << value[i]
                     << endl;);
         char* left;
         char* right;
         while ((left = strchr(value[i], '{')) != NULL) {
            right = strchr(left, '}');
            right[0] = '\0';
            DEBUG4(cerr << "[Properties] expanding " << left+1 << endl;);
            char* expval = NULL;
            for ( uint32 j = 0 ; j < target.size() ; j++ ) {
               if ( strcmp( target[ j ], left + 1 ) == 0 ) {
                  expval = value[ j ];
               }
            }
            if (expval == NULL) {
               cerr << error << "Error in mc2.prop, tried to expand \""
                    << left+1 << "\", key not found!" << endl;
               right[0] = '}';
               break;
            }
            right[0] = '}';
            char* newval = new char[strlen(value[i]) + strlen(expval)];
            left[0] = '\0';
            strcpy(newval, value[i]);
            strcat(newval, expval);
            strcat(newval, right+1);
            delete [] value[i];
            value[i] = newval;
            DEBUG4(cerr << "[Properties] Expanded value, new string: "
                        << newval << endl;);
         }
      }

      for ( uint32 t = 0 ; t < target.size() ; ++t ) {

// DEBUG4(
//           cerr << "[Prop]: " << t << ": " << MC2CITE(target[t]);
//           if(strlen(value[t]) != 0) {
//              cerr << " = " << MC2CITE(value[t]) << ", "
//                     << strlen( value[t] ) << endl;
//           }
//           cerr << endl;
// );

         insert(new Property( target[t],
                              value[t]  ));

         // Clean up allocated memory.
         delete [] target[t];
         delete [] value[t];
      }

      initialized = true;
   }
   return initialized;
}

void
Properties::insert(Property *prop)
{
   prop->setNextInBucket(hashTable[prop->getHashCode() % 43]);
   hashTable[prop->getHashCode() % 43] = prop;
}

uint32
Properties::getMapSet() 
{ 
   return m_mapSet;
}

void 
Properties::setMapSet(uint32 mapSet) 
{
   m_mapSet = mapSet;
}

