/*=========================================================================

  Program:   CMake - Cross-Platform Makefile Generator
  Module:    $RCSfile: cmMakefileTargetGenerator.cxx,v $
  Language:  C++
  Date:      $Date: 2007/02/05 18:21:32 $
  Version:   $Revision: 1.16.2.10 $

  Copyright (c) 2002 Kitware, Inc., Insight Consortium.  All rights reserved.
  See Copyright.txt or http://www.cmake.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#include "cmMakefileTargetGenerator.h"

#include "cmGeneratedFileStream.h"
#include "cmGlobalGenerator.h"
#include "cmGlobalUnixMakefileGenerator3.h"
#include "cmLocalUnixMakefileGenerator3.h"
#include "cmMakefile.h"
#include "cmSourceFile.h"
#include "cmTarget.h"
#include "cmake.h"

#include "cmMakefileExecutableTargetGenerator.h"
#include "cmMakefileLibraryTargetGenerator.h"
#include "cmMakefileUtilityTargetGenerator.h"


cmMakefileTargetGenerator::cmMakefileTargetGenerator()
{
  this->BuildFileStream = 0;
  this->InfoFileStream = 0;
  this->FlagFileStream = 0;
  this->DriveCustomCommandsOnDepends = false;
}

cmMakefileTargetGenerator *
cmMakefileTargetGenerator::New(cmLocalUnixMakefileGenerator3 *lg,
                               cmStdString tgtName, cmTarget *tgt)
{
  cmMakefileTargetGenerator *result = 0;

  switch (tgt->GetType())
    {
    case cmTarget::EXECUTABLE:
      result = new cmMakefileExecutableTargetGenerator;
      break;
    case cmTarget::STATIC_LIBRARY:
    case cmTarget::SHARED_LIBRARY:
    case cmTarget::MODULE_LIBRARY:
      result = new cmMakefileLibraryTargetGenerator;
      break;
    case cmTarget::UTILITY:
      result = new cmMakefileUtilityTargetGenerator;
      break;
    default:
      return result;
      // break; /* unreachable */
    }

  result->TargetName = tgtName;
  result->Target = tgt;
  result->LocalGenerator = lg;
  result->GlobalGenerator =
    static_cast<cmGlobalUnixMakefileGenerator3*>(lg->GetGlobalGenerator());
  result->Makefile = lg->GetMakefile();
  return result;
}

//----------------------------------------------------------------------------
void cmMakefileTargetGenerator::CreateRuleFile()
{
  // Create a directory for this target.
  this->TargetBuildDirectory = 
    this->LocalGenerator->GetTargetDirectory(*this->Target);
  this->TargetBuildDirectoryFull = 
    this->LocalGenerator->ConvertToFullPath(this->TargetBuildDirectory);
  cmSystemTools::MakeDirectory(this->TargetBuildDirectoryFull.c_str());
  
  // Construct the rule file name.
  this->BuildFileName = this->TargetBuildDirectory;
  this->BuildFileName += "/build.make";
  this->BuildFileNameFull = this->TargetBuildDirectoryFull;
  this->BuildFileNameFull += "/build.make";
  
  // Construct the rule file name.
  this->ProgressFileName = this->TargetBuildDirectory;
  this->ProgressFileName += "/progress.make";
  this->ProgressFileNameFull = this->TargetBuildDirectoryFull;
  this->ProgressFileNameFull += "/progress.make";

  // reset the progress count
  this->NumberOfProgressActions = 0;

  // Open the rule file.  This should be copy-if-different because the
  // rules may depend on this file itself.
  this->BuildFileStream = 
    new cmGeneratedFileStream(this->BuildFileNameFull.c_str());
  this->BuildFileStream->SetCopyIfDifferent(true);
  if(!this->BuildFileStream)
    {
    return;
    }
  this->LocalGenerator->WriteDisclaimer(*this->BuildFileStream);
  this->LocalGenerator->WriteSpecialTargetsTop(*this->BuildFileStream);
}

//----------------------------------------------------------------------------
void cmMakefileTargetGenerator::WriteTargetBuildRules()
{
  // write the custom commands for this target
  // Look for files registered for cleaning in this directory.
  if(const char* additional_clean_files =
     this->Makefile->GetProperty
     ("ADDITIONAL_MAKE_CLEAN_FILES"))
    {
    cmSystemTools::ExpandListArgument(additional_clean_files, 
                                      this->CleanFiles);
    }  

  // add custom commands to the clean rules?
  const char* clean_no_custom = 
    this->Makefile->GetProperty("CLEAN_NO_CUSTOM");
  bool clean = cmSystemTools::IsOff(clean_no_custom);
  
  // First generate the object rule files.  Save a list of all object
  // files for this target.
  const std::vector<cmSourceFile*>& sources = this->Target->GetSourceFiles();
  for(std::vector<cmSourceFile*>::const_iterator source = sources.begin();
      source != sources.end(); ++source)
    {
    if(cmCustomCommand* cc = (*source)->GetCustomCommand())
      {
      this->GenerateCustomRuleFile(*cc);
      if (clean)
        {
        const std::vector<std::string>& outputs = cc->GetOutputs();
        for(std::vector<std::string>::const_iterator o = outputs.begin();
            o != outputs.end(); ++o)
          {
          this->CleanFiles.push_back
            (this->Convert(o->c_str(),
                           cmLocalGenerator::START_OUTPUT,
                           cmLocalGenerator::UNCHANGED));
          }
        }
      }
    else if(!(*source)->GetPropertyAsBool("HEADER_FILE_ONLY"))
      {
      if(!this->GlobalGenerator->IgnoreFile
         ((*source)->GetSourceExtension().c_str()))
        {
        // Generate this object file's rule file.
        this->WriteObjectRuleFiles(*(*source));
        }
      else if((*source)->GetPropertyAsBool("EXTERNAL_OBJECT"))
        {
        // This is an external object file.  Just add it.
        this->ExternalObjects.push_back((*source)->GetFullPath());
        }
      else
        {
        // We only get here if a source file is not an external object
        // and has an extension that is listed as an ignored file type
        // for this language.  No message or diagnosis should be
        // given.
        }
      }
    }
}


