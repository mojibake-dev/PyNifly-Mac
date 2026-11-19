# macOS build of NiflyDLL: the same sources the Windows vcxproj compiles, as a native dylib.
#
#   cmake -S NiflyDLL/macos -B build
#   cmake --build build --target addon     # -> build/io_scene_nifly-<version>-macos-<arch>.zip
#
cmake_minimum_required(VERSION 3.24)
project(NiflyDLL_macos LANGUAGES CXX)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()
set(CMAKE_CXX_STANDARD 20)              # the wrapper's C API takes char8_t
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON) # static nifly linked into a dylib

get_filename_component(PYNIFLY_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../.." ABSOLUTE)
set(WRAPPER_DIR "${PYNIFLY_ROOT}/NiflyDLL")

# BadDog's fork of ousnius/nifly, pinned to the commit the Windows DLL is built against.
include(FetchContent)
FetchContent_Declare(nifly
    GIT_REPOSITORY https://github.com/BadDogSkyrim/nifly.git
    GIT_TAG        8b43aaa022c4dec8edd0985630cb101ea9785d92)
FetchContent_MakeAvailable(nifly)

add_library(NiflyDLL SHARED
    ${WRAPPER_DIR}/Anim.cpp
    ${WRAPPER_DIR}/Logger.cpp
    ${WRAPPER_DIR}/NiflyFunctions.cpp
    ${WRAPPER_DIR}/NiflyWrapper.cpp)
target_include_directories(NiflyDLL PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/compat    # windows.h -> dladdr, for FindProjectRoot()
    ${WRAPPER_DIR})
target_compile_definitions(NiflyDLL PRIVATE NIFLYDLL_EXPORTS)
target_compile_options(NiflyDLL PRIVATE -fdeclspec -w)  # __declspec(dllexport) parses; symbols export by default
target_link_libraries(NiflyDLL PRIVATE nifly)

# Installable add-on zip: the upstream add-on folder with the dylib dropped in.
file(STRINGS "${PYNIFLY_ROOT}/io_scene_nifly/__init__.py" _ver REGEX "\"version\": *\\(")
string(REGEX MATCH "([0-9]+), *([0-9]+), *([0-9]+)" _ "${_ver}")
set(ADDON_VERSION "${CMAKE_MATCH_1}.${CMAKE_MATCH_2}.${CMAKE_MATCH_3}")
if(CMAKE_OSX_ARCHITECTURES)
    string(REPLACE ";" "-" ADDON_ARCH "${CMAKE_OSX_ARCHITECTURES}")
else()
    set(ADDON_ARCH "${CMAKE_HOST_SYSTEM_PROCESSOR}")
endif()
set(ADDON_ZIP "${CMAKE_BINARY_DIR}/io_scene_nifly-${ADDON_VERSION}-macos-${ADDON_ARCH}.zip")

add_custom_target(addon
    COMMAND ${CMAKE_COMMAND}
        -DSRC=${PYNIFLY_ROOT}/io_scene_nifly
        -DLIB=$<TARGET_FILE:NiflyDLL>
        -DSTAGE=${CMAKE_BINARY_DIR}/stage
        -DZIP=${ADDON_ZIP}
        -P ${CMAKE_CURRENT_SOURCE_DIR}/package.cmake
    DEPENDS NiflyDLL
    COMMENT "Packaging ${ADDON_ZIP}")
