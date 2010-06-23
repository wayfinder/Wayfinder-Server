/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef HTTPHEADER_H
#define HTTPHEADER_H

// Includes
#include "config.h"
#include <time.h>

#include "MC2String.h"
#include <map>
#include "STLStrComp.h"

class TCPSocket;

/** 
 *    Http Header, represents a HttpHeader.
 *
 */
class HttpHeader {
   public:
      /** 
       * Creates a new HttpHeader and parses Header to header lines.
       * @param Header is the raw header.
       */
      HttpHeader(MC2String* Header);


      /**
       * Constructs a new empty header.
       */
      HttpHeader();


      /**
       * Deletes any content of the header.
       */
      virtual ~HttpHeader();
   

      /** 
       * Returns the Header-value of the Line starting with "HeaderType:"
       * Null if no such header type.
       * @param HeaderType The type of header, eg. "Content-Type".
       *                   HeaderType does NOT contain the ':'.
       */
      const MC2String* getHeaderValue(const MC2String* HeaderType) const;
      

      /** 
       * Returns the Header-value of the Line starting with "HeaderType:"
       * Null if no such header type.
       * @param HeaderType The type of header, eg. "Content-Type".
       *                   HeaderType does NOT contain the ':'.
       */
      const MC2String* getHeaderValue(const char* HeaderType) const;


      /**
       * Returns the Header-value of the Line starting with "HeaderType:"
       * Null if no such header type.
       *
       * @param HeaderType The type of header, eg. "Content-Type".
       *                   HeaderType does NOT contain the ':'.
       */
      const MC2String* getHeaderValue( const MC2String& HeaderType ) const;

      /**
       *   Returns the date in the specified header or 0 if not
       *   parsable or nonexistant.
       */
      time_t getHeaderDateValue( const MC2String& headername ) const;

      /**
       *   Sets a header date value using the correct http date string
       *   format.
       *   If addHeaderLine overwrites old data, then this one does too.
       */
      void addHeaderDateValue( const MC2String& headername,
                               time_t timeVal );
   
      /**
       *   Returns the charset that a client accepts.
       *   If not found "iso-8859-1" is returned.
       *   FIXME: Should get a prioritized list of valid charsets to
       *          choose from.
       */
      const MC2String& getAcceptCharset() const;
      

      /**
       * Returns the ContentLength of the Header, 0 if no or empty body. 
       */
      uint32 getContentLength() const;
      
      
      /**
       * Sets the ContentLength of the body,
       * overwrites prevoius value.
       * @param Length is the new length of this header, doesn't change the
       * actual length of the header.
       */
      void setContentLength(uint32 Length);
      
      
      /** 
       *  Add the headerline HeaderType + ': ' + HeaderValue to the header.
       *  It overwrites any old value of HeaderType.
       *  The string HeaderValue points to may not be deleted.
       *  This function is used by HttpServer.
       *
       *  @param headerType is a string representing the type 
       *                    of the header line.
       *  @param headerValue is the value of the headerline.
       *  @param appendMultipleHeaders If to append if old value, default
       *                               false.
       */
      void addHeaderLine( const MC2String* headerType, 
                          MC2String* headerValue,
                          bool appendMultipleHeaders = false );

      /** 
       *  Add the headerline HeaderType + ': ' + HeaderValue to the header.
       *  It overwrites any old value of HeaderType.
       *  The string HeaderValue points to may not be deleted.
       *  This function is used by HttpServer.
       *  @param HeaderType is a string representing the type 
       *                    of the header line.
       *  @param HeaderValue is the value of the headerline.
       *  @param appendMultipleHeaders If to append if old value, default
       *                               false.
       */
      void addHeaderLine( const MC2String& headerType,
                          const MC2String& headerValue,
                          bool appendMultipleHeaders = false );

      /** 
       *  Add the headerline HeaderType + ': ' + HeaderValue to the header.
       *  It overwrites any old value of HeaderType.
       *  The string HeaderValue points to may not be deleted.
       *  This function is used by HttpServer.
       *  @param HeaderType is a string representing the type 
       *                    of the header line.
       *  @param HeaderValue is the value of the headerline.
       *  @param appendMultipleHeaders If to append if old value, default
       *                               false.
       */
      void addHeaderLine( const char* headerType,
                          const char* headerValue,
                          bool appendMultipleHeaders = false );