//----------------------------------------------------------------------------
void cmMakefileTargetGenerator::WriteCommonCodeRules()
{
  // Include the dependencies for the target.
  std::string dependFileNameFull = this->TargetBuildDirectoryFull;
  dependFileNameFull += "/depend.make";
  *this->BuildFileStream
    << "# Include any dependencies generated for this target.\n"
    << this->LocalGenerator->IncludeDirective << " "
    << this->Convert(dependFileNameFull.c_str(),
                                     cmLocalGenerator::HOME_OUTPUT,
                                     cmLocalGenerator::MAKEFILE)
    << "\n\n";
  
  // Include the progress variables for the target.
  *this->BuildFileStream
    << "# Include the progress variables for this target.\n"
    << this->LocalGenerator->IncludeDirective << " "
    << this->Convert(this->ProgressFileNameFull.c_str(),
                     cmLocalGenerator::HOME_OUTPUT,
                     cmLocalGenerator::MAKEFILE)
    << "\n\n";

  // make sure the depend file exists
  if (!cmSystemTools::FileExists(dependFileNameFull.c_str()))
    {
    // Write an empty dependency file.
    cmGeneratedFileStream depFileStream(dependFileNameFull.c_str());
    depFileStream
      << "# Empty dependencies file for " << this->Target->GetName() << ".\n"
      << "# This may be replaced when dependencies are built." << std::endl;
    }

  // Open the flags file.  This should be copy-if-different because the
  // rules may depend on this file itself.
  this->FlagFileNameFull = this->TargetBuildDirectoryFull;
  this->FlagFileNameFull += "/flags.make";
  this->FlagFileStream = 
    new cmGeneratedFileStream(this->FlagFileNameFull.c_str());
  this->FlagFileStream->SetCopyIfDifferent(true);
  if(!this->FlagFileStream)
    {
    return;
    }
  this->LocalGenerator->WriteDisclaimer(*this->FlagFileStream);
  
  // Include the flags for the target.
  *this->BuildFileStream
    << "# Include the compile flags for this target's objects.\n"
    << this->LocalGenerator->IncludeDirective << " "
    << this->Convert(this->FlagFileNameFull.c_str(), 
                                     cmLocalGenerator::HOME_OUTPUT, 
                                     cmLocalGenerator::MAKEFILE)
    << "\n\n";
}
  
//----------------------------------------------------------------------------
void cmMakefileTargetGenerator::WriteTargetLanguageFlags()
{
  // write language flags for target
  std::map<cmStdString,cmLocalUnixMakefileGenerator3::IntegrityCheckSet>& 
    checkSet = 
    this->LocalGenerator->GetIntegrityCheckSet()[this->Target->GetName()];
  for(std::map<cmStdString, 
        cmLocalUnixMakefileGenerator3::IntegrityCheckSet>::const_iterator
        l = checkSet.begin(); l != checkSet.end(); ++l)
    {
    const char *lang = l->first.c_str();
    std::string flags;
    // Add the export symbol definition for shared library objects.
    bool shared = ((this->Target->GetType() == cmTarget::SHARED_LIBRARY) ||
                   (this->Target->GetType() == cmTarget::MODULE_LIBRARY));
    if(shared)
      {
      flags += "-D";
      if(const char* custom_export_name = 
         this->Target->GetProperty("DEFINE_SYMBOL"))
        {
        flags += custom_export_name;
        }
      else
        {
        std::string in = this->Target->GetName();
        in += "_EXPORTS";
        flags += cmSystemTools::MakeCindentifier(in.c_str());
        }
      }
    
    // Add language-specific flags.
    this->LocalGenerator
      ->AddLanguageFlags(flags, lang,
                         this->LocalGenerator->ConfigurationName.c_str());
    
    // Add shared-library flags if needed.
    this->LocalGenerator->AddSharedFlags(flags, lang, shared);
    
    // Add include directory flags.
    this->LocalGenerator->
      AppendFlags(flags, this->LocalGenerator->GetIncludeFlags(lang));
    // Add include directory flags.
    this->LocalGenerator->
      AppendFlags(flags,this->GetFrameworkFlags().c_str());

    *this->FlagFileStream << lang << "_FLAGS = " << flags << "\n\n";
    }

  // Add target-specific flags.
  if(this->Target->GetProperty("COMPILE_FLAGS"))
    {
    std::string flags;    
    this->LocalGenerator->AppendFlags
      (flags, this->Target->GetProperty("COMPILE_FLAGS"));
    *this->FlagFileStream << "# TARGET_FLAGS = " << flags << "\n\n";
    }
}

