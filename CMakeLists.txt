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

target_sources(
  ${CMAKE_PROJECT_NAME}
  PRIVATE src/plugin-main.cpp
          src/srtdock_dock.cpp
          src/srtdock_dock.hpp
          src/srtdock_url.cpp
          src/srtdock_url.hpp
          src/srtdock_presets.cpp
          src/srtdock_presets.hpp
)

set_target_properties_plugin(${CMAKE_PROJECT_NAME} PROPERTIES OUTPUT_NAME ${_name})

# --- Installer (CPack) ---
install(TARGETS ${CMAKE_PROJECT_NAME} LIBRARY DESTINATION "obs-plugins/64bit")

set(CPACK_PACKAGE_NAME "SRTDock")
set(CPACK_PACKAGE_VENDOR "SRTDock")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "SRT URL Builder Dock Plugin for OBS Studio")
set(CPACK_PACKAGE_VERSION "${_version}")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
set(CPACK_RESOURCE_FILE_README "${CMAKE_CURRENT_SOURCE_DIR}/README.md")
set(CPACK_WIX_PRODUCT_GUID "A1B2C3D4-E5F6-4A5B-9C8D-7E6F5A4B3C2D")
set(CPACK_WIX_UPGRADE_GUID "D4C3B2A1-F6E5-4B5A-8D9C-6F7E5A4B3C2D")

include(CPack)
