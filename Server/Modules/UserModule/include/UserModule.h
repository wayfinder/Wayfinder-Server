/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef USER_MODULE_H
#define USER_MODULE_H

#include "Module.h"

class Queue;
/**
 *  The UserModule
 *
 */
class UserModule: public Module
{
   public:
     
      /**
        *  Initializes the Module
        *  @param argc  Number of command line arguments
        *  @param argv  Array with the command line arguments
        *  @param noUserCache If not to use user caching.
        *  @param noSqlUpdate If not to update sql tables.
        */
      UserModule( int argc, char* argv[] );
      
      /**
        *  Cleans up
        */
      virtual ~UserModule();

      /**
        *  The module's initialization function
        */
      virtual void init();

      /**
        *  Handle a command from the user
        *  @param cmd Pointer to string with the command
        *  @return Integer reporting what happened; 0: error ocurred
        *          1: everything ok, -1: quit now
        */
      int handleCommand(const char* input);

   private:

      bool addUsers(const char* fileName, Queue *queue, bool allok);

      /// User add filename
      char m_userAddFilename[4096];

      /// flag to add users only if all users OK to add
      bool m_onlyUsersAllOK;


      /// If not to use user cache.
      bool m_noUserCache;


      /// If not to update sql tables.
      bool m_noSqlUpdate;
};

#endif // USER_MODULE_H
