/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef LINEPARSER_H
#define LINEPARSER_H

#include "config.h"
#include "MC2String.h"
#include <iostream>

/**
 * Interface for parsing lines.
 */
class LineParser {
public:
   /// Data on line and its line number
   class Line {
   public:
      Line( const MC2String& line,
            uint32 lineNumber):
         m_line( line ),
         m_lineNumber( lineNumber ) {
      }
      /// @return line data at the line number
      const MC2String& getLine() const {
         return m_line;
      }
      /// @return line number 
      uint32 getLineNumber() const {
         return m_lineNumber;
      }
   private:
      MC2String m_line; ///< Data on the current line
      uint32 m_lineNumber; ///< Line number, line number starts a 1
   };

   /// @param tag the tag that the line should start with
   explicit LineParser( const MC2String& tag ):
      m_tag( tag ) { }

   virtual ~LineParser() {
   }
      
   /**
    * Parse a line.
    * @param line the line to parse.
    * @return true if the line was parsed successfully.
    */
   virtual bool parseLine( const Line& line ) = 0;

   /// @return initial tag of this parser
   inline const MC2String& getTag() const;

   /// @return true if the line has a tag that matches this parser
   inline bool isTagLine( const Line& line ) const;

   /// @return true if the line has a tag that matches this parser
   inline bool isTagLine( const MC2String& line ) const;
private:
   /// Initial tag of the line 
   MC2String m_tag;
};

inline 
const MC2String& LineParser::getTag() const {
   return m_tag;
}

inline
bool LineParser::isTagLine( const Line& line ) const {
   return isTagLine( line.getLine() );
}

inline
bool LineParser::isTagLine( const MC2String& line ) const {
   // for old compilers
   return strncmp( getTag().c_str(), line.c_str(), getTag().size() ) == 0;
}

inline ostream& operator << ( ostream& ostr, const LineParser::Line& line ) {
   ostr << "(#" << line.getLineNumber() << "): " << line.getLine() << endl;
   return ostr;
}


#endif // LINEPARSER_H
