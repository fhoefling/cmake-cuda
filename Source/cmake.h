/*=========================================================================

  Program:   CMake - Cross-Platform Makefile Generator
  Module:    $RCSfile: cmake.h,v $
  Language:  C++
  Date:      $Date: 2006/11/10 15:12:55 $
  Version:   $Revision: 1.65.2.5 $

  Copyright (c) 2002 Kitware, Inc., Insight Consortium.  All rights reserved.
  See Copyright.txt or http://www.cmake.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
// This class represents a cmake invocation. It is the top level class when
// running cmake. Most cmake based GUIS should primarily create an instance
// of this class and communicate with it.
//
// The basic process for a GUI is as follows:
//
// 1) Create a cmake instance
// 2) Set the Home & Start directories, generator, and cmake command. this
//    can be done using the Set methods or by using SetArgs and passing in
//    command line arguments.
// 3) Load the cache by calling LoadCache (duh)
// 4) if you are using command line arguments with -D or -C flags then
//    call SetCacheArgs (or if for some other reason you want to modify the 
//    cache, do it now.
// 5) Finally call Configure
// 6) Let the user change values and go back to step 5
// 7) call Generate
//
// If your GUI allows the user to change the start & home directories then
// you must at a minimum redo steps 2 through 7. 
//


#ifndef cmake_h
#define cmake_h

#include "cmSystemTools.h"

class cmGlobalGenerator;
class cmLocalGenerator;
class cmCacheManager;
class cmMakefile;
class cmCommand;
class cmVariableWatch;
class cmFileTimeComparison;

class cmake
{
 public:
  typedef std::map<cmStdString, cmCommand*> RegisteredCommandsMap;

  ///! construct an instance of cmake
  cmake();
  ///! destruct an instance of cmake
  ~cmake();

  /**
   * Return major and minor version numbers for cmake.
   */
  static unsigned int GetMajorVersion(); 
  static unsigned int GetMinorVersion(); 
  static const char *GetReleaseVersion();

  ///! construct an instance of cmake
  static const char *GetCMakeFilesDirectory() {return "/CMakeFiles";};
  static const char *GetCMakeFilesDirectoryPostSlash() {
    return "CMakeFiles/";};
  
  //@{
  /**
   * Set/Get the home directory (or output directory) in the project. The
   * home directory is the top directory of the project. It is where
   * cmake was run. Remember that CMake processes
   * CMakeLists files by recursing up the tree starting at the StartDirectory
   * and going up until it reaches the HomeDirectory.  
   */
  void SetHomeDirectory(const char* dir);
  const char* GetHomeDirectory() const
    {
    return this->cmHomeDirectory.c_str();
    }
  void SetHomeOutputDirectory(const char* lib);
  const char* GetHomeOutputDirectory() const
    {
    return this->HomeOutputDirectory.c_str();
    }
  //@}

  //@{
  /**
   * Set/Get the start directory (or output directory). The start directory
   * is the directory of the CMakeLists.txt file that started the current
   * round of processing. Remember that CMake processes CMakeLists files by
   * recursing up the tree starting at the StartDirectory and going up until
   * it reaches the HomeDirectory.  
   */
  void SetStartDirectory(const char* dir) 
    {
      this->cmStartDirectory = dir;
      cmSystemTools::ConvertToUnixSlashes(this->cmStartDirectory);
    }
  const char* GetStartDirectory() const
    {
      return this->cmStartDirectory.c_str();
    }
  void SetStartOutputDirectory(const char* lib)
    {
      this->StartOutputDirectory = lib;
      cmSystemTools::ConvertToUnixSlashes(this->StartOutputDirectory);
    }
  const char* GetStartOutputDirectory() const
    {
      return this->StartOutputDirectory.c_str();
    }
  //@}

  /**
   * Dump documentation to a file. If 0 is returned, the
   * operation failed.
   */
  int DumpDocumentationToFile(std::ostream&);

  /**
   * Handle a command line invocation of cmake.
   */
  int Run(const std::vector<std::string>&args)
    { return this->Run(args, false); }
  int Run(const std::vector<std::string>&args, bool noconfigure);

  /**
   * Generate the SourceFilesList from the SourceLists. This should only be
   * done once to be safe.  The argument is a list of command line
   * arguments.  The first argument should be the name or full path
   * to the command line version of cmake.  For building a GUI,
   * you would pass in the following arguments:
   * /path/to/cmake -H/path/to/source -B/path/to/build 
   * If you only want to parse the CMakeLists.txt files,
   * but not actually generate the makefiles, use buildMakefiles = false.
   */
  int Generate();

  /**
   * Configure the cmMakefiles. This routine will create a GlobalGenerator if
   * one has not already been set. It will then Call Configure on the
   * GlobalGenerator. This in turn will read in an process all the CMakeList
   * files for the tree. It will not produce any actual Makefiles, or
   * workspaces. Generate does that.  */
  int Configure();

  /**
   * Configure the cmMakefiles. This routine will create a GlobalGenerator if
   * one has not already been set. It will then Call Configure on the
   * GlobalGenerator. This in turn will read in an process all the CMakeList
   * files for the tree. It will not produce any actual Makefiles, or
   * workspaces. Generate does that.  */
  int LoadCache();
  void PreLoadCMakeFiles();

  ///! Create a GlobalGenerator
  cmGlobalGenerator* CreateGlobalGenerator(const char* name);

  ///! Return the global generator assigned to this instance of cmake
  cmGlobalGenerator* GetGlobalGenerator() { return this->GlobalGenerator; };

  ///! Return the global generator assigned to this instance of cmake
  void SetGlobalGenerator(cmGlobalGenerator *);

  ///! Get the names of the current registered generators
  void GetRegisteredGenerators(std::vector<std::string>& names);

  ///! get the cmCachemManager used by this invocation of cmake
  cmCacheManager *GetCacheManager() { return this->CacheManager; }
  
  ///! set the cmake command this instance of cmake should use
  void SetCMakeCommand(const char* cmd) { this->CMakeCommand = cmd; }
  
  /**
   * Given a variable name, return its value (as a string).
   */
  const char* GetCacheDefinition(const char*) const;
  ///! Add an entry into the cache
  void AddCacheEntry(const char* key, const char* value, 
                     const char* helpString, 
                     int type);
  /** 
   * Execute commands during the build process. Supports options such
   * as echo, remove file etc.
   */
  static int ExecuteCMakeCommand(std::vector<std::string>&);

  /**
   * Add a command to this cmake instance
   */
  void AddCommand(cmCommand* );
  void RenameCommand(const char* oldName, const char* newName);

  /**
   * Get a command by its name
   */
  cmCommand *GetCommand(const char *name);

  /** Get list of all commands */
  RegisteredCommandsMap* GetCommands() { return &this->Commands; }

  /** Check if a command exists. */
  bool CommandExists(const char* name) const;
    
  ///! Parse command line arguments
  void SetArgs(const std::vector<std::string>&);

  ///! Is this cmake running as a result of a TRY_COMPILE command
  bool GetIsInTryCompile() { return this->InTryCompile; }
  
  ///! Is this cmake running as a result of a TRY_COMPILE command
  void SetIsInTryCompile(bool i) { this->InTryCompile = i; }
  
  ///! Parse command line arguments that might set cache values
  bool SetCacheArgs(const std::vector<std::string>&);

  typedef  void (*ProgressCallbackType)
    (const char*msg, float progress, void *);
  /**
   *  Set the function used by GUI's to receive progress updates
   *  Function gets passed: message as a const char*, a progress 
   *  amount ranging from 0 to 1.0 and client data. The progress
   *  number provided may be negative in cases where a message is 
   *  to be displayed without any progress percentage.
   */
  void SetProgressCallback(ProgressCallbackType f, void* clientData=0);

  ///! this is called by generators to update the progress
  void UpdateProgress(const char *msg, float prog);


  ///! Get the variable watch object
  cmVariableWatch* GetVariableWatch() { return this->VariableWatch; }

  void GetCommandDocumentation(std::vector<cmDocumentationEntry>&) const;
  void GetGeneratorDocumentation(std::vector<cmDocumentationEntry>&);

  ///! Do all the checks before running configure
  int DoPreConfigureChecks();

  /**
   * Set and get the script mode option. In script mode there is no
   * generator and no cache. Also, language are not enabled, so
   * add_executable and things do not do anything.
   */
  void SetScriptMode(bool mode) { this->ScriptMode = mode; }
  bool GetScriptMode() { return this->ScriptMode; }
  
  ///! Debug the try compile stuff by not delelting the files
  bool GetDebugTryCompile(){return this->DebugTryCompile;}
  void DebugTryCompileOn(){this->DebugTryCompile = true;}

  ///! Get the list of files written by CMake using FILE(WRITE / WRITE_FILE
  void AddWrittenFile(const char* file);
  bool HasWrittenFile(const char* file);
  void CleanupWrittenFiles();

  /**
   * Generate CMAKE_ROOT and CMAKE_COMMAND cache entries
   */
  int AddCMakePaths(const char *arg0);

  /**
   * Get the file comparison class
   */
  cmFileTimeComparison* GetFileComparison() { return this->FileComparison; }

  /**
   * Get the path to ctest
   */
  const char* GetCTestCommand();
  const char* GetCPackCommand();
  const char* GetCMakeCommand() { return this->CMakeCommand.c_str(); }

  // Do we want debug output during the cmake run.
  bool GetDebugOutput() { return this->DebugOutput; }
  void DebugOutputOn() { this->DebugOutput = true;}

