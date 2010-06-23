/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef READLINE_H
#define READLINE_H

#include <libtecla.h>

/**
 *   Wrapper for libtecla, a command-line editor library.
 *   This only includes basic functionality, could later be
 *   extended to handle completion, etc.
 *
 */
class Readline
{
   public:
      /**
        *   Initializes the library
        *   @param prompt Pointer to string to use as prompt
        *   @param history Set to true if you want history, false if you don't
        *   @param histFile Set to a filename if you want the history
        *                   saved/reused, NULL otherwise.
        */
      Readline(const char* prompt, bool history, const char* histFile);

      /**
        *   Cleans up
        */
      ~Readline();

      /**
        *   Get new user input.
        *   @return Pointer to new input
        */
      const char* getInput();

      /**
        *   Clear the history
        */
      void clearHistory();


      /**
       * Set the prompt.
       */
      void setPrompt( const char* prompt );
   /// sets io mode either blocking or non blocking
   void setBlocking( bool status );

   private:

      /**
        * The log level we're using
        */
      int m_level;

      /**
        * History flag
        */
      bool m_history;

      /**
        * History file path
        */
      const char* m_histFile;

      /**
        * The prompt used
        */
      const char* m_prompt;

      /**
        * Last input
        */
      char* m_input;

      /**
        * The tecla handle/instance
        */
      GetLine* m_getLine;
};

#endif // READLINE_H
