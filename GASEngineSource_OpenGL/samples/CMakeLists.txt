cmake_minimum_required(VERSION 3.6)

# example 1, load mesh binary.
add_executable(Sample.ObjectLoader sample_mesh_loader.cpp)
target_link_libraries(Sample.ObjectLoader StandAlone.ObjectLoader)
set_property(TARGET Sample.ObjectLoader PROPERTY CXX_STANDARD 17)

# example 2, show simple graphics.
add_executable(Sample.OpenGLShowTriangle sample_opengl_show_triangle.cpp)
target_link_libraries(Sample.OpenGLShowTriangle StandAlone.OpenGLInterface)
set_property(TARGET Sample.OpenGLShowTriangle PROPERTY CXX_STANDARD 17)

# example 3, ecs test.
add_executable(Sample.ECSSimple sample_ecs_simple.cpp)
target_link_libraries(Sample.ECSSimple ECS)
set_property(TARGET Sample.ECSSimple PROPERTY CXX_STANDARD 17)

# example 4, loader test.
add_executable(Sample.Gas2LoaderTest sample_gas2_loader.cpp)
target_link_libraries(Sample.Gas2LoaderTest ECS)
set_property(TARGET Sample.Gas2LoaderTest PROPERTY CXX_STANDARD 17)

# example 5, material and render test.
add_executable(Sample.CreateSimpleRenderableTest
               sample_create_simple_renderable.cpp)
target_link_libraries(Sample.CreateSimpleRenderableTest ECS)
set_property(TARGET Sample.CreateSimpleRenderableTest PROPERTY CXX_STANDARD 17)

# example 6, skybox render test.
add_executable(Sample.SkyboxRender sample_skybox_render.cpp)
target_link_libraries(Sample.SkyboxRender ECS)
set_property(TARGET Sample.SkyboxRender PROPERTY CXX_STANDARD 17)

# example 7, ecs render test.
add_executable(Sample.ECSRender sample_ecs_render.cpp)
target_link_libraries(Sample.ECSRender ECS)
set_property(TARGET Sample.ECSRender PROPERTY CXX_STANDARD 17)

# example 8, keyframe animator test.
add_executable(Sample.AnimatorLoadTest sample_keyframe_animator_loader.cpp)
target_link_libraries(Sample.AnimatorLoadTest ECS)
set_property(TARGET Sample.AnimatorLoadTest PROPERTY CXX_STANDARD 17)