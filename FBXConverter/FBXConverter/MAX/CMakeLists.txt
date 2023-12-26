cmake_minimum_required(VERSION 3.6)

project(max_crack_min)

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

if(UNIX)
  include_directories(/usr/local/include)
  link_libraries(m)
  link_libraries(pthread)
#  link_libraries(libiconv.a)
  link_libraries(uuid)
  if(NOT APPLE)
    link_libraries(stdc++fs)
  endif()
endif(UNIX)

# On Visual 2005 and above, this module can set the debug working directory
cmake_policy(SET CMP0026 OLD)

add_library(3rd_cfbreader INTERFACE)
target_include_directories(3rd_cfbreader INTERFACE "compoundfilereader/src/include")

# glm
add_library(3rd_glm INTERFACE)
target_include_directories(3rd_glm INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/glm-0.9.9.8/)

# will use fbx_converter's rapidjson
# # rapidjson
# set(RAPIDJSON_BUILD_EXAMPLES
#     OFF
#     CACHE BOOL "")
# set(RAPIDJSON_BUILD_TESTS
#     OFF
#     CACHE BOOL "")
# add_subdirectory("3rd/rapidjson")
# add_library(3rd_rapidjson INTERFACE)
# target_include_directories(
#   3rd_rapidjson INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/3rd/rapidjson/include/)

file(GLOB_RECURSE MAX_SRC src/*.h src/*.cpp)
file(GLOB EXT_SRC *.h *.cpp)
add_library(lib3dsmax STATIC ${MAX_SRC} ${EXT_SRC})
target_include_directories(lib3dsmax PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(lib3dsmax 3rd_cfbreader 3rd_glm)
set_property(TARGET lib3dsmax PROPERTY CXX_STANDARD 17)

