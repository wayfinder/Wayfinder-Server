/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef MC2MAPALLOCATOR_H
#define MC2MAPALLOCATOR_H

#include "config.h"
#include "DeleteHelpers.h"
#include <boost/shared_ptr.hpp>
#include <list>
#define GCC_VERSION (__GNUC__ * 10000 \
                     + __GNUC_MINOR__ * 100 \
                     + __GNUC_PATCHLEVEL__)
/* Test for GCC > 3.2.0 */
#if GCC_VERSION > 30200
# include <bits/allocator.h>
#else
# include <stl_alloc.h>
#endif

namespace MC2Map {

   class AllocatorException : public std::exception {
   public:
      explicit AllocatorException( const MC2String& str) throw():
            m_str( "Allocator: " + str ) { }
      virtual ~AllocatorException() throw() { }
      const char* what() const throw() { return m_str.c_str(); }
   private:
      MC2String m_str;
   };

   /**
    * NOT THREAD SAFE, NOT FOR GENERAL USE.
    *
    * Also see Allocator for the allocator.
    */
   class AllocatorPool {
   public:
      AllocatorPool( uint32 blockSize ) 
            : m_referenceCount( 1 ), m_blockSize( blockSize ),
              m_blockPos( 0 )
      {
         m_memBlocks.push_back( new byte[ m_blockSize ] );
      }

      ~AllocatorPool() {
         STLUtility::deleteArrayValues( m_memBlocks );
      }

      void* allocate( size_t n ) {
         // If bigger than blocksize, make own block for this allocation
         if ( n > m_blockSize/2 ) {
            // Note pushed front not to change current block
            m_memBlocks.push_front( new byte[ n ] );
            return m_memBlocks.front();
         } else {
            // Check current block
            if ( m_blockPos + n > m_blockSize ) {
               // Full new block needed
               m_memBlocks.push_back( new byte[ m_blockSize ] );   
               m_blockPos = 0;
            }
            m_blockPos += n;
            
            return m_memBlocks.back() + m_blockPos - n;
         }
      }

      void addUser() { ++m_referenceCount; }

      bool removeUser() { 
         --m_referenceCount;
         return  m_referenceCount == 0;
      }
   
   private:
      /// The memory blocks
      list< byte* > m_memBlocks;

      /// The number of users
      uint32 m_referenceCount;

      /// The size of the blocks
      uint32 m_blockSize;

      /// Pos in current block
      uint32 m_blockPos;
   };


   /**
    * NOT THREAD SAFE, NOT FOR GENERAL USE.
    */
   template <class T> class Allocator;

   // specialize for void:
   template <> class Allocator<void> {
   public:
      typedef void*       pointer;
      typedef const void* const_pointer;
      // reference to void members are impossible.
      typedef void value_type;
      template <class U> struct rebind { typedef Allocator<U>
                                         other; };
   };

   /**
    * NOT THREAD SAFE, NOT FOR GENERAL USE.
    */
   template <class T> class Allocator : public allocator<T> {
   public:
      typedef size_t    size_type;
      typedef ptrdiff_t difference_type;
      typedef T*        pointer;
      typedef const T*  const_pointer;
      typedef T&        reference;
      typedef const T&  const_reference;
      typedef T         value_type;
      template <class U> struct rebind { typedef Allocator<U>
                                         other; };

      Allocator( uint32 blockSize = MAX_UINT16 ) throw() {
         m_pool = new AllocatorPool( blockSize );
      }

      Allocator( const Allocator& o ) throw() {
         m_pool = o.m_pool;
         m_pool->addUser();
      }

      template <class U> Allocator( const Allocator<U>& o ) throw() {
         m_pool = o.m_pool;
         m_pool->addUser();
      }

      ~Allocator() throw() {
         if ( m_pool->removeUser() ) {
            delete m_pool;
         } // Else still used by other allocator
      }

      pointer address( reference x ) const {
         return &x;
      }

      const_pointer address( const_reference x ) const {
         return &x;
      }

      pointer allocate( size_type size,
                        Allocator<void>::const_pointer hint = 0 ) {
         return static_cast<pointer>( m_pool->allocate( size * sizeof( T ) ) );
      }

      void deallocate( pointer p, size_type n ) {
         // Done, see destructor
      }

      size_type max_size() const throw() {
         return size_type( -1 ) / sizeof( value_type );
      }

      void construct( pointer p, const T& val ) {
         //new ( static_cast<void*> ( p ) ) T( val );
         new( p ) T( val ); // initialize *p by val
      }

      void destroy( pointer p ) {
         // Leaks? no leaks here.
         // // Destroy *p but don't deallocate
      }

      /// The shared memory pool
      AllocatorPool* m_pool;

   private:
      /// Copy assignment
      Allocator& operator=( const Allocator& o ) {
         if ( &o != this ) {
            m_pool = o.m_pool;
            m_pool->addUser();
         }
         return *this;
      }

      template <class U> Allocator& operator=( const Allocator<U>& o ) {
         if ( &o != this ) {
            m_pool = o.m_pool;
            m_pool->addUser();
         }
         return *this;
      }
   };

} // namespace MC2Map


#endif // MC2MAPALLOCATOR_H

