/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef COMMANDLINEOPTIONHANDLER_H
#define COMMANDLINEOPTIONHANDLER_H

#include "config.h"

#include <vector>
#include <map>
#include <memory>
#include "MC2String.h"

/**
  *   Objects of this class parses the given arguments
  *   on then commandline and stores their values in
  *   given variables.
  *
  *   The general grammar for a command line is defined by 
  *   <TT>argv[0] [options] [tail]</TT>.
  *
  *   In the example <TT>cp -r src1 src2 dest </TT>, 
  *   <TT>argv[0]</TT> is @p cp, the option given is @p -r
  *   the tail (of length 3) is <TT>src1 src2 dest</TT>de.
  *
  *   The tailpart can be forced to have a minimum length.
  *
  *   If the handler is run in the so called lazyMode (not
  *   recommended), the parser acts a bit different. One could
  *   write for instance<BR>
  *
  *   <TT>cp src1 -r src2 dest</TT><BR>
  *
  *   or<BR>
  *
  *   <TT>cp src1 src2 dest -r</TT>
  *
  *   and the commandline will (probably) be parsed correctly. This
  *   feature is not very well tested, and therefore not recommended
  *   to use.
  *
  *   Options can have different types as specified in the <TT>enum
  *   value_t</TT>, and each type excluding @p presentVal supports
  *   the possibility of more than one value. The values of an option
  *   must immediately follow the option tag. Example:<BR>
  *
  *   <TT>prog -s ''first string'' string2 src dest</TT>.
  *
  *   Here the @p -s flag is a string option (@p stringVal ),
  *   specified two have two values. The first value becomes
  *   ''first string'', since it's enclosed in quotes. The second value
  *   is ''string2''. The possibility to compound values with quotes
  *   applies @e only to @p stringVal or tail items.
  *
  *   By default, the handler always specifies the options
  *
  *     - <TT>-h/--help</TT> @e presentVal option. The @p parse()
  *         method calls @p printUsage(cout) and exits with returncode 0.
  *     - <TT>v/--version</TT> @e presentVal option. The @p parse()
  *         method calls @p printVersion(cout) and exits with returncode 0.
  *     - <TT>p/--property</TT> @e stringVal option with one value. 
  *         Used to give the user the possibility to specify a 
  *         propertyfile. If the environment variable @p MC2_PROPERTIES 
  *         is defined as a filename, this name is used by default. 
  *         Otherwise the default filename is <TT>mc2.prop</TT> in 
  *         the current path.
  *
  *   A number of @p presentVal options or can be specified
  *   in the same token. For example
  *
  *   <TT>prog -hv</TT>
  *
  *   will print usage an version for @p prog and the exit. This feature
  *   naturally applies @e only to @p presentVal options.
  *
  *   The help feature of the class utilizes the environment variable
  *   COLUMNS. If it's defined and exported from the parent shell, all help
  *   texts is nicely formatted in the terminal window.
  *
  */
class CommandlineOptionHandler {
   public :
      /**
        *   Create a new commandline-option object.
        *   @param   argc           The number of argumens on
        *                           the commandline.
        *   @param   argv           The vector with all the
        *                           commandline arguments.
        *   @param   minTailLength  The minimum number of args
        *                           given in the tail part.
        *   @param   lazyMode       Use lazyMode. See classdescription.
        */
      CommandlineOptionHandler(int argc, char** argv,
                               int minTailLength = 0,
                               bool lazyMode = false);

      /**
        *   Delete this object, and release memory allocated here.
        */
      virtual ~CommandlineOptionHandler();

      /**
       * Deletes the strings in string options.
       * Too many depends on this not being deleted in destructor.
       */
      void deleteTheStrings();

      /**
        *   The possible value types for the options.
        */
      enum value_t {
         /** 
           *   The value of this command line option is a single character.
           *   The optionValue-pointer must point to a "char".
           */
         characterVal,

         /**
           *   The value of this command line option is a string. The 
           *   optionValue-pointer must point to a "char*".
           */
         stringVal,

