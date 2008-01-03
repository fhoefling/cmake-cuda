/*=========================================================================

  Program:   CMake - Cross-Platform Makefile Generator
  Module:    $RCSfile: cmVariableRequiresCommand.h,v $
  Language:  C++
  Date:      $Date: 2006/03/16 14:33:23 $
  Version:   $Revision: 1.11 $

  Copyright (c) 2002 Kitware, Inc., Insight Consortium.  All rights reserved.
  See Copyright.txt or http://www.cmake.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef cmVariableRequiresCommand_h
#define cmVariableRequiresCommand_h

#include "cmCommand.h"

/** \class cmVariableRequiresCommand
 * \brief Displays a message to the user
 *
 */
class cmVariableRequiresCommand : public cmCommand
{
public:
  /**
   * This is a virtual constructor for the command.
   */
  virtual cmCommand* Clone() 
    {
    return new cmVariableRequiresCommand;
    }

  /**
   * This is called when the command is first encountered in
   * the CMakeLists.txt file.
   */
  virtual bool InitialPass(std::vector<std::string> const& args);
  
  /**
   * The name of the command as specified in CMakeList.txt.
   */
  virtual const char* GetName() { return "VARIABLE_REQUIRES";}

  /**
   * Succinct documentation.
   */
  virtual const char* GetTerseDocumentation() 
    {
    return "Assert satisfaction of an option's required variables.";
    }
  
  /**
   * More documentation.
   */
  virtual const char* GetFullDocumentation()
    {
    return
      "  VARIABLE_REQUIRES(TEST_VARIABLE RESULT_VARIABLE\n"
      "                    REQUIRED_VARIABLE1\n"
      "                    REQUIRED_VARIABLE2 ...)\n"
      "The first argument (TEST_VARIABLE) is the name of the variable to be "
      "tested, if that variable is false nothing else is done. If "
      "TEST_VARIABLE is true, then "
      "the next argument (RESULT_VARIABLE) is a variable that is set to true "
      "if all the required variables are set. " 
      "The rest of the arguments are variables that must be true or not "
      "set to NOTFOUND to avoid an error.  If any are not true, an error "
      "is reported.";
    }
  
  cmTypeMacro(cmVariableRequiresCommand, cmCommand);
};


#endif