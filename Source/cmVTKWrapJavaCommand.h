/*=========================================================================

  Program:   CMake - Cross-Platform Makefile Generator
  Module:    $RCSfile: cmVTKWrapJavaCommand.h,v $
  Language:  C++
  Date:      $Date: 2006/10/13 14:52:06 $
  Version:   $Revision: 1.11.2.1 $

  Copyright (c) 2002 Kitware, Inc., Insight Consortium.  All rights reserved.
  See Copyright.txt or http://www.cmake.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef cmVTKWrapJavaCommand_h
#define cmVTKWrapJavaCommand_h

#include "cmCommand.h"

#include "cmSourceFile.h"

/** \class cmVTKWrapJavaCommand
 * \brief Create Java Language bindings for classes
 *
 * cmVTKWrapJavaCommand is used to create wrappers for classes into Java
 */
class cmVTKWrapJavaCommand : public cmCommand
{
public:
  cmTypeMacro(cmVTKWrapJavaCommand, cmCommand);

  /**
   * This is a virtual constructor for the command.
   */
  virtual cmCommand* Clone() 
    {
    return new cmVTKWrapJavaCommand;
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
  virtual const char* GetName() { return "VTK_WRAP_JAVA";}

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
      "  VTK_WRAP_JAVA(resultingLibraryName SourceListName\n"
      "                class1 class2 ...)\n"
      "Create Java wrappers for VTK classes.";
    }
  
  /** This command is kept for compatibility with older CMake versions. */
  virtual bool IsDiscouraged()
    {
    return true;
    }

private:
  std::vector<cmSourceFile> WrapClasses;
  std::vector<std::string> WrapHeaders;
  std::vector<std::string> OriginalNames;
  std::string LibraryName;
  std::string SourceList;
};



#endif