         /**
           *   The value of this command line option is true if present,
           *   false otherwise. The default value must be 'T' (for true)
           *   and 'F' (for false) and the optionValue-pointer must point
           *   to a "bool".
           */
         presentVal,

         /**
           *   The value of this command line option is an integer. The 
           *   optionValue-pointer must point to a "int".
           */
         integerVal,

         /**
          *    The value of this command line option is an unsigned 32-bit
          *    integer. The optionValue-pointer must point to a "uint32".
          */
         uint32Val,

         /**
           *   The value of this command line option is a float. The 
           *   optionValue-pointer must point to a "float64".
           */
         floatVal,

         /**
          * The value of this option is uint32, but each possible
          * value also has it's own long option that works as a presentVal.
          */
         singleChoiceVal,
      };

      /**
       * Returns the character used to print the values on the
       * commandline. uint32Option uses 'u', stringOption uses 's',
       * and so on.
       * @param valueType The type of the option.
       * @return The character to use.
       */
      static inline char getValueChar(enum value_t valueType);

      /**
       *    Set a program summary description.
       *
       *    @param   summary     The summary string (usually long).
       */
      inline void setSummary(const char* summary);

      /**
       *    Set the help text applied to commandline tail.
       *
       *    @param   tailHelp    The tail help string (usually short).
       *                         Example: <TT>"sourcefile destfile"</TT>.
       */
      inline void setTailHelp(const char* tailHelp);

      /**
        *   Add an option to the list of possible options.
        *   
        *   @param   shortName      The shortname of this option
        *                           (e.g. @p -h ). No other
        *                           added option may have the same
        *                           shortName. Set to empty string if
        *                           no shortName shall be added.
        *   @param   longName       The long name of this option
        *                           (e.g. @p --help ). No
        *                           other option may have the same
        *                           longName.
        *   @param   valueType      The type of the variable where
        *                           the value will be stored.
        *   @param   nbrValues      The number of values for this
        *                           option.
        *   @param   optionValue    Pointer to the variable where
        *                           the value will be stored.
        *   @param   defaultValue   The defaultvalue of this option.
        *   @param   helpText       Help that will be displayed to
        *                           the user.
        *
        *   @see  value_t
        *   @see  Option
        */
      void addOption(const char* shortName, const char* longName, 
                     value_t valueType, uint32 nbrValues,
                     void* optionValue, const char* defaultValue,
                     const char* helpText);

      /**
       *    Adds an option with the type uint32. Useful for avoiding
       *    a lot of sprintfs in the code.
       */
      void addUINT32Option(const char* shortName,
                           const char* longName, 
                           uint32 nbrValues,
                           uint32* optionValue,
                           uint32 defaultValue,
                           const char* helpText);

      /**
       * Adds a singleChoiceOption, which is a lot like a uint32option
       * except that each defined value has it's own longname.  The
       * longnames are defined by a pair of MC2String or char* where
       * the first value is the longname (including the '--' prefix)
       * and the second is a short help string for the option.
       * @param shortName The shortname (e.g. @p -h ). No other added
       *                  option may have the sameshortName. May be
       *                  empty.
       * @param firstLongName Forward iterator referring to the first
       *                      pair that defines a longName.
       * @param lastLongName Forward iterator representing the end
       *                     (i.e. one past the last) of the longname
       *                     pairs.
       * @param optionValue pointer to the variable that will receive
       *                    the chosen value.
       * @param defaultValue The default value of this option.
       * @param helpText The helptext for the total option.
       */
      template<class InIt>
      void addSingleChoiceOption(const MC2String& shortName, 
                                 InIt firstLongName, InIt lastLongName, 
                                 uint32* optionValue, uint32 defaultValue,
                                 const char* helpText);
      /** 
        *   Parse the command line.
        *   @param   handleErrors If true parse() will complain about
        *            unknown options, etc.
        *   @return  True on success, oterwise false. If false is
        *            returned, the parser has failed to interpret
        *            the commandline, and nothing can be expected
        *            from the results. Cleanup and exit is recommended.
        */
      bool parse(bool handleErrors = true);