//----------------------------------------------------------------------------
void cmMakefileTargetGenerator::WriteObjectRuleFiles(cmSourceFile& source)
{
  // Identify the language of the source file.
  const char* lang = this->LocalGenerator->GetSourceFileLanguage(source);
  if(!lang)
    {
    // don't know anything about this file so skip it
    return;
    }

  // Get the full path name of the object file.
  std::string objNoTargetDir;
  std::string obj = 
    this->LocalGenerator->GetObjectFileName(*this->Target, source, 
                                            &objNoTargetDir);

  // Avoid generating duplicate rules.
  if(this->ObjectFiles.find(obj) == this->ObjectFiles.end())
    {
    this->ObjectFiles.insert(obj);
    }
  else
    {
    cmOStringStream err;
    err << "Warning: Source file \""
        << source.GetSourceName().c_str() << "."
        << source.GetSourceExtension().c_str()
        << "\" is listed multiple times for target \"" 
        << this->Target->GetName()
        << "\".";
    cmSystemTools::Message(err.str().c_str(), "Warning");
    return;
    }
  
  // Create the directory containing the object file.  This may be a
  // subdirectory under the target's directory.
  std::string dir = cmSystemTools::GetFilenamePath(obj.c_str());
  cmSystemTools::MakeDirectory
    (this->LocalGenerator->ConvertToFullPath(dir).c_str());
  
  // Save this in the target's list of object files.
  if ( source.GetPropertyAsBool("EXTRA_CONTENT") )
    {
    this->ExtraContent.insert(obj);
    }
  this->Objects.push_back(obj);

  // TODO: Remove
  //std::string relativeObj
  //= this->LocalGenerator->GetHomeRelativeOutputPath();
  //relativeObj += obj;

  // we compute some depends when writing the depend.make that we will also
  // use in the build.make, same with depMakeFile
  std::vector<std::string> depends;
  std::string depMakeFile;
  
  // generate the build rule file
  this->WriteObjectBuildFile(obj, lang, source, depends);
  
  // The object file should be checked for dependency integrity.
  this->LocalGenerator->
    CheckDependFiles[this->Target->GetName()][lang].insert(&source);
  // add this to the list of objects for this local generator
  if(cmSystemTools::FileIsFullPath(objNoTargetDir.c_str()))
    {
    objNoTargetDir = cmSystemTools::GetFilenameName(objNoTargetDir);
    }
  this->LocalGenerator->LocalObjectFiles[objNoTargetDir].
    push_back(
      cmLocalUnixMakefileGenerator3::LocalObjectEntry(this->Target, lang)
      );
}

