/*=========================================================================

  Program:   CMake - Cross-Platform Makefile Generator
  Module:    $RCSfile: cmIncludeCommand.cxx,v $
  Language:  C++
  Date:      $Date: 2006/05/11 20:05:57 $
  Version:   $Revision: 1.17.2.1 $

  Copyright (c) 2002 Kitware, Inc., Insight Consortium.  All rights reserved.
  See Copyright.txt or http://www.cmake.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#include "cmIncludeCommand.h"


// cmIncludeCommand
bool cmIncludeCommand::InitialPass(std::vector<std::string> const& args)
{
  if (args.size()< 1 || args.size() > 2)
    {
      this->SetError("called with wrong number of arguments.  "
                     "Include only takes one file.");
      return false;
    }
  bool optional = false;

  std::string fname = args[0].c_str();

  if(args.size() == 2)
    {
    optional = args[1] == "OPTIONAL";
    }
  
  if(!cmSystemTools::FileIsFullPath(fname.c_str()))
    {
    // Not a path. Maybe module.
    std::string module = fname;
    module += ".cmake";
    std::string mfile = this->Makefile->GetModulesFile(module.c_str());
    if ( mfile.size() )
      {
      fname = mfile.c_str();
      }
    }
  bool readit = 
    this->Makefile->ReadListFile( this->Makefile->GetCurrentListFile(), 
                                          fname.c_str() );
  if(!optional && !readit && !cmSystemTools::GetFatalErrorOccured())
    {
    std::string m = "Could not find include file: ";
    m += fname;
    this->SetError(m.c_str());
    return false;
    }
  return true;
}


