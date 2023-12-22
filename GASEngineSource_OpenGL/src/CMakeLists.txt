cmake_minimum_required(VERSION 3.6)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

file(GLOB_RECURSE SRC_UTILS_SRC utils/*.cpp utils/*.h)
add_library(Utils STATIC ${SRC_UTILS_SRC})
set_property(TARGET Utils PROPERTY CXX_STANDARD 17)
target_link_libraries(Utils PUBLIC ${ALL_LIBS})
target_include_directories(Utils PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/utils
                                        ${CMAKE_CURRENT_SOURCE_DIR})

add_library(
  StandAlone.ObjectLoader STATIC
  data_types/mesh_loader.h
  data_types/mesh_loader.cpp
  data_types/shader_factory.cpp
  opengl/renderable_item.cpp
  opengl/global_resource.cpp
  opengl/buffer_type.cpp
  opengl/opengl_interface.cpp
  ecs/entity_factory.cpp
  ecs/component_factory.cpp
  utils/resource_manager.cpp)
set_property(TARGET StandAlone.ObjectLoader PROPERTY CXX_STANDARD 17)
target_link_libraries(StandAlone.ObjectLoader PUBLIC Utils)
target_include_directories(
  StandAlone.ObjectLoader PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/data_types
                                 ${CMAKE_CURRENT_SOURCE_DIR})

add_library(StandAlone.OpenGLInterface STATIC opengl/opengl_interface.cpp
                                              opengl/opengl_interface.h)
set_property(TARGET StandAlone.OpenGLInterface PROPERTY CXX_STANDARD 17)
target_link_libraries(StandAlone.OpenGLInterface PUBLIC Utils ${ALL_LIBS})
target_include_directories(
  StandAlone.OpenGLInterface PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/opengl
                                    ${CMAKE_CURRENT_SOURCE_DIR})

# add_library(StandAlone.Gas2Loader STATIC ecs/loader/gas2_loader.cpp)
# set_property(TARGET StandAlone.Gas2Loader PROPERTY CXX_STANDARD 17)
# target_link_libraries(StandAlone.Gas2Loader StandAlone.ObjectLoader ${ALL_LIBS})
# target_include_directories(StandAlone.Gas2Loader PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/data_types ${CMAKE_CURRENT_SOURCE_DIR})



file(GLOB_RECURSE SRC_ALL_SRC *.cpp *.h)
add_library(ECS STATIC ${SRC_ALL_SRC})
set_property(TARGET ECS PROPERTY CXX_STANDARD 17)
target_link_libraries(ECS PUBLIC ${ALL_LIBS})
target_include_directories(ECS PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
