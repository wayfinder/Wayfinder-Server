/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef ITEMNAMES_H
#define ITEMNAMES_H

#include "config.h"

#include <map>

#include "DataBuffer.h"
#include "STLStringUtility.h"
#include "VectorIncl.h"

#ifndef _WIN32
  #include "MC2String.h"
  #include <unistd.h>
  #include <stdlib.h>
  #include <errno.h>
#endif

#include <fstream>

/**
  *   Stores all the strings used by one map.
  *   The strings are accessed by one StringVector with pointers to 
  *   all the strings. This vector also constains the total number 
  *   of strings in this object. The actual strings are stored in the
  *   memory in one of these two alternatives:
  *   <ul>
  *      <li>The first stings are stored in one memory-area, theStrings.
  *      <li>The strings that have been added later are stored in
  *          small buffers with one string in each.
  *   </ul>
  *
  */
class ItemNames {
   public:
   friend class M3Creator;
      /**
        *   Default constructor
        */
      ItemNames();

      /**
        *   Create the ItemNames with one string, at position 0. That is
        *   the position for the names that does not excist...
        *   @param   firstName   The first name in the ItemNames (with
        *                        string index 0).
        */
      ItemNames(const char* firstName);

      /**
        *   Constructor that reads the strings from infile
        */
      ItemNames(ifstream& infile);

      /**
       *    Constructor that reads the strings from databuffer.
       */
      ItemNames(DataBuffer& dataBuffer);

      /**
        *   Destructor, that delets all the names.
        */
      virtual ~ItemNames();

      /**
        *   Read data from infile
        *   @param   infile   The file to read from.
        *   @return  True if successfull, false otherwise.
        */
      virtual bool load(ifstream& infile);

      /**
        *   Save data to the databuffer
        *   @param   outfile  The file to write to.
        *   @return  True if successfull, false otherwise.
        */
      bool save(int outfile);

      /**
       *    Get the number of strings in this object.
       *    @return  The number of strings in the stringTable.
       */
      inline uint32 getNbrStrings() const;

      /**
       *    Get the size of the strings in theis string table.
       *    @return  The length of the strings in the stringTable (bytes).
       */
      inline uint32 getStringSize();

      /**
        *   Get a string with a specified.
        *
        *   @param   stringID The identification number of the requested
        *            string.
        *   @return  Pointer to the string. This string may not be changed
        *            nor freed. NULL if unsuccessfull.
        */
      inline char* getString(uint32 stringID);

      /**
        *   Add a new string to the stringtable.
        *
        *   @param   str         The string that should be added to 
        *                        the stringtable.
        *   @param   changeCase  Optional parameter that, if set to true,
        *                        changes the case of the incomming string
        *                        to mixed case (makeFirstInWordCapital in
        *                        the class StringUtility).
        *   @return  The identity number of the added string. If the string
        *            allready exists in the database the index to the
        *            existing string is returned.
        */
      uint32 addString(const char* str, bool changeCase = false);

      /**
       *   Requires that createIndex has been run first.
       *   @param   str         The string that should be added to 
       *                        the stringtable.
       *   @param   changeCase  Optional parameter that, if set to true,
       *                        changes the case of the incomming string
       *                        to mixed case (makeFirstInWordCapital in
       *                        the class StringUtility).
       *   @return  The identity number of the added string. If the string
       *            allready exists in the database the index to the
       *            existing string is returned.
       */
      uint32 quickAddString(const char* str, bool changeeCase = false);

      /**
       *   Creates index over strings for faster adding.
       */
      void createIndex();
      
      
      /**
        *   Find out if a certain string excists or not. Since the strings
        *   not nessecery are sorted, a linear search is performed.
        *   
        *   @param   str   String to compare with the stringtable.
        *   @return  The index of str if already in the stringtbale, 
        *            MAX_UINT32 will be returned otherwise;
        */
      uint32 stringExist(const char* str);
      
      /**
        *   @return  The address of the start of the strings (inlined).
        */
      inline char* getStringStart();

      /**
        *   @name Methods to retreive the strings.
        *
        *   Theses methods should be used to retreive all the strings.
        */
      //@{
         /**
          *    Get the number of blocks. Usage:
          *    @code
               ItemNames* in = X;
               for (uint32 i=0; i<in->getNbrBlocks(); i++)
                  write(fd, in->getBlockStart(i), in-getBlockSize(i));
          *    @endcode
          *    @return  The number of string blocks in memory
          */
         inline uint32 getNbrBlocks() const;

