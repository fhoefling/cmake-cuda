/*=========================================================================

  Program:   CMake - Cross-Platform Makefile Generator
  Module:    $RCSfile: cmStringCommand.h,v $
  Language:  C++
  Date:      $Date: 2006/03/10 16:13:14 $
  Version:   $Revision: 1.17 $

  Copyright (c) 2002 Kitware, Inc., Insight Consortium.  All rights reserved.
  See Copyright.txt or http://www.cmake.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef cmStringCommand_h
#define cmStringCommand_h

#include "cmCommand.h"

/** \class cmStringCommand
 * \brief Common string operations
 *
 */
class cmStringCommand : public cmCommand
{
public:
  /**
   * This is a virtual constructor for the command.
   */
  virtual cmCommand* Clone() 
    {
    return new cmStringCommand;
    }

  /**
   * This is called when the command is first encountered in
   * the CMakeLists.txt file.
   */
  virtual bool InitialPass(std::vector<std::string> const& args);

  /**
   * This determines if the command is invoked when in script mode.
   */
  virtual bool IsScriptable() { return true; }

  /**
   * The name of the command as specified in CMakeList.txt.
   */
  virtual const char* GetName() { return "STRING";}

  /**
   * Succinct documentation.
   */
  virtual const char* GetTerseDocumentation() 
    {
    return "String operations.";
    }
  
  /**
   * More documentation.
   */
  virtual const char* GetFullDocumentation()
    {
    return
      "  STRING(REGEX MATCH <regular_expression>\n"
      "         <output variable> <input> [<input>...])\n"
      "  STRING(REGEX MATCHALL <regular_expression>\n"
      "         <output variable> <input> [<input>...])\n"
      "  STRING(REGEX REPLACE <regular_expression>\n"
      "         <replace_expression> <output variable>\n"
      "         <input> [<input>...])\n"
      "  STRING(REPLACE <match_expression>\n"
      "         <replace_expression> <output variable>\n"
      "         <input> [<input>...])\n"
      "  STRING(COMPARE EQUAL <string1> <string2> <output variable>)\n"
      "  STRING(COMPARE NOTEQUAL <string1> <string2> <output variable>)\n"
      "  STRING(COMPARE LESS <string1> <string2> <output variable>)\n"
      "  STRING(COMPARE GREATER <string1> <string2> <output variable>)\n"
      "  STRING(ASCII <number> [<number> ...] <output variable>)\n"
      "  STRING(CONFIGURE <string1> <output variable>\n"
      "         [@ONLY] [ESCAPE_QUOTES])\n"
      "  STRING(TOUPPER <string1> <output variable>)\n"
      "  STRING(TOLOWER <string1> <output variable>)\n"
      "  STRING(LENGTH <string> <output variable>)\n"
      "  STRING(SUBSTRING <string> <begin> <length> <output variable>)\n"
      "REGEX MATCH will match the regular expression once and store the "
      "match in the output variable.\n"
      "REGEX MATCHALL will match the regular expression as many times as "
      "possible and store the matches in the output variable as a list.\n"
      "REGEX REPLACE will match the regular expression as many times as "
      "possible and substitute the replacement expression for the match "
      "in the output.  The replace expression may refer to paren-delimited "
      "subexpressions of the match using \\1, \\2, ..., \\9.  Note that "
      "two backslashes (\\\\1) are required in CMake code to get a "
      "backslash through argument parsing.\n"
      "REPLACE will match the given expression "
      "and substitute the replacement expression for the match "
      "in the output.  The replace expression may refer to paren-delimited "
      "subexpressions of the match using \\1, \\2, ..., \\9.  Note that "
      "two backslashes (\\\\1) are required in CMake code to get a "
      "backslash through argument parsing.\n"
      "COMPARE EQUAL/NOTEQUAL/LESS/GREATER will compare the strings and "
      "store true or false in the output variable.\n"
      "ASCII will convert all numbers into corresponding ASCII characters.\n"
      "CONFIGURE will transform a string like CONFIGURE_FILE transforms "
      "a file.\n"
      "TOUPPER/TOLOWER will convert string to upper/lower characters.\n"
      "LENGTH will return a given string's length.\n"
      "SUBSTRING will return a substring of a given string.";
    }
  
  cmTypeMacro(cmStringCommand, cmCommand);
protected:
  bool HandleConfigureCommand(std::vector<std::string> const& args);
  bool HandleAsciiCommand(std::vector<std::string> const& args);
  bool HandleRegexCommand(std::vector<std::string> const& args);
  bool RegexMatch(std::vector<std::string> const& args);
  bool RegexMatchAll(std::vector<std::string> const& args);
  bool RegexReplace(std::vector<std::string> const& args);
  bool HandleToUpperLowerCommand(std::vector<std::string> const& args,
                                 bool toUpper);
  bool HandleCompareCommand(std::vector<std::string> const& args);
  bool HandleReplaceCommand(std::vector<std::string> const& args);
  bool HandleLengthCommand(std::vector<std::string> const& args);
  bool HandleSubstringCommand(std::vector<std::string> const& args);
  
  class RegexReplacement
  {
  public:
    RegexReplacement(const char* s): number(-1), value(s) {}
    RegexReplacement(const std::string& s): number(-1), value(s) {}
    RegexReplacement(int n): number(n), value() {}
    RegexReplacement() {};
    int number;
    std::string value;
  };
};


#endif
