/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef COMPONENT_H
#define COMPONENT_H

#include "config.h"

#include "ISABThread.h"
#include "SelectableSelector.h"
#include "NotifyPipe.h"

#include <memory>

class CommandlineOptionHandler;

/**
 * Exception for component
 */
class ComponentException: public std::exception {
public:
   explicit ComponentException( const MC2String& str) throw():
      m_str( "Component: " + str ) { }
   virtual ~ComponentException() throw() { }
   const char* what() const throw() { return m_str.c_str(); }
private:
   MC2String m_str;
};

/**
 *  Base class for an MC2 Component
 *
 */
class Component {
public:
   /**
    * Initialize component with specific command line option handler
    * @param componentName
    * @param handler The command line option handler to add new options for.
    */
   Component( const char* componentName,
              CommandlineOptionHandler* handler );

   /**
    *  Initializes the Component
    *  @param componentName String containing the name of this Component
    *  @param argc  Number of command line arguments
    *  @param argv  Array with the command line arguments
    */
   Component(const char* componentName, int argc, char* argv[]);
      
   /**
    *  Cleans up
    */
   virtual ~Component();

   /**
    * Parse the command line.
    * Throws exception.
    */
   virtual void parseCommandLine() throw ( ComponentException );

   /**
    *  The component's initialization function
    */
   virtual void init();

   /**
    *  Handle a command from the user
    *  @param input Pointer to string with the command
    *  @return Integer reporting what happened; 1: error ocurred
    *          1: everything ok, -1: quit now
    */
   virtual int handleCommand(const char* input);

   /**
    * Print help for commands in subclasses.
    */
   virtual void helpCommands();

   /**
    *  Handles user interaction via stdin/stdout, calls handleCommand when
    *  appropriate, and waits for all threads to terminate after shutdown. 
    *  Should be called by main() after creation and start of the 
    *  Component.
    *  @param init Reference to the program's ISABThreadInitialize
    */
   void gotoWork(ISABThreadInitialize& init);

   NotifyPipe& getShutdownPipe() { return m_shutdownPipe; }

protected:
   CommandlineOptionHandler& getCommandlineOptionHandler() {
      return *m_cmdlineOptHandler;
   }
   /**
    *   Splits a string into a vector of strings using ' ' as separator
    *   Handles quotes to some degree.
    *   @param result A string for each part of the command, like argv.
    *   @param input  Input from command line as in handleCommand.
    *   @return "argc"
    */
   static int splitCommand( vector<MC2String>& result, const char* input );
      
   /// This component's command line option handler
   std::auto_ptr<CommandlineOptionHandler> m_cmdlineOptHandler;

   /// The name of this component
   MC2String m_componentName;

   /// flag set to true when the component is shutting down
   bool m_shutdown;

   /// command line parameter "-N", "--noreadline", don't use
   /// ReadLine for the the command line interface, for compatibility
   /// reasons
   bool m_noReadLine;

   /// start time, UNIX timestamp
   uint32 m_startTime;


private:
   /// sets up the command line option handler and properties.
   void setup();

   SelectableSelector m_selector;

   NotifyPipe m_shutdownPipe;

};

#endif // COMPONENT_H
