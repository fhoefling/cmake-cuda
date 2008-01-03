/*=========================================================================

Program:   CMake - Cross-Platform Makefile Generator
Module:    $RCSfile: cmGlobalXCodeGenerator.cxx,v $
Language:  C++
Date:      $Date: 2006/10/27 20:01:47 $
Version:   $Revision: 1.111.2.6 $

Copyright (c) 2002 Kitware, Inc., Insight Consortium.  All rights reserved.
See Copyright.txt or http://www.cmake.org/HTML/Copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even 
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#include "cmGlobalXCodeGenerator.h"
#include "cmGlobalXCode21Generator.h"
#include "cmLocalXCodeGenerator.h"
#include "cmMakefile.h"
#include "cmXCodeObject.h"
#include "cmXCode21Object.h"
#include "cmake.h"
#include "cmGeneratedFileStream.h"
#include "cmSourceFile.h"
#include "cmOrderLinkDirectories.h"

#if defined(CMAKE_BUILD_WITH_CMAKE)
#include "cmXMLParser.h"

// parse the xml file storing the installed version of Xcode on
// the machine
class cmXcodeVersionParser : public cmXMLParser
{
public:
  void StartElement(const char* , const char** )
    {
      this->Data = "";
    }
  void EndElement(const char* name)
    {
      if(strcmp(name, "key") == 0)
        {
        this->Key = this->Data;
        }
      else if(strcmp(name, "string") == 0)
        {
        if(this->Key == "CFBundleShortVersionString")
          {
          this->Version = (int)(10.0 * atof(this->Data.c_str()));
          }
        }
    }
  void CharacterDataHandler(const char* data, int length)
    {
      this->Data.append(data, length);
    }
  int Version;
  std::string Key;
  std::string Data;
};
#endif


//----------------------------------------------------------------------------
cmGlobalXCodeGenerator::cmGlobalXCodeGenerator()
{
  this->FindMakeProgramFile = "CMakeFindXCode.cmake";
  this->RootObject = 0;
  this->MainGroupChildren = 0;
  this->SourcesGroupChildren = 0;
  this->CurrentMakefile = 0;
  this->CurrentLocalGenerator = 0;
  this->XcodeVersion = 15;
}

//----------------------------------------------------------------------------
cmGlobalGenerator* cmGlobalXCodeGenerator::New()
{ 
#if defined(CMAKE_BUILD_WITH_CMAKE)  
  cmXcodeVersionParser parser;
  parser.ParseFile
    ("/Developer/Applications/Xcode.app/Contents/version.plist");
  if(parser.Version == 15)
    {
    return new cmGlobalXCodeGenerator;
    }
  else if (parser.Version == 20)
    {
    cmSystemTools::Message("Xcode 2.0 not really supported by cmake, "
                           "using Xcode 15 generator\n");
    return new cmGlobalXCodeGenerator;
    }
  cmGlobalXCodeGenerator* ret = new cmGlobalXCode21Generator;
  ret->SetVersion(parser.Version);
  return ret;
#else
  std::cerr << "CMake should be built with cmake to use XCode, "
    "default to Xcode 1.5\n";
  return new cmGlobalXCodeGenerator;
#endif
}

//----------------------------------------------------------------------------
void cmGlobalXCodeGenerator::EnableLanguage(std::vector<std::string>const&
                                            lang,
                                            cmMakefile * mf)
{ 
  mf->AddDefinition("XCODE","1");
  if(this->XcodeVersion == 15)
    {
    }
  else
    {
    mf->AddCacheDefinition(
      "CMAKE_CONFIGURATION_TYPES",
      "Debug;Release;MinSizeRel;RelWithDebInfo",
      "Semicolon separated list of supported configuration types, "
      "only supports Debug, Release, MinSizeRel, and RelWithDebInfo, "
      "anything else will be ignored.",
      cmCacheManager::STRING);
    }
  mf->AddDefinition("CMAKE_GENERATOR_CC", "gcc");
  mf->AddDefinition("CMAKE_GENERATOR_CXX", "g++");
  mf->AddDefinition("CMAKE_GENERATOR_NO_COMPILER_ENV", "1");
  this->cmGlobalGenerator::EnableLanguage(lang, mf);
}

//----------------------------------------------------------------------------
std::string cmGlobalXCodeGenerator
::GenerateBuildCommand(const char* makeProgram,
                       const char *projectName, 
                       const char* additionalOptions, 
                       const char *targetName,
                       const char* config, 
                       bool ignoreErrors,
                       bool)
{
  // Config is not used yet
  (void) ignoreErrors;

  // now build the test
  if(makeProgram == 0 || !strlen(makeProgram))
    {
    cmSystemTools::Error(
      "Generator cannot find the appropriate make command.");
    return "";
    }
  std::string makeCommand = 
    cmSystemTools::ConvertToOutputPath(makeProgram);
  std::string lowerCaseCommand = makeCommand;
  cmSystemTools::LowerCase(lowerCaseCommand);

  makeCommand += " -project ";
  makeCommand += projectName;
  makeCommand += ".xcode";
  if(this->XcodeVersion > 20)
    {
    makeCommand += "proj";
    }

  bool clean = false;
  if ( targetName && strcmp(targetName, "clean") == 0 )
    {
    clean = true;
    targetName = "ALL_BUILD";
    }
  if(clean)
    {
    makeCommand += " clean";
    }
  else
    {
    makeCommand += " build";
    }
  makeCommand += " -target ";
  if (targetName && strlen(targetName))
    {
    makeCommand += targetName;
    }
  else
    {
    makeCommand += "ALL_BUILD";
    }
  if(this->XcodeVersion == 15)
    {
    makeCommand += " -buildstyle Development ";
    }
  else
    {
    makeCommand += " -configuration ";
    makeCommand += config?config:"Debug";
    }
  if ( additionalOptions )
    {
    makeCommand += " ";
    makeCommand += additionalOptions;
    }
  return makeCommand;
}

//----------------------------------------------------------------------------
void cmGlobalXCodeGenerator::ConfigureOutputPaths()
{
  // Format the library and executable output paths.
  this->LibraryOutputPath = 
    this->CurrentMakefile->GetSafeDefinition("LIBRARY_OUTPUT_PATH");
  if(this->LibraryOutputPath.size() == 0)
    {
    this->LibraryOutputPath = 
      this->CurrentMakefile->GetCurrentOutputDirectory();
    }
  // make sure there is a trailing slash
  if(this->LibraryOutputPath.size() && 
     this->LibraryOutputPath[this->LibraryOutputPath.size()-1] != '/')
    {
    this->LibraryOutputPath += "/";
    if(!cmSystemTools::MakeDirectory(this->LibraryOutputPath.c_str()))
      {
      cmSystemTools::Error("Error creating directory ",
                           this->LibraryOutputPath.c_str());
      }
    }
  this->CurrentMakefile->AddLinkDirectory(this->LibraryOutputPath.c_str());
  this->ExecutableOutputPath = 
    this->CurrentMakefile->GetSafeDefinition("EXECUTABLE_OUTPUT_PATH");
  if(this->ExecutableOutputPath.size() == 0)
    {
    this->ExecutableOutputPath = 
      this->CurrentMakefile->GetCurrentOutputDirectory();
    }
  // make sure there is a trailing slash
  if(this->ExecutableOutputPath.size() && 
     this->ExecutableOutputPath[this->ExecutableOutputPath.size()-1] != '/')
    {
    this->ExecutableOutputPath += "/";
    if(!cmSystemTools::MakeDirectory(this->ExecutableOutputPath.c_str()))
      {
      cmSystemTools::Error("Error creating directory ",
                           this->ExecutableOutputPath.c_str());
      }
    }
}

//----------------------------------------------------------------------------
///! Create a local generator appropriate to this Global Generator
cmLocalGenerator *cmGlobalXCodeGenerator::CreateLocalGenerator()
{
  cmLocalGenerator *lg = new cmLocalXCodeGenerator;
  lg->SetGlobalGenerator(this);
  return lg;
}

//----------------------------------------------------------------------------
void cmGlobalXCodeGenerator::Generate()
{
  this->cmGlobalGenerator::Generate();
  std::map<cmStdString, std::vector<cmLocalGenerator*> >::iterator it;
  for(it = this->ProjectMap.begin(); it!= this->ProjectMap.end(); ++it)
    { 
    cmLocalGenerator* root = it->second[0];
    this->CurrentProject = root->GetMakefile()->GetProjectName();
    this->SetCurrentLocalGenerator(root);
    this->OutputDir = this->CurrentMakefile->GetHomeOutputDirectory();
    this->OutputDir = 
      cmSystemTools::CollapseFullPath(this->OutputDir.c_str());
    cmSystemTools::SplitPath(this->OutputDir.c_str(),
                             this->ProjectOutputDirectoryComponents);
    this->CurrentLocalGenerator = root;
    // add ALL_BUILD, INSTALL, etc
    this->AddExtraTargets(root, it->second);
    // now create the project
    this->OutputXCodeProject(root, it->second);
    }
}

//----------------------------------------------------------------------------
void 
cmGlobalXCodeGenerator::AddExtraTargets(cmLocalGenerator* root, 
                                        std::vector<cmLocalGenerator*>& gens)
{
  cmMakefile* mf = root->GetMakefile();
  // Add ALL_BUILD
  const char* no_working_directory = 0;
  std::vector<std::string> no_depends;
  mf->AddUtilityCommand("ALL_BUILD", false, no_depends,
                        no_working_directory,
                        "echo", "Build all projects");
  cmTarget* allbuild = mf->FindTarget("ALL_BUILD");
  
  // Add XCODE depend helper 
  std::string dir = mf->GetCurrentOutputDirectory();
  this->CurrentXCodeHackMakefile = dir;
  this->CurrentXCodeHackMakefile += "/CMakeScripts";
  cmSystemTools::MakeDirectory(this->CurrentXCodeHackMakefile.c_str());
  this->CurrentXCodeHackMakefile += "/XCODE_DEPEND_HELPER.make";
  cmCustomCommandLine makecommand;
  makecommand.push_back("make");
  makecommand.push_back("-C");
  makecommand.push_back(dir.c_str());
  makecommand.push_back("-f");
  makecommand.push_back(this->CurrentXCodeHackMakefile.c_str());
  if(this->XcodeVersion > 20)
    {
    makecommand.push_back("all.$(CONFIGURATION)");
    }
  cmCustomCommandLines commandLines;
  commandLines.push_back(makecommand);
  mf->AddUtilityCommand("XCODE_DEPEND_HELPER", false,
                        no_working_directory,
                        no_depends,
                        commandLines);

  // Add Re-Run CMake rules
  this->CreateReRunCMakeFile(root);

  // now make the allbuild depend on all the non-utility targets
  // in the project
  for(std::vector<cmLocalGenerator*>::iterator i = gens.begin();
      i != gens.end(); ++i)
    {
    cmLocalGenerator* lg = *i; 
    if(this->IsExcluded(root, *i))
      {
      continue;
      }
    cmTargets& tgts = lg->GetMakefile()->GetTargets();
    for(cmTargets::iterator l = tgts.begin(); l != tgts.end(); l++)
      {
      cmTarget& target = l->second;
      // make all exe, shared libs and modules depend
      // on the XCODE_DEPEND_HELPER target
      if((target.GetType() == cmTarget::EXECUTABLE ||
          target.GetType() == cmTarget::SHARED_LIBRARY ||
          target.GetType() == cmTarget::MODULE_LIBRARY))
        {
        target.AddUtility("XCODE_DEPEND_HELPER");
        }
      if(target.IsInAll())
        {
        allbuild->AddUtility(target.GetName());
        }
      }
    }
}


//----------------------------------------------------------------------------
void cmGlobalXCodeGenerator::CreateReRunCMakeFile(cmLocalGenerator* root)
{
  cmMakefile* mf = root->GetMakefile();
  std::vector<std::string> lfiles = mf->GetListFiles();
  // sort the array
  std::sort(lfiles.begin(), lfiles.end(), std::less<std::string>()); 
  std::vector<std::string>::iterator new_end = 
    std::unique(lfiles.begin(), lfiles.end());
  lfiles.erase(new_end, lfiles.end());
  std::string dir = mf->GetHomeOutputDirectory();
  this->CurrentReRunCMakeMakefile = dir;
  this->CurrentReRunCMakeMakefile += "/CMakeScripts";
  cmSystemTools::MakeDirectory(this->CurrentReRunCMakeMakefile.c_str());
  this->CurrentReRunCMakeMakefile += "/ReRunCMake.make";
  cmGeneratedFileStream makefileStream
    (this->CurrentReRunCMakeMakefile.c_str());
  makefileStream.SetCopyIfDifferent(true);
  makefileStream << "# Generated by CMake, DO NOT EDIT\n";
  makefileStream << cmake::GetCMakeFilesDirectoryPostSlash();
  makefileStream << "cmake.check_cache: ";
  for(std::vector<std::string>::const_iterator i = lfiles.begin();
      i !=  lfiles.end(); ++i)
    {
    makefileStream << "\\\n" << this->ConvertToRelativeForMake(i->c_str());
    }
  std::string cmake = mf->GetRequiredDefinition("CMAKE_COMMAND");
  makefileStream << "\n\t" << this->ConvertToRelativeForMake(cmake.c_str()) 
                 << " -H" << this->ConvertToRelativeForMake(
                   mf->GetHomeDirectory())
                 << " -B" << this->ConvertToRelativeForMake(
                   mf->GetHomeOutputDirectory()) << "\n";
}

//----------------------------------------------------------------------------
void cmGlobalXCodeGenerator::ClearXCodeObjects()
{
  this->TargetDoneSet.clear();
  for(unsigned int i = 0; i < this->XCodeObjects.size(); ++i)
    {
    delete this->XCodeObjects[i];
    }
  this->XCodeObjects.clear();
  this->GroupMap.clear();
  this->GroupNameMap.clear();
  this->TargetGroup.clear();
}

//----------------------------------------------------------------------------
cmXCodeObject* 
cmGlobalXCodeGenerator::CreateObject(cmXCodeObject::PBXType ptype)
{
  cmXCodeObject* obj;
  if(this->XcodeVersion == 15)
    {
    obj = new cmXCodeObject(ptype, cmXCodeObject::OBJECT);
    }
  else
    {
    obj = new cmXCode21Object(ptype, cmXCodeObject::OBJECT);
    }
  this->XCodeObjects.push_back(obj);
  return obj;
}

//----------------------------------------------------------------------------
cmXCodeObject* 
cmGlobalXCodeGenerator::CreateObject(cmXCodeObject::Type type)
{
  cmXCodeObject* obj = new cmXCodeObject(cmXCodeObject::None, type);
  this->XCodeObjects.push_back(obj);
  return obj;
}

//----------------------------------------------------------------------------
cmXCodeObject*
cmGlobalXCodeGenerator::CreateString(const char* s)
{
  cmXCodeObject* obj = this->CreateObject(cmXCodeObject::STRING);
  obj->SetString(s);
  return obj;
}

//----------------------------------------------------------------------------
cmXCodeObject* cmGlobalXCodeGenerator
::CreateObjectReference(cmXCodeObject* ref)
{
  cmXCodeObject* obj = this->CreateObject(cmXCodeObject::OBJECT_REF);
  obj->SetObject(ref);
  return obj;
}

//----------------------------------------------------------------------------
cmXCodeObject* 
cmGlobalXCodeGenerator::CreateXCodeSourceFile(cmLocalGenerator* lg, 
                                              cmSourceFile* sf,
                                              cmTarget& cmtarget)
{
  std::string flags;
  // Add flags from source file properties.
  if(cmtarget.GetProperty("COMPILE_FLAGS"))
    {
    lg->AppendFlags(flags, cmtarget.GetProperty("COMPILE_FLAGS"));
    }
  lg->AppendFlags(flags, sf->GetProperty("COMPILE_FLAGS"));
  cmXCodeObject* fileRef = 
    this->CreateObject(cmXCodeObject::PBXFileReference);

  cmXCodeObject* group = this->GroupMap[sf];
  cmXCodeObject* children = group->GetObject("children");
  children->AddObject(fileRef);
  cmXCodeObject* buildFile = this->CreateObject(cmXCodeObject::PBXBuildFile);
  std::string fname = sf->GetSourceName();
  fname += ".";
  fname += sf->GetSourceExtension();
  std::string comment = fname;
  comment += " in ";
  std::string gname = group->GetObject("name")->GetString();
  comment += gname.substr(1, gname.size()-2);
  buildFile->SetComment(comment.c_str());
  fileRef->SetComment(fname.c_str());
  buildFile->AddAttribute("fileRef", this->CreateObjectReference(fileRef));
  cmXCodeObject* settings = 
    this->CreateObject(cmXCodeObject::ATTRIBUTE_GROUP);
  settings->AddAttribute("COMPILER_FLAGS", this->CreateString(flags.c_str()));
  buildFile->AddAttribute("settings", settings);
  fileRef->AddAttribute("fileEncoding", this->CreateString("4"));
  const char* lang = 
    this->GetLanguageFromExtension(sf->GetSourceExtension().c_str());
  std::string sourcecode = "sourcecode";
  std::string ext = sf->GetSourceExtension();
  ext = cmSystemTools::LowerCase(ext);
  if(ext == "o")
    {
    sourcecode = "compiled.mach-o.objfile";
    }
  else if(ext == "mm")
    {
    sourcecode += ".cpp.objcpp";
    }
  else if(ext == "m")
    {
    sourcecode += ".cpp.objc";
    }
  else if(ext == "plist")
    {
    sourcecode += ".text.plist";
    }
  else if(!lang)
    {
    sourcecode += ext;
    sourcecode += ".";
    sourcecode += ext;
    }
  else if(strcmp(lang, "C") == 0)
    {
    sourcecode += ".c.c";
    }
  else
    {
    sourcecode += ".cpp.cpp";
    }

  fileRef->AddAttribute("lastKnownFileType", 
                        this->CreateString(sourcecode.c_str()));
  std::string path = 
    this->ConvertToRelativeForXCode(sf->GetFullPath().c_str());
  std::string dir;
  std::string file;
  cmSystemTools::SplitProgramPath(sf->GetFullPath().c_str(),
                                  dir, file);
  
  fileRef->AddAttribute("name", this->CreateString(file.c_str()));
  fileRef->AddAttribute("path", this->CreateString(path.c_str()));
  if(this->XcodeVersion == 15)
    {
    fileRef->AddAttribute("refType", this->CreateString("4"));
    }
  if(path.size() > 1 && path[0] == '.' && path[1] == '.')
    {
    fileRef->AddAttribute("sourceTree", this->CreateString("<group>"));
    }
  else
    {
    fileRef->AddAttribute("sourceTree", this->CreateString("<absolute>"));
    }
  return buildFile;
}

//----------------------------------------------------------------------------
bool cmGlobalXCodeGenerator::SpecialTargetEmitted(std::string const& tname)
{
  if(tname == "ALL_BUILD" || tname == "XCODE_DEPEND_HELPER" ||
     tname == "install" || tname == "RUN_TESTS" )
    {
    if(this->TargetDoneSet.find(tname) != this->TargetDoneSet.end())
      {
      return true;
      }
    this->TargetDoneSet.insert(tname);
    return false;
    }
  return false;
}


void cmGlobalXCodeGenerator::SetCurrentLocalGenerator(cmLocalGenerator* gen)
{
  this->CurrentLocalGenerator = gen;
  this->CurrentMakefile = gen->GetMakefile();
  std::string outdir =
    cmSystemTools::CollapseFullPath(this->CurrentMakefile->
                                    GetCurrentOutputDirectory());
  cmSystemTools::SplitPath(outdir.c_str(), 
                           this->CurrentOutputDirectoryComponents);

  // Select the current set of configuration types.
  this->CurrentConfigurationTypes.clear();
  if(this->XcodeVersion > 20)
    {
    if(const char* types =
       this->CurrentMakefile->GetDefinition("CMAKE_CONFIGURATION_TYPES"))
      {
      cmSystemTools::ExpandListArgument(types, 
                                        this->CurrentConfigurationTypes);
      }
    }
  if(this->CurrentConfigurationTypes.empty())
    {
    if(const char* buildType =
       this->CurrentMakefile->GetDefinition("CMAKE_BUILD_TYPE"))
      {
      this->CurrentConfigurationTypes.push_back(buildType);
      }
    else
      {
      this->CurrentConfigurationTypes.push_back("");
      }
    }
}


//----------------------------------------------------------------------------
void 
cmGlobalXCodeGenerator::CreateXCodeTargets(cmLocalGenerator* gen,
                                           std::vector<cmXCodeObject*>& 
                                           targets)
{
  this->SetCurrentLocalGenerator(gen);
  cmTargets &tgts = this->CurrentMakefile->GetTargets();
  for(cmTargets::iterator l = tgts.begin(); l != tgts.end(); l++)
    { 
    cmTarget& cmtarget = l->second;
    // make sure ALL_BUILD, INSTALL, etc are only done once
    if(this->SpecialTargetEmitted(l->first.c_str()))
      {
      continue;
      }
    if(cmtarget.GetType() == cmTarget::UTILITY ||
       cmtarget.GetType() == cmTarget::GLOBAL_TARGET ||
       cmtarget.GetType() == cmTarget::INSTALL_FILES ||
       cmtarget.GetType() == cmTarget::INSTALL_PROGRAMS)
      {
      if(cmtarget.GetType() == cmTarget::UTILITY ||
         cmtarget.GetType() == cmTarget::GLOBAL_TARGET)
        {
        targets.push_back(this->CreateUtilityTarget(cmtarget));
        }
      continue;
      }

    // create source build phase
    cmXCodeObject* sourceBuildPhase = 
      this->CreateObject(cmXCodeObject::PBXSourcesBuildPhase);
    sourceBuildPhase->AddAttribute("buildActionMask", 
                                   this->CreateString("2147483647"));
    sourceBuildPhase->SetComment("Sources");
    cmXCodeObject* buildFiles = 
      this->CreateObject(cmXCodeObject::OBJECT_LIST);
    sourceBuildPhase->AddAttribute("files", buildFiles);
    sourceBuildPhase->AddAttribute("runOnlyForDeploymentPostprocessing", 
                                   this->CreateString("0"));
    std::vector<cmSourceFile*> &classes = l->second.GetSourceFiles();
    // add all the sources
    std::vector<cmXCodeObject*> externalObjFiles;
    std::vector<cmXCodeObject*> headerFiles;
    std::vector<cmXCodeObject*> specialBundleFiles;
    for(std::vector<cmSourceFile*>::iterator i = classes.begin(); 
        i != classes.end(); ++i)
      {
      cmXCodeObject* xsf =
        this->CreateXCodeSourceFile(this->CurrentLocalGenerator, 
                                    *i, cmtarget);
      cmXCodeObject* fr = xsf->GetObject("fileRef");
      cmXCodeObject* filetype = 
        fr->GetObject()->GetObject("lastKnownFileType");
      if(strcmp(filetype->GetString(), "\"compiled.mach-o.objfile\"") == 0)
        {
        externalObjFiles.push_back(xsf);
        }
      else if((*i)->GetPropertyAsBool("HEADER_FILE_ONLY"))
        {
        headerFiles.push_back(xsf);
        }
      else
        {
        buildFiles->AddObject(xsf);
        }
      }
    // create header build phase
    cmXCodeObject* headerBuildPhase = 
      this->CreateObject(cmXCodeObject::PBXHeadersBuildPhase);
    headerBuildPhase->SetComment("Headers");
    headerBuildPhase->AddAttribute("buildActionMask",
                                   this->CreateString("2147483647"));
    buildFiles = this->CreateObject(cmXCodeObject::OBJECT_LIST);
    for(std::vector<cmXCodeObject*>::iterator i = headerFiles.begin();
        i != headerFiles.end(); ++i)
      {
      buildFiles->AddObject(*i);
      }
    headerBuildPhase->AddAttribute("files", buildFiles);
    headerBuildPhase->AddAttribute("runOnlyForDeploymentPostprocessing",
                                   this->CreateString("0"));
    
    // create framework build phase
    cmXCodeObject* frameworkBuildPhase =
      this->CreateObject(cmXCodeObject::PBXFrameworksBuildPhase);
    frameworkBuildPhase->SetComment("Frameworks");
    frameworkBuildPhase->AddAttribute("buildActionMask",
                                      this->CreateString("2147483647"));
    buildFiles = this->CreateObject(cmXCodeObject::OBJECT_LIST);
    frameworkBuildPhase->AddAttribute("files", buildFiles);
    for(std::vector<cmXCodeObject*>::iterator i =  externalObjFiles.begin();
        i != externalObjFiles.end(); ++i)
      {
      buildFiles->AddObject(*i);
      }
    frameworkBuildPhase->AddAttribute("runOnlyForDeploymentPostprocessing", 
                                      this->CreateString("0"));
    cmXCodeObject* buildPhases = 
      this->CreateObject(cmXCodeObject::OBJECT_LIST);
    this->CreateCustomCommands(buildPhases, sourceBuildPhase,
                               headerBuildPhase, frameworkBuildPhase,
                               cmtarget);
    targets.push_back(this->CreateXCodeTarget(l->second, buildPhases));

    // copy files build phase
    typedef std::map<cmStdString, std::vector<cmSourceFile*> >
      mapOfVectorOfSourceFiles;
    mapOfVectorOfSourceFiles bundleFiles;
    for(std::vector<cmSourceFile*>::iterator i = classes.begin(); 
        i != classes.end(); ++i)
      {
      const char* resLoc = (*i)->GetProperty("MACOSX_PACKAGE_LOCATION");
      if ( !resLoc )
        {
        continue;
        }
      bundleFiles[resLoc].push_back(*i);
      }
    mapOfVectorOfSourceFiles::iterator mit;
    for ( mit = bundleFiles.begin(); mit != bundleFiles.end(); ++ mit )
      {
      cmXCodeObject* copyFilesBuildPhase;
      if ( mit->first == "Resources" )
        {
        copyFilesBuildPhase
          = this->CreateObject(cmXCodeObject::PBXResourcesBuildPhase);
        }
      else
        {
        copyFilesBuildPhase
          = this->CreateObject(cmXCodeObject::PBXCopyFilesBuildPhase);
        copyFilesBuildPhase->SetComment("Copy files");
        copyFilesBuildPhase->AddAttribute("buildActionMask",
          this->CreateString("2147483647"));
        copyFilesBuildPhase->AddAttribute("dstSubfolderSpec",
          this->CreateString("6"));
        cmOStringStream ostr;
        if ( mit->first != "MacOS" )
          {
          ostr << "../" << mit->first.c_str();
          }
        copyFilesBuildPhase->AddAttribute("dstPath",
          this->CreateString(ostr.str().c_str()));
        }
      copyFilesBuildPhase->AddAttribute("runOnlyForDeploymentPostprocessing",
        this->CreateString("0"));
      buildPhases->AddObject(copyFilesBuildPhase);
      buildFiles = this->CreateObject(cmXCodeObject::OBJECT_LIST);
      copyFilesBuildPhase->AddAttribute("files", buildFiles);
      std::vector<cmSourceFile*>::iterator sfIt;
      for ( sfIt = mit->second.begin(); sfIt != mit->second.end(); ++ sfIt )
        {
        cmXCodeObject* xsf =
          this->CreateXCodeSourceFile(this->CurrentLocalGenerator, 
                                      *sfIt, cmtarget);
        buildFiles->AddObject(xsf);
        }
      }
    }
}

//----------------------------------------------------------------------------
cmXCodeObject*
cmGlobalXCodeGenerator::CreateBuildPhase(const char* name, 
                                         const char* name2,
                                         cmTarget& cmtarget,
                                         const std::vector<cmCustomCommand>&
                                         commands)
{
  if(commands.size() == 0 && strcmp(name, "CMake ReRun") != 0)
    {
    return 0;
    }
  cmXCodeObject* buildPhase = 
    this->CreateObject(cmXCodeObject::PBXShellScriptBuildPhase);
  buildPhase->AddAttribute("buildActionMask",
                           this->CreateString("2147483647"));
  cmXCodeObject* buildFiles = this->CreateObject(cmXCodeObject::OBJECT_LIST);
  buildPhase->AddAttribute("files", buildFiles);
  buildPhase->AddAttribute("name", 
                           this->CreateString(name));
  buildPhase->AddAttribute("runOnlyForDeploymentPostprocessing", 
                           this->CreateString("0"));
  buildPhase->AddAttribute("shellPath",
                           this->CreateString("/bin/sh"));
  this->AddCommandsToBuildPhase(buildPhase, cmtarget, commands,
                                name2);
  return buildPhase;
}

//----------------------------------------------------------------------------
void cmGlobalXCodeGenerator::CreateCustomCommands(cmXCodeObject* buildPhases,
                                                  cmXCodeObject*
                                                  sourceBuildPhase,
                                                  cmXCodeObject*
                                                  headerBuildPhase,
                                                  cmXCodeObject*
                                                  frameworkBuildPhase,
                                                  cmTarget& cmtarget)
{
  std::vector<cmCustomCommand> const & prebuild 
    = cmtarget.GetPreBuildCommands();
  std::vector<cmCustomCommand> const & prelink 
    = cmtarget.GetPreLinkCommands();
  std::vector<cmCustomCommand> const & postbuild 
    = cmtarget.GetPostBuildCommands();
  std::vector<cmSourceFile*> &classes = cmtarget.GetSourceFiles();
  // add all the sources
  std::vector<cmCustomCommand> commands;
  for(std::vector<cmSourceFile*>::iterator i = classes.begin(); 
      i != classes.end(); ++i)
    {
    if((*i)->GetCustomCommand())
      {
      commands.push_back(*(*i)->GetCustomCommand());
      }
    }
  std::vector<cmCustomCommand> reruncom;
  cmXCodeObject* cmakeReRunPhase =  
    this->CreateBuildPhase("CMake ReRun", "cmakeReRunPhase",
                                                           cmtarget, reruncom);
  buildPhases->AddObject(cmakeReRunPhase);
  // create prebuild phase
  cmXCodeObject* cmakeRulesBuildPhase =
    this->CreateBuildPhase("CMake Rules",
                           "cmakeRulesBuildPhase",
                           cmtarget, commands);
  // create prebuild phase
  cmXCodeObject* preBuildPhase = 
    this->CreateBuildPhase("CMake PreBuild Rules", "preBuildCommands",
                                                        cmtarget, prebuild);
  // create prebuild phase
  cmXCodeObject* preLinkPhase = 
    this->CreateBuildPhase("CMake PreLink Rules", "preLinkCommands",
                                                       cmtarget, prelink);
  // create prebuild phase
  cmXCodeObject* postBuildPhase = 
    this->CreateBuildPhase("CMake PostBuild Rules", "postBuildPhase",
                           cmtarget, postbuild);
  // the order here is the order they will be built in
  if(preBuildPhase)
    {
    buildPhases->AddObject(preBuildPhase);
    }
  if(cmakeRulesBuildPhase)
    {
    buildPhases->AddObject(cmakeRulesBuildPhase);
    }
  if(sourceBuildPhase)
    {
    buildPhases->AddObject(sourceBuildPhase);
    }
  if(headerBuildPhase)
    {
    buildPhases->AddObject(headerBuildPhase);
    }
  if(preLinkPhase)
    {
    buildPhases->AddObject(preLinkPhase);
    }
  if(frameworkBuildPhase)
    {
    buildPhases->AddObject(frameworkBuildPhase);
    }
  if(postBuildPhase)
    {
    buildPhases->AddObject(postBuildPhase);
    }
}

//----------------------------------------------------------------------------
std::string cmGlobalXCodeGenerator::ExtractFlag(const char* flag,
                                                std::string& flags)
{
  std::string retFlag;
  std::string::size_type pos = flags.find(flag);
  if(pos != flags.npos)
    {
    while(pos < flags.size() && flags[pos] != ' ')
      {
      retFlag += flags[pos];
      flags[pos] = ' ';
      pos++;
      }
    }
  return retFlag;
}
//----------------------------------------------------------------------------
void 
cmGlobalXCodeGenerator::AddCommandsToBuildPhase(cmXCodeObject* buildphase,
                                                cmTarget& target,
                                                std::vector<cmCustomCommand> 
                                                const & commands,
                                                const char* name)
{
  if(strcmp(name, "cmakeReRunPhase") == 0)
    {
    std::string cdir = this->CurrentMakefile->GetHomeOutputDirectory();
    cdir = this->ConvertToRelativeForMake(cdir.c_str());
    std::string makecmd = "make -C ";
    makecmd += cdir;
    makecmd += " -f ";
    makecmd += 
      this->ConvertToRelativeForMake(this->CurrentReRunCMakeMakefile.c_str());
    cmSystemTools::ReplaceString(makecmd, "\\ ", "\\\\ ");
    buildphase->AddAttribute("shellScript",
                             this->CreateString(makecmd.c_str()));
    return;
    }

  std::map<cmStdString, cmStdString> multipleOutputPairs;
  
  std::string dir = this->CurrentMakefile->GetCurrentOutputDirectory();
  dir += "/CMakeScripts";
  cmSystemTools::MakeDirectory(dir.c_str());
  std::string makefile = dir;
  makefile += "/";
  makefile += target.GetName();
  makefile += "_";
  makefile += name;
  makefile += ".make";
  cmGeneratedFileStream makefileStream(makefile.c_str());
  if(!makefileStream)
    {
    return;
    }
  makefileStream.SetCopyIfDifferent(true);
  makefileStream << "# Generated by CMake, DO NOT EDIT\n";
  makefileStream << "# Custom rules for " << target.GetName() << "\n";
  
  // have all depend on all outputs
  makefileStream << "all: ";
  std::map<const cmCustomCommand*, cmStdString> tname;
  int count = 0;
  for(std::vector<cmCustomCommand>::const_iterator i = commands.begin();
      i != commands.end(); ++i)
    {
    cmCustomCommand const& cc = *i; 
    if(!cc.GetCommandLines().empty())
      {
      const std::vector<std::string>& outputs = cc.GetOutputs();
      if(!outputs.empty())
        {
        for(std::vector<std::string>::const_iterator o = outputs.begin();
            o != outputs.end(); ++o)
          {
          makefileStream
            << "\\\n\t" << this->ConvertToRelativeForMake(o->c_str());
          }
        }
      else
        {
        cmOStringStream str;
        str << "_buildpart_" << count++ ;
        tname[&cc] = std::string(target.GetName()) + str.str(); 
        makefileStream << "\\\n\t" << tname[&cc];
        }
      }
    }
  makefileStream << "\n\n";
  for(std::vector<cmCustomCommand>::const_iterator i = commands.begin();
      i != commands.end(); ++i)
    {
    cmCustomCommand const& cc = *i; 
    if(!cc.GetCommandLines().empty())
      {
      bool escapeOldStyle = cc.GetEscapeOldStyle();
      bool escapeAllowMakeVars = cc.GetEscapeAllowMakeVars();
      makefileStream << "\n#" << "Custom command rule: ";
      if(cc.GetComment())
        {
        makefileStream << cc.GetComment();
        }
      makefileStream << "\n";
      const std::vector<std::string>& outputs = cc.GetOutputs();
      if(!outputs.empty())
        {
        // There is at least one output.  If there is more than one treat the
        // first as the primary output and make the rest depend on it.
        std::vector<std::string>::const_iterator o = outputs.begin();
        std::string primary_output =
          this->ConvertToRelativeForMake(o->c_str());
        for(++o; o != outputs.end(); ++o)
          {
          std::string current_output =
            this->ConvertToRelativeForMake(o->c_str());
          makefileStream << current_output << ": "
                         << primary_output << "\n";
          multipleOutputPairs[current_output] = primary_output;
          }

        // Start the rule for the primary output.
        makefileStream << primary_output << ": ";
        }
      else
        {
        // There are no outputs.  Use the generated force rule name.
        makefileStream << tname[&cc] << ": ";
        }
      for(std::vector<std::string>::const_iterator d = 
            cc.GetDepends().begin();
          d != cc.GetDepends().end(); ++d)
        {
        if(!this->FindTarget(this->CurrentProject.c_str(),
                             d->c_str()))
          {
          // if the depend is not a target but
          // is a full path then use it, if not then
          // just skip it
          if(cmSystemTools::FileIsFullPath(d->c_str()))
            {
            makefileStream << "\\\n" << this
              ->ConvertToRelativeForMake(d->c_str());
            }
          }
        else
          {
          // if the depend is a target then make 
          // the target with the source that is a custom command
          // depend on the that target via a AddUtility call
          target.AddUtility(d->c_str());
          }
        }
      makefileStream << "\n";

      // Add each command line to the set of commands.
      for(cmCustomCommandLines::const_iterator cl = 
            cc.GetCommandLines().begin();
          cl != cc.GetCommandLines().end(); ++cl)
        {
        // Build the command line in a single string.
        const cmCustomCommandLine& commandLine = *cl;
        std::string cmd2 = commandLine[0];
        cmSystemTools::ReplaceString(cmd2, "/./", "/");
        cmd2 = this->ConvertToRelativeForMake(cmd2.c_str());
        std::string cmd;
        if(cc.GetWorkingDirectory())
          {
          cmd += "cd ";
          cmd += this->ConvertToRelativeForMake(cc.GetWorkingDirectory());
          cmd += " && ";
          }
        cmd += cmd2;
        for(unsigned int j=1; j < commandLine.size(); ++j)
          {
          cmd += " ";
          if(escapeOldStyle)
            {
            cmd += (this->CurrentLocalGenerator
                    ->EscapeForShellOldStyle(commandLine[j].c_str()));
            }
          else
            {
            cmd += (this->CurrentLocalGenerator->
                    EscapeForShell(commandLine[j].c_str(),
                                   escapeAllowMakeVars));
            }
          }
        makefileStream << "\t" << cmd.c_str() << "\n";
        }
      }
    }

  // Add a rule to deal with multiple outputs of custom commands.
  if(!multipleOutputPairs.empty())
    {
    makefileStream <<
      "\n"
      "cmake_check_multiple_outputs:\n";
    for(std::map<cmStdString, cmStdString>::const_iterator o =
          multipleOutputPairs.begin(); o != multipleOutputPairs.end(); ++o)
      {
      makefileStream << "\t@if [ ! -f "
                     << o->first << " ]; then rm -f "
                     << o->second << "; fi\n";
      }
    }

  std::string cdir = this->CurrentMakefile->GetCurrentOutputDirectory();
  cdir = this->ConvertToRelativeForXCode(cdir.c_str());
  std::string makecmd = "make -C ";
  makecmd += cdir;
  makecmd += " -f ";
  makecmd += this->ConvertToRelativeForMake(makefile.c_str());
  if(!multipleOutputPairs.empty())
    {
    makecmd += " cmake_check_multiple_outputs";
    }
  makecmd += " all";
  cmSystemTools::ReplaceString(makecmd, "\\ ", "\\\\ ");
  buildphase->AddAttribute("shellScript", 
                           this->CreateString(makecmd.c_str()));
}


//----------------------------------------------------------------------------
void cmGlobalXCodeGenerator::CreateBuildSettings(cmTarget& target,
                                                 cmXCodeObject* buildSettings,
                                                 std::string& fileType,
                                                 std::string& productType,
                                                 std::string& productName,
                                                 const char* configName)
{
  this->ConfigureOutputPaths();
  std::string flags;
  std::string defFlags;
  bool shared = ((target.GetType() == cmTarget::SHARED_LIBRARY) ||
                 (target.GetType() == cmTarget::MODULE_LIBRARY));
  if(shared)
    {
    defFlags += "-D";
    if(const char* custom_export_name = target.GetProperty("DEFINE_SYMBOL"))
      {
      defFlags += custom_export_name;
      }
    else
      {
      std::string in = target.GetName();
      in += "_EXPORTS";
      defFlags += cmSystemTools::MakeCindentifier(in.c_str());
      }
    }
  const char* lang = target.GetLinkerLanguage(this);
  std::string cflags;
  if(lang)
    {
    // for c++ projects get the c flags as well
    if(strcmp(lang, "CXX") == 0)
      {
      this->CurrentLocalGenerator->AddLanguageFlags(cflags, "C", configName);
      this->CurrentLocalGenerator->AddSharedFlags(cflags, lang, shared);
      }

    // Add language-specific flags.
    this->CurrentLocalGenerator->AddLanguageFlags(flags, lang, configName);
    
    // Add shared-library flags if needed.
    this->CurrentLocalGenerator->AddSharedFlags(flags, lang, shared);
    }

  // Add define flags
  this->CurrentLocalGenerator->
    AppendFlags(defFlags,
                                       this->CurrentMakefile->GetDefineFlags());
  cmSystemTools::ReplaceString(defFlags, "\"", "\\\"");
  cmSystemTools::ReplaceString(flags, "\"", "\\\"");
  cmSystemTools::ReplaceString(cflags, "\"", "\\\"");
  if(this->XcodeVersion > 15)
    {
    buildSettings->AddAttribute
      ("GCC_PREPROCESSOR_DEFINITIONS", 
                   this->CreateString("CMAKE_INTDIR=\\\\\\\"$(CONFIGURATION)\\\\\\\""));
    }
  std::string extraLinkOptions;
  if(target.GetType() == cmTarget::EXECUTABLE)
    {
    extraLinkOptions = 
      this->CurrentMakefile->GetRequiredDefinition("CMAKE_EXE_LINKER_FLAGS");
    }
  if(target.GetType() == cmTarget::SHARED_LIBRARY)
    {
    extraLinkOptions = this->CurrentMakefile->
      GetRequiredDefinition("CMAKE_SHARED_LINKER_FLAGS");
    }
  if(target.GetType() == cmTarget::MODULE_LIBRARY)
    {
    extraLinkOptions = this->CurrentMakefile->
      GetRequiredDefinition("CMAKE_MODULE_LINKER_FLAGS");
    }
  
  const char* targetLinkFlags = target.GetProperty("LINK_FLAGS");
  if(targetLinkFlags)
    {
    extraLinkOptions += " ";
    extraLinkOptions += targetLinkFlags;
    }

  // The product name is the full name of the target for this configuration.
  productName = target.GetFullName(configName);

  // Get the product name components.
  std::string pnprefix;
  std::string pnbase;
  std::string pnsuffix;
  target.GetFullName(pnprefix, pnbase, pnsuffix, configName);

  // Store the product name for all target types.
  buildSettings->AddAttribute("PRODUCT_NAME",
                              this->CreateString(pnbase.c_str()));

  // Set attributes to specify the proper name for the target.
  if(target.GetType() == cmTarget::STATIC_LIBRARY ||
     target.GetType() == cmTarget::SHARED_LIBRARY ||
     target.GetType() == cmTarget::MODULE_LIBRARY ||
     target.GetType() == cmTarget::EXECUTABLE)
    {
    std::string pndir = target.GetDirectory();
    buildSettings->AddAttribute("SYMROOT", 
                                this->CreateString(pndir.c_str()));
    buildSettings->AddAttribute("EXECUTABLE_PREFIX", 
                                this->CreateString(pnprefix.c_str()));
    buildSettings->AddAttribute("EXECUTABLE_SUFFIX", 
                                this->CreateString(pnsuffix.c_str()));
    }

  // Handle settings for each target type.
  switch(target.GetType())
    {
    case cmTarget::STATIC_LIBRARY:
    {
    fileType = "archive.ar";
    productType = "com.apple.product-type.library.static";

    buildSettings->AddAttribute("LIBRARY_STYLE", 
                                this->CreateString("STATIC"));
    break;
    }
    
    case cmTarget::MODULE_LIBRARY:
    {
    buildSettings->AddAttribute("LIBRARY_STYLE", 
                                this->CreateString("BUNDLE"));
    if(this->XcodeVersion >= 22)
      {
      fileType = "compiled.mach-o.executable";
      productType = "com.apple.product-type.tool";

      buildSettings->AddAttribute("MACH_O_TYPE", 
                                  this->CreateString("mh_bundle"));
      buildSettings->AddAttribute("GCC_DYNAMIC_NO_PIC", 
                                  this->CreateString("NO"));
      // Add the flags to create an executable.
      std::string createFlags =
        this->LookupFlags("CMAKE_", lang, "_LINK_FLAGS", "");
      if(!createFlags.empty())
        {
        extraLinkOptions += " ";
        extraLinkOptions += createFlags;
        }
      }
    else
      {
      fileType = "compiled.mach-o.dylib";
      productType = "com.apple.product-type.library.dynamic";

      // Add the flags to create a module.
      std::string createFlags =
        this->LookupFlags("CMAKE_SHARED_MODULE_CREATE_", lang, "_FLAGS",
                          "-bundle");
      if(!createFlags.empty())
        {
        extraLinkOptions += " ";
        extraLinkOptions += createFlags;
        }
      }
    break;
    }
    case cmTarget::SHARED_LIBRARY:
    {
    fileType = "compiled.mach-o.dylib";
    productType = "com.apple.product-type.library.dynamic";

    buildSettings->AddAttribute("LIBRARY_STYLE", 
                                this->CreateString("DYNAMIC"));
    buildSettings->AddAttribute("DYLIB_COMPATIBILITY_VERSION", 
                                this->CreateString("1"));
    buildSettings->AddAttribute("DYLIB_CURRENT_VERSION", 
                                this->CreateString("1"));

    // Add the flags to create a shared library.
    std::string createFlags =
      this->LookupFlags("CMAKE_SHARED_LIBRARY_CREATE_", lang, "_FLAGS",
                        "-dynamiclib");
    if(!createFlags.empty())
      {
      extraLinkOptions += " ";
      extraLinkOptions += createFlags;
      }
    break;
    }
    case cmTarget::EXECUTABLE:
    {
    fileType = "compiled.mach-o.executable";

    // Add the flags to create an executable.
    std::string createFlags =
      this->LookupFlags("CMAKE_", lang, "_LINK_FLAGS", "");
    if(!createFlags.empty())
      {
      extraLinkOptions += " ";
      extraLinkOptions += createFlags;
      }

    // Handle bundles and normal executables separately.
    if(target.GetPropertyAsBool("MACOSX_BUNDLE"))
      {
      productType = "com.apple.product-type.application";
      std::string f1 =
        this->CurrentMakefile->GetModulesFile("MacOSXBundleInfo.plist.in");
      if ( f1.size() == 0 )
        {
        cmSystemTools::Error("could not find Mac OSX bundle template file.");
        }
      std::string f2 = this->CurrentMakefile->GetCurrentOutputDirectory();
      f2 += "/Info.plist";
      this->CurrentMakefile->ConfigureFile(f1.c_str(), f2.c_str(),
                                       false, false, false);
      std::string path = 
        this->ConvertToRelativeForXCode(f2.c_str());
      buildSettings->AddAttribute("INFOPLIST_FILE", 
                                  this->CreateString(path.c_str()));

      }
    else
      {
      productType = "com.apple.product-type.tool";
      }
    }
    break;
    default:
      break;
    }
  if(this->XcodeVersion >= 22)
    {
    buildSettings->AddAttribute("PREBINDING", 
                                this->CreateString("NO"));
    }
  std::string dirs;
  std::vector<std::string> includes;
  this->CurrentLocalGenerator->GetIncludeDirectories(includes);
  std::vector<std::string>::iterator i = includes.begin();
  std::string fdirs;
  std::set<cmStdString> emitted;
  for(;i != includes.end(); ++i)
    {
    if(cmSystemTools::IsPathToFramework(i->c_str()))
      {
      std::string frameworkDir = *i;
      frameworkDir += "/../";
      frameworkDir = cmSystemTools::CollapseFullPath(frameworkDir.c_str());
      if(emitted.insert(frameworkDir).second)
        {
        fdirs += this->XCodeEscapePath(frameworkDir.c_str());
        fdirs += " ";
        }
      }
    else
      {
      std::string incpath = 
        this->XCodeEscapePath(i->c_str());
      dirs += incpath + " ";
      }
    }
  std::vector<std::string>& frameworks = target.GetFrameworks();
  if(frameworks.size())
    {
    for(std::vector<std::string>::iterator fmIt = frameworks.begin();
        fmIt != frameworks.end(); ++fmIt)
      {
      if(emitted.insert(*fmIt).second)
        {
        fdirs += this->XCodeEscapePath(fmIt->c_str());
        fdirs += " ";
        }
      }
    }
  if(fdirs.size())
    {
    buildSettings->AddAttribute("FRAMEWORK_SEARCH_PATHS", 
                                this->CreateString(fdirs.c_str()));
    }
  if(dirs.size())
    {
    buildSettings->AddAttribute("HEADER_SEARCH_PATHS", 
                                this->CreateString(dirs.c_str()));
    }
  std::string oflagc = this->ExtractFlag("-O", cflags);
  char optLevel[2];
  optLevel[0] = '0';
  optLevel[1] = 0;
  if(oflagc.size() == 3)
    {
    optLevel[0] = oflagc[2];
    }
  if(oflagc.size() == 2)
    {
    optLevel[0] = '1';
    }
  std::string oflag = this->ExtractFlag("-O", flags);
  if(oflag.size() == 3)
    {
    optLevel[0] = oflag[2];
    }
  if(oflag.size() == 2)
    {
    optLevel[0] = '1';
    }
  std::string gflagc = this->ExtractFlag("-g", cflags);
  // put back gdwarf-2 if used since there is no way
  // to represent it in the gui, but we still want debug yes
  if(gflagc == "-gdwarf-2")
    {
    cflags += " ";
    cflags += gflagc;
    }
  std::string gflag = this->ExtractFlag("-g", flags);
  if(gflag == "-gdwarf-2")
    {
    flags += " ";
    flags += gflag;
    }
  const char* debugStr = "YES";
  if(gflagc.size() ==0  && gflag.size() == 0)
    {
    debugStr = "NO";
    }    
  buildSettings->AddAttribute("GCC_GENERATE_DEBUGGING_SYMBOLS",
                              this->CreateString(debugStr));
  buildSettings->AddAttribute("GCC_OPTIMIZATION_LEVEL", 
                              this->CreateString(optLevel));
  buildSettings->AddAttribute("OPTIMIZATION_CFLAGS", 
                              this->CreateString(oflagc.c_str()));
  buildSettings->AddAttribute("GCC_SYMBOLS_PRIVATE_EXTERN",
                              this->CreateString("NO"));
  buildSettings->AddAttribute("GCC_INLINES_ARE_PRIVATE_EXTERN",
                              this->CreateString("NO"));
  if(lang && strcmp(lang, "CXX") == 0)
    {
    flags += " ";
    flags += defFlags;
    buildSettings->AddAttribute("OTHER_CPLUSPLUSFLAGS", 
                                this->CreateString(flags.c_str()));
    cflags += " ";
    cflags += defFlags;
    buildSettings->AddAttribute("OTHER_CFLAGS", 
                                this->CreateString(cflags.c_str()));

    }
  else
    {
    flags += " ";
    flags += defFlags;
    buildSettings->AddAttribute("OTHER_CFLAGS", 
                                this->CreateString(flags.c_str()));
    }

  // Create the INSTALL_PATH attribute.
  std::string install_name_dir;
  if(target.GetType() == cmTarget::SHARED_LIBRARY)
    {
    // Get the install_name directory for the build tree.
    install_name_dir = target.GetInstallNameDirForBuildTree(configName);

    if(install_name_dir.empty())
      {
      // Xcode will not pass the -install_name option at all if INSTALL_PATH
      // is not given or is empty.  We must explicitly put the flag in the
      // link flags to create an install_name with just the library soname.
      extraLinkOptions += " -install_name ";
      extraLinkOptions += productName;
      }
    else
      {
      // Convert to a path for the native build tool.
      cmSystemTools::ConvertToUnixSlashes(install_name_dir);
      // do not escape spaces on this since it is only a single path
      }
    }
  buildSettings->AddAttribute("INSTALL_PATH",
                              this->CreateString(install_name_dir.c_str()));

  buildSettings->AddAttribute("OTHER_LDFLAGS", 
                              this->CreateString(extraLinkOptions.c_str()));
  buildSettings->AddAttribute("OTHER_REZFLAGS", 
                              this->CreateString(""));
  buildSettings->AddAttribute("SECTORDER_FLAGS",
                              this->CreateString(""));
  buildSettings->AddAttribute("USE_HEADERMAP",
                              this->CreateString("NO"));
  buildSettings->AddAttribute("WARNING_CFLAGS",
                              this->CreateString(
                                "-Wmost -Wno-four-char-constants"
                                " -Wno-unknown-pragmas"));
}

//----------------------------------------------------------------------------
cmXCodeObject* 
cmGlobalXCodeGenerator::CreateUtilityTarget(cmTarget& cmtarget)
{
  cmXCodeObject* shellBuildPhase =
    this->CreateObject(cmXCodeObject::PBXShellScriptBuildPhase);
  shellBuildPhase->AddAttribute("buildActionMask", 
                                this->CreateString("2147483647"));
  cmXCodeObject* buildFiles = this->CreateObject(cmXCodeObject::OBJECT_LIST);
  shellBuildPhase->AddAttribute("files", buildFiles);
  cmXCodeObject* inputPaths = this->CreateObject(cmXCodeObject::OBJECT_LIST);
  shellBuildPhase->AddAttribute("inputPaths", inputPaths);
  cmXCodeObject* outputPaths = this->CreateObject(cmXCodeObject::OBJECT_LIST);
  shellBuildPhase->AddAttribute("outputPaths", outputPaths);
  shellBuildPhase->AddAttribute("runOnlyForDeploymentPostprocessing",
                                this->CreateString("0"));
  shellBuildPhase->AddAttribute("shellPath",
                                this->CreateString("/bin/sh"));
  shellBuildPhase->AddAttribute("shellScript",
                                this->CreateString(
                                  "# shell script goes here\nexit 0"));
  cmXCodeObject* target = 
    this->CreateObject(cmXCodeObject::PBXAggregateTarget);
  target->SetComment(cmtarget.GetName());
  cmXCodeObject* buildPhases = 
    this->CreateObject(cmXCodeObject::OBJECT_LIST);
  this->CreateCustomCommands(buildPhases, 0, 0, 0, cmtarget);
  target->AddAttribute("buildPhases", buildPhases);
  cmXCodeObject* buildSettings =
    this->CreateObject(cmXCodeObject::ATTRIBUTE_GROUP);
  std::string fileTypeString;
  std::string productTypeString;
  std::string productName;
  const char* globalConfig = 0;
  if(this->XcodeVersion > 20)
    {
    this->AddConfigurations(target, cmtarget);
    }
  else
    {
    globalConfig = this->CurrentMakefile->GetDefinition("CMAKE_BUILD_TYPE");  
    }
  this->CreateBuildSettings(cmtarget, 
                            buildSettings, fileTypeString, 
                            productTypeString, productName, globalConfig);
  target->AddAttribute("buildSettings", buildSettings);
  cmXCodeObject* dependencies = 
    this->CreateObject(cmXCodeObject::OBJECT_LIST);
  target->AddAttribute("dependencies", dependencies);
  target->AddAttribute("name", this->CreateString(productName.c_str()));
  target->AddAttribute("productName",this->CreateString(productName.c_str()));
  target->SetTarget(&cmtarget);
  return target;
}

//----------------------------------------------------------------------------
void cmGlobalXCodeGenerator::AddConfigurations(cmXCodeObject* target,
                                               cmTarget& cmtarget)
{
  std::string configTypes = 
    this->CurrentMakefile->GetRequiredDefinition("CMAKE_CONFIGURATION_TYPES");
  std::vector<std::string> configVectorIn;
  std::vector<std::string> configVector;
  configVectorIn.push_back(configTypes);
  cmSystemTools::ExpandList(configVectorIn, configVector);
  cmXCodeObject* configlist = 
    this->CreateObject(cmXCodeObject::XCConfigurationList);
  cmXCodeObject* buildConfigurations =
    this->CreateObject(cmXCodeObject::OBJECT_LIST);
  configlist->AddAttribute("buildConfigurations", buildConfigurations);
  std::string comment = "Build configuration list for ";
  comment += cmXCodeObject::PBXTypeNames[target->GetIsA()];
  comment += " \"";
  comment += cmtarget.GetName();
  comment += "\"";
  configlist->SetComment(comment.c_str());
  target->AddAttribute("buildConfigurationList", 
                       this->CreateObjectReference(configlist));
  for(unsigned int i = 0; i < configVector.size(); ++i)
    {
    cmXCodeObject* config = 
      this->CreateObject(cmXCodeObject::XCBuildConfiguration);
    buildConfigurations->AddObject(config);
    cmXCodeObject* buildSettings =
      this->CreateObject(cmXCodeObject::ATTRIBUTE_GROUP);
    std::string fileTypeString;
    std::string productTypeString;
    std::string productName;
    this->CreateBuildSettings(cmtarget, 
                              buildSettings, fileTypeString, 
                              productTypeString, productName,
                              configVector[i].c_str());
    config->AddAttribute("name", this->CreateString(configVector[i].c_str()));
    config->SetComment(configVector[i].c_str());
    config->AddAttribute("buildSettings", buildSettings);
    }
  if(configVector.size())
    {
    configlist->AddAttribute("defaultConfigurationName", 
                             this->CreateString(configVector[0].c_str()));
    configlist->AddAttribute("defaultConfigurationIsVisible", 
                             this->CreateString("0"));
    }
}

//----------------------------------------------------------------------------
cmXCodeObject*
cmGlobalXCodeGenerator::CreateXCodeTarget(cmTarget& cmtarget,
                                          cmXCodeObject* buildPhases)
{
  cmXCodeObject* target = 
    this->CreateObject(cmXCodeObject::PBXNativeTarget);
  target->AddAttribute("buildPhases", buildPhases);
  cmXCodeObject* buildRules = this->CreateObject(cmXCodeObject::OBJECT_LIST);
  target->AddAttribute("buildRules", buildRules);
  cmXCodeObject* buildSettings =
    this->CreateObject(cmXCodeObject::ATTRIBUTE_GROUP);
  std::string fileTypeString;
  std::string productTypeString;
  std::string productName;
  const char* globalConfig = 0;
  if(this->XcodeVersion > 20)
    {
    this->AddConfigurations(target, cmtarget);
    }
  else
    {
    globalConfig = this->CurrentMakefile->GetDefinition("CMAKE_BUILD_TYPE");  
    }
  this->CreateBuildSettings(cmtarget, 
                            buildSettings, fileTypeString, 
                            productTypeString, productName, globalConfig);
  target->AddAttribute("buildSettings", buildSettings);
  cmXCodeObject* dependencies = 
    this->CreateObject(cmXCodeObject::OBJECT_LIST);
  target->AddAttribute("dependencies", dependencies);
  target->AddAttribute("name", this->CreateString(productName.c_str()));
  target->AddAttribute("productName",this->CreateString(productName.c_str()));

  cmXCodeObject* fileRef = 
    this->CreateObject(cmXCodeObject::PBXFileReference);
  fileRef->AddAttribute("explicitFileType", 
                        this->CreateString(fileTypeString.c_str()));
  fileRef->AddAttribute("path", this->CreateString(productName.c_str()));
  fileRef->AddAttribute("refType", this->CreateString("0"));
  fileRef->AddAttribute("sourceTree",
                        this->CreateString("BUILT_PRODUCTS_DIR"));
  fileRef->SetComment(cmtarget.GetName());
  target->AddAttribute("productReference", 
                       this->CreateObjectReference(fileRef));
  target->AddAttribute("productType", 
                       this->CreateString(productTypeString.c_str()));
  target->SetTarget(&cmtarget);
  return target;
}

//----------------------------------------------------------------------------
cmXCodeObject* cmGlobalXCodeGenerator::FindXCodeTarget(cmTarget* t)
{
  if(!t)
    {
    return 0;
    }
  for(std::vector<cmXCodeObject*>::iterator i = this->XCodeObjects.begin();
      i != this->XCodeObjects.end(); ++i)
    {
    cmXCodeObject* o = *i;
    if(o->GetTarget() == t)
      {
      return o;
      }
    }
  return 0;
}

//----------------------------------------------------------------------------
void cmGlobalXCodeGenerator::AddDependTarget(cmXCodeObject* target,
                                             cmXCodeObject* dependTarget)
{
  // make sure a target does not depend on itself
  if(target == dependTarget)
    {
    return;
    }
  // now avoid circular references if dependTarget already
  // depends on target then skip it.  Circular references crashes
  // xcode
  cmXCodeObject* dependTargetDepends = 
    dependTarget->GetObject("dependencies");
  if(dependTargetDepends)
    {
    if(dependTargetDepends->HasObject(target->GetPBXTargetDependency()))
      { 
      return;
      }
    }
  
  cmXCodeObject* targetdep = dependTarget->GetPBXTargetDependency();
  if(!targetdep)
    {
    cmXCodeObject* container = 
      this->CreateObject(cmXCodeObject::PBXContainerItemProxy);
    container->SetComment("PBXContainerItemProxy");
    container->AddAttribute("containerPortal",
                            this->CreateObjectReference(this->RootObject));
    container->AddAttribute("proxyType", this->CreateString("1"));
    container->AddAttribute("remoteGlobalIDString",
                            this->CreateObjectReference(dependTarget));
    container->AddAttribute("remoteInfo", 
                            this->CreateString(
                              dependTarget->GetTarget()->GetName()));
    targetdep = 
      this->CreateObject(cmXCodeObject::PBXTargetDependency);
    targetdep->SetComment("PBXTargetDependency");
    targetdep->AddAttribute("target",
                            this->CreateObjectReference(dependTarget));
    targetdep->AddAttribute("targetProxy", 
                            this->CreateObjectReference(container));
    dependTarget->SetPBXTargetDependency(targetdep);
    }
    
  cmXCodeObject* depends = target->GetObject("dependencies");
  if(!depends)
    {
    cmSystemTools::
      Error("target does not have dependencies attribute error..");
    
    }
  else
    {
    depends->AddUniqueObject(targetdep);
    }
}


//----------------------------------------------------------------------------
void cmGlobalXCodeGenerator::AppendOrAddBuildSetting(cmXCodeObject* settings,
                                                     const char* attribute,
                                                     const char* value)
{
  if(settings)
    {
    cmXCodeObject* attr = settings->GetObject(attribute);
    if(!attr)
      {
      settings->AddAttribute(attribute, this->CreateString(value));
      }
    else
      {
      std::string oldValue = attr->GetString();
      cmSystemTools::ReplaceString(oldValue, "\"", "");
      oldValue += " ";
      oldValue += value;
      attr->SetString(oldValue.c_str());
      }
    }
}

//----------------------------------------------------------------------------
void cmGlobalXCodeGenerator
::AppendBuildSettingAttribute(cmXCodeObject* target,
                                                         const char* attribute,
                                                         const char* value,
                                                         const char* configName)
{
  if(this->XcodeVersion < 21)
    {
    // There is only one configuration.  Add the setting to the buildSettings
    // of the target.
    this->AppendOrAddBuildSetting(target->GetObject("buildSettings"),
                                  attribute, value);
    }
  else
    {
    // There are multiple configurations.  Add the setting to the
    // buildSettings of the configuration name given.
    cmXCodeObject* configurationList = 
      target->GetObject("buildConfigurationList")->GetObject();
    cmXCodeObject* buildConfigs = 
      configurationList->GetObject("buildConfigurations");
    std::vector<cmXCodeObject*> list = buildConfigs->GetObjectList();
    // each configuration and the target itself has a buildSettings in it 
    //list.push_back(target);
    for(std::vector<cmXCodeObject*>::iterator i = list.begin(); 
        i != list.end(); ++i)
      {
      if(configName)
        {
        if(strcmp((*i)->GetObject("name")->GetString(), configName) == 0)
          {
          cmXCodeObject* settings = (*i)->GetObject("buildSettings");
          this->AppendOrAddBuildSetting(settings, attribute, value);
          }
        }
      else
        {
        cmXCodeObject* settings = (*i)->GetObject("buildSettings");
        this->AppendOrAddBuildSetting(settings, attribute, value);
        }
      }
    }
}

//----------------------------------------------------------------------------
void cmGlobalXCodeGenerator
::AddDependAndLinkInformation(cmXCodeObject* target)
{
  cmTarget* cmtarget = target->GetTarget();
  if(!cmtarget)
    {
    cmSystemTools::Error("Error no target on xobject\n");
    return;
    }

  // Add dependencies on other CMake targets.
  if(cmtarget->GetType() != cmTarget::STATIC_LIBRARY)
    {
    // Keep track of dependencies already listed.
    std::set<cmStdString> emitted;

    // A target should not depend on itself.
    emitted.insert(cmtarget->GetName());

    // Loop over all library dependencies.
    const cmTarget::LinkLibraryVectorType& tlibs = 
      cmtarget->GetLinkLibraries();
    for(cmTarget::LinkLibraryVectorType::const_iterator lib = tlibs.begin();
        lib != tlibs.end(); ++lib)
      {
      // Don't emit the same library twice for this target.
      if(emitted.insert(lib->first).second)
        {
        // Add this dependency.
        cmTarget* t = this->FindTarget(this->CurrentProject.c_str(),
                                       lib->first.c_str());
        cmXCodeObject* dptarget = this->FindXCodeTarget(t);
        if(dptarget)
          {
          this->AddDependTarget(target, dptarget);
          }
        }
      }
    }
  
  // write utility dependencies.
  for(std::set<cmStdString>::const_iterator i
        = cmtarget->GetUtilities().begin();
      i != cmtarget->GetUtilities().end(); ++i)
    {
    cmTarget* t = this->FindTarget(this->CurrentProject.c_str(),
                                   i->c_str());
    // if the target is in this project then make target depend
    // on it.  It may not be in this project if this is a sub
    // project from the top.
    if(t)
      {
      cmXCodeObject* dptarget = this->FindXCodeTarget(t);
      if(dptarget)
        {
        this->AddDependTarget(target, dptarget);
        }
      else
        {
        std::string m = "Error Utility: ";
        m += i->c_str();
        m += "\n";
        m += "cmtarget ";
        if(t)
          {
          m += t->GetName();
          }
        m += "\n";
        m += "Is on the target ";
        m += cmtarget->GetName();
        m += "\n";
        m += "But it has no xcode target created yet??\n";
        m += "Current project is ";
        m += this->CurrentProject.c_str();
        cmSystemTools::Error(m.c_str());
        }
      } 
    }

  // Loop over configuration types and set per-configuration info.
  for(std::vector<std::string>::iterator i =
        this->CurrentConfigurationTypes.begin();
      i != this->CurrentConfigurationTypes.end(); ++i)
    {
    // Get the current configuration name.
    const char* configName = i->c_str();
    if(!*configName)
      {
      configName = 0;
      }

    // Compute the link library and directory information.
    std::vector<cmStdString> libNames;
    std::vector<cmStdString> libDirs;
    std::vector<cmStdString> fullPathLibs;
    this->CurrentLocalGenerator->ComputeLinkInformation(*cmtarget, configName,
                                                    libNames, libDirs,
                                                    &fullPathLibs);

    // Add dependencies directly on library files.
    for(std::vector<cmStdString>::iterator j = fullPathLibs.begin();
        j != fullPathLibs.end(); ++j)
      {
      target->AddDependLibrary(configName, j->c_str());
      }

    std::string linkDirs;
    // add the library search paths
    for(std::vector<cmStdString>::const_iterator libDir = libDirs.begin();
        libDir != libDirs.end(); ++libDir)
      {
      if(libDir->size() && *libDir != "/usr/lib")
        {
        if(this->XcodeVersion > 15)
          {
          // now add the same one but append $(CONFIGURATION) to it:
          linkDirs += " ";
          linkDirs += this->XCodeEscapePath(libDir->c_str());
          linkDirs += "/$(CONFIGURATION)";
          }
        linkDirs += " ";
        linkDirs += this->XCodeEscapePath(libDir->c_str());
        }
      }
    this->AppendBuildSettingAttribute(target, "LIBRARY_SEARCH_PATHS",
                                      linkDirs.c_str(), configName);
    // now add the link libraries
    if(cmtarget->GetType() != cmTarget::STATIC_LIBRARY)
      {
      for(std::vector<cmStdString>::iterator lib = libNames.begin();
          lib != libNames.end(); ++lib)
        {
        this->AppendBuildSettingAttribute(target, "OTHER_LDFLAGS",
                                          lib->c_str(), configName);
        }
      }
    }
}

//----------------------------------------------------------------------------
void cmGlobalXCodeGenerator::CreateGroups(cmLocalGenerator* root,
                                          std::vector<cmLocalGenerator*>&
                                          generators)
{
  for(std::vector<cmLocalGenerator*>::iterator i = generators.begin();
      i != generators.end(); ++i)
    {
    if(this->IsExcluded(root, *i))
      {
      continue;
      }
    cmMakefile* mf = (*i)->GetMakefile();
    std::vector<cmSourceGroup> sourceGroups = mf->GetSourceGroups();
    cmTargets &tgts = mf->GetTargets();
    for(cmTargets::iterator l = tgts.begin(); l != tgts.end(); l++)
      { 
      cmTarget& cmtarget = l->second;
      // add the soon to be generated Info.plist file as a source for a
      // MACOSX_BUNDLE file
      if(cmtarget.GetPropertyAsBool("MACOSX_BUNDLE"))
        {
        cmSourceFile file;
        file.SetName("Info",
                     this->CurrentMakefile->GetCurrentOutputDirectory(),
                     "plist", false);
        cmtarget.GetSourceFiles().push_back
          (this->CurrentMakefile->AddSource(file));
        }

      std::vector<cmSourceFile*> & classes = cmtarget.GetSourceFiles();
      for(std::vector<cmSourceFile*>::const_iterator s = classes.begin(); 
          s != classes.end(); s++)
        {
        cmSourceFile* sf = *s;
        // Add the file to the list of sources.
        std::string const& source = sf->GetFullPath();
        cmSourceGroup& sourceGroup = 
          mf->FindSourceGroup(source.c_str(), sourceGroups);
        cmXCodeObject* pbxgroup = 
          this->CreateOrGetPBXGroup(cmtarget, &sourceGroup);
        this->GroupMap[sf] = pbxgroup;
        }
      }
    } 
}
//----------------------------------------------------------------------------
cmXCodeObject* cmGlobalXCodeGenerator
::CreateOrGetPBXGroup(cmTarget& cmtarget, cmSourceGroup* sg)
{
  cmStdString s = cmtarget.GetName();
  s += "/";
  s += sg->GetName();
  std::map<cmStdString, cmXCodeObject* >::iterator i =  
    this->GroupNameMap.find(s);
  if(i != this->GroupNameMap.end())
    {
    return i->second;
    }
  i = this->TargetGroup.find(cmtarget.GetName());
  cmXCodeObject* tgroup = 0;
  if(i != this->TargetGroup.end())
    {
    tgroup = i->second;
    }
  else
    {
    tgroup = this->CreateObject(cmXCodeObject::PBXGroup);
    this->TargetGroup[cmtarget.GetName()] = tgroup;
    cmXCodeObject* tgroupChildren = 
      this->CreateObject(cmXCodeObject::OBJECT_LIST);
    tgroup->AddAttribute("name", this->CreateString(cmtarget.GetName()));
    tgroup->AddAttribute("children", tgroupChildren);
    if(this->XcodeVersion == 15)
      {
      tgroup->AddAttribute("refType", this->CreateString("4"));
      }
    tgroup->AddAttribute("sourceTree", this->CreateString("<group>"));
    this->SourcesGroupChildren->AddObject(tgroup);
    }
  
  cmXCodeObject* tgroupChildren = tgroup->GetObject("children");
  cmXCodeObject* group = this->CreateObject(cmXCodeObject::PBXGroup);
  cmXCodeObject* groupChildren = 
    this->CreateObject(cmXCodeObject::OBJECT_LIST);
  group->AddAttribute("name", this->CreateString(sg->GetName()));
  group->AddAttribute("children", groupChildren);
  if(this->XcodeVersion == 15)
    {
    group->AddAttribute("refType", this->CreateString("4"));
    }
  group->AddAttribute("sourceTree", this->CreateString("<group>"));
  tgroupChildren->AddObject(group);
  this->GroupNameMap[s] = group;
  return group;
}

//----------------------------------------------------------------------------
void cmGlobalXCodeGenerator
::CreateXCodeObjects(cmLocalGenerator* root,
                                                std::vector<cmLocalGenerator*>&
                     generators)
{
  this->ClearXCodeObjects(); 
  this->RootObject = 0;
  this->SourcesGroupChildren = 0;
  this->MainGroupChildren = 0;
  cmXCodeObject* group = this->CreateObject(cmXCodeObject::ATTRIBUTE_GROUP);
  group->AddAttribute("COPY_PHASE_STRIP", this->CreateString("NO"));
  cmXCodeObject* developBuildStyle = 
    this->CreateObject(cmXCodeObject::PBXBuildStyle);
  cmXCodeObject* listObjs = this->CreateObject(cmXCodeObject::OBJECT_LIST);
  if(this->XcodeVersion == 15)
    {
    developBuildStyle->AddAttribute("name", 
                                    this->CreateString("Development"));
    developBuildStyle->AddAttribute("buildSettings", group);
    listObjs->AddObject(developBuildStyle);
    group = this->CreateObject(cmXCodeObject::ATTRIBUTE_GROUP);
    group->AddAttribute("COPY_PHASE_STRIP", this->CreateString("YES"));
    cmXCodeObject* deployBuildStyle =
    this->CreateObject(cmXCodeObject::PBXBuildStyle);
    deployBuildStyle->AddAttribute("name", this->CreateString("Deployment"));
    deployBuildStyle->AddAttribute("buildSettings", group);
    listObjs->AddObject(deployBuildStyle);
    }
  else
    {
    for(unsigned int i = 0; i < this->CurrentConfigurationTypes.size(); ++i)
      {
      cmXCodeObject* buildStyle = 
        this->CreateObject(cmXCodeObject::PBXBuildStyle);
      const char* name = this->CurrentConfigurationTypes[i].c_str();
      buildStyle->AddAttribute("name", this->CreateString(name));
      buildStyle->SetComment(name);
      cmXCodeObject* sgroup =
        this->CreateObject(cmXCodeObject::ATTRIBUTE_GROUP);
      sgroup->AddAttribute("COPY_PHASE_STRIP", this->CreateString("NO"));
      buildStyle->AddAttribute("buildSettings", sgroup);
      listObjs->AddObject(buildStyle);
      }
    }

  cmXCodeObject* mainGroup = this->CreateObject(cmXCodeObject::PBXGroup);
  this->MainGroupChildren = 
    this->CreateObject(cmXCodeObject::OBJECT_LIST);
  mainGroup->AddAttribute("children", this->MainGroupChildren);
  if(this->XcodeVersion == 15)
    {
    mainGroup->AddAttribute("refType", this->CreateString("4"));
    }
  mainGroup->AddAttribute("sourceTree", this->CreateString("<group>"));

  cmXCodeObject* sourcesGroup = this->CreateObject(cmXCodeObject::PBXGroup);
  this->SourcesGroupChildren = 
    this->CreateObject(cmXCodeObject::OBJECT_LIST);
  sourcesGroup->AddAttribute("name", this->CreateString("Sources"));
  sourcesGroup->AddAttribute("children", this->SourcesGroupChildren);
  if(this->XcodeVersion == 15)
    {
    sourcesGroup->AddAttribute("refType", this->CreateString("4"));
    }
  sourcesGroup->AddAttribute("sourceTree", this->CreateString("<group>"));
  this->MainGroupChildren->AddObject(sourcesGroup);
  // now create the cmake groups 
  this->CreateGroups(root, generators);
  
  cmXCodeObject* productGroup = this->CreateObject(cmXCodeObject::PBXGroup);
  productGroup->AddAttribute("name", this->CreateString("Products"));
  if(this->XcodeVersion == 15)
    {
    productGroup->AddAttribute("refType", this->CreateString("4"));
    }
  productGroup->AddAttribute("sourceTree", this->CreateString("<group>"));
  cmXCodeObject* productGroupChildren = 
    this->CreateObject(cmXCodeObject::OBJECT_LIST);
  productGroup->AddAttribute("children", productGroupChildren);
  this->MainGroupChildren->AddObject(productGroup);
  
  
  this->RootObject = this->CreateObject(cmXCodeObject::PBXProject);
  this->RootObject->SetComment("Project object");
  group = this->CreateObject(cmXCodeObject::ATTRIBUTE_GROUP);
  this->RootObject->AddAttribute("mainGroup", 
                             this->CreateObjectReference(mainGroup));
  this->RootObject->AddAttribute("buildSettings", group);
  this->RootObject->AddAttribute("buildStyles", listObjs);
  this->RootObject->AddAttribute("hasScannedForEncodings",
                             this->CreateString("0"));
  cmXCodeObject* configlist = 
    this->CreateObject(cmXCodeObject::XCConfigurationList);
  cmXCodeObject* buildConfigurations =
    this->CreateObject(cmXCodeObject::OBJECT_LIST);
  std::vector<cmXCodeObject*> configs;
  if(this->XcodeVersion == 15)
    {
    cmXCodeObject* configDebug = 
      this->CreateObject(cmXCodeObject::XCBuildConfiguration);
    configDebug->AddAttribute("name", this->CreateString("Debug"));
    configs.push_back(configDebug);
    cmXCodeObject* configRelease = 
      this->CreateObject(cmXCodeObject::XCBuildConfiguration);
    configRelease->AddAttribute("name", this->CreateString("Release"));
    configs.push_back(configRelease);
    }
  else
    {
    for(unsigned int i = 0; i < this->CurrentConfigurationTypes.size(); ++i)
      {
      const char* name = this->CurrentConfigurationTypes[i].c_str();
      cmXCodeObject* config = 
        this->CreateObject(cmXCodeObject::XCBuildConfiguration);
      config->AddAttribute("name", this->CreateString(name));
      configs.push_back(config);
      }
    }
  for(std::vector<cmXCodeObject*>::iterator c = configs.begin();
      c != configs.end(); ++c)
    {
    buildConfigurations->AddObject(*c);
    }
  configlist->AddAttribute("buildConfigurations", buildConfigurations);

  std::string comment = "Build configuration list for PBXProject ";
  comment += " \"";
  comment += this->CurrentProject;
  comment += "\"";
  configlist->SetComment(comment.c_str());
  configlist->AddAttribute("defaultConfigurationIsVisible", 
                           this->CreateString("0"));
  configlist->AddAttribute("defaultConfigurationName", 
                           this->CreateString("Debug"));
  cmXCodeObject* buildSettings =
      this->CreateObject(cmXCodeObject::ATTRIBUTE_GROUP);
  const char* osxArch = 
      this->CurrentMakefile->GetDefinition("CMAKE_OSX_ARCHITECTURES");
  const char* sysroot = 
      this->CurrentMakefile->GetDefinition("CMAKE_OSX_SYSROOT");
  if(osxArch && sysroot)
    {
    this->Architectures.clear();
    cmSystemTools::ExpandListArgument(std::string(osxArch),
                                      this->Architectures);
    if(this->Architectures.size() > 1)
      {
      buildSettings->AddAttribute("SDKROOT", 
                                  this->CreateString(sysroot));
      std::string archString;
      for( std::vector<std::string>::iterator i = 
             this->Architectures.begin();
           i != this->Architectures.end(); ++i)
        {
        archString += *i;
        archString += " ";
        }
      buildSettings->AddAttribute("ARCHS", 
                                  this->CreateString(archString.c_str()));
      }
    }
  for( std::vector<cmXCodeObject*>::iterator i = configs.begin();
       i != configs.end(); ++i)
    {
    (*i)->AddAttribute("buildSettings", buildSettings);
    }
  this->RootObject->AddAttribute("buildConfigurationList", 
                             this->CreateObjectReference(configlist));

  std::vector<cmXCodeObject*> targets;
  for(std::vector<cmLocalGenerator*>::iterator i = generators.begin();
      i != generators.end(); ++i)
    {
    if(!this->IsExcluded(root, *i))
      {
      this->CreateXCodeTargets(*i, targets);
      }
    }
  // loop over all targets and add link and depend info
  for(std::vector<cmXCodeObject*>::iterator i = targets.begin();
      i != targets.end(); ++i)
    {
    cmXCodeObject* t = *i;
    this->AddDependAndLinkInformation(t);
    }
  // now create xcode depend hack makefile
  this->CreateXCodeDependHackTarget(targets);
  // now add all targets to the root object
  cmXCodeObject* allTargets = this->CreateObject(cmXCodeObject::OBJECT_LIST);
  for(std::vector<cmXCodeObject*>::iterator i = targets.begin();
      i != targets.end(); ++i)
    { 
    cmXCodeObject* t = *i;
    allTargets->AddObject(t);
    cmXCodeObject* productRef = t->GetObject("productReference");
    if(productRef)
      {
      productGroupChildren->AddObject(productRef->GetObject());
      }
    }
  this->RootObject->AddAttribute("targets", allTargets);
}


//----------------------------------------------------------------------------
void 
cmGlobalXCodeGenerator::CreateXCodeDependHackTarget(
  std::vector<cmXCodeObject*>& targets)
{ 
  cmGeneratedFileStream 
    makefileStream(this->CurrentXCodeHackMakefile.c_str());
  if(!makefileStream)
    {
    cmSystemTools::Error("Could not create",
                         this->CurrentXCodeHackMakefile.c_str());
    return;
    }
  makefileStream.SetCopyIfDifferent(true);
  // one more pass for external depend information not handled
  // correctly by xcode
  makefileStream << "# DO NOT EDIT\n";
  makefileStream << "# This makefile makes sure all linkable targets are \n";
  makefileStream << "# up-to-date with anything they link to,avoiding a "
    "bug in XCode 1.5\n";
  for(std::vector<std::string>::const_iterator
        ct = this->CurrentConfigurationTypes.begin();
      ct != this->CurrentConfigurationTypes.end(); ++ct)
    {
    if(this->XcodeVersion < 21 || ct->empty())
      {
      makefileStream << "all: ";
      }
    else
      {
      makefileStream << "all." << *ct << ": ";
      }
    const char* configName = 0;
    if(!ct->empty())
      {
      configName = ct->c_str();
      }
    for(std::vector<cmXCodeObject*>::iterator i = targets.begin();
        i != targets.end(); ++i)
      {
      cmXCodeObject* target = *i;
      cmTarget* t =target->GetTarget();
      if(t->GetType() == cmTarget::EXECUTABLE ||
         t->GetType() == cmTarget::SHARED_LIBRARY ||
         t->GetType() == cmTarget::MODULE_LIBRARY)
        {
        makefileStream << "\\\n\t" <<
          this->ConvertToRelativeForMake(
            t->GetFullPath(configName).c_str());
        }
      }
    makefileStream << "\n\n"; 
    }
  makefileStream 
    << "# For each target create a dummy rule "
    "so the target does not have to exist\n";
  std::set<cmStdString> emitted;
  for(std::vector<cmXCodeObject*>::iterator i = targets.begin();
      i != targets.end(); ++i)
    {
    cmXCodeObject* target = *i;
    std::map<cmStdString, cmXCodeObject::StringVec> const& deplibs =
      target->GetDependLibraries();
    for(std::map<cmStdString, cmXCodeObject::StringVec>::const_iterator ci 
          = deplibs.begin(); ci != deplibs.end(); ++ci)
      {
      for(cmXCodeObject::StringVec::const_iterator d = ci->second.begin();
          d != ci->second.end(); ++d)
        {
        if(emitted.insert(*d).second)
          {
          makefileStream << 
            this->ConvertToRelativeForMake(d->c_str()) << ":\n";
          }
        }
      }
    }
  makefileStream << "\n\n";

  // Write rules to help Xcode relink things at the right time.
  makefileStream << 
    "# Rules to remove targets that are older than anything to which they\n"
    "# link.  This forces Xcode to relink the targets from scratch.  It\n"
    "# does not seem to check these dependencies itself.\n";  
  for(std::vector<std::string>::const_iterator
        ct = this->CurrentConfigurationTypes.begin();
      ct != this->CurrentConfigurationTypes.end(); ++ct)
    {
    const char* configName = 0;
    if(!ct->empty())
      {
      configName = ct->c_str();
      }
    for(std::vector<cmXCodeObject*>::iterator i = targets.begin();
        i != targets.end(); ++i)
      {
      cmXCodeObject* target = *i;
      cmTarget* t =target->GetTarget();
      if(t->GetType() == cmTarget::EXECUTABLE ||
         t->GetType() == cmTarget::SHARED_LIBRARY ||
         t->GetType() == cmTarget::MODULE_LIBRARY)
        {
        // Create a rule for this target.
        std::string tfull = t->GetFullPath(configName);
        makefileStream << this->ConvertToRelativeForMake(tfull.c_str()) 
                       << ":";

        // List dependencies if any exist.
        std::map<cmStdString, cmXCodeObject::StringVec>::const_iterator
          x = target->GetDependLibraries().find(*ct);
        if(x != target->GetDependLibraries().end())
          {
          std::vector<cmStdString> const& deplibs = x->second;
          for(std::vector<cmStdString>::const_iterator d = deplibs.begin();
              d != deplibs.end(); ++d)
            {
            makefileStream << "\\\n\t" << 
              this->ConvertToRelativeForMake(d->c_str());
            }
          }
        // Write the action to remove the target if it is out of date.
        makefileStream << "\n";
        makefileStream << "\t/bin/rm -f "
                       << this->ConvertToRelativeForMake(tfull.c_str())
                       << "\n";
        // if building for more than one architecture
        // then remove those exectuables as well
        if(this->Architectures.size() > 1)
          {
          std::string universal = t->GetDirectory();
          universal += "/";
          universal += this->CurrentMakefile->GetProjectName();
          universal += ".build/";
          universal += configName;
          universal += "/";
          universal += t->GetName();
          universal += ".build/Objects-normal/";
          for( std::vector<std::string>::iterator arch = 
                 this->Architectures.begin();
               arch != this->Architectures.end(); ++arch)
            {
            std::string universalFile = universal;
            universalFile += *arch;
            universalFile += "/";
            universalFile += t->GetName();
            makefileStream << "\t/bin/rm -f "
                           << 
              this->ConvertToRelativeForMake(universalFile.c_str())
                           << "\n";
            }
          }
        makefileStream << "\n\n";
        }
      }
    }
}

//----------------------------------------------------------------------------
void
cmGlobalXCodeGenerator::OutputXCodeProject(cmLocalGenerator* root,
                                           std::vector<cmLocalGenerator*>& 
                                           generators)
{
  if(generators.size() == 0)
    {
    return;
    }
#if 1
  // TODO: This block should be moved to a central location for all
  // generators.  It is duplicated in every generator.
  for(std::vector<cmLocalGenerator*>::iterator g = generators.begin();
      g != generators.end(); ++g)
    {
    if(this->IsExcluded(root, *g))
      {
      continue;
      }
    cmMakefile* mf = (*g)->GetMakefile();
    std::vector<cmSourceGroup> sourceGroups = mf->GetSourceGroups();
    cmTargets &tgts = mf->GetTargets();
  // Call TraceVSDependencies on all targets
  for(cmTargets::iterator l = tgts.begin(); 
      l != tgts.end(); l++)
    {
    // INCLUDE_EXTERNAL_MSPROJECT command only affects the workspace
    // so don't build a projectfile for it
    if ((l->second.GetType() != cmTarget::INSTALL_FILES)
        && (l->second.GetType() != cmTarget::INSTALL_PROGRAMS)
        && (strncmp(l->first.c_str(), "INCLUDE_EXTERNAL_MSPROJECT", 26) != 0))
      {
      cmTarget& target = l->second;
      target.TraceVSDependencies(target.GetName(), mf);
      }
    }
  // now for all custom commands that are not used directly in a 
  // target, add them to all targets in the current directory or
  // makefile
  std::set<cmStdString> banned;
  banned.insert("ALL_BUILD");
  banned.insert("XCODE_DEPEND_HELPER");
  banned.insert("install");
  std::vector<cmSourceFile*> & classesmf = mf->GetSourceFiles();
  for(std::vector<cmSourceFile*>::const_iterator i = classesmf.begin(); 
      i != classesmf.end(); i++)
    {
    if(cmCustomCommand* cc = (*i)->GetCustomCommand())
      {
      if(!cc->IsUsed())
        {
        for(cmTargets::iterator l = tgts.begin(); 
            l != tgts.end(); l++)
          {
          if ((l->second.GetType() != cmTarget::INSTALL_FILES)
              && (l->second.GetType() != cmTarget::INSTALL_PROGRAMS)
              && (strncmp(l->first.c_str(), 
                          "INCLUDE_EXTERNAL_MSPROJECT", 26) != 0)
              && banned.find(l->second.GetName()) == banned.end())
            {
            cmTarget& target = l->second;
            bool sameAsTarget = false;
            // make sure we don't add a custom command that depends on
            // this target
            for(unsigned int k =0; k < cc->GetDepends().size(); k++)
              {
              if(cmSystemTools::GetFilenameName
                 (cc->GetDepends()[k]) == target.GetFullName())
                {
                sameAsTarget = true;
                }
              }
            if(!sameAsTarget)
              {
              target.GetSourceFiles().push_back(*i);
              }
            }
          }
        }
      }
    }
    }
#endif
  this->CreateXCodeObjects(root,
                           generators);
  std::string xcodeDir = root->GetMakefile()->GetStartOutputDirectory();
  xcodeDir += "/";
  xcodeDir += root->GetMakefile()->GetProjectName();
  xcodeDir += ".xcode";
  if(this->XcodeVersion > 20)
    {
    xcodeDir += "proj";
    }  
  cmSystemTools::MakeDirectory(xcodeDir.c_str());
  std::string xcodeProjFile = xcodeDir + "/project.pbxproj";
  cmGeneratedFileStream fout(xcodeProjFile.c_str());
  fout.SetCopyIfDifferent(true);
  if(!fout)
    {
    return;
    }
  this->WriteXCodePBXProj(fout, root, generators);
  this->ClearXCodeObjects();
}

//----------------------------------------------------------------------------
void 
cmGlobalXCodeGenerator::WriteXCodePBXProj(std::ostream& fout,
                                          cmLocalGenerator* ,
                                          std::vector<cmLocalGenerator*>& )
{
  fout << "// !$*UTF8*$!\n";
  fout << "{\n";
  cmXCodeObject::Indent(1, fout);
  fout << "archiveVersion = 1;\n";
  cmXCodeObject::Indent(1, fout);
  fout << "classes = {\n";
  cmXCodeObject::Indent(1, fout);
  fout << "};\n";
  cmXCodeObject::Indent(1, fout);
  fout << "objectVersion = 39;\n";
  cmXCodeObject::PrintList(this->XCodeObjects, fout);
  cmXCodeObject::Indent(1, fout);
  fout << "rootObject = " << this->RootObject->GetId() << ";\n";
  fout << "}\n";
}

//----------------------------------------------------------------------------
void cmGlobalXCodeGenerator::GetDocumentation(cmDocumentationEntry& entry)
  const
{
  entry.name = this->GetName();
  entry.brief = "Generate XCode project files.";
  entry.full = "";
}

//----------------------------------------------------------------------------
std::string cmGlobalXCodeGenerator::ConvertToRelativeForMake(const char* p)
{
  if ( !this->CurrentMakefile->IsOn("CMAKE_USE_RELATIVE_PATHS") )
    {
    return cmSystemTools::ConvertToOutputPath(p);
    }
  else
    {
    std::string ret = 
      this->ConvertToRelativePath(this->CurrentOutputDirectoryComponents, p);
    return cmSystemTools::ConvertToOutputPath(ret.c_str());
    }
}

//----------------------------------------------------------------------------
std::string cmGlobalXCodeGenerator::ConvertToRelativeForXCode(const char* p)
{
  if ( !this->CurrentMakefile->IsOn("CMAKE_USE_RELATIVE_PATHS") )
    {
    return cmSystemTools::ConvertToOutputPath(p);
    }
  else
    {
    std::string ret = 
      this->ConvertToRelativePath(this->ProjectOutputDirectoryComponents, p);
    return cmSystemTools::ConvertToOutputPath(ret.c_str());
    }
}

std::string cmGlobalXCodeGenerator::XCodeEscapePath(const char* p)
{
  std::string ret = p;
  if(ret.find(' ') != ret.npos)
    {
    std::string t = ret;
    ret = "\\\"";
    ret += t;
    ret += "\\\"";
    }
  return ret;
}

//----------------------------------------------------------------------------
void
cmGlobalXCodeGenerator
::AppendDirectoryForConfig(const char* prefix,
                           const char* config,
                           const char* suffix,
                           std::string& dir)
{
  if(this->XcodeVersion > 20)
    {
    if(config)
      {
      dir += prefix;
      dir += config;
      dir += suffix;
      }
    }
}

//----------------------------------------------------------------------------
std::string cmGlobalXCodeGenerator::LookupFlags(const char* varNamePrefix,
                                                const char* varNameLang,
                                                const char* varNameSuffix,
                                                const char* default_flags)
{
  if(varNameLang)
    {
    std::string varName = varNamePrefix;
    varName += varNameLang;
    varName += varNameSuffix;
    if(const char* varValue =
       this->CurrentMakefile->GetDefinition(varName.c_str()))
      {
      if(*varValue)
        {
        return varValue;
        }
      }
    }
  return default_flags;
}