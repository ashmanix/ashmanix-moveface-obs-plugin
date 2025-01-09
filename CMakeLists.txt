cmake_minimum_required(VERSION 3.28...3.30)

include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/common/bootstrap.cmake" NO_POLICY_SCOPE)

project(${_name} VERSION ${_version})

option(ENABLE_FRONTEND_API "Use obs-frontend-api for UI functionality" ON)
option(ENABLE_QT "Use Qt functionality" ON)

include(compilerconfig)
include(defaults)
include(helpers)

add_library(${CMAKE_PROJECT_NAME} MODULE)

find_package(libobs REQUIRED)
target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE OBS::libobs)

if(ENABLE_FRONTEND_API)
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

target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/lib)

set(CPP_FILES
  src/classes/blendshapes/blendshape-key.cpp
  src/classes/blendshapes/blendshape-rule.cpp
  src/classes/blendshapes/blendshape.cpp
  src/classes/poses/pose-image-data.cpp
  src/classes/poses/pose.cpp
  src/classes/poses/vector3.cpp
  src/classes/tracking/tracker-data.cpp
  src/classes/tracking/vtube-studio-data.cpp
  src/plugin-main.cpp
  src/utils/utils.cpp
  src/widgets/comms/network-tracking.cpp
  src/widgets/face-tracker.cpp
  src/widgets/main-widget-dock.cpp
  src/widgets/settings-dialog/avatar-graphics-view.cpp
  src/widgets/settings-dialog/settings-dialog.cpp
)

set(HEADER_FILES
  src/classes/blendshapes/blendshape-key.h
  src/classes/blendshapes/blendshape-rule.h
  src/classes/blendshapes/blendshape.h
  src/classes/poses/pixmap-item.h
  src/classes/poses/pose-image-data.h
  src/classes/poses/pose-image.h
  src/classes/poses/pose.h
  src/classes/poses/vector3.h
  src/classes/tracking/tracker-data.h
  src/classes/tracking/vtube-studio-data.h
  src/plugin-support.h
  src/utils/utils.h
  src/widgets/comms/network-tracking.h
  src/widgets/face-tracker.h
  src/widgets/main-widget-dock.h
  src/widgets/obs-dock-wrapper.h
  src/widgets/settings-dialog/avatar-graphics-view.h
  src/widgets/settings-dialog/settings-dialog.h
)

target_sources(
  ${CMAKE_PROJECT_NAME}
  PRIVATE
  ${CPP_FILES}
  ${HEADER_FILES}
)

set_target_properties_plugin(${CMAKE_PROJECT_NAME} PROPERTIES OUTPUT_NAME ${_name})
