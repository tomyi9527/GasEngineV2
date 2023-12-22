cmake_minimum_required(VERSION 3.6)

if(MSVC AND NOT "${MSVC_VERSION}" LESS 1400)
  add_definitions("/MP")
  add_definitions(-DTW_STATIC -DTW_NO_LIB_PRAGMA -DTW_NO_DIRECT3D
                  -D_CRT_SECURE_NO_WARNINGS)
endif()

set(BUILD_TESTING
    OFF
    CACHE BOOL "")
set(WITH_UNIT_TESTS
    OFF
    CACHE BOOL "")

# gflags
add_subdirectory(gflags-2.2.2)
add_library(3rd_gflags INTERFACE)
target_link_libraries(3rd_gflags INTERFACE gflags::gflags)

# glm
add_library(3rd_glm INTERFACE)
target_include_directories(3rd_glm
                           INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/glm-0.9.9.8/)

# # glew
# if(HEADLESS_RENDER)
#   add_definitions(-DGLEW_OSMESA)
#   set(GLEW_OSMESA ON CACHE BOOL "")
# else()
#   add_definitions(-DGLEW_STATIC)
# endif()
# add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/glew)
# add_library(3rd_glew INTERFACE)
# target_include_directories(3rd_glew INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/glew/include)
# target_link_libraries(3rd_glew INTERFACE glew)

# glad
add_library(3rd_glad STATIC ${CMAKE_CURRENT_SOURCE_DIR}/glad/src/glad.c)
target_include_directories(3rd_glad
                           PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/glad/include/)

# glfw
if(HEADLESS_RENDER)
  set(GLFW_USE_OSMESA
      ON
      CACHE BOOL "")
endif()
set(GLFW_BUILD_EXAMPLES
    OFF
    CACHE BOOL "")
set(GLFW_BUILD_TESTS
    OFF
    CACHE BOOL "")
add_subdirectory(glfw-3.3.2)
add_library(3rd_glfw INTERFACE)
target_include_directories(
  3rd_glfw INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/glfw-3.3.2/include/)
target_link_libraries(3rd_glfw INTERFACE ${OPENGL_LIBRARY} ${CMAKE_DL_LIBS}
                                         ${GLFW_LIBRARIES} glfw)

# rapidjson
set(RAPIDJSON_BUILD_EXAMPLES
    OFF
    CACHE BOOL "")
set(RAPIDJSON_BUILD_TESTS
    OFF
    CACHE BOOL "")
add_subdirectory(rapidjson)
add_library(3rd_rapidjson INTERFACE)
target_include_directories(
  3rd_rapidjson INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/rapidjson/include/)

# FreeImage
add_library(3rd_FreeImage INTERFACE)

set(SEARCH_PATHS
    $ENV{ProgramFiles}/freeimage/include
    $ENV{SystemDrive}/freeimage/include
    $ENV{ProgramFiles}/freeimage
    $ENV{SystemDrive}/freeimage
    /usr/include
    /usr/local/include
    /sw/include
    /opt/local/include
    ${CMAKE_CURRENT_SOURCE_DIR}/FreeImage/Dist/x64)
find_path(
  FREEIMAGE_INCLUDE_DIRS
  NAMES FreeImage.h
  PATHS ${SEARCH_PATHS}
  NO_DEFAULT_PATH)
if(NOT FREEIMAGE_INCLUDE_DIRS) # now look in system locations
  find_path(FREEIMAGE_INCLUDE_DIRS NAMES FreeImage.h)
endif()

set(SEARCH_LIBRARY_DIRS
    $ENV{ProgramFiles}/freeimage/lib
    $ENV{SystemDrive}/freeimage/lib
    $ENV{ProgramFiles}/freeimage/Dist/x64
    $ENV{SystemDrive}/freeimage/Dist/x64
    $ENV{ProgramFiles}/freeimage
    $ENV{SystemDrive}/freeimage
    ${CMAKE_CURRENT_SOURCE_DIR}/FreeImage/Dist/x64
    /usr/lib64
    /usr/lib
    /usr/local/lib64
    /usr/local/lib
    /sw/lib
    /opt/local/lib)
find_library(
  FREEIMAGE_LIBRARY
  NAMES freeimage
  PATHS ${SEARCH_LIBRARY_DIRS}
  NO_DEFAULT_PATH)
if(NOT FREEIMAGE_LIBRARY) # now look in system locations
  find_library(FREEIMAGE_LIBRARY NAMES freeimage)
endif()

