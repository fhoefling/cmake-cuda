/*=========================================================================

  Program:   CMake - Cross-Platform Makefile Generator
  Module:    $RCSfile: cmSetSourceFilesPropertiesCommand.h,v $
  Language:  C++
  Date:      $Date: 2006/10/13 14:52:06 $
  Version:   $Revision: 1.9.2.2 $

  Copyright (c) 2002 Kitware, Inc., Insight Consortium.  All rights reserved.
  See Copyright.txt or http://www.cmake.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef cmSetSourceFilesPropertiesCommand_h
#define cmSetSourceFilesPropertiesCommand_h

#include "cmCommand.h"

class cmSetSourceFilesPropertiesCommand : public cmCommand
{
public:
  virtual cmCommand* Clone() 
    {
      return new cmSetSourceFilesPropertiesCommand;
    }

  /**
   * This is called when the command is first encountered in
   * the input file.
   */
  virtual bool InitialPass(std::vector<std::string> const& args);

  /**
   * The name of the command as specified in CMakeList.txt.
   */
  virtual const char* GetName() { return "SET_SOURCE_FILES_PROPERTIES";}

  /**
   * Succinct documentation.
   */
  virtual const char* GetTerseDocumentation() 
    {
    return "Source files can have properties that affect how they are built.";
    }
  
  /**
   * Longer documentation.
   */
  virtual const char* GetFullDocumentation()
    {
      return
        "  SET_SOURCE_FILES_PROPERTIES(file1 file2 ...\n"
        "                              PROPERTIES prop1 value1\n"
        "                              prop2 value2 ...)\n"
        "Set properties on a file. The syntax for the command is to list all "
        "the files you want "
        "to change, and then provide the values you want to set next.  You "
        "can make up your own properties as well.  "
        "The following are used by CMake.  "
        "The ABSTRACT flag (boolean) is used by some class wrapping "
        "commands. "
        "If WRAP_EXCLUDE (boolean) is true then many wrapping commands "
        "will ignore this file. If GENERATED (boolean) is true then it "
        "is not an error if this source file does not exist when it is "
        "added to a target.  Obviously, "
        "it must be created (presumably by a custom command) before the "
        "target is built.  "
        "If the HEADER_FILE_ONLY (boolean) property is true then dependency "
        "information is not created for that file (this is set "
        "automatically, based on the file's name's extension and is probably "
        "only used by Makefiles).  "
        "OBJECT_DEPENDS (string) adds dependencies to the object file.  "
        "COMPILE_FLAGS (string) is passed to the compiler as additional "
        "command line arguments when the source file is compiled.  "
        "If SYMBOLIC (boolean) is set to true the build system will be "
        "informed that the source file is not actually created on disk but "
        "instead used as a symbolic name for a build rule.";
      
    }
  
  cmTypeMacro(cmSetSourceFilesPropertiesCommand, cmCommand);
};



#endif
