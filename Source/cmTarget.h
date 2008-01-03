/*=========================================================================

  Program:   CMake - Cross-Platform Makefile Generator
  Module:    $RCSfile: cmTarget.h,v $
  Language:  C++
  Date:      $Date: 2007/02/05 18:21:32 $
  Version:   $Revision: 1.62.2.4 $

  Copyright (c) 2002 Kitware, Inc., Insight Consortium.  All rights reserved.
  See Copyright.txt or http://www.cmake.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef cmTarget_h
#define cmTarget_h

#include "cmCustomCommand.h"

class cmMakefile;
class cmSourceFile;
class cmGlobalGenerator;

/** \class cmTarget
 * \brief Represent a library or executable target loaded from a makefile.
 *
 * cmTarget represents a target loaded from 
 * a makefile.
 */
class cmTarget
{
public:
  cmTarget();
  enum TargetType { EXECUTABLE, STATIC_LIBRARY,
                    SHARED_LIBRARY, MODULE_LIBRARY, UTILITY, GLOBAL_TARGET,
                    INSTALL_FILES, INSTALL_PROGRAMS, INSTALL_DIRECTORY};
  static const char* TargetTypeNames[];
  enum CustomCommandType { PRE_BUILD, PRE_LINK, POST_BUILD };

  /**
   * Return the type of target.
   */
  TargetType GetType()
    {
      return this->TargetTypeValue;
    }
  
  /**
   * Set the target type
   */
  void SetType(TargetType f, const char* name);

  ///! Set/Get the name of the target
  const char* GetName() {return this->Name.c_str();}

  /**
   * Indicate whether the target is part of the all target
   */
  bool IsInAll() { return this->GetPropertyAsBool("IN_ALL"); }
  bool GetInAll() { return this->GetPropertyAsBool("IN_ALL"); }
  void SetInAll(bool f) { 
    this->SetProperty("IN_ALL", (f) ?"TRUE" : "FALSE"); }

  ///! Set the cmMakefile that owns this target
  void SetMakefile(cmMakefile *mf);
  cmMakefile *GetMakefile() { return this->Makefile;};
  
  /**
   * Get the list of the custom commands for this target
   */
  std::vector<cmCustomCommand> &GetPreBuildCommands() 
    {return this->PreBuildCommands;}
  std::vector<cmCustomCommand> &GetPreLinkCommands() 
    {return this->PreLinkCommands;}
  std::vector<cmCustomCommand> &GetPostBuildCommands() 
    {return this->PostBuildCommands;}

  /**
   * Get the list of the source lists used by this target
   */
  std::vector<std::string> &GetSourceLists() {return this->SourceLists;}

  ///! Return the list of frameworks being linked to this target
  std::vector<std::string> &GetFrameworks() {return this->Frameworks;}
  
  /**
   * Get the list of the source files used by this target
   */
  std::vector<cmSourceFile*> &GetSourceFiles() {return this->SourceFiles;}

  /**
   * Get the list of the source files used by this target
   */
  enum LinkLibraryType {GENERAL, DEBUG, OPTIMIZED};

  typedef std::vector<std::pair<std::string,LinkLibraryType> > 
  LinkLibraryVectorType;
  const LinkLibraryVectorType &GetLinkLibraries() {
    return this->LinkLibraries;}
  const LinkLibraryVectorType &GetOriginalLinkLibraries() 
    {return this->OriginalLinkLibraries;}
  
  /**
   * Clear the dependency information recorded for this target, if any.
   */
  void ClearDependencyInformation(cmMakefile& mf, const char* target);

  // Check to see if a library is a framework and treat it different on Mac
  bool AddFramework(const std::string& lib, LinkLibraryType llt);
  void AddLinkLibrary(cmMakefile& mf,
                      const char *target, const char* lib, 
                      LinkLibraryType llt);

  void AddLinkLibrary(const std::string& lib, 
                      LinkLibraryType llt);

  void MergeLinkLibraries( cmMakefile& mf, const char* selfname, 
                           const LinkLibraryVectorType& libs );

  const std::vector<std::string>& GetLinkDirectories();

  void AddLinkDirectory(const char* d);

  /**
   * Set the path where this target should be installed. This is relative to
   * INSTALL_PREFIX
   */
  std::string GetInstallPath() {return this->InstallPath;}
  void SetInstallPath(const char *name) {this->InstallPath = name;}
  
