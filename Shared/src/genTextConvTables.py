#!/usr/bin/env python
#
# Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
# All rights reserved.
# 
# Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
# 
#     * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
#     * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
#     * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.
# 
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
classname='TextConversionTables'
entrytypename='convTable_entry'
tabletypename='convTable'

commonHeader="""/*
 *
 *    Copyright, Wayfinder Systems AB, 2004-2007
 */
"""

headerFileHeader="""
#ifndef TEXT_CONVERSUIONT_AATBLES_H
#define TEXT_CONVERSUIONT_AATBLES_H

#include \"config.h\"
#include <algorithm>

""" + "class " + classname + " {\n" + """
public:
   /**
    *   One entry in a conv table.
    */
   struct convTable_entry {
      /// The unicode character to be converted
      const uint32 unicodeChar;
      /// The utf-8 string of the converted character
      const char* const utf8String;
      /// Length of the utf-8 string.
      const int utf8Length;
   };

   /**
    *    A table of convTable_entry. Also contains the number
    *    of entries in the table.
    */
   struct convTable {
      uint32 nbrEntries;
      const convTable_entry* const entries;
   };

   class ConvEntryComp {
     public:
      bool operator()(const convTable_entry& a, const convTable_entry& b) {
         return a.unicodeChar < b.unicodeChar;
      }
      bool operator()(const convTable_entry& a, uint32 code ) {
         return a.unicodeChar < code;
      }
   };

   /// Result of a findInTable operation.
   struct convTableFindRes {
      convTableFindRes( const char* aStr, int aLen ) : resString(aStr),
                                                       resStringLength(aLen) {}
      const char* resString;
      int resStringLength;
   };

   /**
    *   Looks for the code <code>code</code> in the table <code>table</code>.
    *   @param table Table to search.
    *   @param code  Code to look for.
    *   @return Result with string set to NULL and length set to zero
    *           if not found.
    */
   static convTableFindRes findInTable( const convTable& table,
                                        uint32 code ) {
      const convTable_entry* begin = table.entries;
      const convTable_entry* end   = table.entries + table.nbrEntries;
      const convTable_entry* found = std::lower_bound( begin, end,
                                                       code,
                                                       ConvEntryComp() );
      if ( found->unicodeChar == code ) {
         return convTableFindRes(found->utf8String,
                                 found->utf8Length );
      } else {
         return convTableFindRes(NULL, 0);
      }
   }
   
"""
headerFileFooter="""
};
#endif
"""

# Function for sorting the hexadecimal number strings in the lists
def num_comp(a, b):
    import string
    return cmp( string.atoi(a[0],16), string.atoi(b[0],16) )

