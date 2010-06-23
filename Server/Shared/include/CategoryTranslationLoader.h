/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef CATEGORYTRANSLATIONLOADER_H
#define CATEGORYTRANSLATIONLOADER_H

#include "MC2String.h"
#include "LangTypes.h"
#include "XMLTool.h"

/**
 * Loads category translations from a file.
 */
class CategoryTranslationLoader {
public:
   /**
    * Loads the translations from  a file ( default: xml ).
    * @param filename file to load from.
    * @return true on success, else false.
    */
   virtual bool loadTranslations( const MC2String& filename );
   /// load translation from a node
   virtual void loadTranslations( const DOMNode* node )
      throw ( XMLTool::Exception );

protected:
   /**
    * Implement this to add new categories with language and translation
    * strings.
    */
   virtual void newTranslationValue( uint32 catID, 
                                     LangTypes::language_t lang, 
                                     const MC2String& stringValue ) = 0;

   /**
    * Implement this to add new categories with language and search
    * name strings.
    * Default implementation is to do nothing.
    */
   virtual void newSearchNameValue( uint32 catID, 
                                    LangTypes::language_t lang, 
                                    const MC2String& stringValue );
};

#endif // CATEGORYTRANSLATIONLOADER_H
