/*=========================================================================

  Program:   CMake - Cross-Platform Makefile Generator
  Module:    $RCSfile: cmakemain.cxx,v $
  Language:  C++
  Date:      $Date: 2006/10/13 14:52:06 $
  Version:   $Revision: 1.50.2.4 $

  Copyright (c) 2002 Kitware, Inc., Insight Consortium.  All rights reserved.
  See Copyright.txt or http://www.cmake.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#include "cmake.h"
#include "cmCacheManager.h"
#include "cmListFileCache.h"
#include "cmakewizard.h"

#ifdef CMAKE_BUILD_WITH_CMAKE
#include "cmDynamicLoader.h"
#include "cmDocumentation.h"

//----------------------------------------------------------------------------
static const cmDocumentationEntry cmDocumentationName[] =
{
  {0,
   "  cmake - Cross-Platform Makefile Generator.", 0},
  {0,0,0}
};

//----------------------------------------------------------------------------
static const cmDocumentationEntry cmDocumentationUsage[] =
{
  {0,
   "  cmake [options] <path-to-source>\n"
   "  cmake [options] <path-to-existing-build>", 0},
  {0,0,0}
};

//----------------------------------------------------------------------------
static const cmDocumentationEntry cmDocumentationDescription[] =
{
  {0,
   "The \"cmake\" executable is the CMake command-line interface.  It may "
   "be used to configure projects in scripts.  Project configuration "
   "settings "
   "may be specified on the command line with the -D option.  The -i option "
   "will cause cmake to interactively prompt for such settings.", 0},
  CMAKE_STANDARD_INTRODUCTION,
  {0,0,0}
};

//----------------------------------------------------------------------------
static const cmDocumentationEntry cmDocumentationOptions[] =
{
  CMAKE_STANDARD_OPTIONS_TABLE,
  {"-E", "CMake command mode.",
   "For true platform independence, CMake provides a list of commands "
   "that can be used on all systems. Run with -E help for the usage "
   "information."},
  {"-i", "Run in wizard mode.",
   "Wizard mode runs cmake interactively without a GUI.  The user is "
   "prompted to answer questions about the project configuration.  "
   "The answers are used to set cmake cache values."},
  {"-L[A][H]", "List non-advanced cached variables.",
   "List cache variables will run CMake and list all the variables from the "
   "CMake cache that are not marked as INTERNAL or ADVANCED. This will "
   "effectively display current CMake settings, which can be then changed "
   "with -D option. Changing some of the variable may result in more "
   "variables being created. If A is specified, then it will display also "
   "advanced variables. If H is specified, it will also display help for "
   "each variable."},
  {"-N", "View mode only.",
   "Only load the cache. Do not actually run configure and generate steps."},
  {"-P <file>", "Process script mode.",
   "Process the given cmake file as a script written in the CMake language.  "
   "No configure or generate step is performed and the cache is not"
   " modified."},
  {"--graphviz=[file]", "Generate graphviz of dependencies.",
   "Generate a graphviz input file that will contain all the library and "
   "executable dependencies in the project."},
  {"--debug-trycompile", "Do not delete the try compile directories..",
   "Do not delete the files and directories created for try_compile calls. "
   "This is useful in debugging failed try_compiles."},
  {"--debug-output", "Put cmake in a debug mode.",
   "Print extra stuff during the cmake run like stack traces with "
   "message(send_error ) calls."},
  {"--help-command cmd [file]", "Print help for a single command and exit.",
   "Full documentation specific to the given command is displayed."},
  {"--help-command-list [file]", "List available listfile commands and exit.",
   "The list contains all commands for which help may be obtained by using "
   "the --help-command argument followed by a command name.  If a file is "
   "specified, the help is written into it."},
  {"--help-module module [file]", "Print help for a single module and exit.",
   "Full documentation specific to the given module is displayed."},
  {"--help-module-list [file]", "List available modules and exit.",
   "The list contains all modules for which help may be obtained by using "
   "the --help-module argument followed by a module name.  If a file is "
   "specified, the help is written into it."},
  {0,0,0}
};

//----------------------------------------------------------------------------
static const cmDocumentationEntry cmDocumentationSeeAlso[] =
{
  {0, "ccmake", 0},
  {0, "ctest", 0},
  {0, 0, 0}
};

//----------------------------------------------------------------------------
static const cmDocumentationEntry cmDocumentationNOTE[] =
{
  {0,
   "CMake no longer configures a project when run with no arguments.  "
   "In order to configure the project in the current directory, run\n"
   "  cmake .", 0},
  {0,0,0}
};
#endif

int do_cmake(int ac, char** av);
void updateProgress(const char *msg, float prog, void *cd);

int main(int ac, char** av)
{
  cmSystemTools::EnableMSVCDebugHook();
  int ret = do_cmake(ac, av);
#ifdef CMAKE_BUILD_WITH_CMAKE
  cmDynamicLoader::FlushCache();
#endif
  return ret;
}

int do_cmake(int ac, char** av)
{
#ifdef CMAKE_BUILD_WITH_CMAKE
  cmDocumentation doc;
#endif
  int nocwd = 0;

  if ( cmSystemTools::GetCurrentWorkingDirectory().size() == 0 )
    {
    std::cerr << "Current working directory cannot be established." 
              << std::endl;
    nocwd = 1;
    }

#ifdef CMAKE_BUILD_WITH_CMAKE
  if(doc.CheckOptions(ac, av) || nocwd)
    { 
    // Construct and print requested documentation.
    cmake hcm;
    hcm.AddCMakePaths(av[0]);
    doc.SetCMakeRoot(hcm.GetCacheDefinition("CMAKE_ROOT"));
    std::vector<cmDocumentationEntry> commands;
    std::vector<cmDocumentationEntry> generators;
    hcm.GetCommandDocumentation(commands);
    hcm.GetGeneratorDocumentation(generators);
    doc.SetName("cmake");
    doc.SetNameSection(cmDocumentationName);
    doc.SetUsageSection(cmDocumentationUsage);
    doc.SetDescriptionSection(cmDocumentationDescription);
    doc.SetGeneratorsSection(&generators[0]);
    doc.SetOptionsSection(cmDocumentationOptions);
    doc.SetCommandsSection(&commands[0]);
    doc.SetSeeAlsoList(cmDocumentationSeeAlso);
    int result = doc.PrintRequestedDocumentation(std::cout)? 0:1;
    
    // If we were run with no arguments, but a CMakeLists.txt file
    // exists, the user may have been trying to use the old behavior
    // of cmake to build a project in-source.  Print a message
    // explaining the change to standard error and return an error
    // condition in case the program is running from a script.
    if((ac == 1) && cmSystemTools::FileExists("CMakeLists.txt"))
      {
      doc.ClearSections();
      doc.AddSection("NOTE", cmDocumentationNOTE);
      doc.Print(cmDocumentation::UsageForm, std::cerr);
      return 1;
      }
    return result;
    }
#else
  if ( nocwd || ac == 1 )
    {
    std::cout << 
      "Bootstrap CMake should not be used outside CMake build process."
              << std::endl;
    return 0;
    }
#endif
  
  bool wiz = false;
  bool command = false;
  bool list_cached = false;
  bool list_all_cached = false;
  bool list_help = false;
  bool view_only = false;
  bool script_mode = false;
  std::vector<std::string> args;
  for(int i =0; i < ac; ++i)
    {
    if(strcmp(av[i], "-i") == 0)
      {
      wiz = true;
      }
    // if command has already been set, then
    // do not eat the -E 
    else if (!command && strcmp(av[i], "-E") == 0)
      {
      command = true;
      }
    else if (strcmp(av[i], "-N") == 0)
      {
      view_only = true;
      }
    else if (strcmp(av[i], "-L") == 0)
      {
      list_cached = true;
      }
    else if (strcmp(av[i], "-LA") == 0)
      {
      list_all_cached = true;
      }
    else if (strcmp(av[i], "-LH") == 0)
      {
      list_cached = true;
      list_help = true;
      }
    else if (strcmp(av[i], "-LAH") == 0)
      {
      list_all_cached = true;
      list_help = true;
      }
    else if (strncmp(av[i], "-P", strlen("-P")) == 0)
      {
      if ( i == ac -1 )
        {
        cmSystemTools::Error("No script specified for argument -P");
        }
      else
        {
      script_mode = true;
      args.push_back(av[i]);
      i++;
      args.push_back(av[i]);
        }
      }
    else 
      {
      args.push_back(av[i]);
      }
    }

  if(command)
    {
    int ret = cmake::ExecuteCMakeCommand(args);
    return ret;
    }
  if (wiz)
    {
    cmakewizard wizard;
    return wizard.RunWizard(args); 
    }
  cmake cm;  
  cm.SetProgressCallback(updateProgress, 0);
  cm.SetScriptMode(script_mode);
  int res = cm.Run(args, view_only);
  if ( list_cached || list_all_cached )
    {
    cmCacheManager::CacheIterator it = 
      cm.GetCacheManager()->GetCacheIterator();
    std::cout << "-- Cache values" << std::endl;
    for ( it.Begin(); !it.IsAtEnd(); it.Next() )
      {
      cmCacheManager::CacheEntryType t = it.GetType();
      if ( t != cmCacheManager::INTERNAL && t != cmCacheManager::STATIC &&
        t != cmCacheManager::UNINITIALIZED )
        {
        bool advanced = it.PropertyExists("ADVANCED");
        if ( list_all_cached || !advanced)
          {
          if ( list_help )
            {
            std::cout << "// " << it.GetProperty("HELPSTRING") << std::endl;
            }
          std::cout << it.GetName() << ":" << 
            cmCacheManager::TypeToString(it.GetType()) 
            << "=" << it.GetValue() << std::endl;
          if ( list_help )
            {
            std::cout << std::endl;
            }
          }
        }
      }
    }
  return res;
}

void updateProgress(const char *msg, float prog, void*)
{
  if ( prog < 0 )
    {
    std::cout << "-- " << msg << std::endl;
    }
  //else
  //{
  //std::cout << "-- " << msg << " " << prog << std::endl;
  //}
}
