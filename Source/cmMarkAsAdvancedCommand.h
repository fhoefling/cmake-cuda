/*=========================================================================

  Program:   CMake - Cross-Platform Makefile Generator
  Module:    $RCSfile: cmMarkAsAdvancedCommand.h,v $
  Language:  C++
  Date:      $Date: 2005/11/17 14:31:43 $
  Version:   $Revision: 1.9 $

  Copyright (c) 2002 Kitware, Inc., Insight Consortium.  All rights reserved.
  See Copyright.txt or http://www.cmake.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef cmMarkAsAdvancedCommand_h
#define cmMarkAsAdvancedCommand_h

#include "cmCommand.h"

/** \class cmMarkAsAdvancedCommand
 * \brief MarkAsAdvanced a CMAKE variable
 *
 * cmMarkAsAdvancedCommand sets a variable to a value with expansion.  
 */
class cmMarkAsAdvancedCommand : public cmCommand
{
public:
  /**
   * This is a virtual constructor for the command.
   */
  virtual cmCommand* Clone() 
    {
    return new cmMarkAsAdvancedCommand;
    }

  /**
   * This is called when the command is first encountered in
   * the CMakeLists.txt file.
   */
  virtual bool InitialPass(std::vector<std::string> const& args);

  /**
   * The name of the command as specified in CMakeList.txt.
   */
  virtual const char* GetName() {return "MARK_AS_ADVANCED";}
  
  /**
   * Succinct documentation.
   */
  virtual const char* GetTerseDocumentation() 
    {
    return "Mark cmake cached variables as advanced.";
    }
  
  /**
   * More documentation.
   */
  virtual const char* GetFullDocumentation()
    {
    return
      "  MARK_AS_ADVANCED([CLEAR|FORCE] VAR VAR2 VAR...)\n"
      "Mark the named cached variables as advanced.  An advanced variable "
      "will not be displayed in any of the cmake GUIs unless the show "
      "advanced option is on.  "
      "If CLEAR is the first argument advanced variables are changed back "
      "to unadvanced.  "
      "If FORCE is the first argument, then the variable is made advanced.  "
      "If neither FORCE nor CLEAR is specified, new values will be marked as "
      "advanced, but if the variable already has an advanced/non-advanced "
      "state, it will not be changed.";
    }
  
  cmTypeMacro(cmMarkAsAdvancedCommand, cmCommand);
};



#endif