      /**
       *   Parses the zero-terminated inLine and adds it to the header.
       *   (For use with lines such as Content-type: dfsak/kfja).
       *   If it finds content-length it sets m_contentLength
       *   Uses addHeaderLine(MC2String*, MC2String*)
       *   This function is used by NavigatorServer.
       *
       *   @param inLine The line to be added.
       *   @param appendMultipleHeaders If to append if old value, default
       *                                false.
       *   @return True if the line could be parsed.
       */
      bool addHeaderLine( const char* inLine,
                          bool appendMultipleHeaders = false );

      /**
       *   Parses the requestLine of the header.
       *   @param inLine The requestline.
       *   @return True if it seems ok.
       */
      bool addRequestLine(const char* inLine);
      
      /** 
       * Deletes the headerline of type HeaderType.
       * Returns true if any line was deleted, false otherwise.
       * @param HeaderType is the header-type to be deleted.
       */
      bool deleteHeaderLine( const MC2String* HeaderType );
      
      
      /**
       * Sets the request/status line of the header.
       * Deletes any old value and replases with the new pointer.
       * Like "GET / HTTP/1.0" or "HTTP 200 Ok".
       * The string startLine is handled and deleted by HttpHeader.
       * @param Line the new startline.
       */
      void setStartLine(MC2String* Line);
      
      
      /**
       * Sets the reply status line of the header.
       * Only for replies.
       *
       */
      void setStartLine( uint32 code );
      
      /**
       * Get the version as string.
       */
      const char* getCurrHTTPVer() const;

      /** 
       *   returns the current startline.
       *   Do not delete the string, deleted with the header.
       *   CR+LF is included in the string.
       */
      const MC2String* getStartLine() const;
      
     
      /**
       * The startlines result code.
       * @return The startlines result code.
       */
      uint32 getStartLineCode() const;


      /** 
       * Writes the header on the socket.
       * @param sock is the socket to write the header on.
       */
      void writeHeader(TCPSocket* sock) const;


      /**
       * Appends the header to a string.
       * @param headerString is the string to append the header to.
       */
      void putHeader(MC2String* headerString) const;
      
      void replacePagename( const MC2String& newPagename );
      /**
       * Empties the Header of any data
       */
      void clear();
      
      
      /**
       * Sets the URL of the Header
       * The string newURLPath is handled and deleted by HttpHeader.
       * @param newURLPath is the new URL for this header.
       */
      void setURLPath(MC2String* newURLPath);
      

      /** 
       * Returns the current URLPath. 
       */
      const MC2String* getURLPath() const;


      /**
       * Sets the pagename of the Header
       * The string newPagename is handled and deleted by HttpHeader.
       * @param newPagename is the new name of the page.
       */
      void setPagename(MC2String* newPagename);

      
      /**
       * Returns the current Pagename
       */
      const MC2String* getPagename() const;
      

      /**
       * Parses a "HTTP/1.X YYY STR" line and returns YYY. 
       *
       * @param data The HTTP responce line.
       * @return Resultcode in HTTP responce, 0 if no code.
       */
      static uint32 parseHTTPResponeLine( const char* data );

      /**
       *   Enum describing the method types.
       */
      enum MethodType {
         GET_METHOD,
         POST_METHOD,
         HEAD_METHOD, // HEAD was already defined
         OPTIONS_METHOD,
         PUT_METHOD,
         DELETE_METHOD,
         TRACE_METHOD,
         CONNECT_METHOD,

         NO_MATCH
      };

      /**
       *   @return The methodtype of this request.
       */
      MethodType getMethod() const;


      /**
       * Set the methodtype of this request.
       * @param method The methodtype of this request.
       */
      void setMethod( MethodType method );


      /**
       * @return The methodtype of this request as a string.
       */
      const char* getMethodString() const;


      /**
       * Get the major version.
       */
      uint32 getMajorVersion() const;


      /**
       * Get the minor version.
       */
      uint32 getMinorVersion() const;


