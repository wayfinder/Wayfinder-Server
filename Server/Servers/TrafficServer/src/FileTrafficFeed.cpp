/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "FileTrafficFeed.h"

#include "FileUtils.h"
#include <queue>

struct FileTrafficFeed::Impl: public TrafficFeed {
   Impl( const vector<MC2String>& filenames ) {
      for ( size_t i = 0; i < filenames.size(); ++i ) {
         m_files.push( filenames[ i ] );
      }
   }

   bool eos() const;
   bool getData( Data& data );

   queue< MC2String > m_files;

};

FileTrafficFeed::FileTrafficFeed( const vector<MC2String>& filenames ):
   m_impl( new Impl( filenames ) ) {
}

FileTrafficFeed::~FileTrafficFeed() {
   delete m_impl;
}

bool FileTrafficFeed::eos() const {
   return m_impl->eos();
}

bool FileTrafficFeed::getData( Data& data ) {
   return m_impl->getData( data );
}

bool FileTrafficFeed::Impl::eos() const {
   return m_files.empty();
}

bool FileTrafficFeed::Impl::getData( Data& data ) {
   if ( m_files.empty() ) {
      return false;
   }

   MC2String filename = m_files.front();
   // remove the file from the queue
   m_files.pop();

   mc2log << "[TCPTrafficFeed] Reading file '" << filename << "'"
          << endl;

   char* content = NULL;
   if ( FileUtils::getFileContent( filename.c_str(), content ) ) {
      data += content;
      delete [] content;
   } else {
      mc2log << "[FileTrafficFeed] Failed to load file"
             << " '" << filename << "'. Skipping to next file." << endl;
      return false;
   }

   return true;
}
