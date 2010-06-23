/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef STEP_LOGGER_H
#define STEP_LOGGER_H

#include "config.h"
#include "MC2String.h"

// Forward
class DebugClock;

/**
 * Class for logging steps and times it takes to do them.
 */
class StepLogger {
public:
   /**
    * Constructor.
    */
   StepLogger();

   /**
    * Destructor.
    */
   virtual ~StepLogger();

   /// The step and time it took to do it.
   typedef vector< pair< MC2String, uint32 > > stepTimes;

   /**
    * Get the list of steps and the times it took to do them.
    *
    * @return The list of setup steps and times.
    */
   const stepTimes getStepTimes() const;

   /**
    * Add the list of steps and the times it took to do them as
    * a comma separated list.
    * 
    * @param out The stream to print to.
    * @return Reference to out.
    */
   ostream& printSteps( ostream& out ) const;

   /**
    * Set if to print steps on destruction.
    */
   void setPrintOnDestruction( bool b );

   /**
    * Turn on logging.
    */
   void setLogging( bool on );

   /**
    * Set the logg prefix
    */
   void setLoggPrefix( const MC2String& prefix );

protected:
   /**
    * Add a step to the log for times it takes to do.
    */
   void addStepTime( const MC2String& step, const DebugClock& time );

   /**
    * Add a step to the log for times it takes to do.
    */
   //void addStepTime( const MC2String& step, uint32 time );

private:
   /**
    * The time for the steps.
    */
   stepTimes m_stepTimes;

   /**
    * If to dbg steps on destruction.
    */
   bool m_printInDestructor;

   /**
    * If logging at all.
    */
   bool m_logging;

   /// The logging prefix
   MC2String m_prefix;
};

#endif // STEP_LOGGER_H