      /**
        *   @name User messages.
        */
      //@{
         /**
          *    Print help and usage.
          *
          *    @param   os    The outstream on which to print.
          */
         virtual void printHelp(ostream& os) const;

         /**
          *    Print summary information.
          *
          *    @param   os    The outstream on which to print.
          */
         virtual void printSummary(ostream& os) const;

         /**
           *   Print usage and help text for all the options on
           *   the specified outstream.
           *
           *   @param   os    The outstream on which to print,
           *                  use for example @p cerr if error.
           */
         virtual void printUsage(ostream& os) const;

         /**
           *   Print the version of this application on the specified
           *   outstream.
           *
           *   @param   os    The outstream on which to print.
           */
         virtual void printVersion(ostream& os) const;
      //@}

      /**
       *    Get the property file name.
       *
       *    @return  The property filename specified on commandline
       *             or else the default. See class description.
       */
      inline char* getPropertyFileName() const;

      /**
       *    Get the length of the command line tail part.
       *
       *    @return  The length of the command line tail part.
       */
      inline uint16 getTailLength() const;

      /**
       *    Get the tail token with given index.
       *
       *    @param   index    The index in the tail vector,
       *                      <TT>0 <= index < getTailLength()</TT>.
       *
       *    @return  The tail token with given index.
       */
      inline const char* getTail(uint16 index) const;

   private :
      /**
        *   The command line option list consists of objects of this class.
        */
      struct Option {
            /**
              *   Create one option and set all the necessery values,
              *
              *   @param   shortName   The shortname of this option (e.g 'h').
              *   @param   longName    The long name of this option (e.g "help")
              *   @param   valueType   The type of the variable where the value
              *                        will be stored.
              *   @param   nbrValues   The number of values for
              *                        this option.
              *   @param   optionValue Pointer to the variable where the value
              *                        will be stored.
              *   @param   defaultValue   The defaultvalue of this option.
              *   @param   helpText    Help that will be displayed to the user.
              */
            Option(const char* shortName, const char* longName, 
                   value_t valueType, uint32 nbrValues,
                   void* optionValue, const char* defaultValue,
                   const char* helpText);

            /**
              *   Delete this object.
              *
              *   <I><B>NB!</B> If there are any strings
              *   among the options that have been allocated here they are
              *   <B>not</B> deleted since they probably will be used 
              *   somewhere else!</I>
              */
            virtual ~Option();

            /**
              *   @name Set option.
              *   Set the value(s) of this option.
              */
            //@{
               /**
                 *   Set the value corresponding to the given string.
                 *
                 *   @param   index    The value index to set,
                 *                     0 <= index < nbrValues.
                 *   @param   newVal   The new value of this option. Stored
                 *                     in the variable pointed to by the 
                 *                     m_optinoValue-member.
                 *   @param   option   The actual option string found on the
                 *                     commandline. Should match
                 *                     either shortName or longName of
                 *                     the Option.
                 *   @return  True if the value is set, false otherwise.
                 */
               virtual bool setValue(const uint32 index,
                                     const MC2String& newVal,
                                     const MC2String& option);

               /**
                 *   Set the default value of this option.
                 *   @return  True if the valueis set, false otherwise.
                 */
               inline bool setDefaultValue();
            //@}
            
            /**
             *    Print help text.
             *
             *    @param   os    The outstream on which to print.
             */
            virtual void printHelpText(ostream& os) const;

            /**
              *   Find out if this option have a value or not.
              *   @param option The name of the option actrually used
              *                 on the commandline.
              *   @return  True if this option has a value (e.g. 
              *            <TT>n 8</TT>) and false otherwise (e.g @p -h ).
              */
            virtual bool hasValue(const MC2String& option) const;

            /** 
              *   The short name for this option.
              */
            MC2String shortName;

            /**
              *   The full name of this option.
              */
            MC2String longName;

