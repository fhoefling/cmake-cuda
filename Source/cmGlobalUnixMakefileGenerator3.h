/*=========================================================================

  Program:   CMake - Cross-Platform Makefile Generator3
  Module:    $RCSfile: cmGlobalUnixMakefileGenerator3.h,v $
  Language:  C++
  Date:      $Date: 2007/01/03 15:19:03 $
  Version:   $Revision: 1.27.2.6 $

  Copyright (c) 2005 Kitware, Inc., Insight Consortium.  All rights reserved.
  See Copyright.txt or http://www.cmake.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef cmGlobalUnixMakefileGenerator3_h
#define cmGlobalUnixMakefileGenerator3_h

#include "cmGlobalGenerator.h"

class cmGeneratedFileStream;
class cmLocalUnixMakefileGenerator3;

/** \class cmGlobalUnixMakefileGenerator3
 * \brief Write a Unix makefiles.
 *
 * cmGlobalUnixMakefileGenerator3 manages UNIX build process for a tree
 
 
 The basic approach of this generator is to produce Makefiles that will all
 be run with the current working directory set to the Home Output
 directory. The one exception to this is the subdirectory Makefiles which are
 created as a convenience and just cd up to the Home Output directory and
 invoke the main Makefiles. 
 
 The make process starts with Makefile. Makefile should only contain the
 targets the user is likely to invoke directly from a make command line. No
 internal targets should be in this file. Makefile2 contains the internal
 targets that are required to make the process work.
 
 Makefile2 in turn will recursively make targets in the correct order. Each
 target has its own directory <target>.dir and its own makefile build.make in
 that directory. Also in that directory is a couple makefiles per source file
 used by the target. Typically these are named source.obj.build.make and
 source.obj.build.depend.make. The source.obj.build.make contains the rules
 for building, cleaning, and computing dependencies for the given source
 file. The build.depend.make contains additional dependencies that were
 computed during dependency scanning. An additional file called
 source.obj.depend is used as a marker to indicate when dependencies must be
 rescanned.

 Rules for custom commands follow the same model as rules for source files.
 
 */

class cmGlobalUnixMakefileGenerator3 : public cmGlobalGenerator
{
public:
  cmGlobalUnixMakefileGenerator3();
  static cmGlobalGenerator* New() {
    return new cmGlobalUnixMakefileGenerator3; }

  ///! Get the name for the generator.
  virtual const char* GetName() const {
    return cmGlobalUnixMakefileGenerator3::GetActualName();}
  static const char* GetActualName() {return "Unix Makefiles";}

  /** Get the documentation entry for this generator.  */
  virtual void GetDocumentation(cmDocumentationEntry& entry) const;
  
  ///! Create a local generator appropriate to this Global Generator3
  virtual cmLocalGenerator *CreateLocalGenerator();

  /**
   * Try to determine system infomation such as shared library
   * extension, pthreads, byte order etc.  
   */
  virtual void EnableLanguage(std::vector<std::string>const& languages,
                              cmMakefile *);

  /**
   * Generate the all required files for building this project/tree. This
   * basically creates a series of LocalGenerators for each directory and
   * requests that they Generate.  
   */
  virtual void Generate();
  
  
  void WriteMainCMakefileLanguageRules(cmGeneratedFileStream& cmakefileStream,
                                       std::vector<cmLocalGenerator *> &);

  // write out the help rule listing the valid targets
  void WriteHelpRule(std::ostream& ruleFileStream,
                     cmLocalUnixMakefileGenerator3 *);

  // write the top lvel target rules
  void WriteConvenienceRules(std::ostream& ruleFileStream, 
                             std::set<cmStdString> &emitted);

  /** In order to support parallel builds for custom commands with
      multiple outputs the outputs are given a serial order, and only
      the first output actually has the build rule.  Other outputs
      just depend on the first one.  The check-build-system step must
      remove a dependee if the depender is missing to make sure both
      are regenerated properly.  This method is used by the local
      makefile generators to register such pairs.  */
  void AddMultipleOutputPair(const char* depender, const char* dependee);

  /** Support for multiple custom command outputs.  Called during
      check-build-system step.  */
  virtual void CheckMultipleOutputs(cmMakefile* mf, bool verbose);

  /** Get the command to use for a non-symbolic target file that has
      no rule.  This is used for multiple output dependencies.  */
  std::string GetEmptyCommandHack() { return this->EmptyCommandsHack; }

  /** Get the fake dependency to use when a rule has no real commands
      or dependencies.  */
  std::string GetEmptyRuleHackDepends() { return this->EmptyRuleHackDepends; }

  // change the build command for speed
  virtual std::string GenerateBuildCommand
  (const char* makeProgram,
   const char *projectName, const char* additionalOptions, 
   const char *targetName,
   const char* config, bool ignoreErrors, bool fast);

  // returns some progress informaiton
  int GetTargetTotalNumberOfActions(cmTarget& target,
                                    std::set<cmStdString> &emitted);
  unsigned long GetNumberOfProgressActionsInAll
  (cmLocalUnixMakefileGenerator3 *lg);

  // what targets does the specified target depend on
  std::vector<cmTarget *>& GetTargetDepends(cmTarget& target);

protected:
  void WriteMainMakefile2();
  void WriteMainCMakefile();
  
  void WriteConvenienceRules2(std::ostream& ruleFileStream, 
                              cmLocalUnixMakefileGenerator3 *,
                              bool exclude);

  void WriteDirectoryRule2(std::ostream& ruleFileStream,
                           cmLocalUnixMakefileGenerator3* lg,
                           const char* pass, bool check_all,
                           bool check_relink);
  void WriteDirectoryRules2(std::ostream& ruleFileStream,
                            cmLocalUnixMakefileGenerator3* lg);

  void AppendGlobalTargetDepends(std::vector<std::string>& depends,
                                 cmTarget& target);
  void AppendAnyGlobalDepend(std::vector<std::string>& depends, 
                             const char* name, 
                             std::set<cmStdString>& emitted,
                             cmTarget &target);

  // does this generator need a requires step for any of its targets
  bool NeedRequiresStep(cmLocalUnixMakefileGenerator3 *lg, const char *);

  // Setup target names
  virtual const char* GetAllTargetName()          { return "all"; }
  virtual const char* GetInstallTargetName()      { return "install"; }
  virtual const char* GetInstallLocalTargetName() { return "install/local"; }
  virtual const char* GetPreinstallTargetName()   { return "preinstall"; }
  virtual const char* GetTestTargetName()         { return "test"; }
  virtual const char* GetPackageTargetName()      { return "package"; }
  virtual const char* GetPackageSourceTargetName(){ return "package_source"; }
  virtual const char* GetEditCacheTargetName()    { return "edit_cache"; }
  virtual const char* GetRebuildCacheTargetName() { return "rebuild_cache"; }

  // Some make programs (Borland) do not keep a rule if there are no
  // dependencies or commands.  This is a problem for creating rules
  // that might not do anything but might have other dependencies
  // added later.  If non-empty this variable holds a fake dependency
  // that can be added.
  std::string EmptyRuleHackDepends;

  // Some make programs (Watcom) do not like rules with no commands
  // for non-symbolic targets.  If non-empty this variable holds a
  // bogus command that may be put in the rule to satisfy the make
  // program.
  std::string EmptyCommandsHack;

  typedef std::map<cmStdString, cmStdString> MultipleOutputPairsType;
  MultipleOutputPairsType MultipleOutputPairs;

  std::map<cmStdString, std::vector<cmTarget *> > TargetDependencies;
  std::map<cmStdString, int > TargetSourceFileCount;
};

#endif