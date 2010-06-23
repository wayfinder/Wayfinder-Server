/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef VECTOR_INCL_H
#define VECTOR_INCL_H

#include "config.h"
#include "string.h" // for memmove
#include "VectorElement.h" // for the class VectorElement.

#include <stdlib.h>

#ifdef __MSC_VER // Visual C++?
   #include <memory.h>
#endif

/*
 *  The following vectorclasses are available here:
 *
 *  Name         | Contains elements
 *  -------------+------------------
 *  Vector       | uint32
 *  IntVector    | int32
 *  Vector16     | uint16
 *  ObjVector    | VectorElement*
 *  StringVector | char*
 *  VoidVector   | void*
 *
 *  Questions to
 *
 *
 *  Some strange looking defines are found below.
 *  The real source code is in the file VectorTemplates.cc
 *
 */

/*
  todo

  VectorElement can be typechecked using templates,
  and casting can be avoided.
  
*/

// "global defines", complicated
#define Default_ElementTypeSearchComp(a,b) \
Deref(a) < Deref(m_buf[b]) ? \
-1 : (Deref(a) > Deref(m_buf[b]) ? 1 : 0)
#define Default_ElementTypeComp(a,b) \
Default_ElementTypeSearchComp(m_buf[a], (b))
#define Default_ElementTypeLinearSearchComp(a, b) \
Deref(a) != Deref(b) ? true : false

// the name of the class
#define TemplateVector Vector
// the invalid element
#define InvalidElement MAX_UINT32
// the type of the elements
#define ElementType    uint32
// a debug string for printing
#define debugString    "uint32"
// ...
#define UINT32
// the elementtype can be passed to ostr to print, no conversion
#define DEFAULT_PRINT
// the dereference operator, if any
#define Deref
// the method of comparison
//#define ElementTypeSEARCH_COMP(a, b) ((int64)((int64)a-(int64)buf[b]))
#define ElementTypeSEARCH_COMP(a, b) (a < m_buf[b] ? -1 : a > m_buf[b] ? 1 : 0)
//)((int64)((int64)a-(int64)buf[b]))
#define ElementTypeCOMP(a, b) (ElementTypeSEARCH_COMP(m_buf[a], (b)))
#define ElementTypeLINEAR_SEARCH_COMP(a, b) Default_ElementTypeLinearSearchComp(a, m_buf[b])
#include "VectorTemplates.cc"
//typedef UINT32VectorClass Vector;
//#define Vector UINT32VectorClass
#undef  TemplateVector
#undef  InvalidElement
#undef  ElementType
#undef  debugString
#undef  UINT32
#undef  DEFAULT_PRINT
#undef  Deref
#undef  ElementTypeSEARCH_COMP
#undef  ElementTypeCOMP
#undef  ElementTypeLINEAR_SEARCH_COMP

#define TemplateVector IntVector
#define InvalidElement MAX_INT32
#define ElementType    int32
#define debugString    "int32"
#define UINT32
#define DEFAULT_PRINT
#define Deref
#define ElementTypeSEARCH_COMP(a, b) (a < m_buf[b] ? -1 : a > m_buf[b] ? 1 : 0)
#define ElementTypeCOMP(a, b) (ElementTypeSEARCH_COMP(m_buf[a], (b)))
#define ElementTypeLINEAR_SEARCH_COMP(a, b) Default_ElementTypeLinearSearchComp(a, m_buf[b])
#include "VectorTemplates.cc"
typedef IntVector INT32VectorClass;
#undef  TemplateVector
#undef  InvalidElement
#undef  ElementType
#undef  debugString
#undef  UINT32
#undef  DEFAULT_PRINT
#undef  Deref
#undef  ElementTypeSEARCH_COMP
#undef  ElementTypeCOMP
#undef  ElementTypeLINEAR_SEARCH_COMP

