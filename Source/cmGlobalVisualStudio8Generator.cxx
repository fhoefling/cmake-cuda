/*=========================================================================

  Program:   CMake - Cross-Platform Makefile Generator
  Module:    $RCSfile: cmGlobalVisualStudio8Generator.cxx,v $
  Language:  C++
  Date:      $Date: 2007/03/16 22:05:42 $
  Version:   $Revision: 1.10.2.5 $

  Copyright (c) 2002 Kitware, Inc., Insight Consortium.  All rights reserved.
  See Copyright.txt or http://www.cmake.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#include "windows.h" // this must be first to define GetCurrentDirectory
#include "cmGlobalVisualStudio8Generator.h"
#include "cmLocalVisualStudio7Generator.h"
#include "cmMakefile.h"
#include "cmake.h"



cmGlobalVisualStudio8Generator::cmGlobalVisualStudio8Generator()
{
  this->FindMakeProgramFile = "CMakeVS8FindMake.cmake";
  this->ProjectConfigurationSectionName = "ProjectConfigurationPlatforms";
  this->PlatformName = "Win32";
}



///! Create a local generator appropriate to this Global Generator
cmLocalGenerator *cmGlobalVisualStudio8Generator::CreateLocalGenerator()
{
  cmLocalVisualStudio7Generator *lg = new cmLocalVisualStudio7Generator;
  lg->SetVersion8();
  lg->SetExtraFlagTable(this->GetExtraFlagTableVS8());
  lg->SetGlobalGenerator(this);
  return lg;
}

  
// ouput standard header for dsw file
void cmGlobalVisualStudio8Generator::WriteSLNHeader(std::ostream& fout)
{
  fout << "Microsoft Visual Studio Solution File, Format Version 9.00\n";
  fout << "# Visual Studio 2005\n";
}

//----------------------------------------------------------------------------
void cmGlobalVisualStudio8Generator
::GetDocumentation(cmDocumentationEntry& entry) const
{
  entry.name = this->GetName();
  entry.brief = "Generates Visual Studio .NET 2005 project files.";
  entry.full = "";
}

//----------------------------------------------------------------------------
void cmGlobalVisualStudio8Generator::AddPlatformDefinitions(cmMakefile* mf)
{
  mf->AddDefinition("MSVC80", "1");
}

//----------------------------------------------------------------------------
void cmGlobalVisualStudio8Generator::Configure()
{
  this->cmGlobalVisualStudio7Generator::Configure();
  this->CreateGUID(CMAKE_CHECK_BUILD_SYSTEM_TARGET);
}

//----------------------------------------------------------------------------
void cmGlobalVisualStudio8Generator::Generate()
{
  // Add a special target on which all other targets depend that
  // checks the build system and optionally re-runs CMake.
  const char* no_working_directory = 0;
  std::vector<std::string> no_depends;
  std::map<cmStdString, std::vector<cmLocalGenerator*> >::iterator it;
  for(it = this->ProjectMap.begin(); it!= this->ProjectMap.end(); ++it)
    {
    std::vector<cmLocalGenerator*>& generators = it->second;
    if(!generators.empty())
      {
      // Add the build-system check target to the first local
      // generator of this project.
      cmLocalVisualStudio7Generator* lg =
        static_cast<cmLocalVisualStudio7Generator*>(generators[0]);
      cmMakefile* mf = lg->GetMakefile();
      std::string cmake_command = mf->GetRequiredDefinition("CMAKE_COMMAND");
      cmCustomCommandLines noCommandLines;
      mf->AddUtilityCommand(CMAKE_CHECK_BUILD_SYSTEM_TARGET, true,
                            no_working_directory, no_depends,
                            noCommandLines);
      cmTarget* tgt = mf->FindTarget(CMAKE_CHECK_BUILD_SYSTEM_TARGET);
      if(!tgt)
        {
        cmSystemTools::Error("Error adding target " 
                             CMAKE_CHECK_BUILD_SYSTEM_TARGET);
        continue;
        }

      // Add a custom rule to re-run CMake if any input files changed.
      const char* suppRegenRule =
        mf->GetDefinition("CMAKE_SUPPRESS_REGENERATION");
      if(!cmSystemTools::IsOn(suppRegenRule))
        {
        // Collect the input files used to generate all targets in this
        // project.
        std::vector<std::string> listFiles;
        for(unsigned int j = 0; j < generators.size(); ++j)
          {
          cmMakefile* lmf = generators[j]->GetMakefile();
          listFiles.insert(listFiles.end(), lmf->GetListFiles().begin(),
                           lmf->GetListFiles().end());
          }
        // Sort the list of input files and remove duplicates.
        std::sort(listFiles.begin(), listFiles.end(), 
                  std::less<std::string>());
        std::vector<std::string>::iterator new_end =
          std::unique(listFiles.begin(), listFiles.end());
        listFiles.erase(new_end, listFiles.end());

        // Create a rule to re-run CMake.
        const char* dsprule = mf->GetRequiredDefinition("CMAKE_COMMAND");
        cmCustomCommandLine commandLine;
        commandLine.push_back(dsprule);
        std::string argH = "-H";
        argH += lg->Convert(mf->GetHomeDirectory(),
                            cmLocalGenerator::START_OUTPUT,
                            cmLocalGenerator::UNCHANGED, true);
        commandLine.push_back(argH);
        std::string argB = "-B";
        argB += lg->Convert(mf->GetHomeOutputDirectory(),
                            cmLocalGenerator::START_OUTPUT,
                            cmLocalGenerator::UNCHANGED, true);
        commandLine.push_back(argB);
        cmCustomCommandLines commandLines;
        commandLines.push_back(commandLine);

        // Add the rule.  Note that we cannot use the CMakeLists.txt
        // file as the main dependency because it would get
        // overwritten by the AddVCProjBuildRule of the ALL_BUILD
        // target.
        const char* no_main_dependency = 0;
        const char* no_working_directory = 0;
        mf->AddCustomCommandToOutput(
          CMAKE_CHECK_BUILD_SYSTEM_TARGET ".vcproj.cmake", listFiles,
          no_main_dependency, commandLines, "Checking Build System",
          no_working_directory, true);
        if(cmSourceFile* file = mf->GetSource(CMAKE_CHECK_BUILD_SYSTEM_TARGET 
                                              ".vcproj.cmake.rule"))
          {
          tgt->GetSourceFiles().push_back(file);
          }
        else
          {
          cmSystemTools::Error("Error adding rule for " 
                               CMAKE_CHECK_BUILD_SYSTEM_TARGET 
                               ".vcproj.cmake");
          }
        }
      }
    }

  // Now perform the main generation.
  this->cmGlobalVisualStudio7Generator::Generate();
}

//----------------------------------------------------------------------------
void cmGlobalVisualStudio8Generator::WriteSLNFile(
  std::ostream& fout, cmLocalGenerator* root,
  std::vector<cmLocalGenerator*>& generators)
{
  // Make all targets depend on their respective project's build
  // system check target.
  unsigned int i;
  for(i = 0; i < generators.size(); ++i)
    {
    if(this->IsExcluded(root, generators[i]))
      {
      continue;
      }
    cmMakefile* mf = generators[i]->GetMakefile();
    std::vector<std::string> dspnames =
      static_cast<cmLocalVisualStudio7Generator*>(generators[i])
      ->GetCreatedProjectNames();
    cmTargets& tgts = mf->GetTargets();
    for(cmTargets::iterator l = tgts.begin(); l != tgts.end(); ++l)
      {
      if(l->first == CMAKE_CHECK_BUILD_SYSTEM_TARGET)
        {
        for(unsigned int j = 0; j < generators.size(); ++j)
          {
          // Every target in all generators should depend on this target.
          cmMakefile* lmf = generators[j]->GetMakefile();
          cmTargets &atgts = lmf->GetTargets();
          for(cmTargets::iterator al = atgts.begin(); al != atgts.end(); ++al)
            {
            al->second.AddUtility(l->first.c_str());
            }
          }
        }
      }
    }

  // Now write the solution file.
  this->cmGlobalVisualStudio71Generator::WriteSLNFile(fout, root, generators);
}

//----------------------------------------------------------------------------
void
cmGlobalVisualStudio8Generator
::WriteSolutionConfigurations(std::ostream& fout)
{
  fout << "\tGlobalSection(SolutionConfigurationPlatforms) = preSolution\n";
  for(std::vector<std::string>::iterator i = this->Configurations.begin();
      i != this->Configurations.end(); ++i)
    {
    fout << "\t\t" << *i << "|" << this->PlatformName << " = " << *i << "|"
         << this->PlatformName << "\n";
    }
  fout << "\tEndGlobalSection\n";
}

//----------------------------------------------------------------------------
void
cmGlobalVisualStudio8Generator
::WriteProjectConfigurations(std::ostream& fout, const char* name,
                             bool partOfDefaultBuild)
{
  std::string guid = this->GetGUID(name);
  for(std::vector<std::string>::iterator i = this->Configurations.begin();
      i != this->Configurations.end(); ++i)
    {
    fout << "\t\t{" << guid << "}." << *i 
         << "|" << this->PlatformName << ".ActiveCfg = " 
           << *i << "|" << this->PlatformName << "\n";
    if(partOfDefaultBuild)
      {
      fout << "\t\t{" << guid << "}." << *i 
           << "|" << this->PlatformName << ".Build.0 = " 
             << *i << "|" << this->PlatformName << "\n";
      }
    }
}

//----------------------------------------------------------------------------
static cmVS7FlagTable cmVS8ExtraFlagTable[] =
{
  // Precompiled header and related options.  Note that the
  // UsePrecompiledHeader entries are marked as "Continue" so that the
  // corresponding PrecompiledHeaderThrough entry can be found.
  {"UsePrecompiledHeader", "Yu", "Use Precompiled Header", "2",
   cmVS7FlagTable::UserValueIgnored | cmVS7FlagTable::Continue},
  {"PrecompiledHeaderThrough", "Yu", "Precompiled Header Name", "",
   cmVS7FlagTable::UserValueRequired},
  // There is no YX option in the VS8 IDE.
  {0,0,0,0,0}
};
cmVS7FlagTable const* cmGlobalVisualStudio8Generator::GetExtraFlagTableVS8()
{
  return cmVS8ExtraFlagTable;
}