class entry:
    def __init__(self, listofvalues):
        [self.code_value,
         self.character_name,
         self.general_category,
         self.canonical_combining_classes,
         self.bidirectional_category,
         self.character_decomposition_mapping,
         self.decimal_digit_value,
         self.digit_value,
         self.numeric_value,
         self.mirrored,
         self.unicode_1_0_name,
         self.iso10646_comment_field,
         self.upper_case_mapping,
         self.lower_case_mapping,
         self.title_case_mapping] = listofvalues
        self.case_folding_status = ''
        self.case_folding_mapping = ''
        self.overridden_strip_strange = ''

    def set_case_folding_data(self, status, mapping):
        import string
        self.case_folding_status  = string.strip(status)
        self.case_folding_mapping = string.strip(mapping)

    def override_strip_strange(self, newchars):
        self.overridden_strip_strange = newchars

    def is_alpha(self):
        # Return true if it is a letter, but not modifier letter
        gc = self.general_category
        return gc[0] == 'L' and gc != 'Lm'

    def is_number(self):
        gc = self.general_category
        return gc == 'No' or gc == 'Nd'

    def is_alnum(self):
        return self.is_number() or self.is_alpha()

    def is_space(self):
        return self.general_category == 'Zs'

    def get_num_code(self):
        import string
        return string.atoi(self.code_value, 16)

    # This function is recursive to really go to the bottom
    def get_strip_strange( self, chartable ):
        if self.overridden_strip_strange != '':
            # Currently only supports overriding to a single char
            x = self.overridden_strip_strange
            temp = chartable[x].get_strip_strange( chartable )
            return ( temp[0], temp[1] + 1 )
        
        if self.character_decomposition_mapping == '':
            if self.is_alnum():
                # Check for case folding, but avoid Turkish
                if ( self.case_folding_mapping != '' ) and \
                   ( self.case_folding_status != 'T' ):
                    return (self.case_folding_mapping, 0)
                else:
                    if self.lower_case_mapping != '':
                        return (self.lower_case_mapping, 0)
                    else:
                        return (self.code_value, 0)
            else:
                if self.is_space():
                    return ('0020', 0)
                else:
                    return ('', 0)
                
        import re
        import string

        overridden = 0
        
        # Remove the <>-tags
        s = re.sub(r'<.*>', '', self.character_decomposition_mapping)        
        s = string.strip(s)
        #print self.code_value + ':' + s
        retval = ''
        for i in string.split(s):
            if chartable.has_key( i ):
                entry = chartable[i]
                decomp1 = entry.get_strip_strange(chartable)
                decomp = decomp1[0]
                overridden = overridden + decomp1[1]
                if decomp == '':
                    decomp = i                
                retval = retval + decomp + ' '
        retval_tmp = retval
        retval = ''
        for i in string.split(retval_tmp):
            if chartable.has_key(i):
                entry = chartable[i]
                if entry.is_alnum():
                    retval = retval + i + ' '
                
        return (string.strip(retval), overridden)

    def get_latin_1_chars( self, chartable, nametable ):
        if ( self.get_num_code() < 256 ):
            return self.code_value
        
        import re        
        import string
        # Remove the <>-tags
        s = re.sub(r'<.*>', '', self.character_decomposition_mapping)        
        s = string.strip(s)
        #print self.code_value + ':' + s
        if s == '':
            # Try to use the name of the character instead
            tmp = string.split( self.character_name, ' WITH')
            if nametable.has_key( tmp[0] ):
                entry = nametable[ tmp[0] ]
                if entry.get_num_code() < 256:
                    return nametable[ tmp[0] ].code_value
                else:
                    return ''
        
        retval = ''
        for i in string.split(s):
            if chartable.has_key( i ):
                entry = chartable[i]
                decomp = entry.get_latin_1_chars(chartable, nametable)
                if decomp == '':
                    decomp = i                
                retval = retval + decomp + ' '
        retval_tmp = retval
        retval = ''
        for i in string.split(retval_tmp):
            if chartable.has_key(i):
                entry = chartable[i]
                if entry.get_num_code() < 256:
                    retval = retval + i + ' '
                
        return string.strip(retval)

class conv_table:
    def __init__(self, tablename, convlist, comment):
        self.conversion_list = convlist
        # Sort the conversion list numerically
        self.conversion_list.sort( num_comp );
        self.tablename = tablename
        self.comment = comment
    
def examine(code, value):
    if value != '':
        print code,
        print " ",
        print value

def readUnicodeData(f, chartable):
    import string
    str = "Not_empty"
    while str != "":
        str=f.readline()
        if str != "":
            splitstring=string.split(str, ';')
            code = splitstring[0]
            chartable[code] = entry(splitstring)
            #examine(code, chartable[code].general_category)

def readCaseFoldingData(f, chartable):
    """ Function that reads the casefolding data and sets some properties"""
    import string
    str = '#'
    while 1:
        str=f.readline()
        if str == '':
            break
        # Remove whitespace first and last
        str = string.strip(str)
        # Remove comment by splitting the string on the comment
        str=string.strip(string.split(str, '#')[0])
        if str == '':
            # Comment - skip
            continue
        else:
            # This is the line to handle
            splitstring=string.split(str,";")
            [ code, status, mapping, comment ] = splitstring
            chartable[code].set_case_folding_data( status, mapping )


def wrap_text( text, width ):
    """ Returns a list of strings wrapped to the supplied width.
    This functionality is builtin in newer python"""
    import string    
    words = string.split( text )
    currow = ''
    reslist = []
    for i in words:
        if currow == '':
            testrow = i
        else:
            testrow = currow + ' ' + i
        if len(testrow) >= width and testrow != '':
            # New row needed
            reslist.append(currow)
            currow = i
        else:
            currow = testrow
    reslist.append( testrow )
    return reslist

def make_wrapped_comment( text, width, indent ):
    rowlist = wrap_text( text, width )
    if len(rowlist) == 0:
        return ''
    res = indent + '/*\n'
    for i in rowlist:
        res = res + indent + ' *  ' + i + '\n'
    res = res + indent + ' */\n'
    return res

def list_to_string_rows( list, prefix ):
    resstring = ''
    for i in list:
        resstring = resstring + prefix + i + '\n'
    return resstring