protected:
  typedef cmGlobalGenerator* (*CreateGeneratorFunctionType)();
  typedef std::map<cmStdString,
                   CreateGeneratorFunctionType> RegisteredGeneratorsMap;
  RegisteredCommandsMap Commands;
  RegisteredGeneratorsMap Generators;
  void AddDefaultCommands();
  void AddDefaultGenerators();

  cmGlobalGenerator *GlobalGenerator;
  cmCacheManager *CacheManager;
  std::string cmHomeDirectory; 
  std::string HomeOutputDirectory;
  std::string cmStartDirectory; 
  std::string StartOutputDirectory;

  std::set<cmStdString> WrittenFiles;

  ///! return true if the same cmake was used to make the cache.
  bool CacheVersionMatches();
  ///! read in a cmake list file to initialize the cache
  void ReadListFile(const char *path);

  ///! Check if CMAKE_CACHEFILE_DIR is set. If it is not, delete the log file.
  ///  If it is set, truncate it to 50kb
  void TruncateOutputLog(const char* fname);
  
  /**
   * Method called to check build system integrity at build time.
   * Returns 1 if CMake should rerun and 0 otherwise.
   */
  int CheckBuildSystem();

  void SetDirectoriesFromFile(const char* arg);

  //! Make sure all commands are what they say they are and there is no
  //macros.
  void CleanupCommandsAndMacros();

  void GenerateGraphViz(const char* fileName);

  static int ExecuteEchoColor(std::vector<std::string>& args);
  static int ExecuteLinkScript(std::vector<std::string>& args);
  
  cmVariableWatch* VariableWatch;