         /**
          *    Get the size of one block.
          *    @param   blockNbr The block whos size should be returned.
          *    @return  The size of block blockNbr;
          */
         inline uint32 getBlockSize(uint32 blockNbr) const;

         /**
          *    Get a pointer to the first string in one of the blocks.
          *    @param   blockNbr The wanted block.
          *    @return  A pointer to the block.
          */
         inline char* getBlockStart(uint32 blockNbr) const;
      //@}
        
      /**
        *   @name Methods mainly for debugging and statistics.
        */
      //@{
         /**
           *   Print all strings to standard out.
           */
         void printAllStrings();
      //@}

      /**
       *    Get the memory usage of this object.
       *    @return The number of bytes used by this object.
       */
      virtual uint32 getMemoryUsage() const;


      /**
       *    Load the strings from a databuffer.
       *    @param buffer  Databuffer with the string-data.
       *    @param utf8InMaps True if the maps are encoded with utf-8.
       *    @return  True if the strings are loaded, false otherwise.
       */
      virtual bool internalLoad(DataBuffer& buffer,
                                bool utf8InMaps = false);


   protected:
      /**
        *   Initiate the membervariables. This should only be called 
        *   by the constructors.
        */
      void init();
   
      /**
        *   Vector with pointers to all the strings.
        */
      StringVector stringTable;
   
      /**
        *   Memoryarea for the first nbrBlockedStrings strings,
        *   pointed to in stringTable.
        */
      char *theStrings;

      /**
        *   Number of strings that lies in the big memory block.
        */
      uint32 nbrBlockedStrings;

      /**
        *   The size in bytes of strings in the big stringTable memory
        *   block.
        */
      uint32 sizeOfBlock;

      /**
        *   The size in bytes of all strings contained in this object.
        */
      uint32 sizeOfStrings;

      /**
       *    Pointer to the index of strings to be used for faster
       *    adding. The string is in first and index in second.
       */
      map<const char*,uint32,STLStringUtility::ltstr>* m_strIndex;

  private:

      /**
       *    Return a copy of the stringtable. {\it {\bf NB!} Since the
       *    array that is returned is allocated here it must be deleted
       *    by the caller. The strings that the elements in the returned
       *    array points at must also be deleted!}
       *    @return A new array with all the strings in this object.
       */
      char** getCopyOfStringTable() const;

      /**
       *    Find out if the strings are fragmented or not.
       *    @return  True if the strings are fragmented, false otherwise.
       */
      bool isFragmented();
      
      /**
       *    Fragments the strings, ie. moves the strings from the blocked
       *    area to be seperate strings instead. Is called upon loading.
       *    @return  True upon success, false otherwise.
       */
      bool fragStrings();

     /**
       *    Exchange a string in the stringtable with a new one.
       *    @warning Make absolutely sure that you know what you are doing 
       *             when you call this method! Make sure that all items 
       *             that has this name would like it to be exchanged with 
       *             the new one.
       *    @note    Only strings that are @b not blocked could be
       *             exchanged!
       *
       *    @param   strIdx   The stringindex to be exchanged with a new
       *                      string.
       *    @param   newName  The new name.
       *    @return  True if the exchanging was succesful, otherwise
       *             false (could happen if the name was in the blocked
       *             strings).
       */
      bool exchangeString(uint32 strIdx, const char* newName);


      /**
       *    To be able to call exchangeString and fragStrings.
       */
      friend class ExtraDataUtility;
      

};


// ==================================================================
//                            Implementation of the inlineded methods

inline uint32
ItemNames::getNbrStrings() const {
   return stringTable.getSize();
}

inline uint32
ItemNames::getStringSize() {
   return sizeOfStrings;
}

inline char*
ItemNames::getString(uint32 stringID) {
   if (stringID >= getNbrStrings())
      return (NULL);
   else
      return ((char*) stringTable.getElementAt(stringID));
}

inline char*
ItemNames::getStringStart() {
   return theStrings;
}


inline uint32 
ItemNames::getNbrBlocks() const 
{
   return getNbrStrings() - nbrBlockedStrings + 1;
}

inline uint32 
ItemNames::getBlockSize(uint32 blockNbr) const 
{
   if (blockNbr == 0)
      return sizeOfBlock;
   else
      return strlen( (char*) stringTable.
                     getElementAt(blockNbr+nbrBlockedStrings-1));
}

inline char* 
ItemNames::getBlockStart(uint32 blockNbr) const 
{
   if (blockNbr == 0)
      return theStrings;
   else
      return (char*) stringTable.
               getElementAt(blockNbr+nbrBlockedStrings-1);
}

#endif

