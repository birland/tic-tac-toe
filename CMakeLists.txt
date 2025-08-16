cmake_minimum_required(VERSION 3.28.3)
project(
  tic-tac-toe
  VERSION 0.10.0
  LANGUAGES CXX)

# Reference:
# https://github.com/TheLartians/ModernCppStarter/blob/master/CMakeLists.txt
# ---- Include guards ----

# if(PROJECT_SOURCE_DIR STREQUAL PROJECT_BINARY_DIR) message( FATAL_ERROR
# "In-source builds not allowed. Please make a new directory (called a build
# directory) and run CMake from there." ) endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
option(BUILD_SHARED_LIBS "Build shared libraries" OFF)
set(CMAKE_CXX_EXTENSIONS OFF)

# For export compile_commands.json to clangd and clang-tidy
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
# For VSCode you can install Output Colorizer for Output window in the VSCode
set(CMAKE_COLOR_DIAGNOSTICS ON)

message("CHECK CURRENT COMPILER")
message(${CMAKE_CXX_COMPILER_ID})
message(${CMAKE_CXX_COMPILER})
if(UNIX)
  message(${CMAKE_CXX_COMPILER_AR})
  message(${CMAKE_CXX_COMPILER_RANLIB})
endif()
message("Build type: ${CMAKE_BUILD_TYPE}")

option(USE_CPM_PACKAGES_ENABLED "Flag to use find package or CPM" ON)
option(PROFILER_ENABLED "Flag to use Tracy Profiler" OFF)
option(STATIC_LINKING_ENABLED "Flag to use static linking" OFF)
option(ENABLE_SANITIZERS "Flag to use sanitizers" ON)

if(USE_CPM_PACKAGES_ENABLED)
  # download CPM.cmake
  file(
    DOWNLOAD
    https://github.com/cpm-cmake/CPM.cmake/releases/download/v0.40.8/CPM.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/cmake/CPM.cmake
    EXPECTED_HASH
      SHA256=78ba32abdf798bc616bab7c73aac32a17bbd7b06ad9e26a6add69de8f3ae4791)
  include(${CMAKE_CURRENT_BINARY_DIR}/cmake/CPM.cmake)

  # FTXUI
  cpmaddpackage("gh:arthursonzogni/ftxui#v6.1.9")

  # Boost Library

  # cpmaddpackage("gh:boostorg/boost#boost-1.88.0")

  # {GSL} Guidelines Support Library
  cpmaddpackage("gh:microsoft/GSL#v4.1.0")

  # {fmt} format
  cpmaddpackage("gh:fmtlib/fmt#11.2.0")

  # toml++
  cpmaddpackage("gh:marzer/tomlplusplus#master")

  # {modern scanf} cpmaddpackage("gh:eliaskosunen/scnlib#v4.0.1")
endif()

file(GLOB_RECURSE SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
     "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c")

file(GLOB_RECURSE HEADER_FILES "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h"
     "${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp")

set(ASSETS_DIR ${PROJECT_SOURCE_DIR}/assets)

# copy assets folder to bin folder
add_custom_target(
  copy_assets COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different
                      ${ASSETS_DIR} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/assets)

# copy compile_commands.json to project folder (for some IDEs)
add_custom_target(
  copy_compile_commands
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
          ${CMAKE_BINARY_DIR}/compile_commands.json ${CMAKE_CURRENT_LIST_DIR})

# copy .clang-tidy-ignore-checks to build folder to ignore third party
# dependencies
add_custom_target(
  copy_clang_tidy_ignore_checks
  COMMAND
    ${CMAKE_COMMAND} -E copy_if_different
    ${PROJECT_SOURCE_DIR}/.clang-tidy-ignore-checks
    ${CMAKE_BINARY_DIR}/.clang-tidy)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)

