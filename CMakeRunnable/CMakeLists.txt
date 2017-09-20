# Cross-build with :
#  cmake -DCROSSBUILD=1 -DCMAKE_SYSTEM_NAME=Darwin -DCMAKE_RULE_MESSAGES:BOOL=OFF -DCMAKE_VERBOSE_MAKEFILE:BOOL=ON .. && make --no-print-directory
# Native build with :
#  cmake -DCMAKE_RULE_MESSAGES:BOOL=OFF -DCMAKE_VERBOSE_MAKEFILE:BOOL=ON .. && make --no-print-directory
cmake_minimum_required (VERSION 3.1)

set (SDL2_DYNAMIC 0)

project (GL31Test)

include (shared.cmake)

# Reference and build GLEW
add_subdirectory (glew)

include_directories(sdl/include)
set (EXTRA_LIBS "glew")

file(GLOB MySRC
    "gl31test.cpp"
    "unix_stub.cpp"
)
add_executable(${PROJECT_NAME} ${MySRC})

if (SDL2_DYNAMIC)
	find_package(SDL2 REQUIRED)
	# Fix trailing whitespace in SDL2_LIBRARIES string
	string(REGEX REPLACE "SDL2 $" "SDL2" SDL2_LIB ${SDL2_LIBRARIES})
else(SDL2_DYNAMIC)
	set (SDL2_LIB ${PROJECT_SOURCE_DIR}/libs/SDL2${EX_PLATFORM}.a)
endif(SDL2_DYNAMIC)

set(CMAKE_FIND_ROOT_PATH /osxcross/target/SDK/MacOSX10.10.sdk)

macro(ADD_OSX_FRAMEWORK fwname target)
    find_library(FRAMEWORK_${fwname}
        NAMES ${fwname}
        PATHS ${CMAKE_OSX_SYSROOT}/System/Library
        PATH_SUFFIXES Frameworks
        NO_DEFAULT_PATH)
    if( ${FRAMEWORK_${fwname}} STREQUAL FRAMEWORK_${fwname}-NOTFOUND)
        MESSAGE(ERROR ": Framework ${fwname} not found")
    else()
        TARGET_LINK_LIBRARIES(${target} "${FRAMEWORK_${fwname}}/${fwname}")
        MESSAGE(STATUS "Framework ${fwname} found at ${FRAMEWORK_${fwname}}")
    endif()
endmacro(ADD_OSX_FRAMEWORK)

if (CMAKE_SYSTEM_NAME MATCHES Darwin)
    if (CROSSBUILD)
        set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DGLM_COMPILER=0")
    endif()
    ADD_OSX_FRAMEWORK(OpenGL ${PROJECT_NAME})
    ADD_OSX_FRAMEWORK(CoreAudio ${PROJECT_NAME})
    ADD_OSX_FRAMEWORK(System ${PROJECT_NAME})
    ADD_OSX_FRAMEWORK(CoreFoundation ${PROJECT_NAME})
    ADD_OSX_FRAMEWORK(CoreServices ${PROJECT_NAME})
    ADD_OSX_FRAMEWORK(AudioUnit ${PROJECT_NAME})
    ADD_OSX_FRAMEWORK(AudioToolbox ${PROJECT_NAME})
    ADD_OSX_FRAMEWORK(Cocoa ${PROJECT_NAME})
    ADD_OSX_FRAMEWORK(CoreVideo ${PROJECT_NAME})
    ADD_OSX_FRAMEWORK(IOKit ${PROJECT_NAME})
    ADD_OSX_FRAMEWORK(ForceFeedback ${PROJECT_NAME})
    ADD_OSX_FRAMEWORK(Carbon ${PROJECT_NAME})
    set(EXTRA_LIBS ${EXTRA_LIBS} iconv)
else ()
    find_package(OpenGL REQUIRED)
endif ()

target_link_libraries(${PROJECT_NAME} ${SDL2_LIB} ${OPENGL_LIBRARIES} ${EXTRA_LIBS})

