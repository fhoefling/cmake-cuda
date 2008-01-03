/*=========================================================================

  Program:   CMake - Cross-Platform Makefile Generator
  Module:    $RCSfile: cmSourceGroupCommand.cxx,v $
  Language:  C++
  Date:      $Date: 2006/03/15 16:02:07 $
  Version:   $Revision: 1.16 $

  Copyright (c) 2002 Kitware, Inc., Insight Consortium.  All rights reserved.
  See Copyright.txt or http://www.cmake.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#include "cmSourceGroupCommand.h"

inline std::vector<std::string> tokenize(const std::string& str,
                                         const std::string& sep)
{
  std::vector<std::string> tokens;
  if(str.size() == 0)
    {
    tokens.push_back("");
    return tokens;
    }
  std::string::size_type tokstart,tokend;

  tokend=0;
  do
    {
    tokstart=str.find_first_not_of(sep,tokend);
    if (tokstart==std::string::npos) 
      {
      break;    // no more tokens
      }
    tokend=str.find_first_of(sep,tokstart);
    if (tokend==std::string::npos)
      {
      tokens.push_back(str.substr(tokstart));
      }
    else
      {
      tokens.push_back(str.substr(tokstart,tokend-tokstart));
      }
    } while (tokend!=std::string::npos);
  return tokens;
}

// cmSourceGroupCommand
bool cmSourceGroupCommand::InitialPass(std::vector<std::string> const& args)
{
  if(args.size() < 1)
    {
    this->SetError("called with incorrect number of arguments");
    return false;
    }  

  std::string delimiter = "\\";
  if(this->Makefile->GetDefinition("SOURCE_GROUP_DELIMITER"))
    {
    delimiter = this->Makefile->GetDefinition("SOURCE_GROUP_DELIMITER");
    }

  std::vector<std::string> folders = tokenize(args[0], delimiter);
 
  const char *parent = NULL;
  cmSourceGroup* sg = NULL;
  for(unsigned int i=0;i<folders.size();++i)
    {
    sg = this->Makefile->GetSourceGroup(folders[i].c_str());
    if(!sg)
      {
      this->Makefile->AddSourceGroup(folders[i].c_str(), 0, parent);
      }
    sg = this->Makefile->GetSourceGroup(folders[i].c_str());
    parent = folders[i].c_str();
    }
  if(!sg)
    {
    this->SetError("Could not create or find source group");
    return false;
    }
  // If only two arguments are given, the pre-1.8 version of the
  // command is being invoked.
  if(args.size() == 2  && args[1] != "FILES")
    {
    sg->SetGroupRegex(args[1].c_str());
    return true;
    }
  
  // Process arguments.
  bool doingFiles = false;
  for(unsigned int i=1; i < args.size(); ++i)
    {
    if(args[i] == "REGULAR_EXPRESSION")
      {
      // Next argument must specify the regex.
      if(i+1 < args.size())
        {
        ++i;
        sg->SetGroupRegex(args[i].c_str());
        }
      else
        {
        this->SetError("REGULAR_EXPRESSION argument given without a regex.");
        return false;
        }
      doingFiles = false;
      }
    else if(args[i] == "FILES")
      {
      // Next arguments will specify files.
      doingFiles = true;
      }
    else if(doingFiles)
      {
      // Convert name to full path and add to the group's list.
      std::string src = args[i].c_str();
      if(!cmSystemTools::FileIsFullPath(src.c_str()))
        {
        src = this->Makefile->GetCurrentDirectory();
        src += "/";
        src += args[i];
        }
      src = cmSystemTools::CollapseFullPath(src.c_str());
      sg->AddGroupFile(src.c_str());
      }
    else
      {
      cmOStringStream err;
      err << "Unknown argument \"" << args[i].c_str() << "\".  "
          << "Perhaps the FILES keyword is missing.\n";
      this->SetError(err.str().c_str());
      return false;
      }
    }
  
  return true;
}
