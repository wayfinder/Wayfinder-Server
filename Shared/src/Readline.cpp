/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "Readline.h"
#include <stdio.h>
#include <stdlib.h>

Readline::Readline(const char* prompt, bool history, const char* histFile)
{
   m_input = NULL; 
   m_history = history;
   m_histFile = histFile;
   m_prompt = prompt;
   m_getLine = new_GetLine(2048, history ? 32768 : 0);
   if (m_histFile != NULL)
      gl_load_history(m_getLine, m_histFile, "#");
}

Readline::~Readline()
{
   if (m_histFile != NULL)
      gl_save_history(m_getLine, m_histFile, "#", -1);
   m_getLine = del_GetLine(m_getLine);
}

void
Readline::clearHistory()
{
   gl_clear_history(m_getLine, 1);
}


void
Readline::setPrompt( const char* prompt ) {
   m_prompt = prompt;
}


const char*
Readline::getInput()
{
   m_input = gl_get_line(m_getLine, m_prompt, NULL, -1);

   return m_input;
}


void Readline::setBlocking( bool status ) {
   /* Implement when version is 1.6.1
   if ( ! status ) {
      gl_io_mode( m_getLine, GL_SERVER_MODE );
   } else {
      gl_io_mode( m_getLine, GL_NORMAL_MODE );
   }
*/
}
