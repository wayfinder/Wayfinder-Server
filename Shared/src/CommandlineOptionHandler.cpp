/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

/* get REG_EIP or REG_RIP from ucontext.h */
#ifndef _GNU_SOURCE
#   define _GNU_SOURCE
#endif
#include <features.h>
#ifndef __USE_GNU
#  define __USE_GNU
#endif
#include <ucontext.h>

#include "CommandlineOptionHandler.h"
#include <cctype>
#include <cstdlib>
#include <sstream>
#include <signal.h>
#include "Utility.h"
#include "SysUtility.h"
#include "sockets.h"
#include "Properties.h"
#include "StringUtility.h"

void dumpHandler(int s)
{
   static int r = 0;

   if (r == 0) {
      r++;
      cerr << "Caught signal: " << s << endl;
      SysUtility::dumpProcessInfo();
   }
   kill(getpid(), SIGTERM);
}                 

#include <stdio.h>
#include <signal.h>
#include <execinfo.h>

#ifdef __i386__
#  define ADDR_REG REG_EIP
#elif defined(__x86_64__)
#  define ADDR_REG REG_RIP
#endif

void bt_sighandler(int sig, siginfo_t *info,
                   void *secret)
{
   static volatile int r = 0;

   if (r == 0) {
      ++r;
   
      ucontext_t *uc = (ucontext_t *)secret;
      
      // Do something useful with siginfo_t 
      if (sig == SIGSEGV) {
         fprintf(stderr, "Got signal %d, faulty address is %p, "
                "from %p\n", sig, info->si_addr, 
                (void*)uc->uc_mcontext.gregs[ADDR_REG]);
      } else {
         fprintf(stderr, "Got signal %d\nn", sig);
      }
      
      // Trace pointers are put here
      void *trace[512];
      int trace_size = backtrace(trace, sizeof(trace) / sizeof(trace[0]) );
      // overwrite sigaction with caller's address
      trace[1] = (void *) uc->uc_mcontext.gregs[ADDR_REG];
      
      char** messages = backtrace_symbols(trace, trace_size);
      /* skip first stack frame (points here) */
      fprintf(stderr, "[bt] Execution path: \n");
      for (int i = 1; i < trace_size; ++i) {
         fprintf(stderr, "[bt] %s\n", messages[i]);
      }
      fprintf(stderr, "\n");
      // Re-kill us
      kill( getpid(), SIGKILL );
   }
   
}

CommandlineOptionHandler::CommandlineOptionHandler( int argc, char** argv, 
                                                    int minTailLength, 
                                                    bool lazyMode) : 
   m_minTailLength(minTailLength),
   m_lazyMode(lazyMode),
   m_argc(argc),
   m_properties(NULL)
{
   //for(int i = 0; i < argc; i++)
   //   cout << "argv[" << i << "] = " << argv[i] << endl;
   //cout << endl;
   
   // shouldn't be here, but there isn't any better place right now
#if 1
   static struct sigaction sa;
   sa.sa_sigaction = bt_sighandler;
   sigemptyset(&sa.sa_mask);
   sa.sa_flags = SA_RESTART | SA_SIGINFO;

   sigaction(SIGSEGV, &sa, NULL);
   sigaction(SIGINT, &sa, NULL);
   sigaction(SIGABRT, &sa, NULL);
#else
#if 0
   signal(SIGSEGV, dumpHandler);
   signal(SIGINT, dumpHandler);
#endif
#endif
   // Test to have pipe off allways
   SysUtility::ignorePipe();
   SysUtility::setChangePipeSignal( false );

   for(int i = 0; i < argc; i++){
      m_argv.push_back(MC2String(argv[i]));
   }
   m_programName = m_argv[0];

   MC2String::size_type pos = m_programName.find_last_of('/');
   if(pos != MC2String::npos){
      m_programName.erase(0, pos+1);
   }

   MC2String propenv;
   if(char* env = getenv("MC2_PROPERTIES")){
      propenv = env;
   } else {
      propenv = "mc2.prop";
   }

   //m_propertyFileName is of type char*[1] 
   m_propertyFileName[0] = NULL;
   
   
   addOption("-h", "--help", presentVal, 0, &m_showHelp, "F",
             "Displays this help message");
   addOption("-v", "--version", presentVal, 0, &m_showVersion, "F",
             "Displays the version of the application");
   addOption("-p", "--property", stringVal, 1, m_propertyFileName,
             propenv.c_str(), "Use given property file");
   addOption("-q", "--no-debug", presentVal, 0, &m_debugFlag,
             "F", "Turn of all debug output");
   addOption( "-P", "--setproperty", stringVal, 1, &m_properties,
              "", "Add property to Properties. Use like this: "
              "\"BMC=2.4.,PPM=34,PPX=4\"" );
   
}

