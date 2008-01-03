/*=========================================================================

  Program:   CMake - Cross-Platform Makefile Generator
  Module:    $RCSfile: cmRemoveCommand.h,v $
  Language:  C++
  Date:      $Date: 2006/10/13 14:52:06 $
  Version:   $Revision: 1.6.6.1 $

  Copyright (c) 2002 Kitware, Inc., Insight Consortium.  All rights reserved.
  See Copyright.txt or http://www.cmake.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef cmRemoveCommand_h
#define cmRemoveCommand_h

#include "cmCommand.h"

/** \class cmRemoveCommand
 * \brief Set a CMAKE variable
 *
 * cmRemoveCommand sets a variable to a value with expansion.  
 */
class cmRemoveCommand : public cmCommand
{
public:
  /**
   * This is a virtual constructor for the command.
   */
  virtual cmCommand* Clone() 
    {
    return new cmRemoveCommand;
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
  virtual const char* GetName() {return "REMOVE";}
  
  /**
   * Succinct documentation.
   */
  virtual const char* GetTerseDocumentation() 
    {
    return "Old list item removal command.  Use the LIST command.";
    }
  
  /**
   * More documentation.
   */
  virtual const char* GetFullDocumentation()
    {
    return
      "This command has been superceded by the LIST(REMOVE ...) command.  "
      "It is provided for compatibility with older CMake code.\n"
      "  REMOVE(VAR VALUE VALUE ...)\n"
      "Removes VALUE from the variable VAR.  "
      "This is typically used to remove entries from a vector "
      "(e.g. semicolon separated list).  VALUE is expanded.";
    }
  
  /** This command is kept for compatibility with older CMake versions. */
  virtual bool IsDiscouraged()
    {
    return true;
    }
  
  cmTypeMacro(cmRemoveCommand, cmCommand);
};



#endif
