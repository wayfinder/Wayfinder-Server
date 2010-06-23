/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef GNU_PLOT_DUMP_H
#define GNU_PLOT_DUMP_H

#include "config.h"

namespace GnuPlotDump {

/**
 *    Iterator dumper. <br />
 *    Class for dumping stuff to mc2dbg8 etc.
 *    Needed since they are defined as if ( false )
 *    Use like so: mc2dbg << it_dump( cont.begin(), cont.end(), ",") << endl;
 */
template<class ITERATOR>
class gp_dumper {
public:
 
   enum dump_type {
      gnuplot = 0,
      vector_gnuplot = 1,
      octave = 2
   };
   
   gp_dumper( const ITERATOR& begin,
              const ITERATOR& end,
              dump_type dumpType ) :
         m_begin(begin),
         m_end(end),
         m_dumpType( dumpType ) {}

   friend ostream& operator<<( ostream& o, const gp_dumper& dumper ) {
      switch ( dumper.m_dumpType ) {
         case ( gnuplot ):
         {
            // Lines - format "x y"
            for ( ITERATOR i = dumper.m_begin; i != dumper.m_end; ++i ) {
               o << GfxUtil::getCoordX( *i ) << " "
                 << GfxUtil::getCoordY( *i ) << endl;
            }
            break;
         }
         case ( vector_gnuplot ):
         {
            // Dump for vector plotting. Format "x y d_x d_y"
            for ( ITERATOR i = dumper.m_begin; i != dumper.m_end; /**/ ) {
               int32 curX = GfxUtil::getCoordX( *i );
               int32 curY = GfxUtil::getCoordY( *i );
               ++i;
               if ( i == dumper.m_end ) {
                  // No more data
                  return o;
               }
               int32 dX = GfxUtil::getCoordX( *i ) - curX;
               int32 dY = GfxUtil::getCoordY( *i ) - curY;
               o << curX << " " << curY << " " << dX << " " << dY << endl;
            }
            break;
         }
         case ( octave ):
         {
            o << "x = [";
            // Lines - format "x y"
            for ( ITERATOR i = dumper.m_begin; i != dumper.m_end; ++i ) {
               o << GfxUtil::getCoordX( *i ) << " ";
            }
            o << " ];" << endl;
            o << "y = [";
            // Lines - format "x y"
            for ( ITERATOR i = dumper.m_begin; i != dumper.m_end; ++i ) {
               o << GfxUtil::getCoordY( *i ) << " ";
            }
            o << " ];" << endl;
            break;
         }
      }
      return o;
   }
   
private:
   ITERATOR m_begin;
   ITERATOR m_end;
   dump_type m_dumpType;
};

template<class ITERATOR>
gp_dumper<ITERATOR>
gp_dump( const ITERATOR& begin,
         const ITERATOR& end ) {
   return gp_dumper<ITERATOR>( begin, end,
                               gp_dumper<ITERATOR>::gnuplot );
}

template<class ITERATOR>
gp_dumper<ITERATOR>
gp_vec_dump( const ITERATOR& begin,
             const ITERATOR& end ) {
   return gp_dumper<ITERATOR>( begin, end,
                               gp_dumper<ITERATOR>::vector_gnuplot );
}

template<class ITERATOR>
gp_dumper<ITERATOR>
octave_dump( const ITERATOR& begin,
             const ITERATOR& end ) {
   return gp_dumper<ITERATOR>( begin, end,
                               gp_dumper<ITERATOR>::octave );
}

}
#endif
