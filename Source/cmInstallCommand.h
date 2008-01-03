/*=========================================================================

  Program:   CMake - Cross-Platform Makefile Generator
  Module:    $RCSfile: cmInstallCommand.h,v $
  Language:  C++
  Date:      $Date: 2006/10/13 14:52:02 $
  Version:   $Revision: 1.9.2.3 $

  Copyright (c) 2002 Kitware, Inc., Insight Consortium.  All rights reserved.
  See Copyright.txt or http://www.cmake.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef cmInstallCommand_h
#define cmInstallCommand_h

#include "cmCommand.h"

/** \class cmInstallCommand
 * \brief Specifies where to install some files
 *
 * cmInstallCommand is a general-purpose interface command for
 * specifying install rules.
 */
class cmInstallCommand : public cmCommand
{
public:
  /**
   * This is a virtual constructor for the command.
   */
  virtual cmCommand* Clone()
    {
    return new cmInstallCommand;
    }

  /**
   * This is called when the command is first encountered in
   * the CMakeLists.txt file.
   */
  virtual bool InitialPass(std::vector<std::string> const& args);

  /**
   * The name of the command as specified in CMakeList.txt.
   */
  virtual const char* GetName() { return "INSTALL";}

  /**
   * Succinct documentation.
   */
  virtual const char* GetTerseDocumentation()
    {
    return "Specify rules to run at install time.";
    }

