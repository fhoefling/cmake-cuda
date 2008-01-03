/*=========================================================================

  Program:   CMake - Cross-Platform Makefile Generator
  Module:    $RCSfile: cmGetCMakePropertyCommand.cxx,v $
  Language:  C++
  Date:      $Date: 2006/03/15 16:02:01 $
  Version:   $Revision: 1.6 $

  Copyright (c) 2002 Kitware, Inc., Insight Consortium.  All rights reserved.
  See Copyright.txt or http://www.cmake.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#include "cmGetCMakePropertyCommand.h"

#include "cmake.h"

// cmGetCMakePropertyCommand
bool cmGetCMakePropertyCommand::InitialPass(
  std::vector<std::string> const& args)
{
  if(args.size() < 2 )
    {
    this->SetError("called with incorrect number of arguments");
    return false;
    }
  
  std::vector<std::string>::size_type cc;
  std::string variable = args[0];
  std::string output = "";

  if ( args[1] == "VARIABLES" || args[1] == "CACHE_VARIABLES" )
    {
    int cacheonly = 0;
    if ( args[1] == "CACHE_VARIABLES" )
      {
      cacheonly = 1;
      }
    std::vector<std::string> vars = this->Makefile->GetDefinitions(cacheonly);
    for ( cc = 0; cc < vars.size(); cc ++ )
      {
      if ( cc > 0 )
        {
        output += ";";
        }
      output += vars[cc];
      }
    }
  else if ( args[1] == "COMMANDS" )
    {
    cmake::RegisteredCommandsMap::iterator cmds 
        = this->Makefile->GetCMakeInstance()->GetCommands()->begin();
    for (cc=0 ; 
      cmds != this->Makefile->GetCMakeInstance()->GetCommands()->end(); 
      ++ cmds )
      {
      if ( cc > 0 )
        {
        output += ";";
        }
      output += cmds->first.c_str();
      cc++;
      }
    }
  else if ( args[1] == "MACROS" )
    {
    this->Makefile->GetListOfMacros(output);
    }
  else
    {
    std::string emsg = "Unknown CMake property: " + args[1];
    this->SetError(emsg.c_str());
    return false;
    }
  this->Makefile->AddDefinition(variable.c_str(), output.c_str());
  
  return true;
}

