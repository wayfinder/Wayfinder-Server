/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef CSV_READER_H
#define CSV_READER_H

#include "config.h"
#include "MC2String.h"

/**
 * A record in a csv file.
 */
class CSVRecord : public vector<MC2String> {
};

/**
 * Reader of CSV, comma separated values, files.
 * If needed add settings for separator and alternative field escapings.
 */
class CSVReader : public vector<CSVRecord> {
public:
   /**
    * Constructs a new CSVReader.
    */
   CSVReader();

   /**
    * Decronstructs the CSVReader.
    */
   virtual ~CSVReader();

   /**
    * Read a csv file.
    * This removes any old content.
    *
    * @param file The path to the file to read.
    * @return 0 if file read ok, -1 if failed to read file, -2 if failed
    *         to parse the file content as a csv file.
    */
   int readFile( const MC2String& file );
};

#endif // CSV_READER_H

