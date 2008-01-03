/*=========================================================================

  Program:   CMake - Cross-Platform Makefile Generator
  Module:    $RCSfile: cmQTWrapUICommand.h,v $
  Language:  C++
  Date:      $Date: 2006/03/15 16:02:07 $
  Version:   $Revision: 1.10 $

  Copyright (c) 2002 Kitware, Inc., Insight Consortium.  All rights reserved.
  See Copyright.txt or http://www.cmake.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef cmQTWrapUICommand_h
#define cmQTWrapUICommand_h

#include "cmCommand.h"

#include "cmSourceFile.h"

/** \class cmQTWrapUICommand
 * \brief Create .h and .cxx files rules for QT user interfaces files
 *
 * cmQTWrapUICommand is used to create wrappers for QT classes into normal C++
 */
class cmQTWrapUICommand : public cmCommand
{
public:
  cmTypeMacro(cmQTWrapUICommand, cmCommand);
  /**
   * This is a virtual constructor for the command.
   */
  virtual cmCommand* Clone() 
    {
    return new cmQTWrapUICommand;
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
  virtual const char* GetName() { return "QT_WRAP_UI";}

  /**
   * Succinct documentation.
   */
  virtual const char* GetTerseDocumentation() 
    {
    return "Create QT user interfaces Wrappers.";
    }
  
  /**
   * More documentation.
   */
  virtual const char* GetFullDocumentation()
    {
    return
      "  QT_WRAP_UI(resultingLibraryName HeadersDestName\n"
      "             SourcesDestName SourceLists ...)\n"
      "Produce .h and .cxx files for all the .ui files listed "
      "in the SourceLists.  "
      "The .h files will be added to the library using the HeadersDestName"
      "source list.  "
      "The .cxx files will be added to the library using the SourcesDestName"
      "source list.";
    }
  
private:
  /**
   * List of produced files.
   */
  std::vector<cmSourceFile> WrapSourcesClasses;
  std::vector<cmSourceFile> WrapHeadersClasses;
  std::vector<cmSourceFile> WrapMocClasses;
  /**
   * List of header files that pprovide the source for WrapClasses.
   */
  std::vector<std::string> WrapUserInterface;
  std::string LibraryName;
  std::string HeaderList;
  std::string SourceList;
};



#endif