//----------------------------------------------------------------------------
void
cmMakefileTargetGenerator
::WriteObjectBuildFile(std::string &obj,
                       const char *lang, 
                       cmSourceFile& source,
                       std::vector<std::string>& depends)
{
  this->LocalGenerator->AppendRuleDepend(depends, 
                                         this->FlagFileNameFull.c_str());

  // generate the depend scanning rule
  this->WriteObjectDependRules(source, depends);

  std::string relativeObj = this->LocalGenerator->GetHomeRelativeOutputPath();
  if ( source.GetPropertyAsBool("MACOSX_CONTENT") )
    {
    relativeObj = "";
    }
  relativeObj += obj;
  if(this->Makefile->GetDefinition("CMAKE_WINDOWS_OBJECT_PATH"))
    {
    relativeObj = cmSystemTools::ConvertToOutputPath(relativeObj.c_str());
    }
  // Write the build rule.

  // Build the set of compiler flags.
  std::string flags;

  // Add language-specific flags.
  std::string langFlags = "$(";
  langFlags += lang;
  langFlags += "_FLAGS)";
  this->LocalGenerator->AppendFlags(flags, langFlags.c_str());

  // Add target-specific flags.
  if(this->Target->GetProperty("COMPILE_FLAGS"))
    {
    this->LocalGenerator->AppendFlags
      (flags, this->Target->GetProperty("COMPILE_FLAGS"));
    }

  // Add flags from source file properties.
  if (source.GetProperty("COMPILE_FLAGS"))
    {
    this->LocalGenerator->AppendFlags
      (flags, source.GetProperty("COMPILE_FLAGS"));
    *this->FlagFileStream << "# Custom flags: "
                          << relativeObj << "_FLAGS = "
                          << source.GetProperty("COMPILE_FLAGS")
                          << "\n"
                          << "\n";
    }
  
  // Get the output paths for source and object files.
  std::string sourceFile = source.GetFullPath();
  if(this->LocalGenerator->UseRelativePaths)
    {
    sourceFile = this->Convert(sourceFile.c_str(),
                                               cmLocalGenerator::HOME_OUTPUT);
    }
  sourceFile = this->Convert(sourceFile.c_str(),
                                             cmLocalGenerator::NONE,
                                             cmLocalGenerator::SHELL);
  std::string objectFile = this->Convert(obj.c_str(),
                                  cmLocalGenerator::START_OUTPUT,
                                  cmLocalGenerator::SHELL);

  // Construct the build message.
  std::vector<std::string> no_commands;
  std::vector<std::string> commands;
  
  // add in a progress call if needed
  std::string progressDir = this->Makefile->GetHomeOutputDirectory();
  progressDir += cmake::GetCMakeFilesDirectory();
  cmOStringStream progCmd;
  progCmd << "$(CMAKE_COMMAND) -E cmake_progress_report ";
  progCmd << this->LocalGenerator->Convert(progressDir.c_str(),
                                           cmLocalGenerator::FULL,
                                           cmLocalGenerator::SHELL);
  this->NumberOfProgressActions++;
  progCmd << " $(CMAKE_PROGRESS_" 
          << this->NumberOfProgressActions 
          << ")";
  commands.push_back(progCmd.str());
  
  std::string buildEcho = "Building ";
  buildEcho += lang;
  buildEcho += " object ";
  buildEcho += relativeObj;
  this->LocalGenerator->AppendEcho(commands, buildEcho.c_str(),
                                   cmLocalUnixMakefileGenerator3::EchoBuild);

  // Construct the compile rules.
  std::string compileRuleVar = "CMAKE_";
  compileRuleVar += lang;
  compileRuleVar += "_COMPILE_OBJECT";
  std::string compileRule =
    this->Makefile->GetRequiredDefinition(compileRuleVar.c_str());
  cmSystemTools::ExpandListArgument(compileRule, commands);

  std::string targetOutPathPDB;
  {
  std::string targetFullPathPDB;
  const char* configName = this->LocalGenerator->ConfigurationName.c_str();
  if(this->Target->GetType() == cmTarget::EXECUTABLE)
    {
    targetFullPathPDB = this->LocalGenerator->ExecutableOutputPath;
    targetFullPathPDB += this->Target->GetPDBName(configName);
    }
  else if(this->Target->GetType() == cmTarget::STATIC_LIBRARY ||
          this->Target->GetType() == cmTarget::SHARED_LIBRARY ||
          this->Target->GetType() == cmTarget::MODULE_LIBRARY)
    {
    targetFullPathPDB = this->LocalGenerator->LibraryOutputPath;
    targetFullPathPDB += this->Target->GetPDBName(configName);
    }
  targetOutPathPDB =
    this->Convert(targetFullPathPDB.c_str(),cmLocalGenerator::FULL,
                          cmLocalGenerator::MAKEFILE);
  }
  cmLocalGenerator::RuleVariables vars;
  vars.Language = lang;
  vars.TargetPDB = targetOutPathPDB.c_str();
  vars.Source = sourceFile.c_str();
  vars.Object = relativeObj.c_str();
  std::string objdir = this->LocalGenerator->GetHomeRelativeOutputPath();
  objdir = this->Convert(objdir.c_str(), 
                         cmLocalGenerator::START_OUTPUT,
                         cmLocalGenerator::SHELL);
  std::string objectDir = cmSystemTools::GetFilenamePath(obj);
  vars.ObjectDir = objectDir.c_str();
  vars.Flags = flags.c_str();
  
  // Expand placeholders in the commands.
  for(std::vector<std::string>::iterator i = commands.begin();
      i != commands.end(); ++i)
    {
    this->LocalGenerator->ExpandRuleVariables(*i, vars);
    }

  // Make the target dependency scanning rule include cmake-time-known
  // dependencies.  The others are handled by the check-build-system
  // path.
  std::string depMark = 
    this->LocalGenerator->GetRelativeTargetDirectory(*this->Target);
  depMark += "/depend.make.mark";
  this->LocalGenerator->WriteMakeRule(*this->BuildFileStream, 0,
                                      depMark.c_str(),
                                      depends, no_commands, false);
  
  // Write the rule.
  this->LocalGenerator->WriteMakeRule(*this->BuildFileStream, 0,
                                      relativeObj.c_str(),
                                      depends, commands, false);

  bool lang_is_c_or_cxx = ((strcmp(lang, "C") == 0) ||
                           (strcmp(lang, "CXX") == 0));
  bool do_preprocess_rules = lang_is_c_or_cxx &&
    this->LocalGenerator->GetCreatePreprocessedSourceRules();
  bool do_assembly_rules = lang_is_c_or_cxx &&
    this->LocalGenerator->GetCreateAssemblySourceRules();
  if(do_preprocess_rules || do_assembly_rules)
    {
    std::vector<std::string> force_depends;
    force_depends.push_back("cmake_force");
    std::string::size_type dot_pos = relativeObj.rfind(".");
    std::string relativeObjBase = relativeObj.substr(0, dot_pos);

    if(do_preprocess_rules)
      {
      commands.clear();
      std::string relativeObjI = relativeObjBase + ".i";

      std::string preprocessEcho = "Preprocessing ";
      preprocessEcho += lang;
      preprocessEcho += " source to ";
      preprocessEcho += relativeObjI;
      this->LocalGenerator->AppendEcho(
        commands, preprocessEcho.c_str(),
        cmLocalUnixMakefileGenerator3::EchoBuild
        );

      std::string preprocessRuleVar = "CMAKE_";
      preprocessRuleVar += lang;
      preprocessRuleVar += "_CREATE_PREPROCESSED_SOURCE";
      if(const char* preprocessRule =
         this->Makefile->GetDefinition(preprocessRuleVar.c_str()))
        {
        cmSystemTools::ExpandListArgument(preprocessRule, commands);

        vars.PreprocessedSource = relativeObjI.c_str();

        // Expand placeholders in the commands.
        for(std::vector<std::string>::iterator i = commands.begin();
            i != commands.end(); ++i)
          {
          this->LocalGenerator->ExpandRuleVariables(*i, vars);
          }
        }
      else
        {
        std::string cmd = "$(CMAKE_COMMAND) -E cmake_unimplemented_variable ";
        cmd += preprocessRuleVar;
        commands.push_back(cmd);
        }

      this->LocalGenerator->WriteMakeRule(*this->BuildFileStream, 0,
                                          relativeObjI.c_str(),
                                          force_depends, commands, false);
      }

    if(do_assembly_rules)
      {
      commands.clear();
      std::string relativeObjS = relativeObjBase + ".s";

      std::string assemblyEcho = "Compiling ";
      assemblyEcho += lang;
      assemblyEcho += " source to assembly ";
      assemblyEcho += relativeObjS;
      this->LocalGenerator->AppendEcho(
        commands, assemblyEcho.c_str(),
        cmLocalUnixMakefileGenerator3::EchoBuild
        );

      std::string assemblyRuleVar = "CMAKE_";
      assemblyRuleVar += lang;
      assemblyRuleVar += "_CREATE_ASSEMBLY_SOURCE";
      if(const char* assemblyRule =
         this->Makefile->GetDefinition(assemblyRuleVar.c_str()))
        {
        cmSystemTools::ExpandListArgument(assemblyRule, commands);

        vars.AssemblySource = relativeObjS.c_str();

        // Expand placeholders in the commands.
        for(std::vector<std::string>::iterator i = commands.begin();
            i != commands.end(); ++i)
          {
          this->LocalGenerator->ExpandRuleVariables(*i, vars);
          }
        }
      else
        {
        std::string cmd = "$(CMAKE_COMMAND) -E cmake_unimplemented_variable ";
        cmd += assemblyRuleVar;
        commands.push_back(cmd);
        }

      this->LocalGenerator->WriteMakeRule(*this->BuildFileStream, 0,
                                          relativeObjS.c_str(),
                                          force_depends, commands, false);
      }
    }

  // If the language needs provides-requires mode, create the
  // corresponding targets.
  std::string objectRequires = relativeObj;
  objectRequires += ".requires";
  std::vector<std::string> p_depends;
  // always provide an empty requires target
  this->LocalGenerator->WriteMakeRule(*this->BuildFileStream, 0,
                                      objectRequires.c_str(), p_depends, 
                                      no_commands, true);

  // write a build rule to recursively build what this obj provides
  std::string objectProvides = relativeObj;
  objectProvides += ".provides";
  std::string temp = relativeObj;
  temp += ".provides.build";
  std::vector<std::string> r_commands;
  std::string tgtMakefileName = 
    this->LocalGenerator->GetRelativeTargetDirectory(*this->Target);
  tgtMakefileName += "/build.make";
  r_commands.push_back
    (this->LocalGenerator->GetRecursiveMakeCall(tgtMakefileName.c_str(),
                                                temp.c_str()));

  p_depends.clear();
  p_depends.push_back(objectRequires);
  this->LocalGenerator->WriteMakeRule(*this->BuildFileStream, 0,
                                      objectProvides.c_str(), p_depends, 
                                      r_commands, true);
  
  // write the provides.build rule dependency on the obj file
  p_depends.clear();
  p_depends.push_back(relativeObj);
  this->LocalGenerator->WriteMakeRule(*this->BuildFileStream, 0,
                                      temp.c_str(), p_depends, no_commands,
                                      true);
}

