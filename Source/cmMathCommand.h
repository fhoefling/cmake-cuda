/*=========================================================================

  Program:   CMake - Cross-Platform Makefile Generator
  Module:    $RCSfile: cmMathCommand.h,v $
  Language:  C++
  Date:      $Date: 2006/05/14 19:22:43 $
  Version:   $Revision: 1.1.4.1 $

  Copyright (c) 2002 Kitware, Inc., Insight Consortium.  All rights reserved.
  See Copyright.txt or http://www.cmake.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef cmMathCommand_h
#define cmMathCommand_h

#include "cmCommand.h"

/** \class cmMathCommand
 * \brief Common string operations
 *
 */
class cmMathCommand : public cmCommand
{
public:
  /**
   * This is a virtual constructor for the command.
   */
  virtual cmCommand* Clone() 
    {
    return new cmMathCommand;
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
  virtual const char* GetName() { return "MATH";}

  /**
   * Succinct documentation.
   */
  virtual const char* GetTerseDocumentation() 
    {
    return "Mathematical expressions.";
    }
  
  /**
   * More documentation.
   */
  virtual const char* GetFullDocumentation()
    {
    return
      "  MATH(EXPR <output variable> <math expression>)\n"
      "EXPR evaluates mathematical expression and return result in the "
      "output variable. Example mathematical expression is "
      "'5 * ( 10 + 13 )'.";
    }
  
  cmTypeMacro(cmMathCommand, cmCommand);
protected:
 
  bool HandleExprCommand(std::vector<std::string> const& args);
};


#endif