            /**
              *   The type of the variable where the result of this option
              *   should be written.
              */
            value_t valueType;

            /**
              *   Pointer to the variable where the result is stored.
              */
            void* optionValue;

            /**
              *   The default value of this option.
              *   Must be of the type given
              *   in the valueType-field.
              */
            MC2String defaultValue;

            /** 
              *   The helptext displayed for this option.
              *   E.g "Shows this help message".
              */
            MC2String helpText;

            /**
              *   The maximum number of values. The most comman value
              *   of this must be 1, that indicate that only one value
              *   is allowed for this option.
              */
            uint32 nbrValues;
      };

      /**
       * Special option class for the singleChoiceOptions. 
       */
       class SingleChoiceOption : public Option {
       public:
          /**
           * Constructor. The arguments match
           * CommandlineOptionHandler::addSingleChoiceOption pretty
           * closely.
           * @param shortName The shortName of the option. May be
           *                  empty.
           * @param firstLongName The start iterator of the longname
           *                      pairs.
           * @param lastLongName The end iterator of the longname
           *                     pairs.
           * @param optionValue Pointer to the option-value-receiving
           *                    variable.
           * @param defaultValue The default value. 
           * @param helpText The long helptext.
           */
          template<typename InIt>
          SingleChoiceOption(const MC2String& shortName,
                             InIt firstLongName, InIt lastLongName, 
                             uint32* optionValue, uint32 defaultValue, 
                             const MC2String& helpText) : 
             Option(shortName.c_str(), "", singleChoiceVal, 1, 
                    optionValue, "", helpText.c_str()), 
             m_longOptions(firstLongName, lastLongName), 
             m_defaultValue(defaultValue)
          {}
          /** Virtual destructor. */
          virtual ~SingleChoiceOption()
          {}
          /** 
           * Prints the helptext for a specific option. 
           * @param os The ostream to print to.
           */
          virtual void printHelpText(ostream& os) const;
          /**
           * Called by the parsre when the value shoul be set.
           * @param index The index of the value. Not used by
           *              SingleChoiceOption.
           * @param val The option value as a string.
           * @param option The option name used on the commandline.
           */
          virtual bool setValue(const uint32 index, const MC2String& val, 
                                const MC2String& option);
          /**
           * Asks the option whether it expects a value or not.
           * @param option The actual option name found on the
           *               commandline.
           * @return True if option equals shortName and false
           *         otherwise.
           */
          virtual bool hasValue(const MC2String& option) const;
          
       private:
          /** Type to hold text pairs. */
          typedef vector<pair<MC2String,MC2String> > optionPairs_t;
          /** Holds the longnames and their helptexts.*/
          optionPairs_t m_longOptions;
          /** The default value. */
          uint32 m_defaultValue;
       };

      /**
        *   All the options that will be handled in this command line 
        *   option handler.
        */
      vector<Option *> m_options;

      /**
       *    A map to make lookups in the @p m_options vector
       *    effective if there is many of them.
       */
      map<MC2String, vector<Option *>::size_type> m_optmap;

      /**
       *    Vector containing the tail part of the commandline.
       */
      vector<MC2String> m_tail;

      /**
       *    The forced minimum length of the tail part.
       */
      int m_minTailLength;

      /**
       *    Run the parser in lazyMode.
       */
      bool m_lazyMode;

      /**
       *    Summary string that describes the program.
       */
      MC2String m_summary;

      /**
       *    Help string for the tail, put in the usage field.
       */
      MC2String m_tailHelp;

      /**
       *    Program name, automatically extracted in constructor.
       */
      MC2String m_programName;

      /**
        *   @name Commandline in raw format.
        *   Rawdata about the current commandline.
        */
      //@{
         /**
           *   The number of arguments on the commandline.
           */
         int m_argc;

         /**
           *   The arguments.
           */
         vector<MC2String> m_argv;
      //@}