#define TemplateVector Vector16
#define InvalidElement MAX_UINT16
#define ElementType    uint16
#define debugString    "uint16"
#define UINT32
#define DEFAULT_PRINT
#define Deref
#define ElementTypeSEARCH_COMP(a, b) (a < m_buf[b] ? -1 : a > m_buf[b] ? 1 : 0)
#define ElementTypeCOMP(a, b) (ElementTypeSEARCH_COMP(m_buf[a], (b)))
#define ElementTypeLINEAR_SEARCH_COMP(a, b) Default_ElementTypeLinearSearchComp(a, m_buf[b])
#include "VectorTemplates.cc"
//typedef UINT16VectorClass Vector16;
typedef Vector16 UINT16VectorClass;
#undef  TemplateVector
#undef  InvalidElement
#undef  ElementType
#undef  debugString
#undef  UINT32
#undef  DEFAULT_PRINT
#undef  Deref
#undef  ElementTypeSEARCH_COMP
#undef  ElementTypeCOMP
#undef  ElementTypeLINEAR_SEARCH_COMP

#define TemplateVector ObjVector
#define InvalidElement NULL
#define ElementType    VectorElement*
#define debugString    "VectorElement*"
#define Deref          *
#define ElementTypeSEARCH_COMP(a, b) Default_ElementTypeSearchComp(a, b)
#define ElementTypeCOMP(a, b) Default_ElementTypeComp(a, b)
#define ElementTypeLINEAR_SEARCH_COMP(a, b) Default_ElementTypeLinearSearchComp(a, m_buf[b])
#include "VectorTemplates.cc"
//typedef ObjVectorClass ObjVector;
typedef ObjVector ObjVectorClass;
//#define ObjVector ObjVectorClass
#undef  TemplateVector
#undef  InvalidElement
#undef  ElementType
#undef  debugString
#undef  Deref      
#undef  ElementTypeSEARCH_COMP
#undef  ElementTypeCOMP
#undef  ElementTypeLINEAR_SEARCH_COMP

#define TemplateVector StringVector
#define InvalidElement NULL
#define ElementType    char*
#define debugString    "char*"
#define DEFAULT_PRINT
#define Deref          *
#define ElementTypeSEARCH_COMP(a, b) strcmp(a, m_buf[b])
#define ElementTypeCOMP(a, b) ElementTypeSEARCH_COMP(m_buf[a], b)
#define ElementTypeLINEAR_SEARCH_COMP(a, b) (ElementTypeSEARCH_COMP(a, b) != 0)
#define DELETE_ARRAY []
#include "VectorTemplates.cc"
typedef StringVector StringVectorClass;
//#define ObjVector ObjVectorClass
#undef  TemplateVector
#undef  InvalidElement
#undef  ElementType
#undef  debugString
#undef  DEFAULT_PRINT
#undef  Deref      
#undef  ElementTypeSEARCH_COMP
#undef  ElementTypeCOMP
#undef  ElementTypeLINEAR_SEARCH_COMP
#undef  DELETE_ARRAY

#define TemplateVector VoidVector
#define InvalidElement NULL
#define ElementType    void*
#define debugString    "void*"
#define VectorInclVOID
#define VectorReallyIsVoidVector // Removes delete allobjs
#define Deref          
#define ElementTypeSEARCH_COMP(a, b) Default_ElementTypeSearchComp(a, b)
#define ElementTypeCOMP(a, b) Default_ElementTypeComp(a, b)
#define ElementTypeLINEAR_SEARCH_COMP(a, b) Default_ElementTypeLinearSearchComp(a, m_buf[b])
#include "VectorTemplates.cc"
typedef VoidVector VoidVectorClass;
//#define VoidVector VoidVectorClass
#undef  TemplateVector
#undef  InvalidElement
#undef  ElementType
#undef  debugString
#undef  VectorInclVOID
#undef  Deref
#undef  ElementTypeSEARCH_COMP
#undef  ElementTypeCOMP
#undef  ElementTypeLINEAR_SEARCH_COMP
#undef  VectorReallyIsVoidVector

#endif
