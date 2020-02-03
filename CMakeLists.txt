cmake_minimum_required(VERSION 3.15)
project(xoshiro C)

set(CMAKE_C_STANDARD 99)

file(GLOB SRC "src/*.c")

if(${CMAKE_BUILD_TYPE} STREQUAL Debug)
    add_compile_definitions(DEBUG)
endif()

add_library(${PROJECT_NAME} SHARED ${SRC} ${CMAKE_JS_SRC})
set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "" SUFFIX ".node")
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_JS_INC})
target_link_libraries(${PROJECT_NAME} ${CMAKE_JS_LIB})
