/*=========================================================================

  Program:   CMake - Cross-Platform Makefile Generator
  Module:    $RCSfile: cmInstallFilesGenerator.h,v $
  Language:  C++
  Date:      $Date: 2006/10/13 14:52:02 $
  Version:   $Revision: 1.3.2.2 $

  Copyright (c) 2002 Kitware, Inc., Insight Consortium.  All rights reserved.
  See Copyright.txt or http://www.cmake.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef cmInstallFilesGenerator_h
#define cmInstallFilesGenerator_h

#include "cmInstallGenerator.h"

/** \class cmInstallFilesGenerator
 * \brief Generate file installation rules.
 */
class cmInstallFilesGenerator: public cmInstallGenerator
{
public:
  cmInstallFilesGenerator(std::vector<std::string> const& files,
                          const char* dest, bool programs,
                          const char* file_permissions,
                          std::vector<std::string> const& configurations,
                          const char* component,
                          const char* rename,
                          bool optional = false);
  virtual ~cmInstallFilesGenerator();

protected:
  virtual void GenerateScript(std::ostream& os);
  std::vector<std::string> Files;
  std::string Destination;
  bool Programs;
  std::string FilePermissions;
  std::vector<std::string> Configurations;
  std::string Component;
  std::string Rename;
  bool Optional;
};

#endif
