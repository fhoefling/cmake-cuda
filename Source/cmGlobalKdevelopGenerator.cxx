/*=========================================================================

  Program:   CMake - Cross-Platform Makefile Generator
  Module:    $RCSfile: cmGlobalKdevelopGenerator.cxx,v $
  Language:  C++
  Date:      $Date: 2007/02/05 18:21:32 $
  Version:   $Revision: 1.10.2.6 $

  Copyright (c) 2002 Kitware, Inc., Insight Consortium.  All rights reserved.
  Copyright (c) 2004 Alexander Neundorf neundorf@kde.org, All rights reserved.
  See Copyright.txt or http://www.cmake.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "cmGlobalKdevelopGenerator.h"
#include "cmLocalUnixMakefileGenerator3.h"
#include "cmMakefile.h"
#include "cmake.h"
#include "cmSourceFile.h"
#include "cmGeneratedFileStream.h"

#include <cmsys/SystemTools.hxx>

cmGlobalKdevelopGenerator::cmGlobalKdevelopGenerator()
{
  // This type of makefile always requires unix style paths
  this->ForceUnixPaths = true;
  this->FindMakeProgramFile = "CMakeUnixFindMake.cmake";
  this->ToolSupportsColor = false;
}

///! Create a local generator appropriate to this Global Generator
cmLocalGenerator *cmGlobalKdevelopGenerator::CreateLocalGenerator()
{
  cmLocalUnixMakefileGenerator3 *lg = new cmLocalUnixMakefileGenerator3;
  lg->SetForceVerboseMakefiles(true);
  lg->SetGlobalGenerator(this);
  return lg;
}

//----------------------------------------------------------------------------
void cmGlobalKdevelopGenerator
::GetDocumentation(cmDocumentationEntry& entry) const
{
  entry.name = this->GetName();
  entry.brief = "Generates KDevelop 3 project files.";
  entry.full =
    "Project files for KDevelop 3 will be created in the top directory "
    "and in every subdirectory which features a CMakeLists.txt file "
    "containing a PROJECT() call. "
    "If you change the settings using KDevelop cmake will try its best "
    "to keep your changes when regenerating the project files. "
    "Additionally a hierarchy of UNIX makefiles is generated into the "
    "build tree.  Any "
    "standard UNIX-style make program can build the project through the "
    "default make target.  A \"make install\" target is also provided.";
}

void cmGlobalKdevelopGenerator::Generate()
{
  this->cmGlobalUnixMakefileGenerator3::Generate();
  // for each sub project in the project create 
  // a kdevelop project
  std::map<cmStdString, std::vector<cmLocalGenerator*> >::iterator it;
  for(it = this->ProjectMap.begin(); it!= this->ProjectMap.end(); ++it)
    {
    cmMakefile* mf = it->second[0]->GetMakefile();
    std::string outputDir=mf->GetStartOutputDirectory();
    std::string projectDir=mf->GetHomeDirectory();
    std::string projectName=mf->GetProjectName();
    std::string cmakeFilePattern("CMakeLists.txt;*.cmake;");
    std::string fileToOpen;
    std::vector<cmLocalGenerator*>& lgs= it->second;
    // create the project.kdevelop.filelist file
    if(!this->CreateFilelistFile(it->second[0], lgs,
                                 outputDir, projectDir,
                                 projectName, cmakeFilePattern, fileToOpen))
      {
      cmSystemTools::Error("Can not create filelist file");
      return;
      }
    //try to find the name of an executable so we have something to
    //run from kdevelop for now just pick the first executable found
    std::string executable;
    for (std::vector<cmLocalGenerator*>::const_iterator lg=lgs.begin();
         lg!=lgs.end(); lg++)
    {
      cmMakefile* makefile=(*lg)->GetMakefile();
       cmTargets& targets=makefile->GetTargets();
    for (cmTargets::iterator ti = targets.begin();
         ti != targets.end(); ti++)
      {
      if (ti->second.GetType()==cmTarget::EXECUTABLE)
        {
        executable = ti->second.GetProperty("LOCATION");
        break;
        }
      }
     if (!executable.empty())
        {
        break;
    }
      }
    // now create a project file
    this->CreateProjectFile(outputDir, projectDir, projectName,
                            executable, cmakeFilePattern, fileToOpen);
    }
}

bool cmGlobalKdevelopGenerator
::CreateFilelistFile(cmLocalGenerator* ,
                     std::vector<cmLocalGenerator*>& lgs,
                     const std::string& outputDir, 
                     const std::string& projectDirIn,
                     const std::string& projectname,
                     std::string& cmakeFilePattern,
                     std::string& fileToOpen)
{
  std::string projectDir = projectDirIn + "/";
  std::string filename = outputDir+ "/" + projectname +".kdevelop.filelist";

  std::set<cmStdString> files;
  std::string tmp;

  for (std::vector<cmLocalGenerator*>::const_iterator it=lgs.begin(); 
       it!=lgs.end(); it++)
    {
    cmMakefile* makefile=(*it)->GetMakefile();
    const std::vector<std::string>& listFiles=makefile->GetListFiles();
    for (std::vector<std::string>::const_iterator lt=listFiles.begin(); 
         lt!=listFiles.end(); lt++)
      {
      tmp=*lt;
      cmSystemTools::ReplaceString(tmp, projectDir.c_str(), "");
      // make sure the file is part of this source tree
      if ((tmp[0]!='/') && 
          (strstr(tmp.c_str(), 
                  cmake::GetCMakeFilesDirectoryPostSlash())==0))
        {
        files.insert(tmp);
        tmp=cmSystemTools::GetFilenameName(tmp);
        //add all files which dont match the default 
        // */CMakeLists.txt;*cmake; to the file pattern
        if ((tmp!="CMakeLists.txt")
            && (strstr(tmp.c_str(), ".cmake")==0))
          {
          cmakeFilePattern+=tmp+";";
          }
        }
      }
  
    //get all sources
    cmTargets& targets=makefile->GetTargets();
    for (cmTargets::iterator ti = targets.begin();
         ti != targets.end(); ti++)
      {
      const std::vector<cmSourceFile*>& sources=ti->second.GetSourceFiles();
      for (std::vector<cmSourceFile*>::const_iterator si=sources.begin();
           si!=sources.end(); si++)
        {
        tmp=(*si)->GetFullPath();
        std::string headerBasename=cmSystemTools::GetFilenamePath(tmp);
        headerBasename+="/";
        headerBasename+=cmSystemTools::GetFilenameWithoutExtension(tmp);

        cmSystemTools::ReplaceString(tmp, projectDir.c_str(), "");

        if ((tmp[0]!='/')  && 
            (strstr(tmp.c_str(), 
                  cmake::GetCMakeFilesDirectoryPostSlash())==0) &&
           (cmSystemTools::GetFilenameExtension(tmp)!=".moc"))
          {
          files.insert(tmp);

          // check if there's a matching header around
          for(std::vector<std::string>::const_iterator
                ext = makefile->GetHeaderExtensions().begin();
              ext !=  makefile->GetHeaderExtensions().end(); ++ext)
            {
            std::string hname=headerBasename;
            hname += ".";
            hname += *ext;
            if(cmSystemTools::FileExists(hname.c_str()))
              {
              cmSystemTools::ReplaceString(hname, projectDir.c_str(), "");
              files.insert(hname);
              break;
              }
            }
          }
        }
      for (std::vector<std::string>::const_iterator lt=listFiles.begin();
           lt!=listFiles.end(); lt++)
        {
        tmp=*lt;
        cmSystemTools::ReplaceString(tmp, projectDir.c_str(), "");
        if ((tmp[0]!='/') && 
            (strstr(tmp.c_str(), 
                    cmake::GetCMakeFilesDirectoryPostSlash())==0))
          {
          files.insert(tmp.c_str());
          }
        }
      }
    }

  //check if the output file already exists and read it
  //insert all files which exist into the set of files
  std::ifstream oldFilelist(filename.c_str());
  if (oldFilelist)
    {
    while (cmSystemTools::GetLineFromStream(oldFilelist, tmp))
      {
      if (tmp[0]=='/')
        {
        continue;
        }
      std::string completePath=projectDir+tmp;
      if (cmSystemTools::FileExists(completePath.c_str()))
        {
        files.insert(tmp);
        }
      }
    oldFilelist.close();
    }

  //now write the new filename
  cmGeneratedFileStream fout(filename.c_str());
  if(!fout)
    {
    return false;
    }
   
  fileToOpen="";
  for (std::set<cmStdString>::const_iterator it=files.begin(); 
       it!=files.end(); it++)
    {
    // get the full path to the file
    tmp=cmSystemTools::CollapseFullPath(it->c_str(), projectDir.c_str());
    // just select the first source file
    if (fileToOpen.empty())
    {
       std::string ext = cmSystemTools::GetFilenameExtension(tmp);
       if ((ext==".c") || (ext==".cc") || (ext==".cpp") 
           || (ext==".C") || (ext==".h"))
       {
          fileToOpen=tmp;
       }
    }
    // make it relative to the project dir
    cmSystemTools::ReplaceString(tmp, projectDir.c_str(), "");
    // only put relative paths
    if (tmp.size() && tmp[0] != '/')
      {
      fout << tmp.c_str() <<"\n";
      }
    }
  return true;
}                             


