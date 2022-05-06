cmake_minimum_required(VERSION 3.22)

add_subdirectory(External/JUCE)

project(AuTestHost LANGUAGES CXX OBJCXX)
set(CMAKE_PROJECT_VERSION "1.0.0")
set(JUCE_COMPANY_COPYRIGHT "(c) 2022 Directed Edge, Inc.")

string(TOLOWER ${PROJECT_NAME} project_name_lowercase)
string(REGEX REPLACE ".[0-9]+$" "" project_version_name ${CMAKE_PROJECT_VERSION})


set(apple_bundle_id "de.decomposer.${PROJECT_NAME}")

set(CMAKE_XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY "Apple Development")
set(CMAKE_XCODE_ATTRIBUTE_DEVELOPMENT_TEAM "8T67D8T434")

juce_add_gui_app("${PROJECT_NAME}"
  PRODUCT_NAME "${PROJECT_NAME}"
  VERSION "${CMAKE_PROJECT_VERSION}"
  BUNDLE_ID "${apple_bundle_id}"
  DOCUMENT_BROWSER_ENABLED FALSE
  COMPANY_NAME "Decomposer"
  COMPANY_WEBSITE "https://decomposer.de/"
  COMPANY_EMAIL "info@decomposer.de"
  COMPANY_COPYRIGHT "${JUCE_COMPANY_COPYRIGHT}"
  DOCUMENT_EXTENSIONS ${juce_document_extensions}
  DESCRIPTION "AudioUnit Test Host"
  STATUS_BAR_HIDDEN TRUE
  REQUIRES_FULL_SCREEN TRUE
  BLUETOOTH_PERMISSION_ENABLED TRUE
  BLUETOOTH_PERMISSION_TEXT "Bluetooth is required to use wireless MIDI devices"
  PLUGINHOST_AU TRUE
)

set(artefacts "${CMAKE_BINARY_DIR}/${PROJECT_NAME}_artefacts")

target_sources("${PROJECT_NAME}" PRIVATE
  Sources/Main.cpp
  Sources/MainComponent.cpp
  Sources/MainComponent.h
  Sources/LevelMeter.cpp
  Sources/LevelMeter.h
  Sources/BusComponent.cpp
  Sources/BusComponent.h
)

set(juce_modules
  audio_basics
  audio_devices
  audio_formats
  audio_plugin_client
  audio_processors
  audio_utils
  core
  cryptography
  data_structures
  events
  graphics
  gui_basics
  gui_extra
  dsp
)

foreach(module ${juce_modules})
  list(APPEND juce_libraries juce::juce_${module})
endforeach()

set(juce_definitions
  JUCE_WEB_BROWSER=1
  JUCE_USE_CURL=1
  JUCE_DISPLAY_SPLASH_SCREEN=0)

function(set_default_target_properties target)
  set_target_properties(${target} PROPERTIES
    CXX_STANDARD 17
    POSITION_INDEPENDENT_CODE TRUE
    C_VISIBILITY_PRESET hidden
    CXX_VISIBILITY_PRESET hidden
    OBJCXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN TRUE)
endfunction()

# On iOS we can compile JUCE with the precompiled header, which is convenient
# since using a separate target for JUCE triggers a CMake bug for iOS.

if(NOT ${CMAKE_SYSTEM_NAME} STREQUAL "iOS")
  add_library(juce_plugin_modules OBJECT)

  target_link_libraries(juce_plugin_modules
    PRIVATE
      ${juce_libraries}
    PUBLIC
      juce::juce_recommended_config_flags
      juce::juce_recommended_lto_flags
      juce::juce_recommended_warning_flags)

  target_compile_definitions(juce_plugin_modules
    PUBLIC
      ${juce_definitions}
    INTERFACE
      $<TARGET_PROPERTY:juce_plugin_modules,COMPILE_DEFINITIONS>)
  set_default_target_properties(juce_plugin_modules)
  target_include_directories(juce_plugin_modules
    INTERFACE
      $<TARGET_PROPERTY:juce_plugin_modules,INCLUDE_DIRECTORIES>)
  set(juce_libraries juce_plugin_modules)
else()
  list(APPEND juce_definitions JUCE_CONTENT_SHARING=1)
endif()

set_default_target_properties(${PROJECT_NAME})

foreach(format ${formats})
  set_default_target_properties("${PROJECT_NAME}_${format}")
endforeach()

#juce_add_binary_data("${PROJECT_NAME}_data" SOURCES
#)

#foreach(format ${formats})
#  juce_add_bundle_resources_directory("${PROJECT_NAME}_${format}" "${CMAKE_SOURCE_DIR}/Kits")
#endforeach()

target_compile_definitions("${PROJECT_NAME}" PUBLIC
  ${juce_definitions}
)

target_link_libraries("${PROJECT_NAME}"
  PRIVATE
    ${juce_libraries}
 #   "${PROJECT_NAME}_data"
    "-framework UniformTypeIdentifiers"
    "-framework StoreKit"

  PUBLIC
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags
    juce::juce_recommended_config_flags
)

juce_generate_juce_header("${PROJECT_NAME}")

target_precompile_headers(${PROJECT_NAME} PRIVATE "$<$<COMPILE_LANGUAGE:CXX>:${juce_header}>")

target_include_directories(${PROJECT_NAME} SYSTEM PRIVATE
  ${CMAKE_SOURCE_DIR}/Source
  ${artefacts}
)
