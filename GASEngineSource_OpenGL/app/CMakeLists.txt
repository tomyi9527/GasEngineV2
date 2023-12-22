cmake_minimum_required(VERSION 3.6)

file(GLOB SRC_COMMON common/*.cpp common/*.h)
add_library(App.common STATIC ${SRC_COMMON})
target_include_directories(App.common
                           PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/common/)
target_link_libraries(App.common ECS)
set_property(TARGET App.common PROPERTY CXX_STANDARD 17)

# local application application 1, take screen-shot using local input file.
file(GLOB SRC_SCREEN_SHOT screen_shot/*.cpp screen_shot/*.h)
add_executable(App.ScreenShot ${SRC_SCREEN_SHOT})
target_link_libraries(App.ScreenShot App.common)
set_property(TARGET App.ScreenShot PROPERTY CXX_STANDARD 17)

if(NOT HEADLESS_RENDER)
  # application 2, viewer app, headless_render should be set to OFF.
  file(GLOB SRC_VIEWER viewer/*.cpp viewer/*.h)
  add_executable(App.Viewer ${SRC_VIEWER})
  target_link_libraries(App.Viewer App.common)
  set_property(TARGET App.Viewer PROPERTY CXX_STANDARD 17)

  # arthub application
  # application 3, online(arthub) viewer app, headless_render should be set to
  # OFF.
  file(GLOB SRC_ARTHUB_VIEWER online_viewer/*.cpp online_viewer/*.h)
  add_executable(App.ArthubViewer ${SRC_ARTHUB_VIEWER})
  target_link_libraries(App.ArthubViewer App.common)
  set_property(TARGET App.ArthubViewer PROPERTY CXX_STANDARD 17)
endif()

# application 4, take screen-shot using arthub input file.
file(GLOB SRC_ARTHUB_SCREEN_SHOT online_screen_shot/*.cpp
     online_screen_shot/*.h)
add_executable(App.ArthubScreenShot ${SRC_ARTHUB_SCREEN_SHOT})
target_link_libraries(App.ArthubScreenShot App.common)
set_property(TARGET App.ArthubScreenShot PROPERTY CXX_STANDARD 17)