/* create the project file, if it already exists, merge it with the
existing one, otherwise create a new one */
void cmGlobalKdevelopGenerator
::CreateProjectFile(const std::string& outputDir,
                                             const std::string& projectDir,
                                             const std::string& projectname, 
                                             const std::string& executable,
                                             const std::string& cmakeFilePattern,
                                             const std::string& fileToOpen)
{
  std::string filename=outputDir+"/";
  filename+=projectname+".kdevelop";
  std::string sessionFilename=outputDir+"/";
  sessionFilename+=projectname+".kdevses";

  if (cmSystemTools::FileExists(filename.c_str()))
    {
    this->MergeProjectFiles(outputDir, projectDir, filename, 
                            executable, cmakeFilePattern, 
                            fileToOpen, sessionFilename);
    }
  else
    {
    this->CreateNewProjectFile(outputDir, projectDir, filename,
                               executable, cmakeFilePattern, 
                               fileToOpen, sessionFilename);
    }
   
}

void cmGlobalKdevelopGenerator
::MergeProjectFiles(const std::string& outputDir, 
                                             const std::string& projectDir, 
                                             const std::string& filename, 
                                             const std::string& executable, 
                                             const std::string& cmakeFilePattern,
                                             const std::string& fileToOpen,
                                             const std::string& sessionFilename)
{
  std::ifstream oldProjectFile(filename.c_str());
  if (!oldProjectFile)
    {
    this->CreateNewProjectFile(outputDir, projectDir, filename, 
                               executable, cmakeFilePattern, 
                               fileToOpen, sessionFilename);
    return;
    }

  /* Read the existing project file (line by line), copy all lines
    into the new project file, except the ones which can be reliably
    set from contents of the CMakeLists.txt */
  std::string tmp;
  std::vector<std::string> lines;
  while (cmSystemTools::GetLineFromStream(oldProjectFile, tmp))
    {
    lines.push_back(tmp);
    }
  oldProjectFile.close();

  cmGeneratedFileStream fout(filename.c_str());
  if(!fout)
    {
    return;
    }

  for (std::vector<std::string>::const_iterator it=lines.begin(); 
       it!=lines.end(); it++)
    {
    const char* line=(*it).c_str();
    // skip these tags as they are always replaced
    if ((strstr(line, "<projectdirectory>")!=0)
        || (strstr(line, "<projectmanagement>")!=0)
        || (strstr(line, "<absoluteprojectpath>")!=0)
        || (strstr(line, "<filelistdirectory>")!=0)
        || (strstr(line, "<buildtool>")!=0)
        || (strstr(line, "<builddir>")!=0))
      {
      continue;
      }

    // output the line from the file if it is not one of the above tags
    fout<<*it<<"\n";
    // if this is the <general> tag output the stuff that goes in the
    // general tag
    if (strstr(line, "<general>"))
      {
      fout<< "  <projectmanagement>KDevCustomProject</projectmanagement>\n";
      fout<< "  <projectdirectory>" <<projectDir.c_str() 
          << "</projectdirectory>\n";   //this one is important
      fout<<"  <absoluteprojectpath>true</absoluteprojectpath>\n";
      //and this one
      }
    // inside kdevcustomproject the <filelistdirectory> must be put
    if (strstr(line, "<kdevcustomproject>"))
      {
      fout<<"    <filelistdirectory>"<<outputDir.c_str()
          <<"</filelistdirectory>\n";
      }
    // buildtool and builddir go inside <build>
    if (strstr(line, "<build>"))
      {
      fout<<"      <buildtool>make</buildtool>\n";
      fout<<"      <builddir>"<<outputDir.c_str()<<"</builddir>\n";
      }
    }
}

