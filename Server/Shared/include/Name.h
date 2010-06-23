/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef NAME_H
#define NAME_H

#include "config.h"

#include "StringUtility.h"
#include "ItemTypes.h"
#include "StringTable.h"
#include "LangTypes.h"

/**
 *   Class representing a name with type and language.
 *
 */
class Name
{
   public:
      /**
       *    Constructor.
       *    @param   name     The name.
       *    @param   lang     The language type.
       *    @param   type     The name type. Default officialName.
       *    @param   copyName Whether the name should be copied or not.
       *                      Default copied.
       */
      Name( const char* name, 
            LangTypes::language_t lang, 
            ItemTypes::name_t type = ItemTypes::officialName,
            bool copyName = true );

      /**
       *    Copy constructor.
       *    @param   name  The name to copy.
       */
      Name( const Name& name );

      /**
       *    Assignment operator.
       */
      const Name& operator=( const Name& other );
      
      /**
       *    Destructor.
       */
      virtual ~Name();

      /**
       *    Get the name.
       *    @return  The name.
       */
      inline const char* getName() const;

      /**
       *    Get the language.
       *    @return  The language.
       */
      inline LangTypes::language_t getLanguage() const;

      /**
       *    Get the name type.
       *    @return  The name type.
       */
      inline ItemTypes::name_t getType() const;

      /**
       *   Print the name on an ostream.
       *   @param stream The stream to print on.
       *   @param name   The resource to print.
       *   @return The stream.
       */
      friend ostream& operator<<( ostream& stream,
                                  const Name& name);
      /**
       *   Only for use with set, map and other datastructures that need
       *   the less than operator.
       */
      bool operator< (const Name& name) const;

   private:
      
      /**
       *    The name.
       */
      const char* m_name;

      /**
       *    The language.
       */
      LangTypes::language_t m_language;

      /**
       *    The type.
       */
      ItemTypes::name_t m_type;

      /**
       *    The name to delete in the destructor. Can be NULL or
       *    a pointer to m_name, depending on if the name was allocated
       *    in the object or not.
       */
      char* m_nameToDelete;

};

// ========================================================================
//                                      Implementation of inlined methods =

inline const char* 
Name::getName() const
{
   return (m_name);
}

inline LangTypes::language_t
Name::getLanguage() const
{
   return (m_language);
}

inline ItemTypes::name_t
Name::getType() const
{
   return (m_type);
}

#endif

