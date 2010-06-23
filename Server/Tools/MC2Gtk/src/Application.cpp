/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "Application.h"

#include "XMLUtility.h"
#include <xercesc/util/PlatformUtils.hpp>
#include <gtk/gtk.h>

namespace MC2Gtk {

Application* Application::s_instance = NULL;

Application::Application( int& argc, char**& argv,
                          unsigned int flags ) throw (Exception):
m_flags( flags ) {

   if ( s_instance != NULL ) {
      throw MC2Gtk::Exception( "Can not have more than one instance"
                               " of MC2Gtk::Application!" );
   }

   gtk_init( &argc, &argv );
   mc2log << "MC2Gtk::Application Flags: " << flags << endl;
   if ( flags & Application::INIT_XML ) {
      mc2log << "init xml" << endl;
      try {
         XMLPlatformUtils::Initialize();
      } catch(const XMLException &toCatch) {
         throw MC2Gtk::
            Exception( MC2String("Could not initialize XML subsystem.")
                       + XMLUtility::transcodefrom( toCatch.getMessage() ) );
      }
   }

   s_instance = this;
}

Application::~Application() {
  if ( m_flags & Application::INIT_XML ) {
     XMLPlatformUtils::Terminate();
  }
}

void Application::loop() {
   gtk_main();
}

void Application::end() {
   gtk_main_quit();
}

}
