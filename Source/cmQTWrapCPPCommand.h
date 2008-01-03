/*=========================================================================

  Program:   CMake - Cross-Platform Makefile Generator
  Module:    $RCSfile: cmQTWrapCPPCommand.h,v $
  Language:  C++
  Date:      $Date: 2006/05/15 22:05:23 $
  Version:   $Revision: 1.10.2.1 $

  Copyright (c) 2002 Kitware, Inc., Insight Consortium.  All rights reserved.
  See Copyright.txt or http://www.cmake.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef cmQTWrapCPPCommand_h
#define cmQTWrapCPPCommand_h

#include "cmCommand.h"

#include "cmSourceFile.h"

/** \class cmQTWrapCPPCommand
 * \brief Create moc file rules for QT classes
 *
 * cmQTWrapCPPCommand is used to create wrappers for QT classes into
 * normal C++
 */
class cmQTWrapCPPCommand : public cmCommand
{
public:
  cmTypeMacro(cmQTWrapCPPCommand, cmCommand);

  /**
   * This is a virtual constructor for the command.
   */
  virtual cmCommand* Clone() 
    {
    return new cmQTWrapCPPCommand;
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
  virtual const char* GetName() { return "QT_WRAP_CPP";}

  /**
   * Succinct documentation.
   */
  virtual const char* GetTerseDocumentation() 
    {
    return "Create QT Wrappers.";
    }
  
  /**
   * More documentation.
   */
  virtual const char* GetFullDocumentation()
    {
    return
      "  QT_WRAP_CPP(resultingLibraryName DestName\n"
      "              SourceLists ...)\n"
      "Produce moc files for all the .h files listed in the SourceLists.  "
      "The moc files will be added to the library using the DestName "
      "source list.";
    }
  
private:
  /**
   * List of produced files.
   */
  std::vector<cmSourceFile> WrapClasses;
  /**
   * List of header files that pprovide the source for WrapClasses.
   */
  std::vector<std::string> WrapHeaders;
  std::string LibraryName;
  std::string SourceList;
};



#endif