CommandlineOptionHandler::~CommandlineOptionHandler()
{
   //delete [] m_propertyFileName[0];
   //delete [] m_properties;
   vector<Option *>::iterator iter;
   for( iter = m_options.begin(); iter != m_options.end(); ++iter )
      delete *iter;
}

void
CommandlineOptionHandler::deleteTheStrings() {
   vector<Option *>::iterator iter;
   for( iter = m_options.begin(); iter != m_options.end(); ++iter ) {
      if ( (*iter)->valueType == stringVal ) {
         delete[] ((char**) (*iter)->optionValue)[ 0 ];
      }
   }
}

void
CommandlineOptionHandler::addOption(const char* shortName, 
                                    const char* longName,
                                    value_t valueType, 
                                    uint32 nbrValues,
                                    void* optionValue,
                                    const char* defaultValue,
                                    const char* helpText)

{
   MC2String _shortName(shortName);
   MC2String _longName(longName);

   //cout << "addOption ("
   //     << shortName << ", "
   //     << longName << ", "
   //     << optionValue << ")" << endl;
   
   if( valueType == singleChoiceVal ) {
      //cannot add singleChoice with this function
      cerr << "singleChoice option '" << shortName <<  "':'" << longName
           << "' cannot be added through "
           << "CommandlineOptionHandler::addOption" << endl;
      exit(1);
   }

   if(  ( (_shortName != "" ) && 
          m_optmap.find(_shortName) != m_optmap.end() )
         || m_optmap.find(_longName) != m_optmap.end()) {
      // This option already in table!!
      // FATAL error!
      cerr << "Option doubling in CommandlineOptionHandler!!\n" 
           << "\tshortName: " << shortName
           << "\tlongName: " << longName << endl;
      exit(1);
   }

   Option* newOpt = new Option(shortName, longName, valueType,
         nbrValues, optionValue, defaultValue, helpText);

   m_options.push_back(newOpt);
   vector<Option *>::size_type si = m_options.size()-1;
   
   if ( _shortName != "" ) {
      m_optmap[_shortName] = si;
   }
   m_optmap[_longName] = si;
}

void
CommandlineOptionHandler::addUINT32Option(const char* shortName,
                                          const char* longName,
                                          uint32 nbrValues,
                                          uint32* optionValue,
                                          uint32 defaultValue,
                                          const char* helpText)
{
   char defaultString[30];
   sprintf(defaultString, "%u", defaultValue);
   addOption(shortName, longName, CommandlineOptionHandler::uint32Val,
             nbrValues, optionValue, defaultString, helpText);
}