//----------------------------------------------------------------------------
void cmMakefileTargetGenerator::WriteTargetRequiresRules()
{
  std::vector<std::string> depends;
  std::vector<std::string> no_commands;

  // Construct the name of the dependency generation target.
  std::string depTarget = 
    this->LocalGenerator->GetRelativeTargetDirectory(*this->Target);
  depTarget += "/requires";

  // This target drives dependency generation for all object files.
  std::string relPath = this->LocalGenerator->GetHomeRelativeOutputPath();
  std::string objTarget;
  for(std::vector<std::string>::const_iterator obj = this->Objects.begin();
      obj != this->Objects.end(); ++obj)
    {
    objTarget = relPath;
    objTarget += *obj;
    objTarget += ".requires";
    depends.push_back(objTarget);
    }

  // Write the rule.
  this->LocalGenerator->WriteMakeRule(*this->BuildFileStream, 0,
                                      depTarget.c_str(),
                                      depends, no_commands, true);
}

//----------------------------------------------------------------------------
void cmMakefileTargetGenerator::WriteTargetCleanRules()
{
  std::vector<std::string> depends;
  std::vector<std::string> commands;

  // Construct the clean target name.
  std::string cleanTarget = 
    this->LocalGenerator->GetRelativeTargetDirectory(*this->Target);
  cleanTarget += "/clean";
  
  // Construct the clean command.
  this->LocalGenerator->AppendCleanCommand(commands, this->CleanFiles,
                                           *this->Target);
  this->LocalGenerator->CreateCDCommand
    (commands,
                                        this->Makefile->GetStartOutputDirectory(),
                                        this->Makefile->GetHomeOutputDirectory());

  // Write the rule.
  this->LocalGenerator->WriteMakeRule(*this->BuildFileStream, 0,
                                      cleanTarget.c_str(),
                                      depends, commands, true);
}


//----------------------------------------------------------------------------
void cmMakefileTargetGenerator::WriteTargetDependRules()
{
  // must write the targets depend info file
  std::string dir = this->LocalGenerator->GetTargetDirectory(*this->Target);
  this->InfoFileNameFull = dir;
  this->InfoFileNameFull += "/DependInfo.cmake";
  this->InfoFileNameFull = 
    this->LocalGenerator->ConvertToFullPath(this->InfoFileNameFull);
  this->InfoFileStream = 
    new cmGeneratedFileStream(this->InfoFileNameFull.c_str());
  this->InfoFileStream->SetCopyIfDifferent(true);
  if(!*this->InfoFileStream)
    {
    return;
    }
  this->LocalGenerator->
    WriteDependLanguageInfo(*this->InfoFileStream,*this->Target);
  
  // and now write the rule to use it
  std::vector<std::string> depends;
  std::vector<std::string> commands;

  // Construct the name of the dependency generation target.
  std::string depTarget = 
    this->LocalGenerator->GetRelativeTargetDirectory(*this->Target);
  depTarget += "/depend";
  
  std::string depMark = depTarget;
  depMark += ".make.mark";
  depends.push_back(depMark);
  
  this->LocalGenerator->WriteMakeRule(*this->BuildFileStream, 0,
                                      depTarget.c_str(),
                                      depends, commands, true);
  depends.clear();
  
  // Write the dependency generation rule.
  std::string depEcho = "Scanning dependencies of target ";
  depEcho += this->Target->GetName();
  this->LocalGenerator->AppendEcho(commands, depEcho.c_str(),
                                   cmLocalUnixMakefileGenerator3::EchoDepend);
  
  // Add a command to call CMake to scan dependencies.  CMake will
  // touch the corresponding depends file after scanning dependencies.
  cmOStringStream depCmd;
  // TODO: Account for source file properties and directory-level
  // definitions when scanning for dependencies.
#if !defined(_WIN32) || defined(__CYGWIN__)
  // This platform supports symlinks, so cmSystemTools will translate
  // paths.  Make sure PWD is set to the original name of the home
  // output directory to help cmSystemTools to create the same
  // translation table for the dependency scanning process.
  depCmd << "cd "
         << (this->LocalGenerator->Convert(
               this->Makefile->GetHomeOutputDirectory(),
               cmLocalGenerator::FULL, cmLocalGenerator::SHELL))
         << " && ";
#endif
  // Generate a call this signature:
  //
  //   cmake -E cmake_depends <generator>
  //                          <home-src-dir> <start-src-dir>
  //                          <home-out-dir> <start-out-dir>
  //                          <dep-info>
  //
  // This gives the dependency scanner enough information to recreate
  // the state of our local generator sufficiently for its needs.
  depCmd << "$(CMAKE_COMMAND) -E cmake_depends \""
         << this->GlobalGenerator->GetName() << "\" "
         << this->Convert(this->Makefile->GetHomeDirectory(),
                          cmLocalGenerator::FULL, cmLocalGenerator::SHELL)
         << " "
         << this->Convert(this->Makefile->GetStartDirectory(),
                          cmLocalGenerator::FULL, cmLocalGenerator::SHELL)
         << " "
         << this->Convert(this->Makefile->GetHomeOutputDirectory(),
                          cmLocalGenerator::FULL, cmLocalGenerator::SHELL)
         << " "
         << this->Convert(this->Makefile->GetStartOutputDirectory(),
                          cmLocalGenerator::FULL, cmLocalGenerator::SHELL)
         << " "
         << this->Convert(this->InfoFileNameFull.c_str(),
                          cmLocalGenerator::FULL, cmLocalGenerator::SHELL);
  commands.push_back(depCmd.str());
  
  // Make sure all custom command outputs in this target are built.
  if(this->DriveCustomCommandsOnDepends)
    {
    this->DriveCustomCommands(depends);
    }

  // Write the rule.
  this->LocalGenerator->WriteMakeRule(*this->BuildFileStream, 0,
                                      depMark.c_str(),
                                      depends, commands, false);
}

