/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "StepLogger.h"
#include "DebugClock.h"

StepLogger::StepLogger() 
      : m_printInDestructor( false ), m_logging( false ),
      m_prefix( "[StepLogger]" )
{
}

StepLogger::~StepLogger() {
   if ( m_printInDestructor && m_logging ) {
      mc2log << m_prefix;
      this->printSteps( mc2log );
      mc2log << endl;
   }
}

const StepLogger::stepTimes
StepLogger::getStepTimes() const {
   return m_stepTimes;
}

ostream& 
StepLogger::printSteps( ostream& out ) const {
   for ( size_t i = 0, end = m_stepTimes.size() ; i < end ; ++i ) {
      if ( i != 0 ) {
         out << ", ";
      }
      out << m_stepTimes[ i ].first << "=" << m_stepTimes[ i ].second << "ms";
   }
   return out;
}

void
StepLogger::setPrintOnDestruction( bool b ) {
   m_printInDestructor = b;
}

void
StepLogger::setLogging( bool on ) {
   m_logging = on;
}

void
StepLogger::setLoggPrefix( const MC2String& prefix ) {
   m_prefix = prefix;
}

void
StepLogger::addStepTime( const MC2String& step, const DebugClock& time ) {
   //mc2dbg << "Step: " << step << "=" << time << endl;
   if ( m_logging ) {
      m_stepTimes.push_back( make_pair( step, time.getTime() ) );
   }
}

/*
void
StepLogger::addStepTime( const MC2String& step, uint32 time ) {
   if ( m_logging ) {
       m_stepTimes.push_back( make_pair( step, time ) );
   }
}
*/