# Converts a string containing and unicode character as hex
# to an utf-8 c-string
def convertUnicodeToUTF8Octal( hexstring ):
    import string
    res = []
    value = string.atoi( hexstring, 16 )
    if (value >= 0x00000000) and (value <= 0x0000007f):
        res.append(value)
    elif (value >= 0x00000080) and (value <= 0x000007ff):
        res.append( 192 + (value / 64) )
        res.append( 128 + (value % 64) )
    elif ((value >= 0x00000800) and (value <= 0x0000ffff) ):
        res.append( 224 + (value / 4096) )
        res.append( 128 + ((value / 64) % 64) )
        res.append( 128 + (value % 64) )
    elif ( (value >= 0x00010000) and (value <= 0x001fffff) ):
        res.append( 240 + (value / 262144) )
        res.append( 128 + ((value / 4096) % 64) )
        res.append( 128 + ((value / 64) % 64) )
        res.append( 128 + (value % 64) )
    elif (value >= 0x00200000) and (value <= 0x03ffffff):
        res.append( 248 + (value / 16777216) )
        res.append( 128 + ((value / 262144) % 64))
        res.append( 28 + ((value / 4096) % 64) )
        res.append( 128 + ((value / 64) % 64) )
        res.append( 128 + (value % 64) )
    elif (value >= 0x04000000) and (value <= 0x7fffffff):
        res.append( 252 + (value / 1073741824) )
        res.append( 128 + ((value / 16777216) % 64) )
        res.append( 128 + ((value / 262144) % 64) )
        res.append( 128 + ((value / 4096) % 64) )
        res.append( 128 + ((value / 64) % 64) )
        res.append( 128 + (value % 64) )
    else:
        # This is wrong
        pass

    # Convert numbers to strings
    retstr = ''
    for i in res:
        tmp_str = oct(i)
        if ( tmp_str[0] == '0' ):
            tmp_str=tmp_str[1:]
        retstr = retstr + '\\' + tmp_str

    # Convert the string into \002 etc.
    return retstr

def make_table_entry_comment( chartable, fromcode, tocode, overridden ):
    indent = '   '
    s = ''
    s = s + indent + "// "
    s = s + chartable[fromcode].character_name
    import string
    sep = "  -> "
    for i in string.split( tocode ):
        s = s + "\n" + indent + "// "
        s = s + sep
        # Change separator to plus
        sep = "   + "
        s = s + chartable[i].character_name

    s = s + "\n"
    
    if overridden != 0:
        s = s + indent + "// (MC2 override)\n"
        

    return s
        
def make_table_entry(chartable, fromcode, tocode, overridden, utf8 = 1):
    import string
    indent = '   '    
    # Create the comment
    s = make_table_entry_comment( chartable, fromcode, tocode, overridden )
    # Create the list of values
    hexfromcode = '0x' + fromcode
    list = []
    if utf8 == 1:
        utf8tocode = "\""
        # Loop over the to-codes
        for i in string.split( tocode ):
            utf8tocode = utf8tocode + convertUnicodeToUTF8Octal(i)
        utf8tocode = utf8tocode + "\""
        stringlength = str( string.count( utf8tocode, '\\' ) )
        list = [ hexfromcode, utf8tocode, stringlength ]
    else:
        isotocode = '\"'
        for i in string.split( tocode ):
            isotocode = isotocode + chr( string.atoi ( i, 16 ) )
        isotocode = isotocode + '\"'
        stringlength = str( len ( isotocode ) - 2 )
        list = [ hexfromcode, isotocode, stringlength ]
        
    s = s + indent + "{ " + string.join(list, ', ') + " },\n"
    return s

def make_table_cpp(chartable, tablename, conversionlist, utf8 = 1 ):
    # Sort the input list
    conversionlist.sort(num_comp)    
    # Start by writing the entries list
    entriesvarname = tablename+'Entries'
    nbrString = str( len(conversionlist) )
    s = classname
    s = s + '::' + entrytypename + '\n'
    s = s + classname + '::' + entriesvarname + '[' + nbrString + '] = {\n'
    for i in conversionlist:
        # indent
        s = s + make_table_entry(chartable, i[0], i[1], i[2], utf8 )

    s = s + "};\n\n"

    # The write the table struct
    s = s + classname
    s = s + '::' + tabletypename + '\n'
    s = s + classname + '::' + tablename + ' = {\n'
    s = s + '   ' + str(len(conversionlist)) + ',\n'
    s = s + '   ' + entriesvarname + ',\n'
    s = s + '};\n\n'
    return s

