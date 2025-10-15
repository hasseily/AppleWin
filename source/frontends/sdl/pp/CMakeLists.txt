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

if ((${CMAKE_SYSTEM_NAME} MATCHES "Darwin") OR (${CMAKE_SYSTEM_NAME} MATCHES "Windows"))
  # only OpenGL supported on MacOS and Windows
  set (PP_OPENGL ON)
else()
  option(PP_OPENGL "Prefer OpenGL over OpenGL ES" OFF)
endif()

if (PP_OPENGL)
  find_package(OpenGL REQUIRED)
else()
  pkg_search_module(GLES2 REQUIRED glesv2)
  target_compile_definitions(pp PRIVATE IMGUI_IMPL_OPENGL_ES2)
endif()

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
set(_pp_asset_dirs
  "${CMAKE_CURRENT_SOURCE_DIR}/presets"
  "${CMAKE_CURRENT_SOURCE_DIR}/assets"
  "${CMAKE_CURRENT_SOURCE_DIR}/shaders"
)
set(_pp_asset_dirs_abs "")
foreach(d IN LISTS _pp_asset_dirs)
  if(EXISTS "${d}")
    get_filename_component(absd "${d}" REALPATH)
    list(APPEND _pp_asset_dirs_abs "${absd}")
  endif()
endforeach()

# Publish the absolute dirs so consumers (sa2) can also copy them if needed
#    (used by non-Apple platforms / non-bundle fallbacks)
set_property(TARGET pp PROPERTY INTERFACE_PP_ASSET_DIRS "${_pp_asset_dirs_abs}")

# IDE visibility everywhere
if(NOT CMAKE_GENERATOR STREQUAL "Xcode")
  # ---- Other IDEs (VS, Ninja, Makefiles): enumerate files so they show up in the project tree
  set(PP_ASSET_FILES "")
  foreach(dir IN LISTS _pp_asset_dirs_abs)
    file(GLOB_RECURSE _one_dir_files CONFIGURE_DEPENDS "${dir}/*")
    list(APPEND PP_ASSET_FILES ${_one_dir_files})
  endforeach()

  # Add files as header-only so IDEs show them but nobody tries to compile them
  if(PP_ASSET_FILES)
    target_sources(pp INTERFACE ${PP_ASSET_FILES})
    set_source_files_properties(${PP_ASSET_FILES} PROPERTIES HEADER_FILE_ONLY TRUE)

    # Put them under a nice virtual group
    source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}"
                 PREFIX "Resources"
                 FILES ${PP_ASSET_FILES})
  endif()
endif()
