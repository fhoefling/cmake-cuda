/*=========================================================================

  Program:   CMake - Cross-Platform Makefile Generator
  Module:    $RCSfile: cmIncludeRegularExpressionCommand.cxx,v $
  Language:  C++
  Date:      $Date: 2006/05/11 20:05:58 $
  Version:   $Revision: 1.8.2.1 $

  Copyright (c) 2002 Kitware, Inc., Insight Consortium.  All rights reserved.
  See Copyright.txt or http://www.cmake.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#include "cmIncludeRegularExpressionCommand.h"

// cmIncludeRegularExpressionCommand
bool cmIncludeRegularExpressionCommand
::InitialPass(std::vector<std::string> const& args)
{
  if((args.size() < 1) || (args.size() > 2))
    {
    this->SetError("called with incorrect number of arguments");
    return false;
    }
  this->Makefile->SetIncludeRegularExpression(args[0].c_str());
  
  if(args.size() > 1)
    {
    this->Makefile->SetComplainRegularExpression(args[1].c_str());
    }
  
  return true;
}