def create_header_file( chartable, table_list):
    # Start with the header
    s = commonHeader + headerFileHeader + '\n'
    for i in table_list:
        s = s + make_wrapped_comment( i.comment, 70, '   ' )
        s = s + '   static ' + tabletypename + ' ' + i.tablename + ';\n'

    s = s + '\n'
    s = s + 'private:\n\n'
    
    for i in table_list:
        s = s + '   static ' + entrytypename + ' ' + i.tablename + 'Entries ['
        s = s + str( len (i.conversion_list) ) + '];\n\n'
        
    return s + headerFileFooter

def save_cpp_files( chartable, table_list, headerfile, cppfile, headername ):
    # Create the header file in an inefficient way
    s = create_header_file( chartable, table_list )
    headerfile.write( s )

    # Make the .cpp file - not efficient
    s = commonHeader + "\n#include \"" + headername + "\"\n"
    s = s + '\n'
    for i in table_list:
        s = s + make_table_cpp( chartable, i.tablename, i.conversion_list)
    cppfile.write( s )

def override_strip_strange(nametable, desc1, desc2):
    nametable[desc1].override_strip_strange(nametable[desc2].code_value)

# main method

import urllib
import sys

# Table of character code -> character data
chartable = {}

# List of tables that should be added to the .h and .cpp files
table_list = []

print "Reading unicodedata"
readUnicodeData(urllib.urlopen(sys.argv[1]), chartable)
print "Reading case folding data"
readCaseFoldingData(urllib.urlopen(sys.argv[2]), chartable)

def removeRange( chartable, rangeStart, rangeEnd ):
    for i in range( rangeStart, rangeEnd+1 ):
        import string
        key = string.upper(hex(i)[2:])

        if len(key) == 3:
            key = '0' + key

        if chartable.has_key( key ):
            del chartable[key]

# Remove GLAGOLITIC
# Glagolitic alphabet was added to unicode in version 4.1
# Range U+2C00 - U+2C5E
# http://en.wikipedia.org/wiki/Glagolitic_alphabet
print "Removing Glagolitic"
removeRange( chartable, 0x2C00, 0x2C5E )

# Remove N'Ko alphabet
# Range U+07C0-U+07FF
# http://en.wikipedia.org/wiki/N%27Ko
print "Removing NKO"
removeRange( chartable, 0x07C0, 0x07FF )

# Remove COPTIC
# Coptic alphabet was added to unicode in version 4.1
# Range U+2C80 - U+2CFF
# http://en.wikipedia.org/wiki/Coptic_alphabet
print "Removing Coptic"
removeRange( chartable, 0x2C80, 0x2CFF )

print "Removing Balinese"
removeRange( chartable, 0x1B00, 0x1B7F )

print "Removing Tifinagh"
removeRange( chartable, 0x2D30, 0x2D7F )

print "Removing Cuneiform"
removeRange( chartable, 0x12000, 0x123FF )

print "Removing Phoenician"
removeRange( chartable, 0x10900, 0x1091F )

print "Removing Syloti Nagri"
removeRange( chartable, 0xA800, 0xA82F )

print "Removing Phags-pa"
removeRange( chartable, 0xA840, 0xA87F )

print "Removing Old Persian"
removeRange( chartable, 0x103A0, 0x103DF )

print "Removing Gothic"
removeRange( chartable, 0x10330, 0x1034F )

print "Removing Kharoshthi"
removeRange( chartable, 0x10A00, 0x10A5F )

print "Removing Ethiopic"
removeRange( chartable, 0x1200, 0x137F );

print "Removing Ethiopic extended"
removeRange( chartable, 0x2D80, 0x2DDF );

print "Building name table"
nametable = {}
for i,j in chartable.items():
    nametable[j.character_name] = j

headername = sys.argv[3]
headerfile = open( headername, 'w' )
sourcefile = open( sys.argv[4], 'w' )

# Add some stuff to the commonheader
import string
extraComment = 'NOTE! This file is machine generated - do not edit directly!!!\ncmdline: ' + string.join(sys.argv, ' ')
commonHeader = commonHeader + make_wrapped_comment(extraComment, 70, '')

# Make lower case mapping
lowercaselist = []
for i,j in chartable.items():
    if j.lower_case_mapping != "":
        lowercaselist.append( ( i,j.lower_case_mapping, 0 ) )

# Print lower case table
#print make_table_cpp(chartable, 'c_lowerTable', lowercaselist)

print 'Making lower table'
table_list.append( conv_table( 'c_lowerTable', lowercaselist, 'Table to convert from upper to lower case. If a character does not exist in the table, it should be kept as is' ) )


# Make upper case mapping
uppercaselist = []
for i,j in chartable.items():
    if j.upper_case_mapping != "":
        uppercaselist.append( ( i,j.upper_case_mapping, 0 ) )