if(UNIX)
  set(RELEASE_OPTIONS
      "-O3 -Wall -Wextra -fno-delete-null-pointer-checks -fno-strict-overflow
      -fno-strict-aliasing -ftrivial-auto-var-init=zero")
endif()

if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
  set(DEBUG_OPTIONS
      "/DEBUG /Zi /EHsc /W4 /external:anglebrackets /external:W0 /external:templates- /std:c++latest"
  )
  set(RELEASE_OPTIONS "/O2")
  message("MSVC COMPILER FLAGS")
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  set(DEBUG_OPTIONS
      "-O0 -g -Weverything -Werror -U_FORTIFY_SOURCE -D_FORTIFY_SOURCE=3 -D_GLIBCXX_ASSERTIONS
      -Wno-c++98-compat -Wno-c++-compat -Wno-pre-c++20-compat-pedantic -Wno-c++98-c++11-compat
      -Wno-c++98-c++11-compat-binary-literal
      -Wno-padded -Wno-covered-switch-default -Wno-global-constructors")
elseif(${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU")
  set(DEBUG_OPTIONS
      "-O0 -g -Wall -Wextra -Werror -Wpedantic -pedantic-errors
        -Winvalid-pch -Wimplicit-fallthrough -Wuninitialized -Wshadow
        -Wlong-long -Wstrict-overflow=5 -Wcast-qual -Wcast-align -Wconversion
        -Wpacked -Wdouble-promotion -Wfloat-equal -Wpointer-arith -Wformat=2
        -Wformat-nonliteral -Wformat-security -Wformat-y2k -Wimport -Winit-self
        -Wwrite-strings -Wstack-protector -Winline
        -Wdisabled-optimization -Winvalid-pch -Wlong-long
        -Wmissing-field-initializers -Wmissing-declarations
        -Wmissing-format-attribute -Wmissing-include-dirs -Wmissing-noreturn
        -Wpacked -Wredundant-decls -Wswitch-default
        -Wswitch-enum -Wunreachable-code -Wunused -Wunused-parameter
        -Wvariadic-macros -Wtrampolines -D_FORTIFY_SOURCE=3 -D_GLIBCXX_ASSERTIONS
        -fexceptions -fstrict-aliasing -ftrivial-auto-var-init=pattern -fPIE -pie -Wl,-z,relro,-z,now
        -fstack-protector-strong -fstack-clash-protection
        ")
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  message("${CMAKE_CXX_COMPILER_ID} COMPILER FLAGS:\n${DEBUG_OPTIONS}")
else()
  message("${CMAKE_CXX_COMPILER_ID} COMPILER FLAGS:\n${RELEASE_OPTIONS}")
endif()

add_executable(${PROJECT_NAME})
add_dependencies(${PROJECT_NAME} copy_assets copy_compile_commands
                 copy_clang_tidy_ignore_checks)
target_sources(${PROJECT_NAME} PUBLIC ${SOURCE_FILES} ${HEADER_FILES})
target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/utils)
target_compile_options(${PROJECT_NAME}
                       PRIVATE $<$<CONFIG:DEBUG>:SHELL:${DEBUG_OPTIONS}>)
target_compile_options(${PROJECT_NAME}
                       PRIVATE $<$<CONFIG:RELEASE>:SHELL:${RELEASE_OPTIONS}>)
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_23)
set_target_properties(${PROJECT_NAME} PROPERTIES CXX_EXTENSIONS OFF)

if(WIN32)
  set(ENABLE_SANITIZERS OFF)
  message(
    AUTHOR_WARNING
      "SANITIZERS DISABLED BECAUSE ARE NOT FULLY TESTED ON WINDOWS YET.")
endif()

if(ENABLE_SANITIZERS AND NOT MSVC)
  target_compile_options(
    ${PROJECT_NAME}
    PRIVATE $<$<CONFIG:DEBUG>:SHELL:-fsanitize=address -fsanitize=undefined>
            # -fsanitize=thread
  )
  target_link_libraries(
    ${PROJECT_NAME} PRIVATE $<$<CONFIG:DEBUG>:-fsanitize=address
                            -fsanitize=undefined> # -fsanitize=thread
  )
endif()

if(UNIX)
  find_package(Freetype)
  target_link_libraries(${PROJECT_NAME} PRIVATE X11 freetype)
endif()

if(PROFILER_ENABLED)
  cpmaddpackage("gh:wolfpld/tracy#v0.12.2")
  option(TRACY_ENABLE "" ON)
  target_link_libraries(${PROJECT_NAME} PUBLIC Tracy::TracyClient)
endif()

if(NOT USE_CPM_PACKAGES_ENABLED)
  find_package(fmt CONFIG REQUIRED)
  find_package(Microsoft.GSL CONFIG REQUIRED)
  find_package(scn CONFIG REQUIRED)
  find_package(tomlplusplus CONFIG REQUIRED)
endif()

if(NOT STATIC_LINKING_ENABLED)
  target_link_libraries(
    ${PROJECT_NAME}
    PRIVATE ftxui::component ftxui::dom ftxui::screen Microsoft.GSL::GSL
            fmt::fmt tomlplusplus::tomlplusplus)
else()
  message("STATIC LINKING ENABLED")
  target_link_libraries(
    ${PROJECT_NAME}
    PRIVATE ftxui::component
            "-static"
            ftxui::dom
            "-static"
            ftxui::screen
            "-static"
            Microsoft.GSL::GSL
            "-static"
            fmt::fmt
            "-static"
            tomlplusplus::tomlplusplus
            "-static")
endif()

# install(TARGETS ${PROJECT_NAME})
