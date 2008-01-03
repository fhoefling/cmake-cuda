/*=========================================================================

  Program:   CMake - Cross-Platform Makefile Generator
  Module:    $RCSfile: cmVTKMakeInstantiatorCommand.cxx,v $
  Language:  C++
  Date:      $Date: 2006/05/14 19:22:44 $
  Version:   $Revision: 1.25.2.1 $

  Copyright (c) 2002 Kitware, Inc., Insight Consortium.  All rights reserved.
  See Copyright.txt or http://www.cmake.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#include "cmVTKMakeInstantiatorCommand.h"

#include "cmCacheManager.h"
#include "cmGeneratedFileStream.h"
#include "cmSourceFile.h"

bool
cmVTKMakeInstantiatorCommand
::InitialPass(std::vector<std::string> const& argsIn)
{
  if(argsIn.size() < 3)
    {
    this->SetError("called with incorrect number of arguments");
    return false;
    }
  std::vector<std::string> args;
  this->Makefile->ExpandSourceListArguments(argsIn, args, 2);
  std::string sourceListValue;
  
  this->ClassName = args[0];
  
  std::vector<cmStdString> inSourceLists;
  this->ExportMacro = "-";
  bool includesMode = false;
  bool oldVersion = true;
  
  // Find the path of the files to be generated.
  std::string filePath = this->Makefile->GetCurrentOutputDirectory();
  std::string headerPath = filePath;
  
  // Check whether to use the old or new form.
  if(this->Makefile->GetDefinition("VTK_USE_INSTANTIATOR_NEW"))
    {
    oldVersion = false;
    }
  
  for(unsigned int i=2;i < args.size();++i)
    {
    if(args[i] == "HEADER_LOCATION")
      {
      includesMode = false;
      if(++i < args.size())
        {
        headerPath = args[i];
        }
      else
        {
        this->SetError("HEADER_LOCATION option used without value.");
        return false;
        }
      }
    else if(args[i] == "EXPORT_MACRO")
      {
      includesMode = false;
      if(++i < args.size())
        {
        this->ExportMacro = args[i];
        }
      else
        {
        this->SetError("EXPORT_MACRO option used without value.");
        return false;
        }
      }
    else if(args[i] == "INCLUDES")
      {
      includesMode = true;
      }
    // If not an option, it must be another input source list name or
    // an include file.
    else
      {
      if(!includesMode)
        {
        inSourceLists.push_back(args[i]);
        }
      else
        {
        this->Includes.push_back(args[i]);
        }
      }
    }
  
  if(this->ExportMacro == "-")
    {
    this->SetError("No EXPORT_MACRO option given.");
    return false;
    }
  
  for(std::vector<cmStdString>::const_iterator s = inSourceLists.begin();
      s != inSourceLists.end(); ++s)
    {
    std::string srcName = cmSystemTools::GetFilenameWithoutExtension(*s);
    cmSourceFile *sf = this->Makefile->GetSource(s->c_str());
    
    // Wrap-excluded and abstract classes do not have a New() method.
    // vtkIndent and vtkTimeStamp are special cases and are not
    // vtkObject subclasses.
    if(
      (!sf || (!sf->GetPropertyAsBool("WRAP_EXCLUDE") && 
               !sf->GetPropertyAsBool("ABSTRACT"))) &&
      ((srcName != "vtkIndent") && (srcName != "vtkTimeStamp")))
      {
      this->Classes.push_back(srcName);
      }
    }    
  
  // Generate the header with the class declaration.
  {
  std::string fileName = this->ClassName + ".h";
  std::string fullName = headerPath+"/"+fileName;
  
  // Generate the output file with copy-if-different.
  cmGeneratedFileStream fout(fullName.c_str());
  fout.SetCopyIfDifferent(true);
  
  // Actually generate the code in the file.
  if(!oldVersion)
    {
    this->GenerateHeaderFile(fout);
    }
  else
    {
    this->OldGenerateHeaderFile(fout);
    }
  }
  
  // Generate the implementation file.
  {
  std::string fileName = this->ClassName + ".cxx";
  std::string fullName = filePath+"/"+fileName;
  
  // Generate the output file with copy-if-different.
  {
  cmGeneratedFileStream fout(fullName.c_str());
  fout.SetCopyIfDifferent(true);
  
  // Actually generate the code in the file.
  if(!oldVersion)
    {
    this->GenerateImplementationFile(fout);
    }
  else
    {
    this->OldGenerateImplementationFile(fout);
    }
  }
  
  // Add the generated source file into the source list.
  cmSourceFile file;
  file.SetProperty("WRAP_EXCLUDE","1");
  file.SetProperty("ABSTRACT","0");
  file.SetName(fileName.c_str(), filePath.c_str(),
               this->Makefile->GetSourceExtensions(),
               this->Makefile->GetHeaderExtensions());
  this->Makefile->AddSource(file);
  sourceListValue += file.GetSourceName() + ".cxx";
  }
  
  if(oldVersion)
    {
    int groupSize = 10;
    size_t numClasses = this->Classes.size();
    size_t numFullBlocks = numClasses / groupSize;
    size_t lastBlockSize = numClasses % groupSize;
    size_t numBlocks = numFullBlocks + ((lastBlockSize>0)? 1:0);
    
    // Generate the files with the ::New() calls to each class.  These
    // are done in groups to keep the translation unit size smaller.
    for(unsigned int block=0; block < numBlocks;++block)
      {
      std::string fileName = this->OldGenerateCreationFileName(block);    
      std::string fullName = filePath+"/"+fileName;
    
      // Generate the output file with copy-if-different.
      {
      cmGeneratedFileStream fout(fullName.c_str());
      fout.SetCopyIfDifferent(true);
    
      size_t thisBlockSize =
        (block < numFullBlocks)? groupSize:lastBlockSize;
    
      // Actually generate the code in the file.
      this->OldGenerateCreationFile(fout,
                                    block*groupSize, 
                                    static_cast<int>(thisBlockSize));
      }
      
      // Add the generated source file into the source list.
      cmSourceFile file;
      file.SetProperty("WRAP_EXCLUDE","1");
      file.SetProperty("ABSTRACT","0");
      file.SetName(fileName.c_str(), filePath.c_str(),
                   this->Makefile->GetSourceExtensions(),
                   this->Makefile->GetHeaderExtensions());
      this->Makefile->AddSource(file);
      sourceListValue += ";";
      sourceListValue += file.GetSourceName() + ".cxx";
      }
    }
  
  this->Makefile->AddDefinition(args[1].c_str(), sourceListValue.c_str());  
  return true;
}

// Generates the class header file with the definition of the class
// and its initializer class.
void
cmVTKMakeInstantiatorCommand
::GenerateHeaderFile(std::ostream& os)
{
  os <<
    "#ifndef __" << this->ClassName.c_str() << "_h\n"
    "#define __" << this->ClassName.c_str() << "_h\n"
    "\n"
    "#include \"vtkInstantiator.h\"\n";
  for(unsigned int i=0;i < this->Includes.size();++i)
    {
    os << "#include \"" << this->Includes[i].c_str() << "\"\n";
    }
  
  // Write the instantiator class definition.
  os <<
    "\n"
    "class " << this->ExportMacro.c_str() 
     << " " << this->ClassName.c_str() << "\n"
    "{\n"
    "public:\n"
    "  " << this->ClassName.c_str() << "();\n"
    "  ~" << this->ClassName.c_str() << "();\n"
    "private:\n"
    "  static void ClassInitialize();\n"
    "  static void ClassFinalize();\n"
    "  static unsigned int Count;\n"
    "};\n"
    "\n";
  
  // Write the initialization instance to make sure the creation
  // functions get registered when this generated header is included.
  os <<
    "static "
     << this->ClassName.c_str() << " "
     << this->ClassName.c_str() << "Initializer;\n"
    "\n"
    "#endif\n";  
}

// Generates the file with the implementation of the class.  All
// methods except the actual object creation functions are generated
// here.
void
cmVTKMakeInstantiatorCommand
::GenerateImplementationFile(std::ostream& os)
{
  // Include the instantiator class header.
  os <<
    "#include \"" << this->ClassName.c_str() << ".h\"\n"
    "\n";
  
  // Write the extern declarations for all the creation functions.
  for(unsigned int i=0;i < this->Classes.size();++i)
    {
    os << "extern vtkObject* vtkInstantiator" << 
      this->Classes[i].c_str() << "New();\n";
    }
  
  // Write the ClassInitialize method to register all the creation functions.
  os <<
    "\n"
    "void " << this->ClassName.c_str() << "::ClassInitialize()\n"
    "{\n";
  
  for(unsigned int i=0;i < this->Classes.size();++i)
    {
    os << "  vtkInstantiator::RegisterInstantiator(\""
       << this->Classes[i].c_str() << "\", vtkInstantiator"
       << this->Classes[i].c_str() << "New);\n";
    }
  
  // Write the ClassFinalize method to unregister all the creation functions.
  os <<
    "}\n"
    "\n"
    "void " << this->ClassName.c_str() << "::ClassFinalize()\n"
    "{\n";
  
  for(unsigned int i=0;i < this->Classes.size();++i)
    {
    os << "  vtkInstantiator::UnRegisterInstantiator(\""
       << this->Classes[i].c_str() << "\", vtkInstantiator"
       << this->Classes[i].c_str() << "New);\n";
    }
  
  // Write the constructor and destructor of the initializer class to
  // call the ClassInitialize and ClassFinalize methods at the right
  // time.
  os <<
    "}\n"
    "\n" <<
    this->ClassName.c_str() << "::" << this->ClassName.c_str() << "()\n"
    "{\n"
    "  if(++" << this->ClassName.c_str() << "::Count == 1)\n"
    "    { " << this->ClassName.c_str() << "::ClassInitialize(); }\n"
    "}\n"
    "\n" <<
    this->ClassName.c_str() << "::~" << this->ClassName.c_str() << "()\n"
    "{\n"
    "  if(--" << this->ClassName.c_str() << "::Count == 0)\n"
    "    { " << this->ClassName.c_str() << "::ClassFinalize(); }\n"
    "}\n"
    "\n"
    "// Number of translation units that include this class's header.\n"
    "// Purposely not initialized.  Default is static initialization to 0.\n"
    "unsigned int " << this->ClassName.c_str() << "::Count;\n";
}

std::string
cmVTKMakeInstantiatorCommand::OldGenerateCreationFileName(unsigned int block)
{
  cmOStringStream nameStr;
  nameStr << this->ClassName.c_str() << block << ".cxx";
  std::string result = nameStr.str();
  return result;
}

// Generates a file that includes the headers of the classes it knows
// how to create and provides functions which create the classes with
// the New() method.
void
cmVTKMakeInstantiatorCommand
::OldGenerateCreationFile(std::ostream& os, unsigned int groupStart,
                       unsigned int groupSize)
{
  // Need to include header of generated class.
  os <<
    "#include \"" << this->ClassName.c_str() << ".h\"\n"
    "\n";
  
  // Include class files.
  for(unsigned int i=0;i < groupSize;++i)
    {
    os << "#include \"" << this->Classes[groupStart+i].c_str() << ".h\"\n";
    }

  os <<
    "\n";

  // Write the create function implementations.
  for(unsigned int i=0;i < groupSize;++i)
    {
    os << "vtkObject* " << this->ClassName.c_str() << "::Create_"
       << this->Classes[groupStart+i].c_str() << "() { return "
       << this->Classes[groupStart+i].c_str() << "::New(); }\n";
    }
}

// Generates the class header file with the definition of the class
// and its initializer class.
void
cmVTKMakeInstantiatorCommand
::OldGenerateHeaderFile(std::ostream& os)
{
  os <<
    "#ifndef __" << this->ClassName.c_str() << "_h\n"
    "#define __" << this->ClassName.c_str() << "_h\n"
    "\n"
    "#include \"vtkInstantiator.h\"\n";
  for(unsigned int i=0;i < this->Includes.size();++i)
    {
    os << "#include \"" << this->Includes[i].c_str() << "\"\n";
    }
  os <<
    "\n"
    "class " << this->ClassName.c_str() << "Initialize;\n"
    "\n"
    "class " << this->ExportMacro.c_str() << " " 
     << this->ClassName.c_str() << "\n"
    "{\n"
    "  friend class " << this->ClassName.c_str() << "Initialize;\n"
    "\n"
    "  static void ClassInitialize();\n"
    "  static void ClassFinalize();\n"
    "\n";
  
  for(unsigned int i=0;i < this->Classes.size();++i)
    {
    os << "  static vtkObject* Create_" 
       << this->Classes[i].c_str() << "();\n";
    }
  
  // Write the initializer class to make sure the creation functions
  // get registered when this generated header is included.
  os <<
    "};\n"
    "\n"
    "class " << this->ExportMacro.c_str() << " " 
     << this->ClassName.c_str() << "Initialize\n"
    "{\n"
    "public:\n"
    "  " << this->ClassName.c_str() << "Initialize();\n"
    "  ~" << this->ClassName.c_str() << "Initialize();\n"
    "private:\n"
    "  static unsigned int Count;\n"
    "};\n"
    "\n"
    "static " << this->ClassName.c_str() << "Initialize " 
     << this->ClassName.c_str() << "Initializer;\n"
    "\n"
    "#endif\n";  
}

// Generates the file with the implementation of the class.  All
// methods except the actual object creation functions are generated
// here.
void cmVTKMakeInstantiatorCommand
::OldGenerateImplementationFile(std::ostream& os)
{
  // Write the ClassInitialize method to register all the creation functions.
  os <<
    "#include \"" << this->ClassName.c_str() << ".h\"\n"
    "\n"
    "void " << this->ClassName.c_str() << "::ClassInitialize()\n"
    "{\n";
    
  for(unsigned int i=0;i < this->Classes.size();++i)
    {
    os << "  vtkInstantiator::RegisterInstantiator(\""
       << this->Classes[i].c_str() << "\", " 
       << this->ClassName.c_str() << "::Create_"
       << this->Classes[i].c_str() << ");\n";
    }
  
  // Write the ClassFinalize method to unregister all the creation functions.
  os <<
    "}\n"
    "\n"
    "void " << this->ClassName.c_str() << "::ClassFinalize()\n"
    "{\n";
  
  for(unsigned int i=0;i < this->Classes.size();++i)
    {
    os << "  vtkInstantiator::UnRegisterInstantiator(\""
       << this->Classes[i].c_str() << "\", " 
       << this->ClassName.c_str() << "::Create_"
       << this->Classes[i].c_str() << ");\n";
    }
  
  // Write the constructor and destructor of the initializer class to
  // call the ClassInitialize and ClassFinalize methods at the right
  // time.
  os <<
    "}\n"
    "\n" <<
    this->ClassName.c_str() << "Initialize::" << 
    this->ClassName.c_str() << "Initialize()\n"
    "{\n"
    "  if(++" << this->ClassName.c_str() << "Initialize::Count == 1)\n"
    "    { " << this->ClassName.c_str() << "::ClassInitialize(); }\n"
    "}\n"
    "\n" <<
    this->ClassName.c_str() << "Initialize::~" << 
    this->ClassName.c_str() << "Initialize()\n"
    "{\n"
    "  if(--" << this->ClassName.c_str() << "Initialize::Count == 0)\n"
    "    { " << this->ClassName.c_str() << "::ClassFinalize(); }\n"
    "}\n"
    "\n"
    "// Number of translation units that include this class's header.\n"
    "// Purposely not initialized.  Default is static initialization to 0.\n"
    "unsigned int " << this->ClassName.c_str() << "Initialize::Count;\n";
}
