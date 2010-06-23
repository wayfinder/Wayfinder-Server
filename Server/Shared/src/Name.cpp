/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "Name.h"
#include <sstream>

Name::Name( const char* name, 
            LangTypes::language_t lang, 
            ItemTypes::name_t type /* = ItemTypes::officialName */,
            bool copyName /* = true */ ) 
{
   if ( copyName ) {
      m_nameToDelete = StringUtility::newStrDup( name );
      m_name = m_nameToDelete;
   } else {
      m_nameToDelete = NULL;
      m_name = name;
   }

   m_language = lang;
   m_type = type;
}


Name::Name( const Name& name )
{
   m_nameToDelete = StringUtility::newStrDup( name.getName() );
   m_name = m_nameToDelete;
   m_language = name.getLanguage();
   m_type = name.getType();
}

const Name&
Name::operator=(const Name& other)
{
   if ( this != &other ) {
      delete [] m_nameToDelete; // Valgrind complains if no []
      m_nameToDelete = StringUtility::newStrDup( other.getName() );
      m_name = m_nameToDelete;
      m_language = other.getLanguage();
      m_type = other.getType();
   }
   return *this;
}

Name::~Name() 
{
   delete [] m_nameToDelete; // Valgrind complains if no []
}

ostream&
operator<< ( ostream& stream, const Name& name )
{
   // Try to print everything at once so that multiple
   // threads will not mix the output.
   stringstream strstr;
   strstr << "\"" << name.getName() << "\":" 
          << LangTypes::getLanguageAsString(
             name.getLanguage(), false)
          << ':'
          << ItemTypes::getNameTypeAsString(name.getType(), true );
   stream << strstr.str();
   return stream;
}

bool
Name::operator< (const Name& name) const
{
   int cmpVal = strcmp(m_name, name.m_name);
   if ( cmpVal < 0 ){
      return true;
   }
   else if (cmpVal > 0){
      return false;
   }
   else {
      if ( m_language < name.m_language ){
         return true;
      }
      else if ( m_language > name.m_language ){
         return false;
      }
      else {
         if ( m_type < name.m_type ){
            return true;
         }
         else {
            // Both equal and greater than.
            return false;
         }
      }
   }
}
