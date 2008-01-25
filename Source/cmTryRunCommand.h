/*=========================================================================

  Program:   CMake - Cross-Platform Makefile Generator
  Module:    $RCSfile: cmTryRunCommand.h,v $
  Language:  C++
  Date:      $Date: 2007/10/25 18:03:48 $
  Version:   $Revision: 1.9.2.1 $

  Copyright (c) 2002 Kitware, Inc., Insight Consortium.  All rights reserved.
  See Copyright.txt or http://www.cmake.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef cmTryRunCommand_h
#define cmTryRunCommand_h

#include "cmCommand.h"

/** \class cmTryRunCommand
 * \brief Specifies where to install some files
 *
 * cmTryRunCommand is used to test if soucre code can be compiled
 */
class cmTryRunCommand : public cmCommand
{
public:
  /**
   * This is a virtual constructor for the command.
   */
  virtual cmCommand* Clone() 
    {
    return new cmTryRunCommand;
    }

  /**
   * This is called when the command is first encountered in
   * the CMakeLists.txt file.
   */
  virtual bool InitialPass(std::vector<std::string> const& args);

  /**
   * The name of the command as specified in CMakeList.txt.
   */
  virtual const char* GetName() { return "TRY_RUN";}

  /**
   * Succinct documentation.
   */
  virtual const char* GetTerseDocumentation() 
    {
    return "Try compiling and then running some code.";
    }
  
  /**
   * More documentation.
   */
  virtual const char* GetFullDocumentation()
    {
    return
      "  TRY_RUN(RUN_RESULT_VAR COMPILE_RESULT_VAR\n"
      "          bindir srcfile [CMAKE_FLAGS <Flags>]\n"
      "          [COMPILE_DEFINITIONS <flags>]\n"
      "          [OUTPUT_VARIABLE var]\n"
      "          [ARGS <arg1> <arg2>...])\n"
      "Try compiling a srcfile.  Return TRUE or FALSE for success or failure "
      "in COMPILE_RESULT_VAR.  Then if the compile succeeded, run the "
      "executable and return its exit code in RUN_RESULT_VAR. "
      "If the executable was built, but failed to run, then RUN_RESULT_VAR "
      "will be set to FAILED_TO_RUN. OUTPUT_VARIABLE "
      "specifies the name of the variable to put all of the standard "
      "output and standard error into.";
    }
  
  cmTypeMacro(cmTryRunCommand, cmCommand);

};


#endif
