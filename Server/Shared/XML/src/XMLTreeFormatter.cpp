/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "XMLTreeFormatter.h"
#include <util/XMLString.hpp>
#include "XMLUtility.h"
#include <iostream>
#include <util/XMLUniDefs.hpp>
#include <framework/XMLFormatter.hpp>


#ifdef USE_XML

// Print XML-string on stream 
ostream& operator<<(ostream& target, const XMLCh* s)
{
   char *p = XMLUtility::transcodefromucs( s );
   target << p;
   delete [] p;
   return target;
}

// Local class, for printing tree on cout
class DOMPrintFormatTarget : public XMLFormatTarget {
   public:
      DOMPrintFormatTarget( ostream& out ) : m_out( out ) {}
      ~DOMPrintFormatTarget() {} 
   
      ///  Implementations of the format target interface
   virtual void writeChars (
      const XMLByte* const toWrite,
      const unsigned int count,
      XMLFormatter* const formatter ) 
      {
         m_out << (char *) toWrite;
      }


   private:
      ///  Unimplemented methods.
      DOMPrintFormatTarget(const DOMPrintFormatTarget& other);
      void operator=(const DOMPrintFormatTarget& rhs);
      ostream& m_out;
};


// Local class, for printing tree into string
class StringPrintFormatTarget : public XMLFormatTarget {
   public:
      StringPrintFormatTarget()  {m_string.reserve(10000);}
      ~StringPrintFormatTarget() {} 

    ///  Implementations of the format target interface
   virtual void writeChars (
      const XMLByte* const toWrite,
      const unsigned int count,
      XMLFormatter* const formatter ) 
      {
         m_string.append( (char *) toWrite );
      }


      MC2String getString() {
         return m_string;
      }

   private:
      MC2String m_string;

      ///  Unimplemented methods.
      StringPrintFormatTarget(const StringPrintFormatTarget& other);
      void operator=(const StringPrintFormatTarget& rhs);
};


void
XMLTreeFormatter::printTree( DOMNode* doc, ostream& out, 
                             const char* charset ) 
{
   DOMPrintFormatTarget* formatTarget = new DOMPrintFormatTarget( out );
   
   formatTree( formatTarget, doc, charset );

   delete formatTarget;
}

void
XMLTreeFormatter::printIndentedTree( DOMNode* doc, ostream& out, 
                                     const char* charset ) 
{
   DOMPrintFormatTarget* formatTarget = new DOMPrintFormatTarget( out );
   
   formatTree( formatTarget, doc, charset, true );

   delete formatTarget;
}


void
XMLTreeFormatter::formatTree( XMLFormatTarget* formatTarget, 
                              DOMNode* doc, const char* charset,
                              bool indent )
{
   try {
      DOMImplementation* impl = 
         DOMImplementationRegistry::getDOMImplementation( X( "LS" ) ); /* Load Save */
      DOMWriter* writer = ((DOMImplementationLS*)impl)->createDOMWriter();
      // Set some stuff
      writer->setEncoding( X( charset ) );
      if( indent ){
         writer->setFeature( X( "format-pretty-print" ), true );
      }

      // Do it
      writer->writeNode( formatTarget, *doc );

      delete writer;
   } catch ( XMLException& e ) {
      mc2log << error 
             << "An XML error occurred during creation of xml output. "
             << "Msg is: "
             << e.getMessage() << endl;
   } catch( DOMException& e ) {
      mc2log << error 
             << "An DOM error occurred during creation of xml output. "
             << "Msg is: "
             << e.msg << endl;
   }
}


MC2String
XMLTreeFormatter::makeStringOfTree( DOMNode* doc, const char* charset ) {
   StringPrintFormatTarget* formatTarget = new StringPrintFormatTarget();  

   formatTree( formatTarget, doc, charset );

   MC2String res = formatTarget->getString();
   delete formatTarget;

   return res;
}

MC2String XMLTreeFormatter::makeIndentedStringOfTree( DOMNode* doc, 
                                                      const char* charset )
{
   StringPrintFormatTarget* formatTarget = new StringPrintFormatTarget();  

   formatTree( formatTarget, doc, charset, true);

   MC2String res = formatTarget->getString();
   delete formatTarget;

   return res;
}


#endif
