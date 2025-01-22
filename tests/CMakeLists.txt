cmake_minimum_required(VERSION 3.28...3.30)

set(OBS_VER 31.0.0)
project(ashmanix_moveface_plugin_tests VERSION 1.0.0)

option(ENABLE_FRONTEND_API "Use obs-frontend-api for UI functionality" ON)
option(ENABLE_QT "Use Qt functionality" ON)

set(LIBOBS_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../.deps/obs-studio-${OBS_VER}/build_universal/libobs")
list(APPEND CMAKE_PREFIX_PATH "${LIBOBS_PATH}")

file(
  GLOB_RECURSE PLUGIN_SOURCES
  "${CMAKE_CURRENT_SOURCE_DIR}/../src/classes/*.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/../src/classes/*.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/../src/utils/*.cpp"
)

file(GLOB_RECURSE TEST_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/unit/**/*.cpp")

# Adds mocks file (with the dummy definitions)
set(STUBS_FILE "${CMAKE_CURRENT_SOURCE_DIR}/mocks/test-mocks.cpp")

add_executable(${PROJECT_NAME} ${PLUGIN_SOURCES} ${TEST_SOURCES} ${STUBS_FILE})

# RPath to find the framework
set_target_properties(${PROJECT_NAME} PROPERTIES MACOSX_RPATH ON BUILD_RPATH "${LIBOBS_PATH}")

if(ENABLE_FRONTEND_API)
  set(OBS_FRONTEND_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../.deps/lib/cmake/obs-frontend-api")
  list(APPEND CMAKE_PREFIX_PATH "${OBS_FRONTEND_PATH}")
  find_package(obs-frontend-api REQUIRED)
  target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE OBS::obs-frontend-api)
endif()

if(ENABLE_QT)
  find_package(Qt6 COMPONENTS Widgets Core Network)
  target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE Qt6::Core Qt6::Widgets Qt6::Network)
  target_compile_options(
    ${CMAKE_PROJECT_NAME}
    PRIVATE $<$<C_COMPILER_ID:Clang,AppleClang>:-Wno-quoted-include-in-framework-header -Wno-comma>
  )
  set_target_properties(
    ${CMAKE_PROJECT_NAME}
    PROPERTIES AUTOMOC ON AUTOUIC ON AUTORCC ON
  )
endif()

# Fetch GoogleTest
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# Include directories for tests
target_include_directories(
  ${CMAKE_PROJECT_NAME}
  PRIVATE
    "${CMAKE_SOURCE_DIR}/../src"
    "${CMAKE_SOURCE_DIR}/../lib"
    "${CMAKE_SOURCE_DIR}/../.deps/obs-studio-${OBS_VER}/libobs"
)

# Link against main project's core library and dependencies
target_link_libraries(
  ${CMAKE_PROJECT_NAME}
  PRIVATE
    # ashmanix_moveface_core  # Ensure your core library is named correctly
    Qt6::Core
    GTest::gtest_main
)

# # Disable -Werror=comma for the test executable
target_compile_options(${CMAKE_PROJECT_NAME} PRIVATE -Wno-error=comma)

# # Enable GoogleTest discovery
include(GoogleTest)
gtest_discover_tests(${CMAKE_PROJECT_NAME})
