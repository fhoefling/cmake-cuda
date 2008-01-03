/*=========================================================================

  Program:   CMake - Cross-Platform Makefile Generator
  Module:    $RCSfile: cmMakefileLibraryTargetGenerator.h,v $
  Language:  C++
  Date:      $Date: 2006/10/13 14:52:06 $
  Version:   $Revision: 1.2.2.1 $

  Copyright (c) 2002 Kitware, Inc., Insight Consortium.  All rights reserved.
  See Copyright.txt or http://www.cmake.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef cmMakefileLibraryTargetGenerator_h
#define cmMakefileLibraryTargetGenerator_h

#include "cmMakefileTargetGenerator.h"

class cmMakefileLibraryTargetGenerator: 
  public cmMakefileTargetGenerator
{
public:
  cmMakefileLibraryTargetGenerator();

  /* the main entry point for this class. Writes the Makefiles associated
     with this target */
  virtual void WriteRuleFiles();  
  
protected:
  void WriteStaticLibraryRules();
  void WriteSharedLibraryRules(bool relink);
  void WriteModuleLibraryRules(bool relink);
  void WriteLibraryRules(const char *linkRule, const char *extraFlags,
                         bool relink);
};

#endif
