/*=========================================================================

  Program:   CMake - Cross-Platform Makefile Generator
  Module:    $RCSfile: cmMakefileLibraryTargetGenerator.cxx,v $
  Language:  C++
  Date:      $Date: 2007/02/05 18:21:32 $
  Version:   $Revision: 1.14.2.7 $

  Copyright (c) 2002 Kitware, Inc., Insight Consortium.  All rights reserved.
  See Copyright.txt or http://www.cmake.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#include "cmMakefileLibraryTargetGenerator.h"

#include "cmGeneratedFileStream.h"
#include "cmGlobalUnixMakefileGenerator3.h"
#include "cmLocalUnixMakefileGenerator3.h"
#include "cmMakefile.h"
#include "cmSourceFile.h"
#include "cmTarget.h"
#include "cmake.h"

#include <memory> // auto_ptr

//----------------------------------------------------------------------------
cmMakefileLibraryTargetGenerator::cmMakefileLibraryTargetGenerator()
{
  this->DriveCustomCommandsOnDepends = true;
}

//----------------------------------------------------------------------------
void cmMakefileLibraryTargetGenerator::WriteRuleFiles()
{
  // create the build.make file and directory, put in the common blocks
  this->CreateRuleFile();
  
  // write rules used to help build object files
  this->WriteCommonCodeRules();
  
  // write in rules for object files and custom commands
  this->WriteTargetBuildRules();

  // write the per-target per-language flags
  this->WriteTargetLanguageFlags();

  // Write the dependency generation rule.
  this->WriteTargetDependRules();

  // write the link rules
  // Write the rule for this target type.
  switch(this->Target->GetType())
    {
    case cmTarget::STATIC_LIBRARY:
      this->WriteStaticLibraryRules();
      break;
    case cmTarget::SHARED_LIBRARY:
      this->WriteSharedLibraryRules(false);
      if(this->Target->NeedRelinkBeforeInstall())
        {
        // Write rules to link an installable version of the target.
        this->WriteSharedLibraryRules(true);
        }
      break;
    case cmTarget::MODULE_LIBRARY:
      this->WriteModuleLibraryRules(false);
      if(this->Target->NeedRelinkBeforeInstall())
        {
        // Write rules to link an installable version of the target.
        this->WriteModuleLibraryRules(true);
        }
      break;
    default:
      // If language is not known, this is an error.
      cmSystemTools::Error("Unknown Library Type");
      break;
    }

  // Write the requires target.
  this->WriteTargetRequiresRules();

  // Write clean target
  this->WriteTargetCleanRules();

  // close the streams
  this->CloseFileStreams();
}


//----------------------------------------------------------------------------
void cmMakefileLibraryTargetGenerator::WriteStaticLibraryRules()
{
  const char* linkLanguage =
    this->Target->GetLinkerLanguage(this->GlobalGenerator);
  std::string linkRuleVar = "CMAKE_";
  if (linkLanguage)
    {
    linkRuleVar += linkLanguage;
    }
  linkRuleVar += "_CREATE_STATIC_LIBRARY";

  std::string extraFlags;
  this->LocalGenerator->AppendFlags
    (extraFlags,this->Target->GetProperty("STATIC_LIBRARY_FLAGS"));
  this->WriteLibraryRules(linkRuleVar.c_str(), extraFlags.c_str(), false);
}

//----------------------------------------------------------------------------
void cmMakefileLibraryTargetGenerator::WriteSharedLibraryRules(bool relink)
{
  const char* linkLanguage =
    this->Target->GetLinkerLanguage(this->GlobalGenerator);
  std::string linkRuleVar = "CMAKE_";
  if (linkLanguage)
    {
    linkRuleVar += linkLanguage;
    }
  linkRuleVar += "_CREATE_SHARED_LIBRARY";

  std::string extraFlags;
  this->LocalGenerator->AppendFlags
    (extraFlags, this->Target->GetProperty("LINK_FLAGS"));
  std::string linkFlagsConfig = "LINK_FLAGS_";
  linkFlagsConfig += 
    cmSystemTools::UpperCase(this->LocalGenerator->ConfigurationName.c_str());
  this->LocalGenerator->AppendFlags
    (extraFlags, this->Target->GetProperty(linkFlagsConfig.c_str()));
                                    
  this->LocalGenerator->AddConfigVariableFlags
    (extraFlags, "CMAKE_SHARED_LINKER_FLAGS",
                                               this->LocalGenerator->ConfigurationName.c_str());
  if(this->Makefile->IsOn("WIN32") && !(this->Makefile->IsOn("CYGWIN") 
                                        || this->Makefile->IsOn("MINGW")))
    {
    const std::vector<cmSourceFile*>& sources = 
      this->Target->GetSourceFiles();
    for(std::vector<cmSourceFile*>::const_iterator i = sources.begin();
        i != sources.end(); ++i)
      {
      if((*i)->GetSourceExtension() == "def")
        {
        extraFlags += " ";
        extraFlags += 
          this->Makefile->GetSafeDefinition("CMAKE_LINK_DEF_FILE_FLAG");
        extraFlags += 
          this->Convert((*i)->GetFullPath().c_str(),
                        cmLocalGenerator::START_OUTPUT,
                        cmLocalGenerator::MAKEFILE);
        }
      }
    }
  this->WriteLibraryRules(linkRuleVar.c_str(), extraFlags.c_str(), relink);
}

//----------------------------------------------------------------------------
void cmMakefileLibraryTargetGenerator::WriteModuleLibraryRules(bool relink)
{
  const char* linkLanguage =
    this->Target->GetLinkerLanguage(this->GlobalGenerator);
  std::string linkRuleVar = "CMAKE_";
  if (linkLanguage)
    {
    linkRuleVar += linkLanguage;
    }
  linkRuleVar += "_CREATE_SHARED_MODULE";

  std::string extraFlags;
  this->LocalGenerator->AppendFlags(extraFlags, 
                                    this->Target->GetProperty("LINK_FLAGS"));
  std::string linkFlagsConfig = "LINK_FLAGS_";
  linkFlagsConfig += 
    cmSystemTools::UpperCase(this->LocalGenerator->ConfigurationName.c_str());
  this->LocalGenerator->AppendFlags
    (extraFlags, this->Target->GetProperty(linkFlagsConfig.c_str()));
  this->LocalGenerator->AddConfigVariableFlags
    (extraFlags, "CMAKE_MODULE_LINKER_FLAGS",
                                               this->LocalGenerator->ConfigurationName.c_str());

  // TODO: .def files should be supported here also.
  this->WriteLibraryRules(linkRuleVar.c_str(), extraFlags.c_str(), relink);
}

//----------------------------------------------------------------------------
void cmMakefileLibraryTargetGenerator::WriteLibraryRules
(const char* linkRuleVar, const char* extraFlags, bool relink)
{
  // TODO: Merge the methods that call this method to avoid
  // code duplication.
  std::vector<std::string> commands;

  std::string relPath = this->LocalGenerator->GetHomeRelativeOutputPath();
  std::string objTarget;

  // Build list of dependencies.
  std::vector<std::string> depends;
  for(std::vector<std::string>::const_iterator obj = this->Objects.begin();
      obj != this->Objects.end(); ++obj)
    {
    objTarget = relPath;
    objTarget += *obj;
    depends.push_back(objTarget);
    }

  // Add dependencies on targets that must be built first.
  this->AppendTargetDepends(depends);

  // Add a dependency on the rule file itself.
  this->LocalGenerator->AppendRuleDepend(depends, 
                                         this->BuildFileNameFull.c_str());
  
  for(std::vector<std::string>::const_iterator obj 
        = this->ExternalObjects.begin();
      obj != this->ExternalObjects.end(); ++obj)
    {
    depends.push_back(*obj);
    }
  
  // Get the language to use for linking this library.
  const char* linkLanguage =
    this->Target->GetLinkerLanguage(this->GlobalGenerator);

  // Make sure we have a link language.
  if(!linkLanguage)
    {
    cmSystemTools::Error("Cannot determine link language for target \"",
                         this->Target->GetName(), "\".");
    return;
    }

  // Create set of linking flags.
  std::string linkFlags;
  this->LocalGenerator->AppendFlags(linkFlags, extraFlags);

  // Construct the name of the library.
  std::string targetName;
  std::string targetNameSO;
  std::string targetNameReal;
  std::string targetNameImport;
  std::string targetNamePDB;
  this->Target->GetLibraryNames(
    targetName, targetNameSO, targetNameReal, targetNameImport, targetNamePDB,
    this->LocalGenerator->ConfigurationName.c_str());

  // Construct the full path version of the names.
  std::string outpath = this->LocalGenerator->LibraryOutputPath;
  if(outpath.length() == 0)
    {
    outpath = this->Makefile->GetStartOutputDirectory();
    outpath += "/";
    }
  if(relink)
    {
    outpath = this->Makefile->GetStartOutputDirectory();
    outpath += cmake::GetCMakeFilesDirectory();
    outpath += "/CMakeRelink.dir";
    cmSystemTools::MakeDirectory(outpath.c_str());
    outpath += "/";
    }
  std::string targetFullPath = outpath + targetName;
  std::string targetFullPathPDB = outpath + targetNamePDB;
  std::string targetFullPathSO = outpath + targetNameSO;
  std::string targetFullPathReal = outpath + targetNameReal;
  std::string targetFullPathImport = outpath + targetNameImport;

  // Construct the output path version of the names for use in command
  // arguments.
  std::string targetOutPathPDB = 
    this->Convert(targetFullPathPDB.c_str(),cmLocalGenerator::FULL,
                  cmLocalGenerator::MAKEFILE);
  std::string targetOutPath = 
    this->Convert(targetFullPath.c_str(),cmLocalGenerator::START_OUTPUT,
                  cmLocalGenerator::MAKEFILE);
  std::string targetOutPathSO = 
    this->Convert(targetFullPathSO.c_str(),cmLocalGenerator::START_OUTPUT,
                  cmLocalGenerator::MAKEFILE);
  std::string targetOutPathReal = 
    this->Convert(targetFullPathReal.c_str(),cmLocalGenerator::START_OUTPUT,
                  cmLocalGenerator::MAKEFILE);
  std::string targetOutPathImport =
    this->Convert(targetFullPathImport.c_str(),cmLocalGenerator::START_OUTPUT,
                  cmLocalGenerator::MAKEFILE);

  // Add the link message.
  std::string buildEcho = "Linking ";
  buildEcho += linkLanguage;
  const char* forbiddenFlagVar = 0;
  switch(this->Target->GetType())
    {
    case cmTarget::STATIC_LIBRARY:
      buildEcho += " static library "; 
      break;
    case cmTarget::SHARED_LIBRARY:
      forbiddenFlagVar = "_CREATE_SHARED_LIBRARY_FORBIDDEN_FLAGS";
      buildEcho += " shared library ";
      break;
    case cmTarget::MODULE_LIBRARY:
      forbiddenFlagVar = "_CREATE_SHARED_MODULE_FORBIDDEN_FLAGS";
      buildEcho += " shared module ";
      break;
    default:
      buildEcho += " library "; 
      break;
    }
  buildEcho += targetOutPath.c_str();
  this->LocalGenerator->AppendEcho(commands, buildEcho.c_str(),
                                   cmLocalUnixMakefileGenerator3::EchoLink);

  // Construct a list of files associated with this library that may
  // need to be cleaned.
  std::vector<std::string> libCleanFiles;
  if(this->Target->GetPropertyAsBool("CLEAN_DIRECT_OUTPUT"))
  {
    // The user has requested that only the files directly built
    // by this target be cleaned instead of all possible names.
    libCleanFiles.push_back(this->Convert(targetFullPath.c_str(),
          cmLocalGenerator::START_OUTPUT,
          cmLocalGenerator::UNCHANGED));
    if(targetNameReal != targetName)
      {
      libCleanFiles.push_back(this->Convert(targetFullPathReal.c_str(),
          cmLocalGenerator::START_OUTPUT,
          cmLocalGenerator::UNCHANGED));
      }
    if(targetNameSO != targetName &&
       targetNameSO != targetNameReal)
      {
      libCleanFiles.push_back(this->Convert(targetFullPathSO.c_str(),
          cmLocalGenerator::START_OUTPUT,
          cmLocalGenerator::UNCHANGED));
      }
    if(!targetNameImport.empty() &&
       targetNameImport != targetName &&
       targetNameImport != targetNameReal &&
       targetNameImport != targetNameSO)
      {
      libCleanFiles.push_back(this->Convert(targetFullPathImport.c_str(),
          cmLocalGenerator::START_OUTPUT,
          cmLocalGenerator::UNCHANGED));
      }
    }
  else
    {
    // This target may switch between static and shared based
    // on a user option or the BUILD_SHARED_LIBS switch.  Clean
    // all possible names.
  std::string cleanStaticName;
  std::string cleanSharedName;
  std::string cleanSharedSOName;
  std::string cleanSharedRealName;
  std::string cleanImportName;
    std::string cleanPDBName;
  this->Target->GetLibraryCleanNames(
    cleanStaticName,
    cleanSharedName,
    cleanSharedSOName,
    cleanSharedRealName,
    cleanImportName,
      cleanPDBName,
    this->LocalGenerator->ConfigurationName.c_str());
  std::string cleanFullStaticName = outpath + cleanStaticName;
  std::string cleanFullSharedName = outpath + cleanSharedName;
  std::string cleanFullSharedSOName = outpath + cleanSharedSOName;
  std::string cleanFullSharedRealName = outpath + cleanSharedRealName;
  std::string cleanFullImportName = outpath + cleanImportName;
    std::string cleanFullPDBName = outpath + cleanPDBName;
  libCleanFiles.push_back
      (this->Convert(cleanFullStaticName.c_str(),
                     cmLocalGenerator::START_OUTPUT,
                   cmLocalGenerator::UNCHANGED));
  if(cleanSharedRealName != cleanStaticName)
    {
    libCleanFiles.push_back(this->Convert(cleanFullSharedRealName.c_str(),
                                          cmLocalGenerator::START_OUTPUT,
                                          cmLocalGenerator::UNCHANGED));
    }
  if(cleanSharedSOName != cleanStaticName &&
     cleanSharedSOName != cleanSharedRealName)
    {
    libCleanFiles.push_back(this->Convert(cleanFullSharedSOName.c_str(),
                                          cmLocalGenerator::START_OUTPUT,
                                          cmLocalGenerator::UNCHANGED));
    }
  if(cleanSharedName != cleanStaticName &&
     cleanSharedName != cleanSharedSOName &&
     cleanSharedName != cleanSharedRealName)
    {
    libCleanFiles.push_back(this->Convert(cleanFullSharedName.c_str(),
                                          cmLocalGenerator::START_OUTPUT,
                                          cmLocalGenerator::UNCHANGED));
    }
  if(!cleanImportName.empty() &&
     cleanImportName != cleanStaticName &&
     cleanImportName != cleanSharedSOName &&
     cleanImportName != cleanSharedRealName &&
     cleanImportName != cleanSharedName)
    {
    libCleanFiles.push_back(this->Convert(cleanFullImportName.c_str(),
                                          cmLocalGenerator::START_OUTPUT,
                                          cmLocalGenerator::UNCHANGED));
    }

    // List the PDB for cleaning only when the whole target is
    // cleaned.  We do not want to delete the .pdb file just before
    // linking the target.
    this->CleanFiles.push_back
      (this->Convert(cleanFullPDBName.c_str(),
                     cmLocalGenerator::START_OUTPUT,
                     cmLocalGenerator::UNCHANGED));
  }

#ifdef _WIN32
  // There may be a manifest file for this target.  Add it to the
  // clean set just in case.
  if(this->Target->GetType() != cmTarget::STATIC_LIBRARY)
    {
    libCleanFiles.push_back(
      this->Convert((targetFullPath+".manifest").c_str(),
                    cmLocalGenerator::START_OUTPUT,
                    cmLocalGenerator::UNCHANGED));
    }
#endif

  // Add a command to remove any existing files for this library.
  std::vector<std::string> commands1;
  this->LocalGenerator->AppendCleanCommand(commands1, libCleanFiles,
                                           *this->Target, "target");
  this->LocalGenerator->CreateCDCommand
    (commands1,
                                        this->Makefile->GetStartOutputDirectory(),
                                        this->Makefile->GetHomeOutputDirectory());
  commands.insert(commands.end(), commands1.begin(), commands1.end());
  commands1.clear();

  // Add the pre-build and pre-link rules building but not when relinking.
  if(!relink)
    {
    this->LocalGenerator
      ->AppendCustomCommands(commands, this->Target->GetPreBuildCommands());
    this->LocalGenerator
      ->AppendCustomCommands(commands, this->Target->GetPreLinkCommands());
    }

  // Open the link script if it will be used.
  bool useLinkScript = false;
  std::string linkScriptName;
  std::auto_ptr<cmGeneratedFileStream> linkScriptStream;
  if(this->GlobalGenerator->GetUseLinkScript() &&
     (this->Target->GetType() == cmTarget::STATIC_LIBRARY ||
      this->Target->GetType() == cmTarget::SHARED_LIBRARY ||
      this->Target->GetType() == cmTarget::MODULE_LIBRARY))
    {
    useLinkScript = true;
    linkScriptName = this->TargetBuildDirectoryFull;
    if(relink)
      {
      linkScriptName += "/relink.txt";
      }
    else
      {
      linkScriptName += "/link.txt";
      }
    std::auto_ptr<cmGeneratedFileStream> lss(
      new cmGeneratedFileStream(linkScriptName.c_str()));
    linkScriptStream = lss;
    }

  std::vector<std::string> link_script_commands;

  // Construct the main link rule.
  std::string linkRule = this->Makefile->GetRequiredDefinition(linkRuleVar);
  if(useLinkScript)
    {
    cmSystemTools::ExpandListArgument(linkRule, link_script_commands);
    std::string link_command = "$(CMAKE_COMMAND) -E cmake_link_script ";
    link_command += this->Convert(linkScriptName.c_str(),
                                  cmLocalGenerator::START_OUTPUT,
                                  cmLocalGenerator::SHELL);
    link_command += " --verbose=$(VERBOSE)";
    commands1.push_back(link_command);
    }
  else
    {
  cmSystemTools::ExpandListArgument(linkRule, commands1);
    }
  this->LocalGenerator->CreateCDCommand
    (commands1,
                                        this->Makefile->GetStartOutputDirectory(),
                                        this->Makefile->GetHomeOutputDirectory());
  commands.insert(commands.end(), commands1.begin(), commands1.end());

  // Add a rule to create necessary symlinks for the library.
  if(targetOutPath != targetOutPathReal)
    {
    std::string symlink = "$(CMAKE_COMMAND) -E cmake_symlink_library ";
    symlink += targetOutPathReal;
    symlink += " ";
    symlink += targetOutPathSO;
    symlink += " ";
    symlink += targetOutPath;
    commands1.clear();
    commands1.push_back(symlink);
    this->LocalGenerator->CreateCDCommand(commands1,
                                  this->Makefile->GetStartOutputDirectory(),
                                  this->Makefile->GetHomeOutputDirectory());
    commands.insert(commands.end(), commands1.begin(), commands1.end());
    }

  // Add the post-build rules when building but not when relinking.
  if(!relink)
    {
    this->LocalGenerator->
      AppendCustomCommands(commands, this->Target->GetPostBuildCommands());
    }

  // Collect up flags to link in needed libraries.
  cmOStringStream linklibs;
  this->LocalGenerator->OutputLinkLibraries(linklibs, *this->Target, relink);

  // Construct object file lists that may be needed to expand the
  // rule.
  std::string variableName;
  std::string variableNameExternal;
  this->WriteObjectsVariable(variableName, variableNameExternal);
  std::string buildObjs;
  if(useLinkScript)
    {
    this->WriteObjectsString(buildObjs);
    }
  else
    {
    buildObjs = "$(";
  buildObjs += variableName;
  buildObjs += ") $(";
  buildObjs += variableNameExternal;
  buildObjs += ")";
    }
  std::string cleanObjs = "$(";
  cleanObjs += variableName;
  cleanObjs += ")";
  cmLocalGenerator::RuleVariables vars;
  vars.TargetPDB = targetOutPathPDB.c_str();

  // Setup the target version.
  std::string targetVersionMajor;
  std::string targetVersionMinor;
  {
  cmOStringStream majorStream;
  cmOStringStream minorStream;
  int major;
  int minor;
  this->Target->GetTargetVersion(major, minor);
  majorStream << major;
  minorStream << minor;
  targetVersionMajor = majorStream.str();
  targetVersionMinor = minorStream.str();
  }
  vars.TargetVersionMajor = targetVersionMajor.c_str();
  vars.TargetVersionMinor = targetVersionMinor.c_str();

  vars.Language = linkLanguage;
  vars.Objects = buildObjs.c_str();
  std::string objdir = cmake::GetCMakeFilesDirectoryPostSlash();
  objdir += this->Target->GetName();
  objdir += ".dir";
  vars.ObjectDir = objdir.c_str(); 
  std::string targetLinkScriptPathReal;
  if(useLinkScript)
    {
    // Paths in the link script are interpreted directly by the shell
    // and not make.
    targetLinkScriptPathReal =
      this->Convert(targetFullPathReal.c_str(),
                    cmLocalGenerator::START_OUTPUT,
                    cmLocalGenerator::SHELL);
    vars.Target = targetLinkScriptPathReal.c_str();
    }
  else
    {
  vars.Target = targetOutPathReal.c_str();
    }
  std::string linkString = linklibs.str();
  vars.LinkLibraries = linkString.c_str();
  vars.ObjectsQuoted = buildObjs.c_str();
  vars.TargetSOName= targetNameSO.c_str();
  vars.LinkFlags = linkFlags.c_str();

  // Compute the directory portion of the install_name setting.
  std::string install_name_dir;
  if(this->Target->GetType() == cmTarget::SHARED_LIBRARY)
    {
    // Get the install_name directory for the build tree.
    const char* config = this->LocalGenerator->ConfigurationName.c_str();
    install_name_dir = this->Target->GetInstallNameDirForBuildTree(config);

    // Set the rule variable replacement value.
    if(install_name_dir.empty())
      {
      vars.TargetInstallNameDir = "";
      }
    else
      {
      // Convert to a path for the native build tool.
      install_name_dir =
        this->LocalGenerator->Convert(install_name_dir.c_str(),
                                      cmLocalGenerator::NONE,
                                      cmLocalGenerator::SHELL, false);
      vars.TargetInstallNameDir = install_name_dir.c_str();
      }
    }
  std::string langFlags;
  this->LocalGenerator
    ->AddLanguageFlags(langFlags, linkLanguage,
                       this->LocalGenerator->ConfigurationName.c_str());
  // remove any language flags that might not work with the
  // particular os
  if(forbiddenFlagVar)
    {
    this->RemoveForbiddenFlags(forbiddenFlagVar,
                               linkLanguage, langFlags);
    }
  vars.LanguageCompileFlags = langFlags.c_str();
  // Expand placeholders in the commands.
  this->LocalGenerator->TargetImplib = targetOutPathImport;
  if(useLinkScript)
    {
    for(std::vector<std::string>::iterator i = link_script_commands.begin();
        i != link_script_commands.end(); ++i)
      {
      this->LocalGenerator->ExpandRuleVariables(*i, vars);
      }
    }
  else
    {
  for(std::vector<std::string>::iterator i = commands.begin();
      i != commands.end(); ++i)
    {
    this->LocalGenerator->ExpandRuleVariables(*i, vars);
    }
    }
  this->LocalGenerator->TargetImplib = "";

  // Optionally convert the build rule to use a script to avoid long
  // command lines in the make shell.
  if(useLinkScript)
    {
    for(std::vector<std::string>::iterator cmd = link_script_commands.begin();
        cmd != link_script_commands.end(); ++cmd)
      {
      // Do not write out empty commands or commands beginning in the
      // shell no-op ":".
      if(!cmd->empty() && (*cmd)[0] != ':')
        {
        (*linkScriptStream) << *cmd << "\n";
        }
      }
    }

  // Write the build rule.
  this->LocalGenerator->WriteMakeRule(*this->BuildFileStream, 0,
                                      targetFullPathReal.c_str(),
                                      depends, commands, false);

  // The symlink names for the target should depend on the real target
  // so if the target version changes it rebuilds and recreates the
  // symlinks.
  if(targetFullPathSO != targetFullPathReal)
    {
    depends.clear();
    commands.clear();
    depends.push_back(targetFullPathReal.c_str());
    this->LocalGenerator->WriteMakeRule(*this->BuildFileStream, 0,
                                        targetFullPathSO.c_str(),
                                        depends, commands, false);
    }
  if(targetFullPath != targetFullPathSO)
    {
    depends.clear();
    commands.clear();
    depends.push_back(targetFullPathSO.c_str());
    this->LocalGenerator->WriteMakeRule(*this->BuildFileStream, 0,
                                        targetFullPath.c_str(),
                                        depends, commands, false);
    }

  // Write the main driver rule to build everything in this target.
  this->WriteTargetDriverRule(targetFullPath.c_str(), relink);

  // Clean all the possible library names and symlinks and object files.
  this->CleanFiles.insert(this->CleanFiles.end(),
                          libCleanFiles.begin(),libCleanFiles.end()); 
  this->CleanFiles.insert(this->CleanFiles.end(),
                          this->Objects.begin(),
                          this->Objects.end());
}
