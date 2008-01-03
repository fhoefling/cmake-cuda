/*=========================================================================

  Program:   CMake - Cross-Platform Makefile Generator
  Module:    $RCSfile: cmIfCommand.h,v $
  Language:  C++
  Date:      $Date: 2007/05/17 18:41:52 $
  Version:   $Revision: 1.32.2.6 $

  Copyright (c) 2002 Kitware, Inc., Insight Consortium.  All rights reserved.
  See Copyright.txt or http://www.cmake.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef cmIfCommand_h
#define cmIfCommand_h

#include "cmCommand.h"
#include "cmFunctionBlocker.h"

/** \class cmIfFunctionBlocker
 * \brief subclass of function blocker
 *
 * 
 */
class cmIfFunctionBlocker : public cmFunctionBlocker
{
public:
  cmIfFunctionBlocker() {this->HasRun = false; this->ScopeDepth = 0;}
  virtual ~cmIfFunctionBlocker() {}
  virtual bool IsFunctionBlocked(const cmListFileFunction& lff,
                                 cmMakefile &mf);
  virtual bool ShouldRemove(const cmListFileFunction& lff,
                            cmMakefile &mf);
  virtual void ScopeEnded(cmMakefile &mf);
  
  std::vector<cmListFileArgument> Args;
  bool IsBlocking;
  bool HasRun;
  unsigned int ScopeDepth;
};

/** \class cmIfCommand
 * \brief starts an if block
 *
 * cmIfCommand starts an if block
 */
class cmIfCommand : public cmCommand
{
public:
  /**
   * This is a virtual constructor for the command.
   */
  virtual cmCommand* Clone() 
    {
    return new cmIfCommand;
    }

  /**
   * This overrides the default InvokeInitialPass implementation.
   * It records the arguments before expansion.
   */
  virtual bool InvokeInitialPass(const std::vector<cmListFileArgument>& args);
    
  /**
   * This is called when the command is first encountered in
   * the CMakeLists.txt file.
   */
  virtual bool InitialPass(std::vector<std::string> const&) { return false; }

  /**
   * The name of the command as specified in CMakeList.txt.
   */
  virtual const char* GetName() { return "IF";}

  /**
   * Succinct documentation.
   */
  virtual const char* GetTerseDocumentation() 
    {
    return "Conditionally execute a group of commands.";
    }
  
  /**
   * This determines if the command is invoked when in script mode.
   */
  virtual bool IsScriptable() { return true; }

  /**
   * More documentation.
   */
  virtual const char* GetFullDocumentation()
    {
    return
      "  IF(expression)\n"
      "    # THEN section.\n"
      "    COMMAND1(ARGS ...)\n"
      "    COMMAND2(ARGS ...)\n"
      "    ...\n"
      "  ELSEIF(expression2)\n"
      "    # ELSEIF section.\n"
      "    COMMAND1(ARGS ...)\n"
      "    COMMAND2(ARGS ...)\n"
      "    ...\n"
      "  ELSE(expression)\n"
      "    # ELSE section.\n"
      "    COMMAND1(ARGS ...)\n"
      "    COMMAND2(ARGS ...)\n"
      "    ...\n"
      "  ENDIF(expression)\n"
      "Evaluates the given expression.  If the result is true, the commands "
      "in the THEN section are invoked.  Otherwise, the commands in the "
      "ELSE section are invoked.  The ELSEIF and ELSE sections are "
      "optional. You may have multiple ELSEIF clauses. Note that "
      "the same expression must be given to IF, and ENDIF.  Long "
      "expressions can be used and the order or precedence is that the "
      "EXISTS, COMMAND, and DEFINED operators will be evaluated first. "
      "Then any EQUAL, LESS, GREATER, STRLESS, STRGREATER, STREQUAL, MATCHES "
      "will be evaluated. Then NOT operators and finally AND, OR operators "
      "will be evaluated. Possible expressions are:\n"
      "  IF(variable)\n"
      "True if the variable's value is not empty, 0, N, NO, OFF, FALSE, "
      "NOTFOUND, or <variable>-NOTFOUND.\n"
      "  IF(NOT variable)\n"
      "True if the variable's value is empty, 0, N, NO, OFF, FALSE, "
      "NOTFOUND, or <variable>-NOTFOUND.\n"
      "  IF(variable1 AND variable2)\n"
      "True if both variables would be considered true individually.\n"
      "  IF(variable1 OR variable2)\n"
      "True if either variable would be considered true individually.\n"
      "  IF(COMMAND command-name)\n"
      "True if the given name is a command that can be invoked.\n"
      "  IF(EXISTS file-name)\n"
      "  IF(EXISTS directory-name)\n"
      "True if the named file or directory exists.  "
      "Behavior is well-defined only for full paths.\n"
      "  IF(file1 IS_NEWER_THAN file2)\n"
      "True if file1 is newer than file2 or if one of the two files "
      "doesn't exist. "
      "Behavior is well-defined only for full paths.\n"
      "  IF(IS_DIRECTORY directory-name)\n"
      "True if the given name is a directory.  "
      "Behavior is well-defined only for full paths.\n"
      "  IF(variable MATCHES regex)\n"
      "  IF(string MATCHES regex)\n"
      "True if the given string or variable's value matches the given "
      "regular expression.\n"
      "  IF(variable LESS number)\n"
      "  IF(string LESS number)\n"
      "  IF(variable GREATER number)\n"
      "  IF(string GREATER number)\n"
      "  IF(variable EQUAL number)\n"
      "  IF(string EQUAL number)\n"
      "True if the given string or variable's value is a valid number and "
      "the inequality or equality is true.\n"
      "  IF(variable STRLESS string)\n"
      "  IF(string STRLESS string)\n"
      "  IF(variable STRGREATER string)\n"
      "  IF(string STRGREATER string)\n"
      "  IF(variable STREQUAL string)\n"
      "  IF(string STREQUAL string)\n"
      "True if the given string or variable's value is lexicographically "
      "less (or greater, or equal) than the string on the right.\n"
      "  IF(DEFINED variable)\n"
      "True if the given variable is defined. It does not matter if the "
      "variable is true or false just if it has been set.";
    }

  // this is a shared function for both If and Else to determine if the
  // arguments were valid, and if so, was the response true. If there is
  // an error, the errorString will be set.
  static bool IsTrue(const std::vector<std::string> &args, 
    char** errorString, const cmMakefile *mf);
  
  // Get a definition from the makefile.  If it doesn't exist,
  // return the original string.
  static const char* GetVariableOrString(const char* str,
                                         const cmMakefile* mf);
  static const char* GetVariableOrNumber(const char* str,
                                         const cmMakefile* mf);
  
  
  cmTypeMacro(cmIfCommand, cmCommand);
};


#endif