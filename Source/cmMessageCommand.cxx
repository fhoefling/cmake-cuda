/*=========================================================================

  Program:   CMake - Cross-Platform Makefile Generator
  Module:    $RCSfile: cmMessageCommand.cxx,v $
  Language:  C++
  Date:      $Date: 2006/10/13 14:52:06 $
  Version:   $Revision: 1.18.2.2 $

  Copyright (c) 2002 Kitware, Inc., Insight Consortium.  All rights reserved.
  See Copyright.txt or http://www.cmake.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#include "cmMessageCommand.h"

// cmLibraryCommand
bool cmMessageCommand::InitialPass(std::vector<std::string> const& args)
{
  if(args.size() < 1 )
    {
    this->SetError("called with incorrect number of arguments");
    return false;
    }
  std::string message;
  std::vector<std::string>::const_iterator i = args.begin();

  bool send_error = false;
  bool fatal_error = false;
  bool status = false;
  if (*i == "SEND_ERROR")
    {
    send_error = true;
    ++i;
    }
  else
    {
    if (*i == "STATUS")
      {
      status = true;
      ++i;
      }
    else
      {
      if (*i == "FATAL_ERROR")
        {
        fatal_error = true;
        ++i;
        }
      }
    }

  for(;i != args.end(); ++i)
    {
    message += *i;
    }

  if (send_error || fatal_error)
    {
    if( !this->Makefile->GetCMakeInstance()->GetDebugOutput())
      {
      cmSystemTools::Error(message.c_str());
      }
    else
      {
    this->SetError(message.c_str());
    }
    }
  else
    {
      if (status)
        {
          this->Makefile->DisplayStatus(message.c_str(), -1);
        }
      else
        {
          cmSystemTools::Message(message.c_str());
        }
    }
  if(fatal_error )
    {
    cmSystemTools::SetFatalErrorOccured();
    }
  // if debug is on then retru
  if(this->Makefile->GetCMakeInstance()->GetDebugOutput())
    {
  return (!send_error && !fatal_error);
    }
  return true;
}

