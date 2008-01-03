/*=========================================================================

  Program:   CMake - Cross-Platform Makefile Generator
  Module:    $RCSfile: cmEndIfCommand.h,v $
  Language:  C++
  Date:      $Date: 2005/11/16 15:36:58 $
  Version:   $Revision: 1.11 $

  Copyright (c) 2002 Kitware, Inc., Insight Consortium.  All rights reserved.
  See Copyright.txt or http://www.cmake.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef cmEndIfCommand_h
#define cmEndIfCommand_h

#include "cmIfCommand.h"

/** \class cmEndIfCommand
 * \brief ends an if block
 *
 * cmEndIfCommand ends an if block
 */
class cmEndIfCommand : public cmCommand
{
public:
  /**
   * This is a virtual constructor for the command.
   */
  virtual cmCommand* Clone() 
    {
    return new cmEndIfCommand;
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
  virtual const char* GetName() { return "ENDIF";}

  /**
   * Succinct documentation.
   */
  virtual const char* GetTerseDocumentation() 
    {
    return "Ends a list of commands in an IF block.";
    }
  
  /**
   * More documentation.
   */
  virtual const char* GetFullDocumentation()
    {
    return
      "  ENDIF(expression)\n"
      "See the IF command.";
    }
  
  cmTypeMacro(cmEndIfCommand, cmCommand);
};


#endif