bool
CommandlineOptionHandler::parse(bool handleErrors)
{
   typedef MC2String::size_type st;
   
   setDefaultValues();

/*
   {//----------------------------------------------TEMPORARY TESTCODE
      cout << "TABLE DUMP:" << endl;
      typedef vector<Option *>::const_iterator ci;
      for(ci i = m_options.begin(); i != m_options.end(); i++)
         cout << "\t" << (*i)->shortName << "/" << (*i)->longName
              << " (" << (*i)->optionValue << ")" << endl;
   }//------------------------------------------------------------------
*/
   
   // Loop over all the given command line parameters

   Option* opt = 0;
   MC2String optionString = "";
   uint32 index = 0;
   uint32 nbrValues = 0;
   int i;                                 // Declare the counter
                                          // outside the loop
                                          // in order to be able
                                          // to read it after.

   for(i = 1; i < m_argc; i++) {
      MC2String arg = m_argv[i];
      uint32 arglen = arg.size();
      if(arg.substr(0, 1) == "-") {       // New option or negative number
         if(nbrValues == 0) {             // Not waiting for value, so
            opt = 0;                      // consider it's a new option
            index = 0;
         }
         if(arg.substr(0, 2) == "--") {   // Longname given
            if(arglen == 2) {
               // We've found the special '--' option
               // that indicates end of options!
               i++;
               break;
            }
            st pos = arg.find("=");
            if(pos != MC2String::npos) {     // '=' found
               optionString = arg.substr(0, pos);
               opt = findOption( optionString );
               if(opt && opt->hasValue(optionString)) {
                  opt->setValue(0, arg.substr(pos+1, arg.size() - pos),
                                optionString );
                  opt = 0;
               }
               else if (!opt && handleErrors) {
                  mc2log << error << "Option '" << arg << "' not found" << endl;
                  return false;
               }
               else {
                  mc2log << error << "Found '=' in boolean option '" << arg
                         << "'" << endl;
                  return false;
               }
            }
            else {                        // '=' NOT found
               optionString = arg;
               opt = findOption(optionString);
               if(opt && opt->hasValue(optionString)) {
                  mc2log << error << "Value-longoption '" << arg
                         << "' without '='" << endl;
                  return false;
               }
               else if(opt) {
                  opt->setValue(0, "T", optionString);
                  opt = 0;
               }
               else if (handleErrors) {
                  mc2log << error << "Option '" << arg << "' not found!!!"
                         << endl;
                  return false;
               }
            }
         }
         else {                           // Shortname or negative number
            if(nbrValues > 0) {           // Waiting for value
               if(opt && opt->setValue(index++, arg, optionString)) {
                  if(--nbrValues == 0) {
                     opt = 0;
                     index = 0;
                  }
                  continue;
               }
               else {
                  mc2log << error << "CommandLineOptionHandler::parse(): Parse"
                                     " error at '" << arg << "'" << endl;
                  return false;
               }
            }
            if(arglen > 2) {              // more than one option
               for(st j = 1; j < arglen; j++) {
                  char _arg[3] = { '-', arg[j], '\0' };
                  optionString = _arg;
                  opt = findOption(optionString);
                  if(opt) {
                     if(opt->hasValue(optionString)) {
                        mc2log << error << "CommandLineOptionHandler::parse(): "
                                           "Option '" << _arg << "' given wo/ "
                                           "value" << endl;
                        return false;
                     }
                     opt->setValue(0, "T", optionString);
                     opt = 0;
                  }
                  else if (handleErrors) {
                     mc2log << error << "CommandLineOptionHandler::parse(): "
                                        "Option '" << _arg << "' not found"
                            << endl;
                     exit(1);
                  }
               }
            }
            else if(arglen == 2) {
               optionString = arg;
               opt = findOption(optionString);
               if(!opt && handleErrors) {
                  mc2log << error << "CommandLineOptionHandler::parse(): "
                                     "Option '" << arg << "' not found" 
                         << endl;
                  return false;
               }
               if(opt && opt->hasValue(optionString)) {
                  nbrValues = opt->nbrValues;
                  index = 0;
                  continue;
               }
               else if (opt){
                  opt->setValue(0, "T", optionString);
                  opt = 0;
               }
            }
            else {
               mc2log << error << "CommandLineOptionHandler::parse(): Illegal "
                                  "option '-'" << endl;
               return false;
            }
         }
      }
      else {                              // This must be a value
                                          // or tail
         if(opt) {
            opt->setValue(index++, arg, optionString);
            if(--nbrValues == 0) {
               opt = 0;
               index = 0;
            }
            continue;
         }
         else {
            if(m_lazyMode) {
               m_tail.push_back(arg);
               continue;
            }
            else
               break;
         }
      }
   }

   for(int j = i; j < m_argc; j++) {
      m_tail.push_back(MC2String(m_argv[j]));
      mc2dbg2 << "CommandLineOptionHandler::parse(): push back: "
              << m_tail[j-i] << endl;
   }

   if(!(m_showHelp || m_showVersion)
         && (m_tail.size() < (vector<MC2String>::size_type)m_minTailLength)) {
      mc2log << error << "CommandLineOptionHandler::parse(): Tail too small!"
             << endl;
      return false;
   }
   
   // Check if the help or the version option was given.
   if(handleErrors && (m_showHelp || m_showVersion)) {
      if (m_showHelp) {
         printHelp(cout);
      }
      if (m_showVersion) {
         printVersion(cout);
      }
      exit(0);
   }

   if ( m_properties != NULL ) {
      Properties::setCohProperties( m_properties );
   }

   MC2Logging::getInstance().setDebugOutput(!m_debugFlag);

   return true;
}

void
CommandlineOptionHandler::printHelp(ostream& os) const
{
   typedef vector<Option *>::const_iterator ci;

   printSummary(os);
   printUsage(os);
}

