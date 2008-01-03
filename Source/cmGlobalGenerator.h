/*=========================================================================

  Program:   CMake - Cross-Platform Makefile Generator
  Module:    $RCSfile: cmGlobalGenerator.h,v $
  Language:  C++
  Date:      $Date: 2007/03/16 22:05:42 $
  Version:   $Revision: 1.58.2.6 $

  Copyright (c) 2002 Kitware, Inc., Insight Consortium.  All rights reserved.
  See Copyright.txt or http://www.cmake.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef cmGlobalGenerator_h
#define cmGlobalGenerator_h

#include "cmStandardIncludes.h"

#include "cmTarget.h" // For cmTargets

class cmake;
class cmMakefile;
class cmLocalGenerator;
class cmTarget;

/** \class cmGlobalGenerator
 * \brief Responable for overseeing the generation process for the entire tree
 *
 * Subclasses of this class generate makefiles for various
 * platforms.
 */
class cmGlobalGenerator
{
public:
  ///! Free any memory allocated with the GlobalGenerator
  cmGlobalGenerator();
  virtual ~cmGlobalGenerator();
  
  ///! Create a local generator appropriate to this Global Generator
  virtual cmLocalGenerator *CreateLocalGenerator();

  ///! Get the name for this generator
  virtual const char *GetName() const { return "Generic"; };
  
  /** Get the documentation entry for this generator.  */
  virtual void GetDocumentation(cmDocumentationEntry& entry) const;
  
  /**
   * Create LocalGenerators and process the CMakeLists files. This does not
   * actually produce any makefiles, DSPs, etc.  
   */
  virtual void Configure();

  /**
   * Generate the all required files for building this project/tree. This
   * basically creates a series of LocalGenerators for each directory and
   * requests that they Generate.  
   */
  virtual void Generate();

  /**
   * Set/Get and Clear the enabled languages.  
   */
  void SetLanguageEnabled(const char*, cmMakefile* mf);
  bool GetLanguageEnabled(const char*);
  void ClearEnabledLanguages();
  void GetEnabledLanguages(std::vector<std::string>& lang);
  /**
   * Try to determine system infomation such as shared library
   * extension, pthreads, byte order etc.  
   */
  virtual void EnableLanguage(std::vector<std::string>const& languages,
                              cmMakefile *);

  /**
   * Try to determine system infomation, get it from another generator
   */
  virtual void EnableLanguagesFromGenerator(cmGlobalGenerator *gen);

  /**
   * Try running cmake and building a file. This is used for dynalically
   * loaded commands, not as part of the usual build process.
   */
  virtual int TryCompile(const char *srcdir, const char *bindir,
                         const char *projectName, const char *targetName,
                         std::string *output, cmMakefile* mf);

  
  /**
   * Build a file given the following information. This is a more direct call
   * that is used by both CTest and TryCompile. If target name is NULL or
   * empty then all is assumed. clean indicates if a "make clean" should be
   * done first.
   */
  virtual int Build(const char *srcdir, const char *bindir,
                    const char *projectName, const char *targetName,
                    std::string *output, 
                    const char *makeProgram, const char *config,
                    bool clean, bool fast);
  virtual std::string GenerateBuildCommand
  (const char* makeProgram,
   const char *projectName, const char* additionalOptions, 
   const char *targetName,
   const char* config, bool ignoreErrors, bool fast);


  ///! Set the CMake instance
  void SetCMakeInstance(cmake *cm);
  
  ///! Get the CMake instance
  cmake *GetCMakeInstance() { return this->CMakeInstance; };

  void SetConfiguredFilesPath(const char* s){this->ConfiguredFilesPath = s;}
  cmLocalGenerator* GetLocalGenerator(int p) { 
    return this->LocalGenerators[p];}
  void GetLocalGenerators(std::vector<cmLocalGenerator *>&g) { 
    g = this->LocalGenerators;}

  void AddLocalGenerator(cmLocalGenerator *lg);

  void AddInstallComponent(const char* component);
  void EnableInstallTarget();
  
  static int s_TryCompileTimeout;
  
  bool GetForceUnixPaths() {return this->ForceUnixPaths;}
  bool GetToolSupportsColor() { return this->ToolSupportsColor; }
  ///! return the language for the given extension
  const char* GetLanguageFromExtension(const char* ext);
  ///! is an extension to be ignored
  bool IgnoreFile(const char* ext);
  ///! What is the preference for linkers and this language (None or Prefered)
  const char* GetLinkerPreference(const char* lang);
  ///! What is the output extension for a given language.
  const char* GetLanguageOutputExtensionForLanguage(const char* lang);
  ///! What is the output extension for a given source file extension.
  const char* GetLanguageOutputExtensionFromExtension(const char* lang);

