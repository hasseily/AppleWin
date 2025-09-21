cmake_minimum_required(VERSION 3.20)

set(SOURCE_FILES
  glad/glad.cpp
  postprocessor.cpp
  shader.cpp
  extras/ImGuiFileDialog.cpp
)

set(HEADER_FILES
  postprocessor.h
  shader.h
  extras/ImGuiFileDialog.h
  extras/ImGuiFileDialogConfig.h
)

add_library(pp STATIC
  ${SOURCE_FILES}
  ${HEADER_FILES}
)

# Public so consumers can `#include "postprocessor.h"`
target_include_directories(pp
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/extras
    ${CMAKE_SOURCE_DIR}/source/frontends/sdl/imgui/imgui
	${SDL2_INCLUDE_DIRS}
    ${SDL2_IMAGE_INCLUDE_DIRS}
    ${GLES2_INCLUDE_DIRS}
)

target_compile_features(pp PUBLIC cxx_std_20)

target_link_libraries(pp
  PUBLIC
	${SDL2_LIBRARIES}
    ${SDL2_IMAGE_LIBRARIES}
    ${GLES2_LIBRARIES}
    OpenGL::GL
  )

target_include_directories(pp PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/extras)

# Declare pp's resource dirs (semicolon-separated)
# and turn them into absolute paths
set(_pp_asset_dirs_raw
  "${CMAKE_CURRENT_SOURCE_DIR}/presets"
  "${CMAKE_CURRENT_SOURCE_DIR}/assets"
  "${CMAKE_CURRENT_SOURCE_DIR}/shaders"
)

set(_pp_asset_dirs "")
foreach(d IN LISTS _pp_asset_dirs_raw)
  if(EXISTS "${d}")
    get_filename_component(absd "${d}" REALPATH)
    list(APPEND _pp_asset_dirs "${absd}")
  endif()
endforeach()

# Publish them via an INTERFACE property
set_property(TARGET pp PROPERTY INTERFACE_PP_ASSET_DIRS "${_pp_asset_dirs}")

# show asset files in the IDE
# collect them but don't compile them
file(GLOB ASSET_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/presets/*"
    "${CMAKE_CURRENT_SOURCE_DIR}/assets/*"
    "${CMAKE_CURRENT_SOURCE_DIR}/shaders/*"
)
foreach(dir presets assets shaders)
    file(GLOB FILES "${CMAKE_CURRENT_SOURCE_DIR}/${dir}/*")
    source_group(${dir} FILES ${FILES})
    target_sources(pp PRIVATE ${FILES})
endforeach()