void
CommandlineOptionHandler::printSummary(ostream& os) const
{
   typedef MC2String::size_type st;
 
   uint16 columns;
   char* cols = getenv("COLUMNS");
   if(cols != NULL) {
      if(atoi(cols) > 10) {
         columns = atoi(cols) - 10;
      }
      else {
         columns = atoi(cols);
      }
   }
   else {
      columns = 70;
   }
      
   columns -= 8;           // -= tabspace
  
   os << endl
      << MC2String(columns, '-') << endl
      << "  " << m_programName << endl
      << MC2String(columns, '-') << endl;
   
   if(m_summary.size() > 0) {
      os << endl << "Summary:" << endl;

      MC2String s = m_summary;
      st length = 0;
      st pos, pos2;

      while(s[0] == ' ' || s[0] == '\t')
         s.erase(0, 1);
      os << '\t';
      do {
         pos = s.find(' ');
         if(pos == MC2String::npos)
            pos = s.size();
         pos2 = s.find('\n');
         if(pos2 != MC2String::npos && pos2 < pos)
            pos = pos2;
         if(length+pos+1 > columns) {
            os << "\n\t";
            length = 0;
         }
         else {
            if(length > 0)
               os << ' ';
            os << s.substr(0, pos);
            s.erase(0, pos);
            length += pos + 1;
            while(s[0] == ' ' || s[0] == '\n') {
               if(s[0] == ' ')
                  s.erase(0, 1);
               else {
                  s.erase(0,1);
                  os << "\n\t";
                  length = 0;
               }
            }
         }
      } while (s.size() > 0);
   }
   os << endl;
}

void
CommandlineOptionHandler::printUsage(ostream& os) const
{
   typedef vector<Option *>::const_iterator ci;

   uint16 columns;
   char* cols = getenv("COLUMNS");
   if(cols != NULL) {
      if(atoi(cols) > 10) {
         columns = atoi(cols) - 10;
      }
      else {
         columns = atoi(cols);
      }
   }
   else {
      columns = 70;
   }
      
   columns -= 8;           // -= tabspace

   os << endl
      << "Usage: " << m_programName << " [options]";
   if(m_tailHelp.size() > 0)
      os << " " << m_tailHelp;
   os << endl;

   os << endl
      << "Specification of options:" << endl
      << MC2String(columns, '-') << endl;

   for(ci i = m_options.begin(); i != m_options.end(); i++){
      (*i)->printHelpText(os);
   }
}

void
CommandlineOptionHandler::printVersion(ostream& os) const
{
   os << m_argv[0] << " was compiled "
      << __DATE__ << ", " << __TIME__ << endl;
}

void
CommandlineOptionHandler::setDefaultValues()
{
   typedef vector<Option *>::const_iterator ci;

   for(ci i = m_options.begin(); i != m_options.end(); i++)
      (*i)->setDefaultValue();
}

// ========================================================================
//                                                                 Option =
CommandlineOptionHandler::Option::Option(const char* shortName, 
                                         const char* longName,
                                         value_t valueType, 
                                         uint32 nbrValues,
                                         void* optionValue,
                                         const char* defaultValue, 
                                         const char* helpText)
{
   this->shortName = MC2String(shortName);
   this->longName = MC2String(longName),
   this->valueType = valueType;
   this->nbrValues = nbrValues;
   this->optionValue = optionValue;
   this->defaultValue = MC2String(defaultValue);
   this->helpText = MC2String(helpText);

   // Initialize optionValue if it's a stringVal.
   if( valueType == stringVal ) {
      for( uint32 i = 0; i < nbrValues; i++ )
         ((char**) optionValue)[i] = NULL;
   }
}

CommandlineOptionHandler::Option::~Option()
{
   if ( valueType == stringVal ) {
      // Too many depends on this not being deleted
//      delete[] ((char**) optionValue)[ 0 ];
   }
}

