/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "config.h"
#include "CCSet.h"
#include "MC2Exception.h"

// =======================================================================
//                                                               Linkage =

Linkage::Linkage()
   :  m_next(NULL),
      m_prev(NULL),
      m_type(LINKAGE),
      imFirst(false),
      imLast(false)
{
   // Nothing to do
}


Linkage::Linkage( Linkage  *next,
                  Linkage  *prev,
                  int      type,
                  bool     first,
                  bool     last  )
                  :  m_next(next),
                     m_prev(prev),
                     m_type(type),
                     imFirst(first),
                     imLast(last)
{
   // Nothing to do
}

Linkage::~Linkage()
{
   // Nothing to do
}

// =======================================================================
//                                                                  Head =

// disable warning in visual c++ about using 'this' in base member initializer list before the object is finished.
#ifdef _MSC_VER
#  pragma warning (disable: 4355)
#endif

Head::Head() : Linkage(this, this, HEAD)
{
   card = 0;
   m_first = m_last = NULL;
}

// restore the above warning
#ifdef _MSC_VER
#  pragma warning (default: 4355)
#endif

Head::~Head()
{
   clear();
}

Linkage* 
Head::last() const
{
   return m_last;
}

void 
Head::clear()
{
   Linkage* linkage = last();

   while (linkage != NULL) {
      ((Link *)linkage)->out();
      delete linkage;
      linkage = last();
   }
}


// =======================================================================
//                                                                  Link =

Link::Link() : Linkage(NULL, NULL, LINK)
{
   m_h = NULL;
}


Link::~Link()
{
}

void 
Link::out()
{
   // last
   if (m_h != NULL) {
      if (imLast) {
         // last and first
         if (imFirst) {
            m_h->m_first = m_h->m_last = NULL;
         }
         // last but not first
         else {
            m_prev->m_next = NULL;
            m_h->m_last    = m_prev;
            m_prev->imLast = true;
         }
      }
      // not last
      else {
         // first
         if (imFirst) {
            m_next->m_prev  = NULL;
            m_h->m_first    = m_next;
            m_next->imFirst = true;
         }
         // not first
         else {
            m_next->m_prev = m_prev;
            m_prev->m_next = m_next;
         }
      }
      this->m_h->card--;
      this->m_h = NULL;
   }
   this->imFirst = this->imLast = false;
   this->m_next = this->m_prev = NULL;
} // out()


void
Link::into( Head *h )
{
   if (h == NULL) {
      // Very bad
      throw MC2Exception("Link::int(h): h == NULL");
   }

   out();   // just for safety
   this->m_h = h;
   
   // h not empty
   if (h->m_last != NULL) {
      h->m_last->m_next = this;
      h->m_last->imLast = false;
      this->m_prev = h->m_last;
      h->m_last = this;
      this->imFirst = false;
   }
   // h empty
   else {
      h->m_first = h->m_last = this;
      this->imFirst = true;
   }
   this->m_h = h;
   this->imLast = true;
   h->card++;
} // into(Head *)


void
Link::intoAsFirst(Head *h)
{
   if (h == NULL) {
      PANIC("Link::intoAsFirst(h): ", "h == NULL");
   }

   if (h->empty())
      this->into(h);
   else
      this->precede(h->first());
}


void
Link::follow(Linkage *linkage)
{
   if (linkage == NULL) {
      DEBUG8(cerr << "Link::follow(linkage): linkage == NULL" << endl);
      exit(-1);
   }

   // explicit cast
   Link *link;
   if (linkage->m_type == LINK && ((Link *)linkage)->m_h != NULL)
      link = (Link *)linkage;
   else {
      DEBUG8(cerr << "Link::follow(linkage): linkage not in any head" << endl);
      exit(-1);
   }
   
   out();   // just for safety
   
   // last
   if (link->imLast) {
      this->into(link->m_h);
   }
   // not last
   else {
      this->m_next = link->m_next;
      this->m_prev = link;
      link->m_next->m_prev = this;
      link->m_next = this;
      this->m_h = link->m_h;
      this->m_h->card++;
   }
} // follow(Linkage *)


void
Link::precede(Linkage *linkage)
{
   if (linkage == NULL) {
      DEBUG8(cerr << "Link::precede(linkage): linkage == NULL" << endl);
      exit(-1);
   }
   
   // explicit cast
   Link *link;
   if (linkage->m_type == LINK && ((Link *)linkage)->m_h != NULL)
      link = (Link *)linkage;
   else {
      DEBUG8(cerr << "Link::precede(linkage): linkage not in any head" << endl);
      exit(-1);
   }

   out();   // just for safety;

   // first
   if (link->imFirst) {
      this->m_prev = NULL;
      link->m_h->m_first = this;
      link->imFirst = false;
      this->imFirst = true;
   }
   // not first
   else {
      this->m_prev = link->m_prev;
      link->m_prev->m_next = this;
   }
   link->m_prev = this;
   this->m_next = link;
   this->m_h = link->m_h;
   this->m_h->card++;
} // precede(Linkage *)

