set(CMAKE_INCLUDE_CURRENT_DIR ON)

include_directories(BEFORE ${CMAKE_CURRENT_SOURCE_DIR}) 

if(FEATURE_STANDALONE_GC)
  include_directories(env)
else()
  include_directories(BEFORE ${CLR_DIR}/src/vm)
  include_directories(BEFORE ${CLR_DIR}/src/vm/${ARCH_SOURCES_DIR})
endif(FEATURE_STANDALONE_GC)

if(CLR_CMAKE_PLATFORM_UNIX)
  add_compile_options(-fPIC)
endif(CLR_CMAKE_PLATFORM_UNIX)

if(CMAKE_CONFIGURATION_TYPES)
  foreach (Config DEBUG CHECKED)  
     set_property(DIRECTORY APPEND PROPERTY COMPILE_DEFINITIONS $<$<CONFIG:${Config}>:WRITE_BARRIER_CHECK=1>)
  endforeach (Config)
else()
  if(UPPERCASE_CMAKE_BUILD_TYPE STREQUAL DEBUG OR UPPERCASE_CMAKE_BUILD_TYPE STREQUAL CHECKED)
    add_definitions(-DWRITE_BARRIER_CHECK=1)
  endif(UPPERCASE_CMAKE_BUILD_TYPE STREQUAL DEBUG OR UPPERCASE_CMAKE_BUILD_TYPE STREQUAL CHECKED)
endif(CMAKE_CONFIGURATION_TYPES)

set( GC_SOURCES_DAC_AND_WKS_COMMON
  gccommon.cpp
  gcscan.cpp
  gcsvr.cpp
  gcwks.cpp
  handletable.cpp
  handletablecore.cpp
  handletablescan.cpp
  objecthandle.cpp
  softwarewritewatch.cpp)

set( GC_SOURCES_WKS
  ${GC_SOURCES_DAC_AND_WKS_COMMON}
  gceesvr.cpp
  gceewks.cpp
  handletablecache.cpp)

set( GC_SOURCES_DAC
  ${GC_SOURCES_DAC_AND_WKS_COMMON})

if(FEATURE_STANDALONE_GC)
  if(NOT CLR_CMAKE_PLATFORM_UNIX)
    set ( GC_SOURCES_WKS
      ${GC_SOURCES_WKS}
      windows/gcenv.windows.cpp)
  endif(NOT CLR_CMAKE_PLATFORM_UNIX)
endif(FEATURE_STANDALONE_GC)

convert_to_absolute_path(GC_SOURCES_WKS ${GC_SOURCES_WKS})
convert_to_absolute_path(GC_SOURCES_DAC ${GC_SOURCES_DAC})

if(FEATURE_STANDALONE_GC)
  add_library(gc_wks STATIC ${GC_SOURCES_WKS})
else()
  add_subdirectory(wks)
  add_subdirectory(dac)
endif(FEATURE_STANDALONE_GC)