bool
CommandlineOptionHandler::Option::setValue(const uint32 index,
                                           const MC2String& val,
                                           const MC2String& option)
{
   // The return value
   bool retVal = false;
   const char* newVal = val.c_str();

   mc2dbg4 << "CommandLineOptionHandler::Option::setValue() value '" << val 
           << "' for option '" << shortName << "' (" << optionValue << ")" 
           << endl;

   if (index < nbrValues || valueType == presentVal) {
      switch (valueType) {
         case (characterVal) :
               // Set character value
               if (strlen(newVal) == 1) {
                  ((char*) optionValue)[index] = newVal[0];
                  retVal = true;
               }

            break;

         case (stringVal) : {
               // Set string value
               char* dest = ((char**) optionValue)[index];
               delete[] dest;
               if (strlen(newVal) > 0) {
                  dest = new char[strlen(newVal)+1];
                  strcpy(dest, newVal);
               } else {
                  dest = NULL;
               }
               ((char**) optionValue)[index] = dest;
               retVal = true;

            } break;

         case (presentVal) :
               // Set boolean value
               if ((strlen(newVal) > 0) && (newVal[0] == 'T')) {
                  ((bool*) optionValue)[index] = true;
               } else {
                  ((bool*) optionValue)[index] = false;
               }
               retVal = true;
            break;

         case integerVal:
            {
               // Set integer value
        
               if (strlen(newVal) > 0) {
                  int theValue = 0;
                  char* foo;
                  if (Utility::getNumber(newVal, foo, theValue)) {
                     // Integer found
                     ((int*) optionValue)[index] = theValue;
                     retVal = true;
                  }
               }
        
                                
            } break;

         case uint32Val:
            {
               // Set unsigned int value

               if(strlen(newVal) > 0) {
                  // Changed to 0 to allow 0xffff, etc.
                  unsigned long theValue = strtoul(newVal, NULL, 0);
                  if((theValue == ULONG_MAX) && (errno != 0)) {
                     cerr << "CommandlineOptionHandler FATAL error: "
                          << strerror(errno) << endl
                          << "Value not set correctly!" << endl
                          << "Exiting!" << endl;
                     exit(1);
                  }
                  ((uint32*) optionValue)[index] = theValue;
                  retVal = true;
               }
            } break;

         case (floatVal) : {
               // Set float value
               if (strlen(newVal) > 0) {
                  float64 theValue = 0;
                  char* foo;
                  if (Utility::getFloat64(newVal, foo, theValue)) {
                     // Integer found
                     ((float64*) optionValue)[index] = theValue;
                     retVal = true;
                  }
               }
            } break;
      case singleChoiceVal: 
         cerr << "singleChoiceVal shouldn't be handled here!" << endl;
         exit(1);
         break;
         default :
            cout << "value type " << (uint32) valueType 
                 << " not implemented" << endl;
      }
   }

   return (retVal);
}

int helpTextWidth(int tabWidth = 8)
{
   int columns = 70;
   char* cols = getenv("COLUMNS");
   if(cols != NULL) {
      char* end = cols;
      const long tmp = strtol(cols, &end, 0);
      if(cols != end){
         if(tmp > 10) {
            columns = tmp - 10;
         } else {
            columns = tmp;
         }
      }
   }
   return columns - tabWidth;
}

void
CommandlineOptionHandler::Option::printHelpText(ostream& os) const
{
   typedef MC2String::size_type st;

   os << "[ ";
   if(valueType != presentVal) {
      if (nbrValues == 1) {
         if( !shortName.empty() ){
            os << "(" << shortName << " | " << longName << "=)";
         } else {
            os << longName << "=";
         }
      } else {
         os << shortName << " ";
      }
   } else {
      if( ! shortName.empty() ){
         os << shortName << " | " << longName;
      } else {
         os << longName;
      }
   }

   if( valueType != presentVal ){
      for(uint32 i = 1; i <= nbrValues; ++i){
         os << ' ' << CommandlineOptionHandler::getValueChar(valueType) << i;
      }
   }
   os << " ]" << endl;
   
   MC2String s = helpText;
   st length = 0;
   st pos, pos2;
   uint16 columns = helpTextWidth();
   
   while(s[0] == ' ' || s[0] == '\t')
      s.erase(0, 1);
   os << '\t';
   do {
      pos = s.find(' ');
      if(pos == MC2String::npos)
         pos = s.size();
      pos2 = s.find('\n');
      if(pos2 != MC2String::npos && pos2 < pos)
         pos = pos2;
      if(length+pos+1 > columns) {
         os << "\n\t";
         length = 0;
      }
      else {
         if(length > 0)
            os << ' ';
         os << s.substr(0, pos);
         s.erase(0, pos);
         length += pos + 1;
         while( s.length() > 0 && (s[0] == ' ' || s[0] == '\n') ) {
            if(s[0] == ' ')
               s.erase(0, 1);
            else {
               s.erase(0,1);
               os << "\n\t";
               length = 0;
            }
         }
      }
   } while (s.size() > 0);
   os << endl << endl;
}