void cmGlobalKdevelopGenerator
::CreateNewProjectFile(const std::string& outputDir,
                                                const std::string& projectDir,
                                                const std::string& filename,
                                                const std::string& executable,
                                                const std::string& cmakeFilePattern,
                                                const std::string& fileToOpen,
                                                const std::string& sessionFilename)
{
  cmGeneratedFileStream fout(filename.c_str());
  if(!fout)
    {
    return;
    }

  fout<<"<?xml version = '1.0'?>\n";
  fout<<"<kdevelop>\n";
  fout<<"  <general>\n";
  fout<<"  <author></author>\n";
  fout<<"  <email></email>\n";
  fout<<"  <version>$VERSION$</version>\n";
  fout<<"  <projectmanagement>KDevCustomProject</projectmanagement>\n";
  fout<<"  <primarylanguage>C++</primarylanguage>\n";
  fout<<"  <ignoreparts/>\n";
  fout<<"  <projectdirectory>"<<projectDir.c_str()
      <<"</projectdirectory>\n";   //this one is important
  fout<<"  <absoluteprojectpath>true</absoluteprojectpath>\n";          //and this one
  fout<<"  <secondaryLanguages>\n";
  fout<<"     <language>C</language>\n";
  fout<<"  </secondaryLanguages>\n";
  fout<<"  </general>\n";
  fout<<"  <kdevcustomproject>\n";
  fout<<"    <filelistdirectory>"<<outputDir.c_str()
      <<"</filelistdirectory>\n";
  fout<<"    <run>\n";
  fout<<"      <mainprogram>"<<executable.c_str()<<"</mainprogram>\n";
  fout<<"      <directoryradio>custom</directoryradio>\n";
  fout<<"      <customdirectory>"<<outputDir.c_str()<<"</customdirectory>\n";
  fout<<"      <programargs></programargs>\n";
  fout<<"      <terminal>false</terminal>\n";
  fout<<"      <autocompile>true</autocompile>\n";
  fout<<"      <envvars/>\n";
  fout<<"    </run>\n";
  fout<<"    <build>\n";
  fout<<"      <buildtool>make</buildtool>\n";                                        //this one is important
  fout<<"      <builddir>"<<outputDir.c_str()<<"</builddir>\n";   //and this one
  fout<<"    </build>\n";
  fout<<"    <make>\n";
  fout<<"      <abortonerror>false</abortonerror>\n";
  fout<<"      <numberofjobs>1</numberofjobs>\n";
  fout<<"      <dontact>false</dontact>\n";
  fout<<"      <makebin></makebin>\n";
  fout<<"      <selectedenvironment>default</selectedenvironment>\n";
  fout<<"      <environments>\n";
  fout<<"        <default/>\n";
  fout<<"      </environments>\n";
  fout<<"    </make>\n";
  fout<<"  </kdevcustomproject>\n";
  fout<<"  <kdevfilecreate>\n";
  fout<<"    <filetypes/>\n";
  fout<<"    <useglobaltypes>\n";
  fout<<"      <type ext=\"ui\" />\n";
  fout<<"      <type ext=\"cpp\" />\n";
  fout<<"      <type ext=\"h\" />\n";
  fout<<"    </useglobaltypes>\n";
  fout<<"  </kdevfilecreate>\n";
  fout<<"  <kdevdoctreeview>\n";
  fout<<"    <projectdoc>\n";
  fout<<"      <userdocDir>html/</userdocDir>\n";
  fout<<"      <apidocDir>html/</apidocDir>\n";
  fout<<"    </projectdoc>\n";
  fout<<"    <ignoreqt_xml/>\n";
  fout<<"    <ignoredoxygen/>\n";
  fout<<"    <ignorekdocs/>\n";
  fout<<"    <ignoretocs/>\n";
  fout<<"    <ignoredevhelp/>\n";
  fout<<"  </kdevdoctreeview>\n";
  fout<<"  <cppsupportpart>\n";
  fout<<"    <filetemplates>\n";
  fout<<"      <interfacesuffix>.h</interfacesuffix>\n";
  fout<<"      <implementationsuffix>.cpp</implementationsuffix>\n";
  fout<<"    </filetemplates>\n";
  fout<<"  </cppsupportpart>\n";
  fout<<"  <kdevcppsupport>\n";
  fout<<"    <codecompletion>\n";
  fout<<"      <includeGlobalFunctions>true</includeGlobalFunctions>\n";
  fout<<"      <includeTypes>true</includeTypes>\n";
  fout<<"      <includeEnums>true</includeEnums>\n";
  fout<<"      <includeTypedefs>false</includeTypedefs>\n";
  fout<<"      <automaticCodeCompletion>true</automaticCodeCompletion>\n";
  fout<<"      <automaticArgumentsHint>true</automaticArgumentsHint>\n";
  fout<<"      <automaticHeaderCompletion>true</automaticHeaderCompletion>\n";
  fout<<"      <codeCompletionDelay>250</codeCompletionDelay>\n";
  fout<<"      <argumentsHintDelay>400</argumentsHintDelay>\n";
  fout<<"      <headerCompletionDelay>250</headerCompletionDelay>\n";
  fout<<"    </codecompletion>\n";
  fout<<"    <references/>\n";
  fout<<"  </kdevcppsupport>\n";
  fout<<"  <kdevfileview>\n";
  fout<<"    <groups>\n";
  fout<<"      <group pattern=\""<<cmakeFilePattern.c_str()
      <<"\" name=\"CMake\" />\n";
  fout<<"      <group pattern=\"*.h;*.hxx\" name=\"Header\" />\n";
  fout<<"      <group pattern=\"*.cpp;*.c;*.C;*.cxx\" name=\"Sources\" />\n";
  fout<<"      <group pattern=\"*.ui\" name=\"Qt Designer files\" />\n";
  fout<<"      <hidenonprojectfiles>true</hidenonprojectfiles>\n";
  fout<<"    </groups>\n";
  fout<<"    <tree>\n";
  fout<<"      <hidepatterns>*.o,*.lo,CVS,*~,cmake*</hidepatterns>\n";
  fout<<"      <hidenonprojectfiles>true</hidenonprojectfiles>\n";
  fout<<"    </tree>\n";
  fout<<"  </kdevfileview>\n";
  fout<<"</kdevelop>\n";
  
  if (sessionFilename.empty())
     return;

  // and a session file, so that kdevelop opens a file if it opens the
  // project the first time
  cmGeneratedFileStream devses(sessionFilename.c_str());
  if(!devses)
  {
     return;
  }
  devses<<"<?xml version = '1.0' encoding = \'UTF-8\'?>\n";
  devses<<"<!DOCTYPE KDevPrjSession>\n";
  devses<<"<KDevPrjSession>\n";
  devses<<" <DocsAndViews NumberOfDocuments=\"1\" >\n";
  devses<<"  <Doc0 NumberOfViews=\"1\" URL=\"file://"
        <<fileToOpen.c_str()<<"\" >\n";
  devses<<"   <View0 line=\"0\" Type=\"Source\" />\n";
  devses<<"  </Doc0>\n";
  devses<<" </DocsAndViews>\n";
  devses<<"</KDevPrjSession>\n";
}

