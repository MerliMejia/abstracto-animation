cmake_minimum_required(VERSION 3.20)
project(abstracto_animation LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(glm CONFIG REQUIRED)

add_library(abstracto_animation INTERFACE)
target_include_directories(abstracto_animation INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(abstracto_animation INTERFACE glm::glm)

add_executable(AbstractoAnimationExample apps/animation_example/main.cpp)
target_link_libraries(AbstractoAnimationExample PRIVATE abstracto_animation)