  /**
   * Set the path where this target (if it has a runtime part) should be
   * installed. This is relative to INSTALL_PREFIX
   */
  std::string GetRuntimeInstallPath() {return this->RuntimeInstallPath;}
  void SetRuntimeInstallPath(const char *name) {
    this->RuntimeInstallPath = name;}

  /**
   * Get/Set whether there is an install rule for this target.
   */
  bool GetHaveInstallRule() { return this->HaveInstallRule; }
  void SetHaveInstallRule(bool h) { this->HaveInstallRule = h; }

  /**
   * Generate the SourceFilesList from the SourceLists. This should only be
   * done once to be safe.  
   */
  void GenerateSourceFilesFromSourceLists(cmMakefile &mf);

  /** Add a utility on which this project depends. A utility is an executable
   * name as would be specified to the ADD_EXECUTABLE or UTILITY_SOURCE
   * commands. It is not a full path nor does it have an extension.  
   */
  void AddUtility(const char* u) { this->Utilities.insert(u);}
  ///! Get the utilities used by this target
  std::set<cmStdString>const& GetUtilities() { return this->Utilities; }

  void AnalyzeLibDependencies( const cmMakefile& mf );

  ///! Set/Get a property of this target file
  void SetProperty(const char *prop, const char *value);
  const char *GetProperty(const char *prop);
  bool GetPropertyAsBool(const char *prop);

  /** Get the directory in which this target will be built.  If the
      configuration name is given then the generator will add its
      subdirectory for that configuration.  Otherwise just the canonical
      output directory is given.  */
  const char* GetDirectory(const char* config = 0);

  /** Get the location of the target in the build tree for the given
      configuration.  This location is suitable for use as the LOCATION
      target property.  */
  const char* GetLocation(const char* config);

  /** Get the target major and minor version numbers interpreted from
      the VERSION property.  Version 0 is returned if the property is
      not set or cannot be parsed.  */
  void GetTargetVersion(int& major, int& minor);

  /**
   * Trace through the source files in this target and add al source files
   * that they depend on, used by the visual studio generators
   */
  void TraceVSDependencies(std::string projName, cmMakefile *mf);  

  ///! Return the prefered linker language for this target
  const char* GetLinkerLanguage(cmGlobalGenerator*);
  
  ///! Return the rule variable used to create this type of target, 
  //  need to add CMAKE_(LANG) for full name.
  const char* GetCreateRuleVariable();

  /** Get the full name of the target according to the settings in its
      makefile.  */
  std::string GetFullName(const char* config=0, bool implib = false);
  void GetFullName(std::string& prefix, 
                   std::string& base, std::string& suffix,
                   const char* config=0, bool implib = false);

  /** Get the name of the pdb file for the target.  */
  std::string GetPDBName(const char* config=0);
  
  /** Get the full path to the target according to the settings in its
      makefile and the configuration type.  */
  std::string GetFullPath(const char* config=0, bool implib = false);

  /** Get the names of the library needed to generate a build rule
      that takes into account shared library version numbers.  This
      should be called only on a library target.  */
  void GetLibraryNames(std::string& name, std::string& soName,
                       std::string& realName, std::string& impName,
                       std::string& pdbName, const char* config);

  /** Get the names of the library used to remove existing copies of
      the library from the build tree either before linking or during
      a clean step.  This should be called only on a library
      target.  */
  void GetLibraryCleanNames(std::string& staticName,
                            std::string& sharedName,
                            std::string& sharedSOName,
                            std::string& sharedRealName,
                            std::string& importName,
                            std::string& pdbName,
                            const char* config);

  /** Get the names of the executable needed to generate a build rule
      that takes into account executable version numbers.  This should
      be called only on an executable target.  */
  void GetExecutableNames(std::string& name, std::string& realName,
                          std::string& pdbName, const char* config);

  /** Get the names of the executable used to remove existing copies
      of the executable from the build tree either before linking or
      during a clean step.  This should be called only on an
      executable target.  */
  void GetExecutableCleanNames(std::string& name, std::string& realName,
                               std::string& pdbName, const char* config);

  /**
   * Compute whether this target must be relinked before installing.
   */
  bool NeedRelinkBeforeInstall();