  ///! What is the configurations directory variable called?
  virtual const char* GetCMakeCFGInitDirectory()  { return "."; }

  /**
   * Convert the given remote path to a relative path with respect to
   * the given local path.  The local path must be given in component
   * form (see SystemTools::SplitPath) without a trailing slash.  The
   * remote path must use forward slashes and not already be escaped
   * or quoted.
   */
  std::string ConvertToRelativePath(const std::vector<std::string>& local,
                                    const char* remote);

  /** Get whether the generator should use a script for link commands.  */
  bool GetUseLinkScript() { return this->UseLinkScript; }

  /** Get whether the generator should produce special marks on rules
      producing symbolic (non-file) outputs.  */
  bool GetNeedSymbolicMark() { return this->NeedSymbolicMark; }

  /*
   * Determine what program to use for building the project.
   */
  void FindMakeProgram(cmMakefile*);

  ///! Find a target by name by searching the local generators.
  cmTarget* FindTarget(const char* project, const char* name);

  /** If check to see if the target is linked to by any other
      target in the project */
  bool IsDependedOn(const char* project, cmTarget* target);
  ///! Find a local generator by its startdirectory
  cmLocalGenerator* FindLocalGenerator(const char* start_dir);

  /** Append the subdirectory for the given configuration.  If anything is
      appended the given prefix and suffix will be appended around it, which
      is useful for leading or trailing slashes.  */
  virtual void AppendDirectoryForConfig(const char* prefix,
                                        const char* config,
                                        const char* suffix,
                                        std::string& dir);

  /** Get the manifest of all targets that will be built for each
      configuration.  This is valid during generation only.  */
  cmTargetManifest const& GetTargetManifest() { return this->TargetManifest; }

  void AddTarget(cmTargets::value_type &v) { 
    this->TotalTargets[v.first] = &v.second;};
  
  /** Support for multiple custom command outputs.  */
  virtual void CheckMultipleOutputs(cmMakefile* mf, bool verbose);

  virtual const char* GetAllTargetName()          { return "ALL_BUILD"; }
  virtual const char* GetInstallTargetName()      { return "INSTALL"; }
  virtual const char* GetInstallLocalTargetName() { return 0; }
  virtual const char* GetPreinstallTargetName()   { return 0; }
  virtual const char* GetTestTargetName()         { return "RUN_TESTS"; }
  virtual const char* GetPackageTargetName()      { return "PACKAGE"; }
  virtual const char* GetPackageSourceTargetName(){ return 0; }
  virtual const char* GetEditCacheTargetName()    { return 0; }
  virtual const char* GetRebuildCacheTargetName() { return 0; }

protected:
  // Fill the ProjectMap, this must be called after LocalGenerators 
  // has been populated.
  void FillProjectMap();
  bool IsExcluded(cmLocalGenerator* root, cmLocalGenerator* gen);

  void ConfigureRelativePaths();
  bool RelativePathsConfigured;
  void CreateDefaultGlobalTargets(cmTargets* targets);
  cmTarget CreateGlobalTarget(const char* name, const char* message,
    const cmCustomCommandLines* commandLines,
    std::vector<std::string> depends, bool depends_on_all = false);

  bool NeedSymbolicMark;
  bool UseLinkScript;
  bool ForceUnixPaths;
  bool ToolSupportsColor;
  cmStdString FindMakeProgramFile;
  cmStdString ConfiguredFilesPath;
  cmake *CMakeInstance;
  std::vector<cmLocalGenerator *> LocalGenerators;
  // map from project name to vector of local generators in that project
  std::map<cmStdString, std::vector<cmLocalGenerator*> > ProjectMap;

  // Set of named installation components requested by the project.
  std::set<cmStdString> InstallComponents;
  bool InstallTargetEnabled;

  // Manifest of all targets that will be built for each configuration.
  // This is computed just before local generators generate.
  cmTargetManifest TargetManifest;

private:
  // If you add a new map here, make sure it is copied
  // in EnableLanguagesFromGenerator 
  std::map<cmStdString, bool> IgnoreExtensions;
  std::map<cmStdString, bool> LanguageEnabled;
  std::map<cmStdString, cmStdString> OutputExtensions;
  std::map<cmStdString, cmStdString> LanguageToOutputExtension;
  std::map<cmStdString, cmStdString> ExtensionToLanguage;
  std::map<cmStdString, cmStdString> LanguageToLinkerPreference; 

  // The paths to the tops of the source and binary trees used for
  // relative path computation.  A path must be either in the source
  // tree or the build tree to be converted to a relative path.  The
  // ConfigureRelativePaths method may set these to be empty when
  // using relative paths is unsafe.
  std::string RelativePathTopSource;
  std::string RelativePathTopBinary;

  // this is used to improve performance 
  std::map<cmStdString,cmTarget *> TotalTargets;
};

#endif
