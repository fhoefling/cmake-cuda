/*=========================================================================

  Program:   CMake - Cross-Platform Makefile Generator
  Module:    $RCSfile: cmVTKWrapPythonCommand.h,v $
  Language:  C++
  Date:      $Date: 2006/10/13 14:52:06 $
  Version:   $Revision: 1.11.2.1 $

  Copyright (c) 2002 Kitware, Inc., Insight Consortium.  All rights reserved.
  See Copyright.txt or http://www.cmake.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef cmVTKWrapPythonCommand_h
#define cmVTKWrapPythonCommand_h

#include "cmCommand.h"

#include "cmSourceFile.h"

/** \class cmVTKWrapPythonCommand
 * \brief Create Python Language bindings for classes
 *
 * cmVTKWrapPythonCommand is used to create wrappers for classes into Python
 */
class cmVTKWrapPythonCommand : public cmCommand
{
public:
  cmTypeMacro(cmVTKWrapPythonCommand, cmCommand);

  /**
   * This is a virtual constructor for the command.
   */
  virtual cmCommand* Clone() 
    {
    return new cmVTKWrapPythonCommand;
    }

  /**
   * This is called when the command is first encountered in
   * the CMakeLists.txt file.
   */
  virtual bool InitialPass(std::vector<std::string> const& args);
  
  /**
   * This is called at the end after all the information
   * specified by the command is accumulated. Most commands do
   * not implement this method.  At this point, reading and
   * writing to the cache can be done.
   */
  virtual void FinalPass();

  /**
   * The name of the command as specified in CMakeList.txt.
   */
  virtual const char* GetName() { return "VTK_WRAP_PYTHON";}

  /**
   * Succinct documentation.
   */
  virtual const char* GetTerseDocumentation() 
    {
    return "Deprecated.  For use only in VTK 4.0.";
    }
  
  /**
   * More documentation.
   */
  virtual const char* GetFullDocumentation()
    {
    return
      "  VTK_WRAP_PYTHON(resultingLibraryName SourceListName\n"
      "                  class1 class2 ...)\n"
      "Create Python wrappers for VTK classes.";
    }

  /** This command is kept for compatibility with older CMake versions. */
  virtual bool IsDiscouraged()
    {
    return true;
    }

  /**
   * Helper methods
   */
  virtual bool CreateInitFile(std::string &name);
  virtual bool WriteInit(const char *kitName, std::string& outFileName,
                         std::vector<std::string>& classes);
  
private:
  std::vector<cmSourceFile> WrapClasses;
  std::vector<std::string> WrapHeaders;
  std::string LibraryName;
  std::string SourceList;
};



#endif