private:
  ProgressCallbackType ProgressCallback;
  void* ProgressCallbackClientData;
  bool Verbose;
  bool InTryCompile;
  bool ScriptMode;
  bool DebugOutput;
  std::string CMakeCommand;
  std::string CXXEnvironment;
  std::string CCEnvironment;
  std::string CheckBuildSystemArgument;
  std::string CTestCommand;
  std::string CPackCommand;
  bool ClearBuildSystem;
  bool DebugTryCompile;
  cmFileTimeComparison* FileComparison;
  std::string GraphVizFile;
  
  void UpdateConversionPathTable();
};

#define CMAKE_STANDARD_OPTIONS_TABLE \
  {"-C <initial-cache>", "Pre-load a script to populate the cache.", \
   "When cmake is first run in an empty build tree, it creates a " \
   "CMakeCache.txt file and populates it with customizable settings " \
   "for the project.  This option may be used to specify a file from " \
   "which to load cache entries before the first pass through " \
   "the project's cmake listfiles.  The loaded entries take priority " \
   "over the project's default values.  The given file should be a CMake " \
   "script containing SET commands that use the CACHE option, " \
   "not a cache-format file."}, \
  {"-D <var>:<type>=<value>", "Create a cmake cache entry.", \
   "When cmake is first run in an empty build tree, it creates a " \
   "CMakeCache.txt file and populates it with customizable settings " \
   "for the project.  This option may be used to specify a setting " \
   "that takes priority over the project's default value.  The option " \
   "may be repeated for as many cache entries as desired."}, \
  {"-G <generator-name>", "Specify a makefile generator.", \
   "CMake may support multiple native build systems on certain platforms.  " \
   "A makefile generator is responsible for generating a particular build " \
   "system.  Possible generator names are specified in the Generators " \
   "section."}

#define CMAKE_STANDARD_INTRODUCTION \
  {0, \
   "CMake is a cross-platform build system generator.  Projects " \
   "specify their build process with platform-independent CMake listfiles " \
   "included in each directory of a source tree with the name " \
   "CMakeLists.txt. " \
   "Users build a project by using CMake to generate a build system " \
   "for a native tool on their platform.", 0}
#endif