      /**
       * Set the HTTP version.
       */
      void setHttpVersion( uint32 major, uint32 minor );


      /**
       * Counts strings in the string
       * @param str The string to count NULL-bytes in.
       * @return The number of strings in str.
       */
      static uint32 nbrStrings( const MC2String* str );

      
      /**
       * Pointer to the first string in str.
       * @param str The string.
       * @param pos Set to the starting position of the first string
       *            in str.
       */
      static const char* getFirstStringIn( const MC2String* str, 
                                           uint32& pos );


      /**
       * Pointer to the next string after pos in str.
       * @param str The string to work with.
       * @param pos The current position in str is updated to the starting
       *            position of the next string in str.
       * return Pointer to the next string in str or NULL if no more
       *        strings in str.
       */
      static const char* getNextStringIn( const MC2String* str, uint32& pos );


      /**
       * Pointer to the index:th string in str, NULL if no such string
       * @param index The index of the string to return.
       * @param str The string to work with.
       * @return Pointer to the index:th string in str or NULL if no such
       *         string in str.
       */
      static const char* getStringIn( uint32 index, const MC2String* str );

      /**
       *   Prints the header on the stream.
       *   @param ostr   Stream to print on.
       *   @param header Header to print.
       */
      friend ostream& operator<<( ostream& ostr, const HttpHeader& header );

      /**
       *   Defines a HeaderMap
       */
      typedef map<MC2String, MC2String*, strNoCaseCompareLess> HeaderMap;

      /**
       *   Get the header map.
       */
      const HeaderMap& getHeaderMap() const;

      typedef map< uint32, pair< MC2String, MC2String > > httpResponsMap;

      /**
       * Get the start line for a status code.
       */
      static const MC2String& getHttpStartLine( uint32 code );

      /**
       * Get the header and body for a status code.
       */
      static bool getHttpErrorPage( uint32 code, const MC2String*& header,
                                    const MC2String*& body );
      
  private:

      /**
       *   Reset cached values.
       */
      void resetCached();

      /** 
       * Parse a Header
       * @param Header is the header to parse.
       */
      void parseHeader(MC2String* Header);

      /**
       *   Convert a method to MethodType. E.g. "GET" to
       *   MethodType::GET_METHOD.
       *   @return The corresponding method for a string
       *           containing the method as text.
       */
      MethodType findMethod(const char* method) const;

      /**
       * Makes an httpResponsMap.
       */
      static httpResponsMap initHttpResponsMap();

      /**
       * The map with http error codes, header and body, as strings.
       */
      static httpResponsMap httpResponses;
      
      /** The original Header string, unchanged */
      MC2String* m_header;

      /** The header line with content-length */
      uint32 m_contentLength;

      /** The URL of the header */
      MC2String* m_URLPath;

      /** The Pagename of the header */
      MC2String* m_pagename;

      /** The request/status-line INCLUDING CR+LF */
      MC2String* m_startLine;

      /** 
       * The header line are stored in a map for easy lookup and access.
       */
      HeaderMap m_Headers;

      /** The method of the httpheader */
      MethodType m_method;

      /**
       *   The accept-charset is stored here.
       */
      MC2String m_acceptCharset;

      /**
       * The Major Version Number.
       */
      uint32 m_majorVersion;


      /**
       * The Minor Version Number.
       */
      uint32 m_minorVersion;


      /** Tabletype for looking up the methods */
      struct methodTable_t {
         const char* methodString;
         MethodType method;
      };

      /** The table for looking up the request-line methods e.g. GET */
      static const methodTable_t methodTable[];
};


// -----------------------------------------------------------------------
//                                     Implementation of inlined methods -


inline const MC2String*
HttpHeader::getHeaderValue( const MC2String& HeaderType ) const {
   return getHeaderValue( &HeaderType );
}


inline const HttpHeader::HeaderMap&
HttpHeader::getHeaderMap() const {
   return m_Headers;
}

inline void
HttpHeader::setHttpVersion( uint32 major, uint32 minor ) {
   m_majorVersion = major;
   m_minorVersion = minor;
}


#endif // HTTPHEADER_H
