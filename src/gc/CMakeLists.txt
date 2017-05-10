set(CMAKE_INCLUDE_CURRENT_DIR ON)

include_directories(BEFORE ${CMAKE_CURRENT_SOURCE_DIR})
include_directories(BEFORE ${CMAKE_CURRENT_SOURCE_DIR}/env)

add_definitions(-DBUILD_AS_STANDALONE)

if(CLR_CMAKE_PLATFORM_UNIX)
  add_compile_options(-fPIC)
  # dprintf causes many warnings
  add_compile_options(-Wno-format)
endif(CLR_CMAKE_PLATFORM_UNIX)

if(CLR_CMAKE_PLATFORM_DARWIN)
  add_definitions(-D_DARWIN_C_SOURCE)
endif(CLR_CMAKE_PLATFORM_DARWIN)

remove_definitions(-DFEATURE_EVENT_TRACE)
remove_definitions(-DPROFILING_SUPPORTED)
remove_definitions(-DSTRESS_HEAP)
remove_definitions(-DFEATURE_APPDOMAIN_RESOURCE_MONITORING)

set( GC_SOURCES
  gcconfig.cpp
  gccommon.cpp
  gcscan.cpp
  gcsvr.cpp
  gcwks.cpp
  handletable.cpp
  handletablecore.cpp
  handletablescan.cpp
  objecthandle.cpp
  softwarewritewatch.cpp
  gchandletable.cpp
  gceesvr.cpp
  gceewks.cpp
  handletablecache.cpp)

if(CLR_CMAKE_PLATFORM_UNIX)
include(unix/configure.cmake)
set ( GC_SOURCES
  ${GC_SOURCES}
  unix/gcenv.unix.cpp
  unix/events.cpp
  unix/cgroup.cpp)
else()
set ( GC_SOURCES
  ${GC_SOURCES}
  windows/gcenv.windows.cpp)
endif(CLR_CMAKE_PLATFORM_UNIX)

convert_to_absolute_path(GC_SOURCES ${GC_SOURCES})

add_library(gc_standalone STATIC ${GC_SOURCES})