//----------------------------------------------------------------------------
void
cmMakefileTargetGenerator
::DriveCustomCommands(std::vector<std::string>& depends)
{
  // Depend on all custom command outputs.
  const std::vector<cmSourceFile*>& sources =
    this->Target->GetSourceFiles();
  for(std::vector<cmSourceFile*>::const_iterator source = sources.begin();
      source != sources.end(); ++source)
    {
    if(cmCustomCommand* cc = (*source)->GetCustomCommand())
      {
      const std::vector<std::string>& outputs = cc->GetOutputs();
      for(std::vector<std::string>::const_iterator o = outputs.begin();
          o != outputs.end(); ++o)
        {
        depends.push_back(*o);
        }
      }
    }
}

//----------------------------------------------------------------------------
void cmMakefileTargetGenerator
::WriteObjectDependRules(cmSourceFile& source,
                         std::vector<std::string>& depends)
{
  // Create the list of dependencies known at cmake time.  These are
  // shared between the object file and dependency scanning rule.
  depends.push_back(source.GetFullPath());
  if(const char* objectDeps = source.GetProperty("OBJECT_DEPENDS"))
    {
    std::vector<std::string> deps;
    cmSystemTools::ExpandListArgument(objectDeps, deps);
    for(std::vector<std::string>::iterator i = deps.begin();
        i != deps.end(); ++i)
      {
      depends.push_back(i->c_str());
      }
    }
}

//----------------------------------------------------------------------------
void cmMakefileTargetGenerator
::GenerateCustomRuleFile(const cmCustomCommand& cc)
{
  // Collect the commands.
  std::vector<std::string> commands;
  std::string comment = this->LocalGenerator->ConstructComment(cc);
  if(!comment.empty())
    {
    // add in a progress call if needed
    std::string progressDir = this->Makefile->GetHomeOutputDirectory();
    progressDir += cmake::GetCMakeFilesDirectory();
    cmOStringStream progCmd;
    progCmd << "$(CMAKE_COMMAND) -E cmake_progress_report ";
    progCmd << this->LocalGenerator->Convert(progressDir.c_str(),
                                             cmLocalGenerator::FULL,
                                             cmLocalGenerator::SHELL);
    this->NumberOfProgressActions++;
    progCmd << " $(CMAKE_PROGRESS_" 
            << this->NumberOfProgressActions 
            << ")";
    commands.push_back(progCmd.str());
    this->LocalGenerator
      ->AppendEcho(commands, comment.c_str(),
                   cmLocalUnixMakefileGenerator3::EchoGenerate);
    }
  this->LocalGenerator->AppendCustomCommand(commands, cc);
  
  // Collect the dependencies.
  std::vector<std::string> depends;
  this->LocalGenerator->AppendCustomDepend(depends, cc);

  // Check whether we need to bother checking for a symbolic output.
  bool need_symbolic = this->GlobalGenerator->GetNeedSymbolicMark();

  // Write the rule.
  const std::vector<std::string>& outputs = cc.GetOutputs();
  std::vector<std::string>::const_iterator o = outputs.begin();
  {
  bool symbolic = false;
  if(need_symbolic)
    {
    if(cmSourceFile* sf = this->Makefile->GetSource(o->c_str()))
      {
      symbolic = sf->GetPropertyAsBool("SYMBOLIC");
      }
    }
  this->LocalGenerator->WriteMakeRule(*this->BuildFileStream, 0,
                                      o->c_str(), depends, commands,
                                      symbolic);
  }

  // If the rule has multiple outputs, add a rule for the extra
  // outputs to just depend on the first output with no command.  Also
  // register the extra outputs as paired with the first output so
  // that the check-build-system step will remove the primary output
  // if any extra outputs are missing, forcing the rule to regenerate
  // all outputs.
  depends.clear();
  depends.push_back(*o);
  commands.clear();
  std::string emptyCommand = this->GlobalGenerator->GetEmptyCommandHack();
  if(!emptyCommand.empty())
    {
    commands.push_back(emptyCommand);
    }
  for(++o; o != outputs.end(); ++o)
    {
    bool symbolic = false;
    if(need_symbolic)
      {
      if(cmSourceFile* sf = this->Makefile->GetSource(o->c_str()))
        {
        symbolic = sf->GetPropertyAsBool("SYMBOLIC");
        }
      }
    this->LocalGenerator->WriteMakeRule(*this->BuildFileStream, 0,
                                        o->c_str(), depends, commands,
                                        symbolic);
    this->GlobalGenerator->AddMultipleOutputPair(o->c_str(),
                                                 depends[0].c_str());
    }
}