  bool HaveBuildTreeRPATH();
  bool HaveInstallTreeRPATH();

  std::string GetInstallNameDirForBuildTree(const char* config);
  std::string GetInstallNameDirForInstallTree(const char* config);

private:
  /**
   * A list of direct dependencies. Use in conjunction with DependencyMap.
   */
  typedef std::vector<cmStdString> DependencyList;

  /**
   * This map holds the dependency graph. map[x] returns a set of
   * direct dependencies of x. Note that the direct depenencies are
   * ordered. This is necessary to handle direct dependencies that
   * themselves have no dependency information.
   */
  typedef std::map< cmStdString, std::vector< cmStdString > > DependencyMap;

  /**
   * Maps a library name to its internal structure
   */
  typedef std::map< cmStdString, std::pair<cmStdString,LinkLibraryType> >
  LibTypeMap;

  /**
   * Inserts \a dep at the end of the dependency list of \a lib.
   */
  void InsertDependency( DependencyMap& depMap,
                         const cmStdString& lib,
                         const cmStdString& dep );

  /*
   * Deletes \a dep from the dependency list of \a lib.
   */
  void DeleteDependency( DependencyMap& depMap,
                         const cmStdString& lib,
                         const cmStdString& dep );

  /**
   * Emits the library \a lib and all its dependencies into link_line.
   * \a emitted keeps track of the libraries that have been emitted to
   * avoid duplicates--it is more efficient than searching
   * link_line. \a visited is used detect cycles. Note that \a
   * link_line is in reverse order, in that the dependencies of a
   * library are listed before the library itself.
   */
  void Emit( const std::string& lib,
             const DependencyMap& dep_map,
             std::set<cmStdString>& emitted,
             std::set<cmStdString>& visited,
             std::vector<std::string>& link_line );

  /**
   * Finds the dependencies for \a lib and inserts them into \a
   * dep_map.
   */
  void GatherDependencies( const cmMakefile& mf, const std::string& lib,
                           DependencyMap& dep_map ); 

  const char* GetSuffixVariableInternal(TargetType type, bool implib);
  const char* GetPrefixVariableInternal(TargetType type, bool implib);
  std::string GetFullNameInternal(TargetType type, const char* config,
                                  bool implib);
  void GetFullNameInternal(TargetType type, const char* config, bool implib,
                           std::string& outPrefix, std::string& outBase,
                           std::string& outSuffix);
  void GetLibraryNamesInternal(std::string& name,
                               std::string& soName,
                               std::string& realName,
                               std::string& impName,
                               std::string& pdbName,
                               TargetType type,
                               const char* config);
  void GetExecutableNamesInternal(std::string& name,
                                  std::string& realName,
                                  std::string& pdbName,
                                  TargetType type,
                                  const char* config);

  // Use a makefile variable to set a default for the given property.
  // If the variable is not defined use the given default instead.
  void SetPropertyDefault(const char* property, const char* default_value);
private:
  std::string Name;
  std::vector<cmCustomCommand> PreBuildCommands;
  std::vector<cmCustomCommand> PreLinkCommands;
  std::vector<cmCustomCommand> PostBuildCommands;
  std::vector<std::string> SourceLists;
  TargetType TargetTypeValue;
  std::vector<cmSourceFile*> SourceFiles;
  LinkLibraryVectorType LinkLibraries;
  LinkLibraryVectorType PrevLinkedLibraries;
  bool LinkLibrariesAnalyzed;
  bool LinkDirectoriesComputed;
  std::vector<std::string> Frameworks;
  std::vector<std::string> LinkDirectories;
  std::vector<std::string> ExplicitLinkDirectories;
  bool HaveInstallRule;
  std::string InstallPath;
  std::string RuntimeInstallPath;
  std::string Directory;
  std::string Location;
  std::set<cmStdString> Utilities;
  bool RecordDependencies; 
  std::map<cmStdString,cmStdString> Properties;
  LinkLibraryVectorType OriginalLinkLibraries;

  // The cmMakefile instance that owns this target.  This should
  // always be set.
  cmMakefile* Makefile;
};

typedef std::map<cmStdString,cmTarget> cmTargets;

class cmTargetSet: public std::set<cmStdString> {};
class cmTargetManifest: public std::map<cmStdString, cmTargetSet> {};

#endif