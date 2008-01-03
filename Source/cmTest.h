/*=========================================================================

  Program:   CMake - Cross-Platform Makefile Generator
  Module:    $RCSfile: cmTest.h,v $
  Language:  C++
  Date:      $Date: 2006/03/15 16:02:07 $
  Version:   $Revision: 1.4 $

  Copyright (c) 2002 Kitware, Inc., Insight Consortium.  All rights reserved.
  See Copyright.txt or http://www.cmake.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef cmTest_h
#define cmTest_h

#include "cmCustomCommand.h"

/** \class cmTest
 * \brief Represent a test
 *
 * cmTest is representation of a test.
 */
class cmTest
{
public:
  /**
   */
  cmTest();
  ~cmTest();

  ///! Set the test name
  void SetName(const char* name);
  const char* GetName() const { return this->Name.c_str(); }
  void SetCommand(const char* command);
  const char* GetCommand() const { return this->Command.c_str(); }
  void SetArguments(const std::vector<cmStdString>& args);
  const std::vector<cmStdString>& GetArguments() const
    {
    return this->Args;
    }

  /**
   * Print the structure to std::cout.
   */
  void Print() const;

  ///! Set/Get a property of this source file
  void SetProperty(const char *prop, const char *value);
  const char *GetProperty(const char *prop) const;
  bool GetPropertyAsBool(const char *prop) const;
  const std::map<cmStdString,cmStdString>& GetProperties() const
    {
    return this->Properties;
    }
    
private:
  std::map<cmStdString,cmStdString> Properties;
  cmStdString Name;
  cmStdString Command;
  std::vector<cmStdString> Args;
};

#endif

