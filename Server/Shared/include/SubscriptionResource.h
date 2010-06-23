/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef SUBSCRIPTIONRESOURCE_H
#define SUBSCRIPTIONRESOURCE_H


#include "config.h"
#include "StringUtility.h"


/**
 *    Abstract class for a push subscription resource.
 *
 */
class SubscriptionResource {
   public:
      /**
       * Destructor.
       */
      virtual ~SubscriptionResource();


      /**
       * Makes a copy of this.
       */
      virtual SubscriptionResource* clone() const = 0;


      /**
       * Lessthan Compare operator.
       */
      virtual bool operator<(const SubscriptionResource& other) const = 0;

      
      /**
       * Greaterthan Compare operator.
       */
      virtual bool operator>(const SubscriptionResource& other) const= 0;


      /**
       * Unequal Compare operator.
       */
      virtual bool operator!=(const SubscriptionResource& other) const = 0;
   

      /**
       * Equal Compare operator.
       */
      virtual bool operator==(const SubscriptionResource& other) const = 0;


      /**
       * Print on ostream.
       */
      virtual ostream& operator<< ( ostream& stream ) const= 0;

      /**
       *   Also print on ostream.
       *   @param stream The stream to print on.
       *   @param res    The resource to print.
       *   @return The stream.
       */
      friend ostream& operator<<( ostream& stream,
                                  SubscriptionResource& res){
         return res.operator<<(stream);
      }
        
};


/**
 * Class for a push subscription map resource.
 */
class MapSubscriptionResource : public SubscriptionResource {
   public:
      /**
       * Contructor with mapID.
       */
      MapSubscriptionResource( uint32 mapID ) 
         : m_mapID( mapID )
      {}


      /**
       * Makes a copy of this.
       */
      virtual SubscriptionResource* clone() const;


      /**
       * Return the mapID operator.
       */
      uint32 operator*() const {
         return m_mapID;
      }
   
      
      /**
       * Lessthan Compare operator.
       */
      virtual bool operator<( const SubscriptionResource& other) const {
         return m_mapID < static_cast<const MapSubscriptionResource&> (
            other ).m_mapID;
      }


      /**
       * Greaterthan Compare operator.
       */
      virtual bool operator>( const SubscriptionResource& other) const {
         return m_mapID > static_cast<const MapSubscriptionResource&> (
            other ).m_mapID;
      }


      /**
       * Unequal Compare operator.
       */
      virtual bool operator!=( const SubscriptionResource& other) const {
         return m_mapID != static_cast<const MapSubscriptionResource&> (
            other ).m_mapID;
      }
   

      /**
       * Equal Compare operator.
       */
      virtual bool operator==( const SubscriptionResource& other) const {
         return m_mapID == static_cast<const MapSubscriptionResource&> (
            other ).m_mapID;
      }


      /**
       * Print on ostream.
       */
      virtual ostream& operator<< ( ostream& stream ) const
      {
         stream << m_mapID;
         return stream;
      }


   private:
      /**
       * The mapID.
       */
      uint32 m_mapID;
};


/**
 * Class for a push subscription for string resource.
 */
class StringSubscriptionResource : public SubscriptionResource {
   public:
      /**
       * Contructor with string.
       */
      StringSubscriptionResource( const char* str ) 
         : m_str( StringUtility::newStrDup( str ) )
      {}

      
      /**
       * Destructor.
       */
      ~StringSubscriptionResource();


      /**
       * Makes a copy of this.
       */
      virtual SubscriptionResource* clone() const;


      /**
       * Return the string operator.
       */
      const char* operator*() const {
         return m_str;
      }
   
      
      /**
       * Lessthan Compare operator.
       */
      virtual bool operator<( const SubscriptionResource& other) const {
         return StringUtility::strcmp( 
            m_str, static_cast<const StringSubscriptionResource&> ( 
               other ).m_str ) < 0;
      }


      /**
       * Greaterthan Compare operator.
       */
      virtual bool operator>( const SubscriptionResource& other) const {
         return StringUtility::strcmp( 
            m_str, static_cast<const StringSubscriptionResource&> ( 
               other ).m_str ) > 0;
      }


      /**
       * Unequal Compare operator.
       */
      virtual bool operator!=( const SubscriptionResource& other) const {
         return StringUtility::strcmp( 
            m_str, static_cast<const StringSubscriptionResource&> ( 
               other ).m_str ) != 0;
      }
   

      /**
       * Equal Compare operator.
       */
      virtual bool operator==( const SubscriptionResource& other) const {
         return StringUtility::strcmp( 
            m_str, static_cast<const StringSubscriptionResource&> ( 
               other ).m_str ) == 0;
      }


      /**
       * Print on ostream.
       */
      virtual ostream& operator<< ( ostream& stream ) const
      {
         stream << m_str;
         return stream;
      }



   private:
      /**
       * The string.
       */
      char* m_str;
};


/**
 * Notiuce class for SubscriptionResource.
 */
class SubscriptionResourceNotice {
   public:
      /**
       * Constructor with SubscriptionResource.
       */
      SubscriptionResourceNotice( const SubscriptionResource& resource ) 
         : m_resource( resource.clone() )
      {}


      SubscriptionResourceNotice( 
         const SubscriptionResourceNotice& other ) 
      {
         m_resource = 
            other.
            m_resource->
            clone();
      }


      SubscriptionResourceNotice& operator=( 
         const SubscriptionResourceNotice& other ) 
      {
         if ( this != &other ) {
            delete m_resource;
            m_resource = other.m_resource->clone();
         }
         return *this;
      }
      

      ~SubscriptionResourceNotice() {
         delete m_resource;
      }
      

      /**
       * Return the SubscriptionResource operator.
       */
      SubscriptionResource& operator*() const{
         return *m_resource;
      }


      /**
       * Lessthan Compare operator.
       */
      bool operator<( const SubscriptionResourceNotice& other) const {
         return *m_resource < *other.m_resource;
      }


      /**
       * Greaterthan Compare operator.
       */
      bool operator>( const SubscriptionResourceNotice& other) const {
         return *m_resource > *other.m_resource;
      }


      /**
       * Unequal Compare operator.
       */
      bool operator!=( const SubscriptionResourceNotice& other) const {
         return *m_resource != *other.m_resource;
      }
   

      /**
       * Equal Compare operator.
       */
      bool operator==( const SubscriptionResourceNotice& other) const {
         return *m_resource == *other.m_resource;
      }


   private:
      /**
       * The SubscriptionResource.
       */
      SubscriptionResource* m_resource;
};


#endif // SUBSCRIPTIONRESOURCE_H