if(FREEIMAGE_INCLUDE_DIRS AND FREEIMAGE_LIBRARY)
  message(
    STATUS
      "Found FreeImage: headers at ${FREEIMAGE_INCLUDE_DIRS}, libraries at ${FREEIMAGE_LIBRARY}"
  )
  target_include_directories(3rd_FreeImage INTERFACE ${FREEIMAGE_INCLUDE_DIRS})
  target_link_libraries(3rd_FreeImage INTERFACE ${FREEIMAGE_LIBRARY})
else()
  message(SEND_ERROR "FreeImage Not Found")
endif()

# log4cplus 一个log4j的port
set(WITH_UNIT_TESTS
    OFF
    CACHE BOOL "")
set(WITH_APPS
    OFF
    CACHE BOOL "")
set(LOG4CPLUS_BUILD_TESTING
    OFF
    CACHE BOOL "")
set(LOG4CPLUS_WORKING_LOCALE
    ON
    CACHE BOOL "")
set(LOG4CPLUS_ENABLE_DECORATED_LIBRARY_NAME
    OFF
    CACHE BOOL "") # 不开的话项目名会有后缀
if(WIN32)
  set(UNICODE
      OFF
      CACHE BOOL "")
endif()
add_subdirectory(log4cplus)
add_library(3rd_log4cplus INTERFACE)
target_include_directories(
  3rd_log4cplus INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/log4cplus/include)
target_link_libraries(3rd_log4cplus INTERFACE log4cplus)

# httplib
add_library(3rd_httplib INTERFACE)
target_include_directories(
  3rd_httplib INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/cpp-httplib-0.7.13)

# zlib
add_subdirectory(zlib)
add_library(3rd_zlib INTERFACE)
target_include_directories(3rd_zlib INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/zlib)
target_link_libraries(3rd_zlib INTERFACE zlibstatic)

# openssl
add_library(3rd_openssl INTERFACE)
add_subdirectory(openssl)
target_include_directories(
  3rd_openssl INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/openssl/include)
# find_package(OpenSSL REQUIRED)  # centos自带的版本太低了
target_link_libraries(3rd_openssl INTERFACE crypto ssl)
if(WIN32)
  target_link_libraries(3rd_openssl INTERFACE Bcrypt wininet Version winhttp)
endif()

# image_magick
find_package(ImageMagick COMPONENTS MagickCore Magick++)
add_library(3rd_ImageMagick INTERFACE)
if(ImageMagick_FOUND)
    # Find Imagemagick Library directory
    get_filename_component(MAGICK_LIB_DIR ${ImageMagick_MagickCore_LIBRARY} DIRECTORY)
    # Find where Magick++-config lives
    file(GLOB_RECURSE MAGICK_CONFIG FOLLOW_SYMLINKS ${MAGICK_LIB_DIR}/Magick++-config)
    # Ask about CXX and lib flags/locations
    set(MAGICK_CONFIG ${MAGICK_CONFIG} CACHE string "Path to Magick++-config utility")
    execute_process(COMMAND "${MAGICK_CONFIG}" "--cxxflags" OUTPUT_VARIABLE MAGICK_CXX_FLAGS)
    execute_process(COMMAND "${MAGICK_CONFIG}" "--libs" OUTPUT_VARIABLE MAGICK_LD_FLAGS)
    # Add these to cache
    set(MAGICK_CXX_FLAGS "${MAGICK_CXX_FLAGS}" CACHE string "ImageMagick configuration specific compilation flags." )
    set(MAGICK_LD_FLAGS  "${MAGICK_LD_FLAGS}" CACHE string "ImageMagick configuration specific linking flags.")
    # Split into list:
    string(REGEX MATCHALL "([^\ ]+)" MAGICK_CXX_FLAGS "${MAGICK_CXX_FLAGS}")
    string(REGEX MATCHALL "([^\ ]+)" MAGICK_LD_FLAGS "${MAGICK_LD_FLAGS}")
    # Remove trailing whitespace (CMAKE warns about this)
    string(STRIP "${MAGICK_CXX_FLAGS}" MAGICK_CXX_FLAGS)
    string(STRIP "${MAGICK_LD_FLAGS}" MAGICK_LD_FLAGS)

    target_compile_options(3rd_ImageMagick INTERFACE ${MAGICK_CXX_FLAGS} -DGIF_MAKER_USING_MAGICK)
    target_include_directories(3rd_ImageMagick INTERFACE ${ImageMagick_INCLUDE_DIRS})
    target_link_libraries(3rd_ImageMagick INTERFACE ${MAGICK_LD_FLAGS} ${ImageMagick_LIBRARIES})
else()
	message("Image Magick not found, will not use magick for gif making.")
endif()
