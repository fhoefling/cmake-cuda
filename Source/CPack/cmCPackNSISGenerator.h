/*=========================================================================

  Program:   CMake - Cross-Platform Makefile Generator
  Module:    $RCSfile: cmCPackNSISGenerator.h,v $
  Language:  C++
  Date:      $Date: 2007/02/05 18:21:32 $
  Version:   $Revision: 1.8.2.1 $

  Copyright (c) 2002 Kitware, Inc. All rights reserved.
  See Copyright.txt or http://www.cmake.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef cmCPackNSISGenerator_h
#define cmCPackNSISGenerator_h


#include "cmCPackGenericGenerator.h"

/** \class cmCPackNSISGenerator
 * \brief A generator for NSIS files
 *
 * http://people.freebsd.org/~kientzle/libarchive/
 */
class cmCPackNSISGenerator : public cmCPackGenericGenerator
{
public:
  cmCPackTypeMacro(cmCPackNSISGenerator, cmCPackGenericGenerator);

  /**
   * Construct generator
   */
  cmCPackNSISGenerator();
  virtual ~cmCPackNSISGenerator();

protected:
  virtual int InitializeInternal();
  int CompressFiles(const char* outFileName, const char* toplevel,
    const std::vector<std::string>& files);
  virtual const char* GetOutputExtension() { return ".exe"; }
  virtual const char* GetOutputPostfix() { return "win32"; }

  bool GetListOfSubdirectories(const char* dir,
    std::vector<std::string>& dirs);
};

#endif
