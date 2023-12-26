# author: beanpliu@tencent.com
cmake_minimum_required(VERSION 3.6)

project(fbx_converter)

# set module path
set (CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_CURRENT_SOURCE_DIR}")
# set flags
add_compile_options("$<$<CONFIG:Debug>:-g>")
add_compile_options("$<$<CONFIG:Debug>:-D_DEBUG>")
# set output binary path
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../../build/ CACHE PATH "" FORCE)
# for multi config generator:
foreach(OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES})
string(TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CMAKE_CURRENT_SOURCE_DIR}/../../build/ CACHE PATH "" FORCE)
endforeach()

# for tootle
add_compile_options(-D_SOFTWARE_ONLY_VERSION)

# replace MD with MT
foreach(flag_var
    CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE
    CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO)
  if(${flag_var} MATCHES "/MD")
    string(REGEX REPLACE "/MD" "/MT" ${flag_var} "${${flag_var}}")
  endif()
endforeach()

# some other options
if(CMAKE_COMPILER_IS_GNUCC)
  add_definitions(
    -Wall -Wpointer-arith -Wmissing-declarations -fno-strict-aliasing
    # -Wold-style-definition -Wdeclaration-after-statement
  )
  set(CMAKE_C_FLAGS
      "-Wmissing-prototypes -Wnested-externs -Wbad-function-cast ${CMAKE_CXX_FLAGS}"
  )
endif()

if(CMAKE_BINARY_DIR STREQUAL CMAKE_SOURCE_DIR)
  message(
    FATAL_ERROR
      "Please select another Build Directory ! (and give it a clever name, like bin_Visual2012_64bits/)"
  )
endif()
if(CMAKE_SOURCE_DIR MATCHES " ")
  message(
    "Your Source Directory contains spaces. If you experience problems when compiling, this can be the cause."
  )
endif()
if(CMAKE_BINARY_DIR MATCHES " ")
  message(
    "Your Build Directory contains spaces. If you experience problems when compiling, this can be the cause."
  )
endif()

# link libraries
if(UNIX)
  include_directories(/usr/local/include)
  link_libraries(m dl rt uuid stdc++ pthread libiconv.a liblzma.a)
  if(NOT APPLE)
    link_libraries(stdc++fs)
  endif()
endif(UNIX)

if (WIN32)
  link_libraries(Rpcrt4.lib)
  add_compile_options(-DNOMINMAX)
endif()

# On Visual 2005 and above, this module can set the debug working directory
cmake_policy(SET CMP0026 OLD)


# 3rd: find fbx sdk
# linux should be install to /usr/local so that FindFbx will automatically find that.
if(WIN32)
    set(FBX_DIR ${CMAKE_CURRENT_SOURCE_DIR}/FBXSDK2020_0_1)
endif()

set(FBX_MSVC_RT_DLL OFF CACHE BOOL "" FORCE)
find_package(Fbx REQUIRED)
if (NOT FBX_FOUND)
message("fbx sdk not found")
endif()
add_library(3rd_fbxsdk INTERFACE)
target_include_directories(3rd_fbxsdk INTERFACE ${FBX_INCLUDE_DIRS})
target_link_libraries(3rd_fbxsdk INTERFACE "$<$<CONFIG:Debug>:${FBX_LIBRARIES_DEBUG}>")
target_link_libraries(3rd_fbxsdk INTERFACE "$<$<NOT:$<CONFIG:Debug>>:${FBX_LIBRARIES}>")

# MAX:
add_subdirectory(MAX)
# will import target: lib3dsmax

# find all source code
# some of 3rd party code do not need to use as module/library, they are also included here

