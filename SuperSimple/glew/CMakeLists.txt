cmake_minimum_required (VERSION 3.1)

project (glew)

include (../shared.cmake)

add_definitions (-DGLEW_NO_GLU)
add_definitions (-DGLEW_STATIC)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../)

file(GLOB MySRC "*.c")

add_library (${PROJECT_NAME} STATIC ${MySRC})