# Print upper case table
#print make_table_cpp(chartable, 'c_upperTable', uppercaselist)
print 'Making upper table'
table_list.append( conv_table( 'c_upperTable', uppercaselist, 'Table to convert from lower to uppercase. If a character does not exist in the table, it should be kept as is.' ) )

print 'Making stripStrange'
# Make case folding table

# Danish / Norwegian
override_strip_strange(nametable,'LATIN CAPITAL LETTER O WITH STROKE',\
                       'LATIN CAPITAL LETTER O')
override_strip_strange(nametable,'LATIN SMALL LETTER O WITH STROKE',\
                       'LATIN SMALL LETTER O')
override_strip_strange(nametable,'LATIN CAPITAL LETTER AE',\
                       'LATIN SMALL LETTER A')
override_strip_strange(nametable,'LATIN SMALL LETTER AE',\
                       'LATIN SMALL LETTER A')


# Some greek characters that look like some latin characters
override_strip_strange(nametable,'GREEK CAPITAL LETTER ALPHA',
                       'LATIN CAPITAL LETTER A')
override_strip_strange(nametable, 'GREEK CAPITAL LETTER BETA',
                       'LATIN CAPITAL LETTER B')
override_strip_strange(nametable, 'GREEK CAPITAL LETTER EPSILON',
                       'LATIN CAPITAL LETTER E')
override_strip_strange(nametable, 'GREEK CAPITAL LETTER ETA',
                       'LATIN CAPITAL LETTER H')
override_strip_strange(nametable, 'GREEK CAPITAL LETTER IOTA',
                       'LATIN CAPITAL LETTER I')
override_strip_strange(nametable, 'GREEK CAPITAL LETTER KAPPA',
                       'LATIN CAPITAL LETTER K')
override_strip_strange(nametable, 'GREEK CAPITAL LETTER OMICRON',
                       'LATIN CAPITAL LETTER O')
override_strip_strange(nametable, 'GREEK CAPITAL LETTER RHO',
                       'LATIN CAPITAL LETTER P')
override_strip_strange(nametable, 'GREEK CAPITAL LETTER TAU',
                       'LATIN CAPITAL LETTER T')
override_strip_strange(nametable, 'GREEK CAPITAL LETTER UPSILON',
                       'LATIN CAPITAL LETTER U')
override_strip_strange(nametable, 'GREEK CAPITAL LETTER ZETA',
                       'LATIN CAPITAL LETTER Z')

# Polish
override_strip_strange(nametable, 'LATIN CAPITAL LETTER L WITH STROKE',
                       'LATIN CAPITAL LETTER L')
override_strip_strange(nametable, 'LATIN SMALL LETTER L WITH STROKE',
                       'LATIN SMALL LETTER L')


casefoldinglist = []



for i,j in chartable.items():
    # Returns the characters in first and if overridden in second
    decomp = j.get_strip_strange(chartable)
    if decomp[0] != "":
        casefoldinglist.append( ( i,decomp[0], decomp[1] ) )

import string

# Create CJK - Chinese Japanese Korean unicode translation table
# range U+4E00-U+9FFF
print "Adding CJK Unicode range."
for i in range(40959, 19968,-1):
    hexval = hex(i);
    cjktrans = convertUnicodeToUTF8Octal( hexval[2:] )
    chartable[ string.upper(hexval[2:]) ] =  entry( [ hexval[2:],
                                                      '','','','','','','','','','','','','','' ] )
    casefoldinglist.append( ( string.upper(hexval[2:]), string.upper(hexval[2:]), 0 ) )

# Print case folding table
#print make_table_cpp(chartable, 'c_removeStrange', casefoldinglist)
table_list.append( conv_table( 'c_removeStrange', casefoldinglist, 'Table remove strange characters and convert accented characters to non-accented ones. For use in the SM. If a character does not exist in the table, it should be removed. If a space is returned, the character corresponds to a space.' ) )



print 'Writing files'
import os.path
save_cpp_files( chartable, table_list, headerfile, sourcefile, os.path.basename(headername) )

# Make case folding table
latin_1_list = []
for i,j in chartable.items():
    if string.atoi( i, 16 ) > 255:
        latinchars = j.get_latin_1_chars(chartable, nametable)
        if latinchars != "":
            latin_1_list.append( ( i, latinchars, 0 ) )

#print make_table_cpp( chartable, 'c_toLatin1', latin_1_list, 0 )

# Att plocka bort <> sed -e 's/<.*>//g'

#print list_to_string_rows(wrap_text('Detta var mig en väldigt lång text som handlar om olika saker som man kan hitta i hus', 70), '   ')