      /**
        *   @name Default values.
        *   Members for default command-line options.
        */
      //@{
         /**
          *    The filename of the property file is stored here.
          */
         char* m_propertyFileName[1];

         /**
           *   The value of the "help"-option is stored here.
           */
         bool m_showHelp;

         /**
           *   The value of the "version"-option is stored here.
           */
         bool m_showVersion;

         /**
           *   The value of the -q/--no-debug option is stored here.
           */
         bool m_debugFlag;

         /**
          *    The properties to add to Properties.
          */
         char* m_properties;
      //@}

      /**
        *   Default constructor declaried private to avoid usage!
        */
      CommandlineOptionHandler() {}

      /**
        *   Loop over all the options and set their default values.
        */
      void setDefaultValues();

      /**
        *   @name Search 
        *   Search for an option with a given value.
        */
      //@{
         /**
           *   Get an option with the given name (long or short).
           *   
           *   @param   s  The name of the option.
           *   
           *   @return  An option with the same name as s.
           */
         inline Option* findOption(const MC2String& s) const;
      //@}
};

//--------------------------------------------------------------------------
//---------------------------------------------------- [ I N L I N E S ] ---
//--------------------------------------------------------------------------

inline char 
CommandlineOptionHandler::getValueChar(enum CommandlineOptionHandler::value_t valueType)
{
   static char valueChar[] = "cs iufu";
   return valueChar[valueType];
}


inline void
CommandlineOptionHandler::setSummary(const char* summary)
{
   m_summary = summary;
}

inline void
CommandlineOptionHandler::setTailHelp(const char* tailHelp)
{
   m_tailHelp = tailHelp;
}

inline char*
CommandlineOptionHandler::getPropertyFileName() const
{
   return m_propertyFileName[0];
}

inline uint16
CommandlineOptionHandler::getTailLength() const
{
   return (uint16)m_tail.size();
}

inline const char*
CommandlineOptionHandler::getTail(uint16 index) const
{
   typedef vector<MC2String>::size_type st;
   st _index = (st)index;
   return (_index < m_tail.size()) ? m_tail[_index].c_str() : 0;
}

inline bool
CommandlineOptionHandler::Option::setDefaultValue()
{
   return setValue(0, defaultValue, 
                   (shortName.empty() ? longName : shortName));
}

inline bool
CommandlineOptionHandler::Option::hasValue(const MC2String&) const
{
   return (valueType != presentVal);
}

inline CommandlineOptionHandler::Option*
CommandlineOptionHandler::findOption(const MC2String& s) const
{
   typedef map<MC2String, vector<Option *>::size_type>::const_iterator ci;

   ci i = m_optmap.find(s);
   return (i != m_optmap.end()) ? m_options[i->second] : 0;
}


template<class InIt>
void CommandlineOptionHandler::addSingleChoiceOption(const MC2String& shortName, 
                                                     InIt firstLongName, 
                                                     InIt lastLongName, 
                                                     uint32* optionValue, 
                                                     uint32 defaultValue,
                                                     const char* helpText)
{
   for(InIt i = firstLongName; i != lastLongName; ++i){
      if(m_optmap.end() != m_optmap.find( MC2String( i->first ))){
         cerr << "Option " << i->first 
              << " doubled in CommandlineOptionHandler!" << endl;
      }
   }
   if(!shortName.empty() && 
      m_optmap.end() != m_optmap.find( MC2String( shortName ))){
      cerr << "Option " << shortName
           << " doubled in CommandlineOptionHandler!" << endl;
   }
   auto_ptr<SingleChoiceOption> sco(
      new SingleChoiceOption(shortName,
                             firstLongName, lastLongName, 
                             optionValue, defaultValue, helpText) );
   m_options.push_back(sco.get());
   sco.release();
   const vector<Option*>::size_type index = m_options.size() - 1;
   if( !shortName.empty() ){
      m_optmap[shortName] = index;
   }
   for(InIt i = firstLongName; i != lastLongName; ++i){
      m_optmap[MC2String(i->first)] = index;   
   }
}


#endif

