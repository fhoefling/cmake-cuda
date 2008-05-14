SET(CMAKE_LIBRARY_PATH_FLAG "libpath ")
SET(CMAKE_LINK_LIBRARY_FLAG "library ")
SET(CMAKE_LINK_LIBRARY_FILE_FLAG "library")

IF(CMAKE_VERBOSE_MAKEFILE)
  SET(CMAKE_WCL_QUIET)
  SET(CMAKE_WLINK_QUIET)
  SET(CMAKE_LIB_QUIET)
ELSE(CMAKE_VERBOSE_MAKEFILE)
  SET(CMAKE_WCL_QUIET "-zq")
  SET(CMAKE_WLINK_QUIET "option quiet")
  SET(CMAKE_LIB_QUIET "-q")
ENDIF(CMAKE_VERBOSE_MAKEFILE)

SET(CMAKE_BUILD_TYPE_INIT Debug)
SET (CMAKE_CXX_FLAGS_INIT "-w=3 -xs")
SET (CMAKE_CXX_FLAGS_DEBUG_INIT "-br -bm  -d2")
SET (CMAKE_CXX_FLAGS_MINSIZEREL_INIT "-br -bm -os -dNDEBUG")
SET (CMAKE_CXX_FLAGS_RELEASE_INIT "-br -bm -ot -dNDEBUG")
SET (CMAKE_CXX_FLAGS_RELWITHDEBINFO_INIT "-br -bm  -d2 -ot -dNDEBUG")
SET (CMAKE_C_FLAGS_INIT "-w=3 ")
SET (CMAKE_C_FLAGS_DEBUG_INIT "-br -bm  -od")
SET (CMAKE_C_FLAGS_MINSIZEREL_INIT "-br -bm -os -dNDEBUG")
SET (CMAKE_C_FLAGS_RELEASE_INIT "-br -bm -ot -dNDEBUG")
SET (CMAKE_C_FLAGS_RELWITHDEBINFO_INIT "-br -bm  -ot -dNDEBUG")
SET (CMAKE_C_STANDARD_LIBRARIES_INIT "library clbrdll.lib library plbrdll.lib  library kernel32.lib library user32.lib library gdi32.lib library winspool.lib library comdlg32.lib library advapi32.lib library shell32.lib library ole32.lib library oleaut32.lib library uuid.lib library odbc32.lib library odbccp32.lib")
SET (CMAKE_CXX_STANDARD_LIBRARIES_INIT "${CMAKE_C_STANDARD_LIBRARIES_INIT}")

SET(CMAKE_C_CREATE_IMPORT_LIBRARY
  "wlib -q -n -b <TARGET_IMPLIB> +<TARGET>")
SET(CMAKE_CXX_CREATE_IMPORT_LIBRARY ${CMAKE_C_CREATE_IMPORT_LIBRARY})

SET(CMAKE_C_LINK_EXECUTABLE
    "wlink ${CMAKE_START_TEMP_FILE} ${CMAKE_WLINK_QUIET} name <TARGET> option caseexact file {<OBJECTS>} <LINK_LIBRARIES> ${CMAKE_END_TEMP_FILE}")

SET(CMAKE_CXX_LINK_EXECUTABLE ${CMAKE_C_LINK_EXECUTABLE})

# compile a C++ file into an object file
SET(CMAKE_CXX_COMPILE_OBJECT
    "<CMAKE_CXX_COMPILER> ${CMAKE_START_TEMP_FILE} ${CMAKE_WCL_QUIET} <FLAGS> -dWIN32 -d+ <DEFINES> -fo<OBJECT> -c -cc++ <SOURCE>${CMAKE_END_TEMP_FILE}")

# compile a C file into an object file
SET(CMAKE_C_COMPILE_OBJECT
    "<CMAKE_C_COMPILER> ${CMAKE_START_TEMP_FILE} ${CMAKE_WCL_QUIET} <FLAGS> -dWIN32 -d+ <DEFINES> -fo<OBJECT> -c -cc <SOURCE>${CMAKE_END_TEMP_FILE}")

# preprocess a C source file
SET(CMAKE_C_CREATE_PREPROCESSED_SOURCE
    "<CMAKE_C_COMPILER> ${CMAKE_START_TEMP_FILE} ${CMAKE_WCL_QUIET} <FLAGS> -dWIN32 -d+ <DEFINES> -fo<PREPROCESSED_SOURCE> -pl -cc <SOURCE>${CMAKE_END_TEMP_FILE}")

# preprocess a C++ source file
SET(CMAKE_CXX_CREATE_PREPROCESSED_SOURCE
    "<CMAKE_CXX_COMPILER> ${CMAKE_START_TEMP_FILE} ${CMAKE_WCL_QUIET} <FLAGS> -dWIN32 -d+ <DEFINES> -fo<PREPROCESSED_SOURCE> -pl -cc++ <SOURCE>${CMAKE_END_TEMP_FILE}")

SET(CMAKE_CXX_CREATE_SHARED_MODULE
 "wlink ${CMAKE_START_TEMP_FILE} system nt_dll  ${CMAKE_WLINK_QUIET} name <TARGET> option caseexact  file {<OBJECTS>} <LINK_LIBRARIES> ${CMAKE_END_TEMP_FILE}")
SET(CMAKE_CXX_CREATE_SHARED_LIBRARY
  ${CMAKE_CXX_CREATE_SHARED_MODULE}
  ${CMAKE_CXX_CREATE_IMPORT_LIBRARY})

# create a C shared library
SET(CMAKE_C_CREATE_SHARED_LIBRARY ${CMAKE_CXX_CREATE_SHARED_LIBRARY})

# create a C shared module
SET(CMAKE_C_CREATE_SHARED_MODULE ${CMAKE_CXX_CREATE_SHARED_MODULE})

# create a C++ static library
SET(CMAKE_CXX_CREATE_STATIC_LIBRARY  "wlib ${CMAKE_LIB_QUIET} -n -b <TARGET> <OBJECTS> ")

# create a C static library
SET(CMAKE_C_CREATE_STATIC_LIBRARY ${CMAKE_CXX_CREATE_STATIC_LIBRARY})
