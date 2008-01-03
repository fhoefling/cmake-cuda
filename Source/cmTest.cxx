/*=========================================================================

  Program:   CMake - Cross-Platform Makefile Generator
  Module:    $RCSfile: cmTest.cxx,v $
  Language:  C++
  Date:      $Date: 2006/04/28 12:59:48 $
  Version:   $Revision: 1.3.2.1 $

  Copyright (c) 2002 Kitware, Inc., Insight Consortium.  All rights reserved.
  See Copyright.txt or http://www.cmake.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#include "cmTest.h"
#include "cmSystemTools.h"


cmTest::cmTest() 
{
}

cmTest::~cmTest()
{
}

void cmTest::SetName(const char* name)
{
  if ( !name )
    {
    name = "";
    }
  this->Name = name;
}

void cmTest::SetCommand(const char* command)
{
  if ( !command )
    {
    command = "";
    }
  this->Command = command;
  cmSystemTools::ConvertToUnixSlashes(this->Command);
}

void cmTest::SetArguments(const std::vector<cmStdString>& args)
{
  this->Args = args;
}

const char *cmTest::GetProperty(const char* prop) const
{
  std::map<cmStdString,cmStdString>::const_iterator i = 
    this->Properties.find(prop);
  if (i != this->Properties.end())
    {
    return i->second.c_str();
    }
  return 0;
}

bool cmTest::GetPropertyAsBool(const char* prop) const
{
  std::map<cmStdString,cmStdString>::const_iterator i = 
    this->Properties.find(prop);
  if (i != this->Properties.end())
    {
    return cmSystemTools::IsOn(i->second.c_str());
    }
  return false;
}

void cmTest::SetProperty(const char* prop, const char* value)
{
  if (!prop)
    {
    return;
    }
  if (!value)
    {
    value = "NOTFOUND";
    }
  this->Properties[prop] = value;
}

