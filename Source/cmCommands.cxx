/*=========================================================================

  Program:   CMake - Cross-Platform Makefile Generator
  Module:    $RCSfile: cmCommands.cxx,v $
  Language:  C++
  Date:      $Date: 2006/10/13 14:52:02 $
  Version:   $Revision: 1.103.2.2 $

  Copyright (c) 2002 Kitware, Inc., Insight Consortium.  All rights reserved.
  See Copyright.txt or http://www.cmake.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#include "cmCommands.h"
#if defined(CMAKE_BUILD_WITH_CMAKE)
#include "cmAddSubDirectoryCommand.cxx"
#include "cmAuxSourceDirectoryCommand.cxx"
#include "cmBuildNameCommand.cxx"
#include "cmCreateTestSourceList.cxx"
#include "cmElseIfCommand.cxx"
#include "cmEnableLanguageCommand.cxx"
#include "cmEndMacroCommand.cxx"
#include "cmEndWhileCommand.cxx"
#include "cmExecuteProcessCommand.cxx"
#include "cmExportLibraryDependencies.cxx"
#include "cmFLTKWrapUICommand.cxx"
#include "cmGetDirectoryPropertyCommand.cxx"
#include "cmGetTargetPropertyCommand.cxx"
#include "cmGetTestPropertyCommand.cxx"
#include "cmIncludeExternalMSProjectCommand.cxx"
#include "cmInstallCommand.cxx"
#include "cmInstallProgramsCommand.cxx"
#include "cmLinkLibrariesCommand.cxx"
#include "cmListCommand.cxx"
#include "cmLoadCacheCommand.cxx"
#include "cmMathCommand.cxx"
#include "cmOutputRequiredFilesCommand.cxx"
#include "cmQTWrapCPPCommand.cxx"
#include "cmQTWrapUICommand.cxx"
#include "cmRemoveCommand.cxx"
#include "cmRemoveDefinitionsCommand.cxx"
#include "cmSeparateArgumentsCommand.cxx"
#include "cmSetDirectoryPropertiesCommand.cxx"
#include "cmSetTargetPropertiesCommand.cxx"
#include "cmSetTestsPropertiesCommand.cxx"
#include "cmSourceGroupCommand.cxx"
#include "cmSubdirDependsCommand.cxx"
#include "cmUseMangledMesaCommand.cxx"
#include "cmUtilitySourceCommand.cxx"
#include "cmVTKMakeInstantiatorCommand.cxx"
#include "cmVTKWrapJavaCommand.cxx"
#include "cmVTKWrapPythonCommand.cxx"
#include "cmVTKWrapTclCommand.cxx"
#include "cmVariableRequiresCommand.cxx"
#include "cmWhileCommand.cxx"
#include "cmWriteFileCommand.cxx"

// This one must be last because it includes windows.h and
// windows.h #defines GetCurrentDirectory which is a member
// of cmMakefile
#include "cmLoadCommandCommand.cxx"
#endif

void GetPredefinedCommands(std::list<cmCommand*>&
#if defined(CMAKE_BUILD_WITH_CMAKE)
  commands
#endif
  )
{
#if defined(CMAKE_BUILD_WITH_CMAKE)
  commands.push_back(new cmAddSubDirectoryCommand);
  commands.push_back(new cmAuxSourceDirectoryCommand);
  commands.push_back(new cmBuildNameCommand);
  commands.push_back(new cmCreateTestSourceList);
  commands.push_back(new cmElseIfCommand);
  commands.push_back(new cmEnableLanguageCommand);
  commands.push_back(new cmEndMacroCommand);
  commands.push_back(new cmEndWhileCommand);
  commands.push_back(new cmExecuteProcessCommand);
  commands.push_back(new cmExportLibraryDependenciesCommand);
  commands.push_back(new cmFLTKWrapUICommand);
  commands.push_back(new cmGetDirectoryPropertyCommand);
  commands.push_back(new cmGetTargetPropertyCommand);
  commands.push_back(new cmGetTestPropertyCommand);
  commands.push_back(new cmIncludeExternalMSProjectCommand);
  commands.push_back(new cmInstallCommand);
  commands.push_back(new cmInstallProgramsCommand);
  commands.push_back(new cmLinkLibrariesCommand);
  commands.push_back(new cmListCommand);
  commands.push_back(new cmLoadCacheCommand);
  commands.push_back(new cmLoadCommandCommand);
  commands.push_back(new cmMathCommand);
  commands.push_back(new cmOutputRequiredFilesCommand);
  commands.push_back(new cmQTWrapCPPCommand);
  commands.push_back(new cmQTWrapUICommand);
  commands.push_back(new cmRemoveCommand);
  commands.push_back(new cmRemoveDefinitionsCommand);
  commands.push_back(new cmSeparateArgumentsCommand);
  commands.push_back(new cmSetDirectoryPropertiesCommand);
  commands.push_back(new cmSetTargetPropertiesCommand);
  commands.push_back(new cmSetTestsPropertiesCommand);
  commands.push_back(new cmSourceGroupCommand);
  commands.push_back(new cmSubdirDependsCommand);
  commands.push_back(new cmUseMangledMesaCommand);
  commands.push_back(new cmUtilitySourceCommand);
  commands.push_back(new cmVTKMakeInstantiatorCommand);
  commands.push_back(new cmVTKWrapJavaCommand);
  commands.push_back(new cmVTKWrapPythonCommand);
  commands.push_back(new cmVTKWrapTclCommand);
  commands.push_back(new cmVariableRequiresCommand);
  commands.push_back(new cmWhileCommand);
  commands.push_back(new cmWriteFileCommand);
#endif
}
