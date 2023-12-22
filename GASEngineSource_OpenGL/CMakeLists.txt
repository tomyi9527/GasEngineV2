cmake_minimum_required(VERSION 3.6)

project(OGL_test)

cmake_policy(SET CMP0072 OLD)

include(FindPkgConfig)

option(HEADLESS_RENDER "use osmesa for rendering" ON)

if(HEADLESS_RENDER)
  add_definitions(-DHEADLESS_RENDER)
  if(PKG_CONFIG_FOUND AND NOT WIN32)
    pkg_check_modules(OSMESA osmesa)
  endif()
else()
  find_package(OpenGL REQUIRED)
endif()

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

# Compile external dependencies
add_subdirectory(3rd)

if(HEADLESS_RENDER)
  add_definitions(-DUSE_OSMESA)
endif()

if(UNIX)
  include_directories(/usr/local/include)
  link_libraries(m)
  if(NOT APPLE)
    link_libraries(stdc++fs)
  endif()
endif(UNIX)

# On Visual 2005 and above, this module can set the debug working directory
cmake_policy(SET CMP0026 OLD)

# list(APPEND CMAKE_MODULE_PATH
# "${CMAKE_SOURCE_DIR}/3rd/rpavlik-cmake-modules-a3dd481")
# include(CreateLaunchers) include(MSVCMultipleProcessCompile) # /MP

# gather git information

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_MODULE_PATH})

include(GetGitRevisionDescription)

set(REF_NAME "unknown")
set(COMMIT_HASH "unknown")
set(BRANCH_NAME "unknown")
set(COMMIT_AUTHOR "unknown")
set(COMMIT_TIME "unknown")
set(COMMIT_MESSAGE "unknown")
set(COMMIT_TAG "unknown")
set(TAG_AUTHOR "unknown")
set(TAG_TIME "unknown")
set(TAG_MESSAGE "unknown")

get_git_head_revision(REF_NAME COMMIT_HASH)
git_describe(COMMIT_TAG --exact-match ${COMMIT_HASH})
git_tag(${COMMIT_TAG} TAG_AUTHOR "--format=%(authorname)")
git_tag(${COMMIT_TAG} TAG_TIME "--format=%(authordate)")
git_tag(${COMMIT_TAG} TAG_MESSAGE "--format=%(contents)")
git_log(${COMMIT_HASH} COMMIT_AUTHOR --pretty=format:%an)
git_log(${COMMIT_HASH} COMMIT_TIME --pretty=format:%cd --date=iso)
git_log(${COMMIT_HASH} COMMIT_MESSAGE --pretty=format:%s)
git_current_branch(BRANCH_NAME)

string(TIMESTAMP BUILD_TIME "%Y-%m-%d %H:%M:%S")

message(STATUS "run build at ${BUILD_TIME}")
message(STATUS "Git version is ${BRANCH_NAME}:${COMMIT_HASH}")
message(STATUS "commit time is ${COMMIT_TIME}, commit author is ${COMMIT_AUTHOR}")
message(STATUS "commit message is \"${COMMIT_MESSAGE}\"")
message(STATUS "commit tag is ${COMMIT_TAG}, message is \"${TAG_MESSAGE}\"")

# 轻量标签取不到这两个值
message(STATUS "tag time is ${TAG_TIME}, tag author is ${TAG_AUTHOR}")

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/ver.h.in
  ${CMAKE_BINARY_DIR}/ver.h
  @ONLY
  )
include_directories(${CMAKE_BINARY_DIR})


set(ALL_LIBS
    ${OPENGL_LIBRARY}
    ${OSMESA_LIBRARIES}
    # 以上两个总会有一个的
    3rd_gflags
    3rd_glfw
    3rd_glad
    3rd_glm
    3rd_rapidjson
    3rd_FreeImage
    3rd_log4cplus
    3rd_httplib
    3rd_zlib
    3rd_openssl)

option(USE_MAGICK "use magick for gif making" OFF)
if (USE_MAGICK)
  list(APPEND ALL_LIBS 3rd_ImageMagick)
endif()

if(UNIX AND NOT APPLE)
  list(APPEND ALL_LIBS stdc++fs)
endif()

add_definitions(-DTW_STATIC -DTW_NO_LIB_PRAGMA -DTW_NO_DIRECT3D
                -D_CRT_SECURE_NO_WARNINGS -DGLFW_INCLUDE_NONE)

set(COMMON_SRC LoadShaders.cpp bmp.cpp 3rd/glad/src/glad.c)

add_subdirectory(src)
add_subdirectory(test)
add_subdirectory(samples)
add_subdirectory(app)
