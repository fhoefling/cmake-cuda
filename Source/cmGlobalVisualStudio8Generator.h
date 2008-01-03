/*=========================================================================

  Program:   CMake - Cross-Platform Makefile Generator
  Module:    $RCSfile: cmGlobalVisualStudio8Generator.h,v $
  Language:  C++
  Date:      $Date: 2007/03/16 22:05:42 $
  Version:   $Revision: 1.4.2.5 $

  Copyright (c) 2002 Kitware, Inc., Insight Consortium.  All rights reserved.
  See Copyright.txt or http://www.cmake.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef cmGlobalVisualStudio8Generator_h
#define cmGlobalVisualStudio8Generator_h

#include "cmGlobalVisualStudio71Generator.h"


/** \class cmGlobalVisualStudio8Generator
 * \brief Write a Unix makefiles.
 *
 * cmGlobalVisualStudio8Generator manages UNIX build process for a tree
 */
class cmGlobalVisualStudio8Generator : public cmGlobalVisualStudio71Generator
{
public:
  cmGlobalVisualStudio8Generator();
  static cmGlobalGenerator* New() { 
    return new cmGlobalVisualStudio8Generator; }
  
  ///! Get the name for the generator.
  virtual const char* GetName() const {
    return cmGlobalVisualStudio8Generator::GetActualName();}
  static const char* GetActualName() {return "Visual Studio 8 2005";}

  /** Get the documentation entry for this generator.  */
  virtual void GetDocumentation(cmDocumentationEntry& entry) const;
  
  ///! Create a local generator appropriate to this Global Generator
  virtual cmLocalGenerator *CreateLocalGenerator();

  /**
   * Override Configure and Generate to add the build-system check
   * target.
   */
  virtual void Configure();
  virtual void Generate();
protected:
  static cmVS7FlagTable const* GetExtraFlagTableVS8();
  virtual void AddPlatformDefinitions(cmMakefile* mf);
  virtual void WriteSLNFile(std::ostream& fout, cmLocalGenerator* root,
                            std::vector<cmLocalGenerator*>& generators);
  virtual void WriteSLNHeader(std::ostream& fout);
  virtual void WriteSolutionConfigurations(std::ostream& fout);
  virtual void WriteProjectConfigurations(std::ostream& fout,
                                          const char* name,
                                          bool partOfDefaultBuild);
  std::string PlatformName; // Win32 or x64 
};
#endif