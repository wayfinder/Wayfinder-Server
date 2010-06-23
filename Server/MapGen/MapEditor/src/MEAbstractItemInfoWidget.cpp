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
#include "MEAbstractItemInfoWidget.h"
#include "MEMapArea.h"

MEAbstractItemInfoWidget::MEAbstractItemInfoWidget(const char* name)
   : Gtk::Frame(name)
{
   m_active = false;
}

MEAbstractItemInfoWidget::~MEAbstractItemInfoWidget()
{
   // Nothing is allocated here, so nothing to do
}

void
MEAbstractItemInfoWidget::activate(MEMapArea* mapArea, OldItem* item)
{
   MC2_ASSERT(mapArea != NULL);
   MC2_ASSERT(item != NULL);

   m_active = true;
   m_mapArea = mapArea;
   m_item = item;
}


void
MEAbstractItemInfoWidget::inactivate()
{
   m_active = false;
   hide();
}


bool
MEAbstractItemInfoWidget::isActive()
{
   return m_active;
}

#ifdef MAP_EDITABLE
void
MEAbstractItemInfoWidget::saveChanges(
            MELogCommentInterfaceBox* logCommentBox)
{
   // Default implementation that does not do anything!
}
#endif // MAP_EDITABLE