//----------------------------------------------------------------------------
void
cmMakefileTargetGenerator
::WriteObjectsVariable(std::string& variableName,
                       std::string& variableNameExternal)
{
  // Write a make variable assignment that lists all objects for the
  // target.
  variableName = 
    this->LocalGenerator->CreateMakeVariable(this->Target->GetName(), 
                                             "_OBJECTS");
  *this->BuildFileStream
    << "# Object files for target " << this->Target->GetName() << "\n"
    << variableName.c_str() << " =";
  std::string object;
  const char* objName =  
    this->Makefile->GetDefinition("CMAKE_NO_QUOTED_OBJECTS");
  const char* lineContinue = 
    this->Makefile->GetDefinition("CMAKE_MAKE_LINE_CONTINUE");
  if(!lineContinue)
    {
    lineContinue = "\\";
    }
  for(std::vector<std::string>::const_iterator i = this->Objects.begin();
      i != this->Objects.end(); ++i)
    {
    if ( this->ExtraContent.find(i->c_str()) != this->ExtraContent.end() )
      {
      continue;
      }
    *this->BuildFileStream << " " << lineContinue << "\n";
    if(objName)
      {
      *this->BuildFileStream << 
        this->Convert(i->c_str(), cmLocalGenerator::START_OUTPUT, 
                      cmLocalGenerator::MAKEFILE);
      }
    else
      {
      *this->BuildFileStream  << 
        this->LocalGenerator->ConvertToQuotedOutputPath(i->c_str());
      }
    }
  *this->BuildFileStream << "\n";

  // Write a make variable assignment that lists all external objects
  // for the target.
  variableNameExternal = 
    this->LocalGenerator->CreateMakeVariable(this->Target->GetName(),
                                             "_EXTERNAL_OBJECTS");
  *this->BuildFileStream
    << "\n"
    << "# External object files for target " 
    << this->Target->GetName() << "\n"
    << variableNameExternal.c_str() << " =";
  for(std::vector<std::string>::const_iterator i = 
        this->ExternalObjects.begin();
      i != this->ExternalObjects.end(); ++i)
    {
    object = this->Convert(i->c_str(),cmLocalGenerator::START_OUTPUT);
    *this->BuildFileStream
      << " " << lineContinue << "\n"
      << this->Makefile->GetSafeDefinition("CMAKE_OBJECT_NAME");
    if(objName)
      {
      *this->BuildFileStream  << 
        this->Convert(i->c_str(), cmLocalGenerator::START_OUTPUT, 
                                               cmLocalGenerator::MAKEFILE);
      }
    else
      {
      *this->BuildFileStream  << 
        this->LocalGenerator->ConvertToQuotedOutputPath(i->c_str());
      }
    }
  *this->BuildFileStream << "\n" << "\n";
}

//----------------------------------------------------------------------------
void
cmMakefileTargetGenerator
::WriteObjectsString(std::string& buildObjs)
{
  std::string object;
  const char* no_quoted =
    this->Makefile->GetDefinition("CMAKE_NO_QUOTED_OBJECTS");
  const char* space = "";
  for(std::vector<std::string>::const_iterator i = this->Objects.begin();
      i != this->Objects.end(); ++i)
    {
    if ( this->ExtraContent.find(i->c_str()) != this->ExtraContent.end() )
      {
      continue;
      }
    buildObjs += space;
    space = " ";
    if(no_quoted)
      {
      buildObjs +=
        this->Convert(i->c_str(), cmLocalGenerator::START_OUTPUT,
                      cmLocalGenerator::SHELL);
      }
    else
      {
      buildObjs +=
        this->LocalGenerator->ConvertToQuotedOutputPath(i->c_str());
      }
    }
  for(std::vector<std::string>::const_iterator i =
        this->ExternalObjects.begin();
      i != this->ExternalObjects.end(); ++i)
    {
    buildObjs += space;
    space = " ";
    if(no_quoted)
      {
      buildObjs +=
        this->Convert(i->c_str(), cmLocalGenerator::START_OUTPUT,
                      cmLocalGenerator::SHELL);
      }
    else
      {
      buildObjs +=
        this->LocalGenerator->ConvertToQuotedOutputPath(i->c_str());
      }
    }
}

//----------------------------------------------------------------------------
void cmMakefileTargetGenerator::WriteTargetDriverRule(const char* main_output,
                                                      bool relink)
{
  // Compute the name of the driver target.
  std::string dir =
    this->LocalGenerator->GetRelativeTargetDirectory(*this->Target);
  std::string buildTargetRuleName = dir;
  buildTargetRuleName += relink?"/preinstall":"/build";
  buildTargetRuleName = this->Convert(buildTargetRuleName.c_str(),
                                      cmLocalGenerator::HOME_OUTPUT,
                                      cmLocalGenerator::MAKEFILE);

  // Build the list of target outputs to drive.
  std::vector<std::string> depends;
  if(main_output)
    {
    depends.push_back(main_output);
    }

  const char* comment = 0;
  if(relink)
    {
    // Setup the comment for the preinstall driver.
    comment = "Rule to relink during preinstall.";
    }
  else
    {
    // Setup the comment for the main build driver.
    comment = "Rule to build all files generated by this target.";

    // Make sure all custom command outputs in this target are built.
    if(!this->DriveCustomCommandsOnDepends)
          {
      this->DriveCustomCommands(depends);
      }
    }

  // Write the driver rule.
  std::vector<std::string> no_commands;
  this->LocalGenerator->WriteMakeRule(*this->BuildFileStream, comment,
                                      buildTargetRuleName.c_str(),
                                      depends, no_commands, true);
}

