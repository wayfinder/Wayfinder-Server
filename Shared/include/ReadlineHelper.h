/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef READLINEHELPER_H
#define READLINEHELPER_H


#include "config.h"
#include "Readline.h"
#include "MC2String.h"


/**
 * Subclass to Readline that has some usefull methods.
 */
class ReadlineHelper : public Readline {
   public:
      /**
       * Constructor. See Readline for details.
       */
      ReadlineHelper( const char* prompt, bool history, 
                      const char* histFile );


      /**
       * Destructor.
       */
      virtual ~ReadlineHelper();


      MC2String getText( const char* name, const char* def = "" );
      uint32 getLong( const char* name, const char* def = ""  );
      uint16 getShort( const char* name, const char* def = ""  );
      byte getByte( const char* name, const char* def = ""  );
      vector< byte > getByteArray( const char* name, 
                                   const char* def = ""  );

      void setPromptWithDefault( const char* name, const char* def );
      void trimEndOfNewLIne( MC2String& line );
      
   private:
         
      MC2String prompt;
};


#endif // READLINEHELPER_H