# zlib
file(GLOB_RECURSE ZLIB_SRC 
zlib-1.2.8/*.cpp
zlib-1.2.8/*.cxx
zlib-1.2.8/*.c
zlib-1.2.8/*.cc
zlib-1.2.8/*.h
zlib-1.2.8/*.hpp)
include_directories(zlib-1.2.8)
# tootle
file(GLOB_RECURSE TOOTLE_SRC 
TootleLib/*.cpp
TootleLib/*.cxx
TootleLib/*.c
TootleLib/*.cc
TootleLib/*.h
TootleLib/*.hpp)
include_directories(TootleLib)
include_directories(TootleLib/include)
include_directories(TootleLib/RayTracer)
include_directories(TootleLib/RayTracer/Math)
include_directories(TootleLib/RayTracer/JRT)
# tristripper
file(GLOB_RECURSE TRISTRIPPER_SRC 
tristripper/*.cpp
tristripper/*.cxx
tristripper/*.c
tristripper/*.cc
tristripper/*.h
tristripper/*.hpp)
include_directories(tristripper)
include_directories(tristripper/include)
# NvTriStrip
file(GLOB_RECURSE NVTRISTRIP_SRC 
NvTriStrip/*.cpp
NvTriStrip/*.cxx
NvTriStrip/*.c
NvTriStrip/*.cc
NvTriStrip/*.h
NvTriStrip/*.hpp)
include_directories(NvTriStrip)
# tga
file(GLOB_RECURSE TGA_SRC 
libtga-1.0.1/*.cpp
libtga-1.0.1/*.cxx
libtga-1.0.1/*.c
libtga-1.0.1/*.cc
libtga-1.0.1/*.h
libtga-1.0.1/*.hpp)
include_directories(libtga-1.0.1)
# jpeg-6b
file(GLOB_RECURSE JPEG_SRC 
jpeg-6b/*.cpp
jpeg-6b/*.cxx
jpeg-6b/*.c
jpeg-6b/*.cc
jpeg-6b/*.h
jpeg-6b/*.hpp)
include_directories(jpeg-6b)
# lpng1627
file(GLOB_RECURSE PNG_SRC 
lpng1627/*.cpp
lpng1627/*.cxx
lpng1627/*.c
lpng1627/*.cc
lpng1627/*.h
lpng1627/*.hpp)
include_directories(lpng1627)
# TexturePipeline
file(GLOB_RECURSE TEXTURE_PIPELINE_SRC 
TexturePipeline/*.cpp
TexturePipeline/*.cxx
TexturePipeline/*.c
TexturePipeline/*.cc
TexturePipeline/*.h
TexturePipeline/*.hpp)
include_directories(TexturePipeline)
# optimization
file(GLOB_RECURSE OPTIMIZATION_SRC 
optimization/*.cpp
optimization/*.cxx
optimization/*.c
optimization/*.cc
optimization/*.h
optimization/*.hpp)
include_directories(optimization)
include_directories(optimization/GAS_V1)
include_directories(optimization/GAS_V2)
include_directories(optimization/GLTF_V2)
# this dir
file(GLOB SRC *.cpp *.cxx *.c *.h *.hpp *.cc)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
file(GLOB_RECURSE COMMON_SRC
Common/*.cpp
Common/*.cxx
Common/*.cc
Common/*.c
Common/*.h
Common/*.hpp)
file(GLOB_RECURSE CJSON_SRC
cJSON/*.cpp
cJSON/*.cxx
cJSON/*.cc
cJSON/*.c
cJSON/*.h
cJSON/*.hpp)
file(GLOB_RECURSE MMD_SRC
MMD/*.cpp
MMD/*.cxx
MMD/*.cc
MMD/*.c
MMD/*.h
MMD/*.hpp)
file(GLOB_RECURSE STL_SRC
STL/*.cpp
STL/*.cxx
STL/*.cc
STL/*.c
STL/*.h
STL/*.hpp)
# rapidjson
include_directories(rapidjson)

set(MAIN_SRC getopt_win.c getopt_win.h FBXConverter.cpp)
list(REMOVE_ITEM SRC ${MAIN_SRC})

source_group(TREE ${CMAKE_SOURCE_DIR} FILES ${ZLIB_SRC})
source_group(TREE ${CMAKE_SOURCE_DIR} FILES ${TOOTLE_SRC})
source_group(TREE ${CMAKE_SOURCE_DIR} FILES ${TRISTRIPPER_SRC})
source_group(TREE ${CMAKE_SOURCE_DIR} FILES ${NVTRISTRIP_SRC})
source_group(TREE ${CMAKE_SOURCE_DIR} FILES ${TGA_SRC})
source_group(TREE ${CMAKE_SOURCE_DIR} FILES ${JPEG_SRC})
source_group(TREE ${CMAKE_SOURCE_DIR} FILES ${PNG_SRC})
source_group(TREE ${CMAKE_SOURCE_DIR} FILES ${TEXTURE_PIPELINE_SRC})
source_group(TREE ${CMAKE_SOURCE_DIR} FILES ${OPTIMIZATION_SRC})
source_group(TREE ${CMAKE_SOURCE_DIR} FILES ${COMMON_SRC})
source_group(TREE ${CMAKE_SOURCE_DIR} FILES ${CJSON_SRC})
source_group(TREE ${CMAKE_SOURCE_DIR} FILES ${MMD_SRC})
source_group(TREE ${CMAKE_SOURCE_DIR} FILES ${STL_SRC})
source_group(TREE ${CMAKE_SOURCE_DIR} FILES ${SRC})
source_group(TREE ${CMAKE_SOURCE_DIR} FILES ${MAIN_SRC})

# target
add_executable(FBXConverter
${ZLIB_SRC}
${TOOTLE_SRC}
${TRISTRIPPER_SRC}
${NVTRISTRIP_SRC}
${TGA_SRC}
${JPEG_SRC}
${PNG_SRC}
${TEXTURE_PIPELINE_SRC}
${OPTIMIZATION_SRC}
${COMMON_SRC}
${CJSON_SRC}
${MMD_SRC}
${STL_SRC}
${SRC}
${MAIN_SRC})
target_link_libraries(FBXConverter 3rd_fbxsdk lib3dsmax)
set_target_properties(FBXConverter PROPERTIES OUTPUT_NAME "FBXConverter_$<$<CONFIG:Debug>:debug>$<$<NOT:$<CONFIG:Debug>>:release>")
set_property(TARGET FBXConverter PROPERTY CXX_STANDARD 17)