//----------------------------------------------------------------------------
std::string cmMakefileTargetGenerator::GetFrameworkFlags()
{
#ifndef __APPLE__
  return std::string();
#else
  std::set<cmStdString> emitted;
  std::vector<std::string> includes;
  this->LocalGenerator->GetIncludeDirectories(includes);
  std::vector<std::string>::iterator i;
  // check all include directories for frameworks as this
  // will already have added a -F for the framework
  for(i = includes.begin(); i != includes.end(); ++i)
    {
    if(cmSystemTools::IsPathToFramework(i->c_str()))
      {
      std::string frameworkDir = *i;
      frameworkDir += "/../";
      frameworkDir = cmSystemTools::CollapseFullPath(frameworkDir.c_str());
      emitted.insert(frameworkDir);
      }
    }

  std::string flags;
  std::vector<std::string>& frameworks = this->Target->GetFrameworks();
  for(i = frameworks.begin();
      i != frameworks.end(); ++i)
    {
    if(emitted.insert(*i).second)
      {
      flags += "-F";
      flags += this->LocalGenerator->ConvertToOutputForExisting(i->c_str());
      flags += " ";
      }
    }
  return flags;
#endif
}

//----------------------------------------------------------------------------
void cmMakefileTargetGenerator
::AppendTargetDepends(std::vector<std::string>& depends)
{
  // Static libraries never depend on anything for linking.
  if(this->Target->GetType() == cmTarget::STATIC_LIBRARY)
    {
    return;
    }
  // Compute which library configuration to link.
  cmTarget::LinkLibraryType linkType = cmTarget::OPTIMIZED;
  if(cmSystemTools::UpperCase(
       this->LocalGenerator->ConfigurationName.c_str()) == "DEBUG")
    {
    linkType = cmTarget::DEBUG;
    }
  // Keep track of dependencies already listed.
  std::set<cmStdString> emitted;

  // A target should not depend on itself.
  emitted.insert(this->Target->GetName());

  // Loop over all library dependencies.
  const cmTarget::LinkLibraryVectorType& tlibs = 
    this->Target->GetLinkLibraries();
  for(cmTarget::LinkLibraryVectorType::const_iterator lib = tlibs.begin();
      lib != tlibs.end(); ++lib)
    {
    // skip the library if it is not general and the link type
    // does not match the current target
    if(lib->second != cmTarget::GENERAL &&
       lib->second != linkType)
      {
      continue;
      }
       
    // Don't emit the same library twice for this target.
    if(emitted.insert(lib->first).second)
      {
      // Depend on other CMake targets.
      if(cmTarget* tgt = 
         this->GlobalGenerator->FindTarget(0, lib->first.c_str()))
        {
        if(const char* location =
           tgt->GetLocation(this->LocalGenerator->ConfigurationName.c_str()))
          {
          depends.push_back(location);
          }
        }
      // depend on full path libs as well
      else if(cmSystemTools::FileIsFullPath(lib->first.c_str()))
        {
        depends.push_back(lib->first.c_str());
        }
      }
    }
}

//----------------------------------------------------------------------------
void cmMakefileTargetGenerator
::CloseFileStreams()
{
  delete this->BuildFileStream;
  delete this->InfoFileStream;
  delete this->FlagFileStream;
}

void cmMakefileTargetGenerator::RemoveForbiddenFlags(const char* flagVar,
                                                     const char* linkLang, 
                                                     std::string& linkFlags)
{
  // check for language flags that are not allowed at link time, and
  // remove them, -w on darwin for gcc -w -dynamiclib sends -w to libtool
  // which fails, there may be more]
  
  std::string removeFlags = "CMAKE_";
  removeFlags += linkLang;
  removeFlags += flagVar;
  std::string removeflags = 
    this->Makefile->GetSafeDefinition(removeFlags.c_str());
  std::vector<std::string> removeList;
  cmSystemTools::ExpandListArgument(removeflags, removeList);
  for(std::vector<std::string>::iterator i = removeList.begin();
      i != removeList.end(); ++i)
    {
    cmSystemTools::ReplaceString(linkFlags, i->c_str(), "");
    }
}

void cmMakefileTargetGenerator::WriteProgressVariables(unsigned long total,
                                                       unsigned long &current)
{
  cmGeneratedFileStream *progressFileStream = 
    new cmGeneratedFileStream(this->ProgressFileNameFull.c_str());
  if(!progressFileStream)
    {
    return;
    }

  unsigned long num;
  unsigned long i;
  for (i = 1; i <= this->NumberOfProgressActions; ++i)
    {
    *progressFileStream
      << "CMAKE_PROGRESS_" << i << " = ";
    if (total <= 100)
      {
      num = i + current;
      *progressFileStream << num;
      this->LocalGenerator->ProgressFiles[this->Target->GetName()]
        .push_back(num);
      }
    else if (((i+current)*100)/total > ((i-1+current)*100)/total)
      {
      num = ((i+current)*100)/total;
      *progressFileStream << num;
      this->LocalGenerator->ProgressFiles[this->Target->GetName()]
        .push_back(num);
      }
    *progressFileStream << "\n";
    }
  *progressFileStream << "\n";
  current += this->NumberOfProgressActions;
  delete progressFileStream;
}

