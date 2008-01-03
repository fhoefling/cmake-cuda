/*=========================================================================

  Program:   CMake - Cross-Platform Makefile Generator
  Module:    $RCSfile: cmGlobalNMakeMakefileGenerator.cxx,v $
  Language:  C++
  Date:      $Date: 2006/10/13 14:52:02 $
  Version:   $Revision: 1.19.2.4 $

  Copyright (c) 2002 Kitware, Inc., Insight Consortium.  All rights reserved.
  See Copyright.txt or http://www.cmake.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#include "cmGlobalNMakeMakefileGenerator.h"
#include "cmLocalUnixMakefileGenerator3.h"
#include "cmMakefile.h"

cmGlobalNMakeMakefileGenerator::cmGlobalNMakeMakefileGenerator()
{
  this->FindMakeProgramFile = "CMakeNMakeFindMake.cmake";
  this->ForceUnixPaths = false;
  this->ToolSupportsColor = true;
  this->UseLinkScript = false;
}

void cmGlobalNMakeMakefileGenerator
::EnableLanguage(std::vector<std::string>const& l, cmMakefile *mf)
{
  // pick a default 
  mf->AddDefinition("CMAKE_GENERATOR_CC", "cl");
  mf->AddDefinition("CMAKE_GENERATOR_CXX", "cl");
  this->cmGlobalUnixMakefileGenerator3::EnableLanguage(l, mf);
}

///! Create a local generator appropriate to this Global Generator
cmLocalGenerator *cmGlobalNMakeMakefileGenerator::CreateLocalGenerator()
{
  cmLocalUnixMakefileGenerator3* lg = new cmLocalUnixMakefileGenerator3;
  lg->SetDefineWindowsNULL(true);
  lg->SetWindowsShell(true);
  lg->SetMakeSilentFlag("/nologo");
  lg->SetGlobalGenerator(this);
  lg->SetIgnoreLibPrefix(true);
  lg->SetPassMakeflags(true);
  lg->SetUnixCD(false);
  return lg;
}

//----------------------------------------------------------------------------
void cmGlobalNMakeMakefileGenerator
::GetDocumentation(cmDocumentationEntry& entry) const
{
  entry.name = this->GetName();
  entry.brief = "Generates NMake makefiles.";
  entry.full = "";
}