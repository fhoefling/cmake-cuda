/*=========================================================================

  Program:   CMake - Cross-Platform Makefile Generator
  Module:    $RCSfile: cmSourceFile.cxx,v $
  Language:  C++
  Date:      $Date: 2006/05/14 19:22:43 $
  Version:   $Revision: 1.31.2.2 $

  Copyright (c) 2002 Kitware, Inc., Insight Consortium.  All rights reserved.
  See Copyright.txt or http://www.cmake.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#include "cmSourceFile.h"
#include "cmSystemTools.h"



// Set the name of the class and the full path to the file.
// The class must be found in dir and end in name.cxx, name.txx, 
// name.c or it will be considered a header file only class
// and not included in the build process
bool cmSourceFile::SetName(const char* name, const char* dir,
                           const std::vector<std::string>& sourceExts,
                           const std::vector<std::string>& headerExts,
                           const char* target)
{

  this->SetProperty("HEADER_FILE_ONLY","1");
  this->SourceNameWithoutLastExtension = "";

  // Save the original name given.
  this->SourceName = name;

  // Convert the name to a full path in case the given name is a
  // relative path.
  std::string pathname = cmSystemTools::CollapseFullPath(name, dir);

  // First try and see whether the listed file can be found
  // as is without extensions added on.
  std::string hname = pathname;
  if(cmSystemTools::FileExists(hname.c_str()))
    {
    this->SourceName = cmSystemTools::GetFilenamePath(name);
    if ( this->SourceName.size() > 0 )
      {
      this->SourceName += "/";
      }
    this->SourceName += cmSystemTools::GetFilenameWithoutLastExtension(name);
    std::string::size_type pos = hname.rfind('.');
    if(pos != std::string::npos)
      {
      this->SourceExtension = hname.substr(pos+1, hname.size()-pos);
      if ( cmSystemTools::FileIsFullPath(name) )
        {
        std::string::size_type pos2 = hname.rfind('/');
        if(pos2 != std::string::npos)
          {
          this->SourceName = hname.substr(pos2+1, pos - pos2-1);
          }
        }
      }

    // See if the file is a header file
    if(std::find( headerExts.begin(), headerExts.end(), 
                  this->SourceExtension ) == headerExts.end())
      {
      this->SetProperty("HEADER_FILE_ONLY","0");
      }
    this->FullPath = hname;

    // Mark this as an external object file if it has the proper
    // extension.  THIS CODE IS DUPLICATED IN THE OTHER SetName METHOD.
    // THESE METHODS SHOULD BE MERGED.
    if ( this->SourceExtension == "obj" || this->SourceExtension == "o" ||
      this->SourceExtension == "lo" )
      {
      this->SetProperty("EXTERNAL_OBJECT", "1");
      }
    return true;
    }
  
  // Next, try the various source extensions
  for( std::vector<std::string>::const_iterator ext = sourceExts.begin();
       ext != sourceExts.end(); ++ext )
    {
    hname = pathname;
    hname += ".";
    hname += *ext;
    if(cmSystemTools::FileExists(hname.c_str()))
      {
      this->SourceExtension = *ext;
      this->SetProperty("HEADER_FILE_ONLY","0");
      this->FullPath = hname;
      return true;
      }
    }

  // Finally, try the various header extensions
  for( std::vector<std::string>::const_iterator ext = headerExts.begin();
       ext != headerExts.end(); ++ext )
    {
    hname = pathname;
    hname += ".";
    hname += *ext;
    if(cmSystemTools::FileExists(hname.c_str()))
      {
      this->SourceExtension = *ext;
      this->FullPath = hname;
      return true;
      }
    }

  cmOStringStream e;
  e << "Cannot find source file \"" << pathname << "\"";
  if(target)
    {
    e << " for target \"" << target << "\"";
    }
  e << "\n\nTried extensions";
  for( std::vector<std::string>::const_iterator ext = sourceExts.begin();
       ext != sourceExts.end(); ++ext )
    {
    e << " ." << *ext;
    }
  for( std::vector<std::string>::const_iterator ext = headerExts.begin();
       ext != headerExts.end(); ++ext )
    {
    e << " ." << *ext;
    }
  cmSystemTools::Error(e.str().c_str());
  return false;
}

void cmSourceFile::SetName(const char* name, const char* dir, const char *ext,
                           bool hfo)
{
  this->SetProperty("HEADER_FILE_ONLY",(hfo ? "1" : "0"));
  this->SourceNameWithoutLastExtension = "";
  this->SourceName = name;
  std::string fname = this->SourceName;
  if(ext && strlen(ext))
    {
    fname += ".";
    fname += ext;
    }
  this->FullPath = cmSystemTools::CollapseFullPath(fname.c_str(), dir);
  cmSystemTools::ConvertToUnixSlashes(this->FullPath);
  this->SourceExtension = ext;

  // Mark this as an external object file if it has the proper
  // extension.  THIS CODE IS DUPLICATED IN THE OTHER SetName METHOD.
  // THESE METHODS SHOULD BE MERGED.
  if ( this->SourceExtension == "obj" || this->SourceExtension == "o" ||
       this->SourceExtension == "lo" )
    {
    this->SetProperty("EXTERNAL_OBJECT", "1");
    }
  return;
}

void cmSourceFile::Print() const
{
  std::cerr << "this->FullPath: " <<  this->FullPath << "\n";
  std::cerr << "this->SourceName: " << this->SourceName << std::endl;
  std::cerr << "this->SourceExtension: " << this->SourceExtension << "\n";
}

void cmSourceFile::SetProperty(const char* prop, const char* value)
{
  if (!prop)
    {
    return;
    }
  if (!value)
    {
    value = "NOTFOUND";
    }
  this->Properties[prop] = value;
}

const char *cmSourceFile::GetProperty(const char* prop) const
{
  // watch for special "computed" properties that are dependent on other
  // properties or variables, always recompute them
  if (!strcmp(prop,"LOCATION"))
    {
    return this->FullPath.c_str();
    }

  std::map<cmStdString,cmStdString>::const_iterator i = 
    this->Properties.find(prop);
  if (i != this->Properties.end())
    {
    return i->second.c_str();
    }
  return 0;
}

bool cmSourceFile::GetPropertyAsBool(const char* prop) const
{
  std::map<cmStdString,cmStdString>::const_iterator i = 
    this->Properties.find(prop);
  if (i != this->Properties.end())
    {
    return cmSystemTools::IsOn(i->second.c_str());
    }
  return false;
}

void cmSourceFile::SetCustomCommand(cmCustomCommand* cc)
{
  if(this->CustomCommand)
    {
    delete this->CustomCommand;
    }
  this->CustomCommand = cc;
}

const std::string& cmSourceFile::GetSourceNameWithoutLastExtension()
{
  if ( this->SourceNameWithoutLastExtension.empty() )
    {
    this->SourceNameWithoutLastExtension = 
      cmSystemTools::GetFilenameWithoutLastExtension(this->FullPath);
    }
  return this->SourceNameWithoutLastExtension;
}
