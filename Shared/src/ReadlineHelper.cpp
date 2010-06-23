/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "ReadlineHelper.h"
#include <stdlib.h>


ReadlineHelper::ReadlineHelper( const char* prompt, bool history, 
                                const char* histFile )
      : Readline( prompt, history, histFile )
{
}


ReadlineHelper::~ReadlineHelper() {
}


void
ReadlineHelper::setPromptWithDefault( const char* name, const char* def ) {
   prompt = name;

   if ( def != NULL && *def != '\0' ) {
      prompt.append( " [" );
      prompt.append( def );
      prompt.append( "]: " );
   } else {
      prompt.append( ": " );
   }

   setPrompt( prompt.c_str() );
}


void
ReadlineHelper::trimEndOfNewLIne( MC2String& line ) {
   while( line.size() > 0 && 
          (line[ line.size() -1 ] == '\r' ||
           line[ line.size() -1 ] == '\n' ) )
   {
      line.erase( line.size() -1 );
   }
}


MC2String 
ReadlineHelper::getText( const char* name, const char* def ) {
   setPromptWithDefault( name, def );
   bool gotIt = false;
   MC2String line;
   while ( !gotIt ) {
      line = getInput();
      trimEndOfNewLIne( line );
      if ( line[ 0 ] == '\0' ) {
         line = def;
      }
      gotIt = true;
   }

   return line;
}


uint32
ReadlineHelper::getLong( const char* name, const char* def  ) {
   setPromptWithDefault( name, def );

   bool gotIt = false;
   MC2String line;
   uint32 res = 0;
   while ( !gotIt ) {
      line = getInput();
      trimEndOfNewLIne( line );
      if ( line[ 0 ] == '\0' ) {
         line = def;
      }

      char* tmp = NULL;
      errno = 0;
      res = strtoul( line.c_str(), &tmp, 0 );
      if ( tmp != NULL && tmp != line.c_str() && *tmp == '\0' ) {
         gotIt = true;
      } else {
         cout << "Error in input [" << line << "] " << endl;
      }
      if ( res == MAX_UINT32 && errno == ERANGE ) {
         gotIt = false;
         cout << "Error long too big [" << line << "] " << endl;
      }
   }

   return res;
}


uint16
ReadlineHelper::getShort( const char* name, const char* def ) {
   setPromptWithDefault( name, def );

   bool gotIt = false;
   MC2String line;
   uint32 res = 0;
   while ( !gotIt ) {
      line = getInput();
      trimEndOfNewLIne( line );
      if ( line[ 0 ] == '\0' ) {
         line = def;
      }

      char* tmp = NULL;
      res = strtoul( line.c_str(), &tmp, 0 );
      if ( tmp != NULL && tmp != line.c_str() && *tmp == '\0' ) {
         gotIt = true;
      } else {
         cout << "Error in input [" << line << "] " << endl;
      }
      if ( res > MAX_UINT16 ) {
         gotIt = false;
         cout << "Error short too big [" << line << "] " << endl;
      }
   }

   return res;
}


byte
ReadlineHelper::getByte( const char* name, const char* def  ) {
   setPromptWithDefault( name, def );

   bool gotIt = false;
   MC2String line;
   uint32 res = 0;
   while ( !gotIt ) {
      line = getInput();
      trimEndOfNewLIne( line );
      if ( line[ 0 ] == '\0' ) {
         line = def;
      }

      char* tmp = NULL;
      res = strtoul( line.c_str(), &tmp, 0 );
      if ( tmp != NULL && tmp != line.c_str() && *tmp == '\0' ) {
         gotIt = true;
      } else {
         cout << "Error in input [" << line << "] " << endl;
      }
      if ( res > MAX_BYTE ) {
         gotIt = false;
         cout << "Error byte too big [" << line << "] " << endl;
      }
   }

   return res;
}


vector< byte >
ReadlineHelper::getByteArray( const char* name, const char* def  ) {
   MC2String p( name );
   p.append( ". End with \"..\"" );
   setPromptWithDefault( p.c_str(), def ); 
   vector< byte > b;
   
   bool done = false;
   MC2String line;
   uint32 res = 0;
   while ( !done ) {
      line = getInput();
      trimEndOfNewLIne( line );
      if ( line[ 0 ] == '\0' ) {
         line = def;
      } else if ( line == ".." ) {
         done = true;
         continue;
      }

      char* tmp = NULL;
      res = strtoul( line.c_str(), &tmp, 0 );
      if ( tmp != NULL && tmp != line.c_str() && *tmp == '\0' ) {
      } else {
         cout << "Error in input [" << line << "] " << endl;
      }
      if ( res > MAX_BYTE ) {
         cout << "Error byte too big [" << line << "] " << endl;
      } else {
         b.push_back( byte( res ) );
      }
   }

   return b;
}