  /**
   * More documentation.
   */
  virtual const char* GetFullDocumentation()
    {
    return
      "This command generates installation rules for a project.  "
      "Rules specified by calls to this command within a source directory "
      "are executed in order during installation.  "
      "The order across directories is not defined."
      "\n"
      "There are multiple signatures for this command.  Some of them define "
      "installation properties for files and targets.  Properties common to "
      "multiple signatures are covered here but they are valid only for "
      "signatures that specify them.  "
      "DESTINATION arguments specify "
      "the directory on disk to which a file will be installed.  "
      "If a full path (with a leading slash or drive letter) is given it "
      "is used directly.  If a relative path is given it is interpreted "
      "relative to the value of CMAKE_INSTALL_PREFIX.  "
      "PERMISSIONS arguments specify permissions for installed files.  "
      "Valid permissions are "
      "OWNER_READ, OWNER_WRITE, OWNER_EXECUTE, "
      "GROUP_READ, GROUP_WRITE, GROUP_EXECUTE, "
      "WORLD_READ, WORLD_WRITE, WORLD_EXECUTE, "
      "SETUID, and SETGID.  "
      "Permissions that do not make sense on certain platforms are ignored "
      "on those platforms.  "
      "The CONFIGURATIONS argument specifies a list of build configurations "
      "for which the install rule applies (Debug, Release, etc.).  "
      "The COMPONENT argument specifies an installation component name "
      "with which the install rule is associated, such as \"runtime\" or "
      "\"development\".  During component-specific installation only "
      "install rules associated with the given component name will be "
      "executed.  During a full installation all components are installed.  "
      "The RENAME argument specifies a name for an installed file that "
      "may be different from the original file.  Renaming is allowed only "
      "when a single file is installed by the command.  "
      "The OPTIONAL argument specifies that it is not an error if the "
      "file to be installed does not exist.  "
      "\n"
      "The TARGETS signature:\n"
      "  INSTALL(TARGETS targets...\n"
      "          [[ARCHIVE|LIBRARY|RUNTIME]\n"
      "                              [DESTINATION <dir>]\n"
      "                              [PERMISSIONS permissions...]\n"
      "                              [CONFIGURATIONS [Debug|Release|...]]\n"
      "                              [COMPONENT <component>]\n"
      "                              [OPTIONAL]\n"
      "                             ] [...])\n"
      "The TARGETS form specifies rules for installing targets from a "
      "project.  There are three kinds of target files that may be "
      "installed: archive, library, and runtime.  "

      "Executables are always treated as runtime targets. "
      "Static libraries are always treated as archive targets. "
      "Module libraries are always treated as library targets. "
      "For non-DLL platforms shared libraries are treated as library "
      "targets. "
      "For DLL platforms the DLL part of a shared library is treated as "
      "a runtime target and the corresponding import library is treated as "
      "an archive target. "
      "All Windows-based systems including Cygwin are DLL platforms. "
      "The ARCHIVE, LIBRARY, and RUNTIME "
      "arguments change the type of target to which the subsequent "
      "properties "
      "apply.  If none is given the installation properties apply to "
      "all target types.  If only one is given then only targets of that "
      "type will be installed (which can be used to install just a DLL or "
      "just an import library)."
      "\n"
      "One or more groups of properties may be specified in a single call "
      "to the TARGETS form of this command.  A target may be installed more "
      "than once to different locations.  Consider hypothetical "
      "targets \"myExe\", \"mySharedLib\", and \"myStaticLib\".  The code\n"
      "    INSTALL(TARGETS myExe mySharedLib myStaticLib\n"
      "            RUNTIME DESTINATION bin\n"
      "            LIBRARY DESTINATION lib\n"
      "            ARCHIVE DESTINATION lib/static)\n"
      "    INSTALL(TARGETS mySharedLib DESTINATION /some/full/path)\n"
      "will install myExe to <prefix>/bin and myStaticLib to "
      "<prefix>/lib/static.  "
      "On non-DLL platforms mySharedLib will be installed to <prefix>/lib "
      "and /some/full/path.  On DLL platforms the mySharedLib DLL will be "
      "installed to <prefix>/bin and /some/full/path and its import library "
      "will be installed to <prefix>/lib/static and /some/full/path. "
      "On non-DLL platforms mySharedLib will be installed to <prefix>/lib "
      "and /some/full/path."
      "\n"
      "The FILES signature:\n"
      "  INSTALL(FILES files... DESTINATION <dir>\n"
      "          [PERMISSIONS permissions...]\n"
      "          [CONFIGURATIONS [Debug|Release|...]]\n"
      "          [COMPONENT <component>]\n"
      "          [RENAME <name>] [OPTIONAL])\n"
      "The FILES form specifies rules for installing files for a "
      "project.  File names given as relative paths are interpreted with "
      "respect to the current source directory.  Files installed by this "
      "form are by default given permissions OWNER_WRITE, OWNER_READ, "
      "GROUP_READ, and WORLD_READ if no PERMISSIONS argument is given."
      "\n"
      "The PROGRAMS signature:\n"
      "  INSTALL(PROGRAMS files... DESTINATION <dir>\n"
      "          [PERMISSIONS permissions...]\n"
      "          [CONFIGURATIONS [Debug|Release|...]]\n"
      "          [COMPONENT <component>]\n"
      "          [RENAME <name>] [OPTIONAL])\n"
      "The PROGRAMS form is identical to the FILES form except that the "
      "default permissions for the installed file also include "
      "OWNER_EXECUTE, GROUP_EXECUTE, and WORLD_EXECUTE.  "
      "This form is intended to install programs that are not targets, "
      "such as shell scripts.  Use the TARGETS form to install targets "
      "built within the project."
      "\n"
      "The DIRECTORY signature:\n"
      "  INSTALL(DIRECTORY dirs... DESTINATION <dir>\n"
      "          [FILE_PERMISSIONS permissions...]\n"
      "          [DIRECTORY_PERMISSIONS permissions...]\n"
      "          [USE_SOURCE_PERMISSIONS]\n"
      "          [CONFIGURATIONS [Debug|Release|...]]\n"
      "          [COMPONENT <component>]\n"
      "          [[PATTERN <pattern> | REGEX <regex>]\n"
      "           [EXCLUDE] [PERMISSIONS permissions...]] [...])\n"
      "The DIRECTORY form installs contents of one or more directories "
      "to a given destination.  "
      "The directory structure is copied verbatim to the destination.  "
      "The last component of each directory name is appended to the "
      "destination directory but a trailing slash may be used to "
      "avoid this because it leaves the last component empty.  "
      "Directory names given as relative paths are interpreted with "
      "respect to the current source directory.  "
      "If no input directory names are given the destination directory "
      "will be created but nothing will be installed into it.  "
      "The FILE_PERMISSIONS and DIRECTORY_PERMISSIONS options specify "
      "permissions given to files and directories in the destination.  "
      "If USE_SOURCE_PERMISSIONS is specified and FILE_PERMISSIONS is not, "
      "file permissions will be copied from the source directory structure.  "
      "If no permissions are specified files will be given the default "
      "permissions specified in the FILES form of the command, and the "
      "directories will be given the default permissions specified in the "
      "PROGRAMS form of the command.  "
      "The PATTERN and REGEX options specify a globbing pattern or regular "
      "expression to match directories or files encountered during traversal "
      "of an input directory.  The full path to an input file or directory "
      "(with forward slashes) is matched against the expression.  "
      "A PATTERN will match only complete file names: the portion of the "
      "full path matching the pattern must occur at the end of the file name "
      "and be preceded by a slash.  "
      "A REGEX will match any portion of the full path but it may use "
      "'/' and '$' to simulate the PATTERN behavior.  "
      "Options following one of these matching expressions "
      "are applied only to files or directories matching them.  "
      "The EXCLUDE option will skip the matched file or directory.  "
      "The PERMISSIONS option overrides the permissions setting for the "
      "matched file or directory.  "
      "For example the code\n"
      "  INSTALL(DIRECTORY icons scripts/ DESTINATION share/myproj\n"
      "          PATTERN \"CVS\" EXCLUDE\n"
      "          PATTERN \"scripts/*\"\n"
      "          PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ\n"
      "                      GROUP_EXECUTE GROUP_READ)\n"
      "will install the icons directory to share/myproj/icons and the "
      "scripts directory to share/myproj.  The icons will get default file "
      "permissions, the scripts will be given specific permissions, and "
      "any CVS directories will be excluded."
      "\n"
      "The SCRIPT and CODE signature:\n"
      "  INSTALL([[SCRIPT <file>] [CODE <code>]] [...])\n"
      "The SCRIPT form will invoke the given CMake script files during "
      "installation.  If the script file name is a relative path "
      "it will be interpreted with respect to the current source directory.  "
      "The CODE form will invoke the given CMake code during installation.  "
      "Code is specified as a single argument inside a double-quoted string.  "
      "For example, the code\n"
      "  INSTALL(CODE \"MESSAGE(\\\"Sample install message.\\\")\")\n"
      "will print a message during installation.\n"
      "NOTE: This command supercedes the INSTALL_TARGETS command and the "
      "target properties PRE_INSTALL_SCRIPT and POST_INSTALL_SCRIPT.  "
      "It also replaces the FILES forms of the INSTALL_FILES and "
      "INSTALL_PROGRAMS commands.  "
      "The processing order of these install rules relative to those "
      "generated by INSTALL_TARGETS, INSTALL_FILES, and INSTALL_PROGRAMS "
      "commands is not defined.\n"
      ;
    }

  cmTypeMacro(cmInstallCommand, cmCommand);

private:
  bool HandleScriptMode(std::vector<std::string> const& args);
  bool HandleTargetsMode(std::vector<std::string> const& args);
  bool HandleFilesMode(std::vector<std::string> const& args);
  bool HandleDirectoryMode(std::vector<std::string> const& args);
  void ComputeDestination(const char* destination, std::string& dest);
  bool CheckPermissions(std::string const& arg, std::string& permissions);
};


#endif