# Detect platform
set (EX_TARGET_OS Linux)
if (CMAKE_SYSTEM_NAME MATCHES Darwin)
  set (EX_TARGET_OS Darwin)
endif ()

if (CMAKE_SIZEOF_VOID_P EQUAL 8)
  set (EX_PLATFORM "${EX_TARGET_OS}64")
  set (EX_BITS 64)
else (CMAKE_SIZEOF_VOID_P EQUAL 8)
  set (EX_PLATFORM "${EX_TARGET_OS}32")
  set (EX_BITS 32)
endif (CMAKE_SIZEOF_VOID_P EQUAL 8)
message (${PROJECT_NAME} ": Building ${EX_PLATFORM} "  ${CMAKE_BUILD_TYPE})


# Enable C++11
set (CMAKE_CXX_STANDARD 11)

if (CMAKE_SYSTEM_NAME MATCHES Darwin)
  set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++ -mmacosx-version-min=10.7 -arch x86_64 -Wno-switch -Wno-logical-op-parentheses -Wno-parentheses")
  set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mmacosx-version-min=10.7 -arch x86_64")
  set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -mmacosx-version-min=10.7 -arch x86_64")
else ()
  # 32-bit libstdc++ is not standard on 64-bit Linux
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static-libstdc++ -static-libgcc")
endif ()
