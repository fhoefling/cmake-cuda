/*=========================================================================

  Program:   CMake - Cross-Platform Makefile Generator
  Module:    $RCSfile: cmCPackPackageMakerGenerator.h,v $
  Language:  C++
  Date:      $Date: 2007/02/05 18:21:32 $
  Version:   $Revision: 1.7.2.1 $

  Copyright (c) 2002 Kitware, Inc. All rights reserved.
  See Copyright.txt or http://www.cmake.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef cmCPackPackageMakerGenerator_h
#define cmCPackPackageMakerGenerator_h


#include "cmCPackGenericGenerator.h"

/** \class cmCPackPackageMakerGenerator
 * \brief A generator for PackageMaker files
 *
 * http://developer.apple.com/documentation/Darwin
 * /Reference/ManPages/man1/packagemaker.1.html
 */
class cmCPackPackageMakerGenerator : public cmCPackGenericGenerator
{
public:
  cmCPackTypeMacro(cmCPackPackageMakerGenerator, cmCPackGenericGenerator);

  /**
   * Construct generator
   */
  cmCPackPackageMakerGenerator();
  virtual ~cmCPackPackageMakerGenerator();

protected:
  virtual int InitializeInternal();
  int CompressFiles(const char* outFileName, const char* toplevel,
    const std::vector<std::string>& files);
  virtual const char* GetOutputExtension() { return ".dmg"; }
  virtual const char* GetOutputPostfix() { return "darwin"; }
  virtual const char* GetInstallPrefix() { return "/usr"; }

  bool CopyCreateResourceFile(const char* name);
  bool CopyResourcePlistFile(const char* name);

  float PackageMakerVersion;
};

#endif
