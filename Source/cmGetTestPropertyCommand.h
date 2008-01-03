/*=========================================================================

  Program:   CMake - Cross-Platform Makefile Generator
  Module:    $RCSfile: cmGetTestPropertyCommand.h,v $
  Language:  C++
  Date:      $Date: 2005/11/16 19:02:30 $
  Version:   $Revision: 1.2 $

  Copyright (c) 2002 Kitware, Inc., Insight Consortium.  All rights reserved.
  See Copyright.txt or http://www.cmake.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef cmGetTestPropertyCommand_h
#define cmGetTestPropertyCommand_h

#include "cmCommand.h"

class cmGetTestPropertyCommand : public cmCommand
{
public:
  virtual cmCommand* Clone() 
    {
    return new cmGetTestPropertyCommand;
    }

  /**
   * This is called when the command is first encountered in
   * the input file.
   */
  virtual bool InitialPass(std::vector<std::string> const& args);

  /**
   * The name of the command as specified in CMakeList.txt.
   */
  virtual const char* GetName() { return "GET_TEST_PROPERTY";}

  /**
   * Succinct documentation.
   */
  virtual const char* GetTerseDocumentation() 
    {
    return "Get a property of the test.";
    }

  /**
   * Longer documentation.
   */
  virtual const char* GetFullDocumentation()
    {
    return
      "  GET_TEST_PROPERTY(test VAR property)\n"
      "Get a property from the Test.  The value of the property is " 
      "stored in the variable VAR. If the property is not found, "
      "CMake will report an error.";
    }

  cmTypeMacro(cmGetTestPropertyCommand, cmCommand);
};



#endif
