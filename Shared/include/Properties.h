/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef PROPERTIES_H
#define PROPERTIES_H

#include "config.h"

#include "MC2String.h"

class Property;

/** 
  *   Class to handle global properties.
  *   All data and methods are static. Data is read once from mc2.prop.
  *   <br />
  *   Special variable USER is set to the same as the USER environment
  *   variable.
  *   <br />
  *   <b>NB!</b> This class is static and allocates memory that never
  *               is released unless the method Destroy() is called.
  *   <br />
  *   
  */
class Properties {
   public:

      /**
        *   Delete this property.  NB! This is never called
        */
      virtual ~Properties();

      /**
       *    Clean up the allocated memory.
       */
      static void Destroy();

      /**
       *    Set the name of the file to read properties from.
       *
       *    @param   fileName    The filename
       *    @return  True if the file exists, false otherwise.
       */
      static bool setPropertyFileName(const char* fileName);
      
      /**
       *    Get the name of the file to read properties from.
       *
       *    @return  The name of the file
       */
      static const char* getPropertyFileName();

      /**
        *   Get a property value.
        *
        *   @param   key    The key to the property.
        *   @param   defVal Default value if property not found.
        *   @return  The value of the given property.
        */
      static const char *getProperty(const char *key,
                                     const char* defVal = NULL );

      /**
       *    Get a property value.
       *    @param   key   The key to the property.
       *    @return  The value of the given property.       
       */
      static const char* getProperty(const MC2String& key);

      /**
       *    Gets an integer property with the default value 
       *    defVal. If the value is prefixed with 0x, the value
       *    will be interpreted as hex.
       *    @param key The key to the property.
       *    @param defVal The default value if the key does not exist.
       *    @return The value of the key or the default value.
       */
      static uint32 getUint32Property( const char* key,
                                       uint32 defVal = MAX_UINT32 );

      /**
       *    Gets an integer property with the default value 
       *    defVal. If the value is prefixed with 0x, the value
       *    will be interpreted as hex.
       *    @param key The key to the property.
       *    @param defVal The default value if the key does not exist.
       *    @return The value of the key or the default value.
       */
      static uint32 getUint32Property( const MC2String& key,
                                       uint32 defVal = MAX_UINT32 );
      
      /**
       *    Gets an integer property with the default value 
       *    defVal. If the value is prefixed with 0x, the value
       *    will be interpreted as hex.
       *    @param key The key to the property.
       *    @param defVal The default value if the key does not exist.
       *    @return The value of the key or the default value.
       */
      static uint64 getUint64Property( const char* key,
                                       uint64 defVal = MAX_UINT64 );
      
      /**
       *    Gets an integer property with the default value 
       *    defVal. If the value is prefixed with 0x, the value
       *    will be interpreted as hex.
       *    @param key The key to the property.
       *    @param defVal The default value if the key does not exist.
       *    @return The value of the key or the default value.
       */
      static uint64 getUint64Property( const MC2String& key,
                                       uint64 defVal = MAX_UINT64 );

      /**
       *    Gets a boolean property with the default value 
       *    defVal. 
       *
       *    @param key The key to the property.
       *    @param defVal The default value if the key does not exist.
       *    @return The value of the key or the default value.
       */
      static bool getBoolProperty( const char* key,
                                   bool defVal = false );

      /**
       *    Gets a boolean property with the default value 
       *    defVal. 
       *
       *    @param key The key to the property.
       *    @param defVal The default value if the key does not exist.
       *    @return The value of the key or the default value.
       */
      static bool getBoolProperty( const MC2String& key,
                                   bool defVal = false );

      /**
       *    Returns the values of a propery consisting of a list
       *    separated by the supplied character.
       */
      static int getListProperty( vector<MC2String>& outlist,
                                  const MC2String& key,
                                  const char* defVal = "",
                                  char separator = ',' );

      /**
       *    Returns the values of a propery consisting of a list
       *    separated by the supplied character.
       */
      static int getUint32ListProperty( vector<uint32>& outlist,
                                        const MC2String& key,
                                        const char* defVal = "",
                                        char separator = ',' );

      /**
       *   Return the mapset of this application instance
       */
      static uint32 getMapSet();

      /**
       *   Set the mapset of this application instance
       */
      static void setMapSet(uint32 mapSet);


      /**
       * Insert a property.
       */
      static void insertProperty( const char* name, const char* value );


      /**
       * Insert a property.
       */
      static void insertProperty( const MC2String& name, 
                                  const MC2String& value );


      /**
       * Set the properties from command line.
       */
      static void setCohProperties( const MC2String& s );


   private:
      /**
        *   The symbol table with properties
        */
      static Property** hashTable;


      /**
       *   The old symbol table with properties
       */
      static Property** oldHashTable;

      /**
       *    The name of the file to read properties from.
       */
      static MC2String filename;

      /**
       * The properties from command line.
       */
      static MC2String cohProperties;

      /**
       *    Flag to indicate whether (spelling?) is initialized
       *    with the setPropertyFileName() method. If not, do not
       *    make any initialization.
       */
      static bool propertyFileNameSet;

      /**
        *   Insert one property into the hashtable.
        */
      static void insert(Property *prop);
      
      /** 
        *   Private method to read data from properties on disk once.
        *   If data is already read, this method does nothing.
        *   It opens the mc2.prop file and reads out properties one by one.
        *   Then it sets log, log_port, log_ip, log_name accordingly.
        */
      static bool initialize();

      /** 
        *   True if data is initialized, used by initialize().
        */
      static bool initialized;

      /**
       *   Stores which map set to use in the particular application instance
       *   FIXME Shouldn't be like this, but let it be until new config system
       //        in place
       */
      static uint32 m_mapSet;
};


#endif