// template<class T, class F>
// struct pair_mem_fun_ref_t : public unary_function<pair<F,F>, pair<T,T> >
// {
//    typedef typename pair<T,T>::first_type function_result_type;
//    typedef typename pair<F,F>::first_type function_class_type;
//    typedef function_result_type (function_class_type::*function_type)();
//    explicit pair_mem_fun_ref_t(function_type f) : m_f(f) {}
//    pair<T,T> operator()(const pair<F,F>& arg)
//    {
//       return pair<T,T>( (arg.first.*m_f)(), (arg.second.*m_f)());
//    }
// private:
//    function_type m_f;
// };

// template<typename T, typename F>
// pair_mem_fun_ref_t<T, F> pair_mem_fun_ref(T (F::*pm)())
// {
//    return pair_mem_fun_ref_t<T,F>(pm);
// }

bool
CommandlineOptionHandler::SingleChoiceOption::setValue(const uint32 index,
                                                       const MC2String& val,
                                                       const MC2String& option)
{
   if(option == shortName){
      *((uint32*)optionValue) = strtoul(val.c_str(), NULL, 10);
      return true;
   } else {
      for(optionPairs_t::iterator it = m_longOptions.begin(); 
          it != m_longOptions.end(); ++it){
         if(it->first == option){
            const uint32 uVal = std::distance(m_longOptions.begin(), it);
            *((uint32*)optionValue) = uVal;
            return true;
         }
      }
   }
   return false;
}

inline bool
CommandlineOptionHandler::SingleChoiceOption::
hasValue(const MC2String& option) const
{
   return option == shortName;
}

void
CommandlineOptionHandler::SingleChoiceOption::printHelpText(ostream& os) const
{
   const MC2String::size_type lineWidth = helpTextWidth(0);
   MC2String::size_type sz = 2; //"[ "
   os << "[ ";
   if( ! shortName.empty() ) {
      const char unsignedOne[] = " u1 | ";
      sz += shortName.length() + sizeof(unsignedOne) - 1;
      os << shortName << unsignedOne;
   }
   for(optionPairs_t::const_iterator i = m_longOptions.begin();
       i != m_longOptions.end(); ++i){
      optionPairs_t::const_iterator next = i;
      const MC2String separator = (++next != m_longOptions.end() ? " | " : " ");

      const MC2String::size_type addon = i->first.length() + separator.length();
      if((sz + addon) >= lineWidth){
         os << "\n  ";
         sz = 2;//"  "
      }
      os << i->first << separator;
      sz += addon;
   }
   os << "]\n\t";
   const uint16 cols = helpTextWidth();
   MC2String localHelp = StringUtility::trimStartEnd(helpText);

   MC2String::size_type length = 0;
   while(!localHelp.empty()){
      MC2String::size_type pos = localHelp.find(' ');
      if(pos == MC2String::npos){
         pos = localHelp.size();
      }
      MC2String::size_type pos2 = localHelp.find('\n');
      if(pos2 == MC2String::npos){
         pos2 = localHelp.size();
      }
      pos = std::min(pos, pos2);
      if(length+pos+1 > cols){
         os << "\n\t";
         length = 0;
      } else {
         if(length > 0){
            os << ' ';
         }
         os << localHelp.substr(0, pos);
         localHelp.erase(0,pos);
         length += pos + 1;
         while( !localHelp.empty() && 
                (localHelp[0] == ' ' || localHelp[0] == '\n')){
            if(localHelp[0] == '\n'){
               os << "\n\t";
               length = 0;
            }
            localHelp.erase(0,1);
         }
      }
   }

   MC2String::size_type maxOptionLength = 0;
   MC2String::size_type maxHelpLength = 0;
   for(optionPairs_t::const_iterator i = m_longOptions.begin(); 
       i != m_longOptions.end(); ++i){
      maxOptionLength = std::max(maxOptionLength, i->first.length());
      maxHelpLength   = std::max(maxHelpLength,  i->second.length());
   }   

   for(optionPairs_t::const_iterator i = m_longOptions.begin(); 
       i != m_longOptions.end(); ++i){
      os << "\n\t" << i->first 
         << MC2String((maxOptionLength - i->first.length()) + 2, ' ') 
         << i->second
         << MC2String((maxHelpLength - i->second.length()) + 3, ' ')
         << "( " << shortName << " " 
         << distance(m_longOptions.begin(), i) << " )";
   }

   os << "\n" << endl;
}
