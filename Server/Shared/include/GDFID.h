/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef GDFID_H
#define GDFID_H

#include "config.h"
#include <map>

/**
  *   This class keeps a full GDF ID, i.e. dataset ID, section ID and 
  *   feature ID.
  *
  */
class GDFID
{
 public:
   
   /**
    * Constructor.
    */
   GDFID(uint32 datasetID, uint32 sectionID, uint32 featureID):
      m_featureID(featureID),
      m_sectionID(sectionID),
      m_datasetID(datasetID)
      {};

   /**
    * Default constructor. Results in an invalid GDFID.
    */
   GDFID():    
      m_featureID(MAX_UINT32),
      m_sectionID(MAX_UINT32),
      m_datasetID(MAX_UINT32)
      {};
   

   /**
    * Destructor.
    */
   ~GDFID(){};
   
   /**
    * Compares two GDFID objects.
    */
   int lessThan( const GDFID& gdfID) const;
   
   /**
    * Get and set methods.
    */
   //@{
   uint32 getFeatureID() const{
      return m_featureID;
   };
   uint32 getSectionID() const{
      return m_sectionID;
   };
   uint32 getDatasetID() const{
      return m_datasetID;
   };

   void setFeatureID(uint32 ID){
      m_featureID = ID;
   };
   void setSectionID(uint32 ID){
      m_sectionID = ID;
   };
   void setDatasetID(uint32 ID){
      m_datasetID = ID;
   };
   
   //@}
   
   bool isValid() const {
      return !((m_featureID == MAX_UINT32) &&
               (m_sectionID == MAX_UINT32) &&
               (m_datasetID == MAX_UINT32));
   }

 private:
   uint32 m_featureID;
   uint32 m_sectionID;
   uint32 m_datasetID;
    
}; // GDFID

inline bool
operator< ( const GDFID& lhs, const GDFID& rhs ) {
   return lhs.lessThan( rhs );
}


ostream& operator<< ( ostream& stream, const GDFID& gdfID );


#endif // GDFID_H
