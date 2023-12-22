cmake_minimum_required(VERSION 3.6)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_executable(Test.utils utils_test.cpp)
target_link_libraries(Test.utils Utils)
set_property(TARGET Test.utils PROPERTY CXX_STANDARD 17)

add_executable(Test.bbox bbox_test.cpp)
target_link_libraries(Test.bbox Utils)
set_property(TARGET Test.bbox PROPERTY CXX_STANDARD 17)

add_executable(Test.json_maker json_maker_test.cpp)
target_link_libraries(Test.json_maker Utils)
set_property(TARGET Test.json_maker PROPERTY CXX_STANDARD 17)

add_executable(
  Test.arthub_client
  arthub_client_test.cpp ../app/common/arthub_client.cpp
  ../app/common/arthub_client.h ../app/common/arthub_client_format.cpp
  ../app/common/arthub_client_format.h)
target_link_libraries(Test.arthub_client 3rd_httplib Utils)
set_property(TARGET Test.arthub_client PROPERTY CXX_STANDARD 17)

add_executable(Test.quat_ext quat_ext_test.cpp)
target_link_libraries(Test.quat_ext Utils)
set_property(TARGET Test.quat_ext PROPERTY CXX_STANDARD 17)