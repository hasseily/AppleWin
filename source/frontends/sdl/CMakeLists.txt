include(FindPkgConfig)
include(FindOpenGL)

set(CMAKE_OSX_DEPLOYMENT_TARGET "14.6" CACHE STRING "Minimum OS X deployment version" FORCE)

add_executable(sa2 MACOSX_BUNDLE)

set(APP_ICON "${CMAKE_CURRENT_SOURCE_DIR}/AppIcon.icns")
set_source_files_properties(${APP_ICON}
	PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")
target_sources(sa2 PRIVATE ${APP_ICON})

set_target_properties(sa2 PROPERTIES
  MACOSX_BUNDLE TRUE
  MACOSX_BUNDLE_ICON_FILE "AppIcon.icns"
)

set(IMGUI_PATH "imgui/imgui")
set(IMGUI_CLUB_PATH "imgui/imgui_club")

find_package(SDL2 REQUIRED)
find_package(SDL2_IMAGE REQUIRED)

target_compile_features(common2 PUBLIC cxx_std_20)

if ((${CMAKE_SYSTEM_NAME} MATCHES "Darwin") OR (${CMAKE_SYSTEM_NAME} MATCHES "Windows"))
  # only OpenGL supported on MacOS and Windows
  set (SA2_OPENGL ON)
else()
  option(SA2_OPENGL "Prefer OpenGL over OpenGL ES" OFF)
endif()

if (SA2_OPENGL)
  find_package(OpenGL REQUIRED)
else()
  pkg_search_module(GLES2 REQUIRED glesv2)
  target_compile_definitions(sa2 PRIVATE
    IMGUI_IMPL_OPENGL_ES2
  )
endif()

set(SOURCE_FILES
  main.cpp
  gamepad.cpp
  sdirectsound.cpp
  utils.cpp
  sdlframe.cpp
  processfile.cpp
  renderer/sdlrendererframe.cpp
  )

set(HEADER_FILES
  gamepad.h
  sdirectsound.h
  utils.h
  sdlframe.h
  processfile.h
  renderer/sdlrendererframe.h
  )


target_include_directories(sa2 PRIVATE
  ${GLES2_INCLUDE_DIRS}
  )

target_link_libraries(sa2 PRIVATE
  appleii
  common2
  minizip
  yaml

  ${PCAP_LIBRARIES}
  ${SLIRP_LIBRARIES}
  ${ZLIB_LIBRARIES}

  SDL2::SDL2
  SDL2::SDL2main
  SDL2_image::SDL2_image
  ${GLES2_LIBRARIES}
  OpenGL::GL
  )

if (NOT WIN32)
  target_link_libraries(sa2 PUBLIC
    windows
    )
endif()

target_link_directories(sa2 PRIVATE
  ${GLES2_LIBRARY_DIRS}
  )

target_sources(sa2 PRIVATE
  ${SOURCE_FILES}
  ${HEADER_FILES}
  )

target_sources(sa2 PRIVATE
  imgui/sdlimguiframe.cpp
  imgui/image.cpp
  imgui/settingshelper.cpp
  imgui/sdlsettings.cpp
  imgui/sdldebugger.cpp
  imgui/sdlmemory.cpp
  imgui/inputtexthistory.cpp
  imgui/cycletabitems.cpp

  imgui/sdlimguiframe.h
  imgui/image.h
  imgui/settingshelper.h
  imgui/sdlsettings.h
  imgui/sdldebugger.h
  imgui/sdlmemory.h
  imgui/imconfig.h
  imgui/glselector.h
  imgui/inputtexthistory.h
  imgui/cycletabitems.h

  ${IMGUI_PATH}/imgui.h
  ${IMGUI_PATH}/imgui.cpp
  ${IMGUI_PATH}/imgui_demo.cpp
  ${IMGUI_PATH}/imgui_draw.cpp
  ${IMGUI_PATH}/imgui_tables.cpp
  ${IMGUI_PATH}/imgui_widgets.cpp
  ${IMGUI_PATH}/backends/imgui_impl_sdl2.cpp
  ${IMGUI_PATH}/backends/imgui_impl_opengl3.cpp

  ${IMGUI_CLUB_PATH}/imgui_memory_editor/imgui_memory_editor.h
  )

target_include_directories(sa2 PRIVATE
  ${IMGUI_PATH}
  ${IMGUI_PATH}/backends
  ${IMGUI_CLUB_PATH}/imgui_memory_editor
  )

target_compile_definitions(sa2 PRIVATE
  IMGUI_USER_CONFIG="frontends/sdl/imgui/imconfig.h"
)

if (MSVC)
  target_compile_definitions(sa2 PRIVATE
    NOMINMAX            # to avoid conflicts with std::min/max
    DISABLE_SPEECH_API  # undefine because the original AppleWin speech API is not supported here
  )
endif()

install(TARGETS sa2
  RUNTIME_DEPENDENCIES
    PRE_EXCLUDE_REGEXES "api-ms-" "ext-ms-"    # skip Windows system DLLs
    POST_EXCLUDE_REGEXES ".*system32/.*"       # skip more system DLLs
  FRAMEWORK DESTINATION Frameworks
  BUNDLE DESTINATION bin
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION bin
  ARCHIVE DESTINATION lib
)

# ==== PostProcessing ====
add_subdirectory(pp)
target_link_libraries(sa2 PRIVATE pp)

# Pull the resources directories from pp and copy to the sa2 output
# The INTERFACE_PP_ASSET_DIRS are absolute full paths of directories
# On Windows/Linux or if the target isn’t a bundle: copy next to the exe.
# We use the INTERFACE property published by 'pp' to know which dirs to copy.

get_target_property(PP_SUBDIRS pp INTERFACE_PP_ASSET_DIRS)

if(PP_SUBDIRS AND CMAKE_GENERATOR STREQUAL "Xcode")
  target_sources(sa2 PRIVATE ${PP_SUBDIRS})
  # Make them blue folder refs and route them to Contents/Resources
  set_source_files_properties(${PP_SUBDIRS} PROPERTIES
    XCODE_FILE_ATTRIBUTES "folder"
    MACOSX_PACKAGE_LOCATION "Resources")
endif()

if(PP_SUBDIRS AND NOT CMAKE_GENERATOR STREQUAL "Xcode")
  foreach(srcdir IN LISTS PP_SUBDIRS)
    if(EXISTS "${srcdir}")
      get_filename_component(dstname "${srcdir}" NAME)

      if(APPLE)     # only if not using Xcode above
        set(_dst_root "$<TARGET_BUNDLE_CONTENT_DIR:sa2>/Resources")
      else()
        set(_dst_root "$<TARGET_FILE_DIR:sa2>")
      endif()

      add_custom_command(TARGET sa2 POST_BUILD
        #COMMAND ${CMAKE_COMMAND} -E echo "Copying: ${srcdir} -> ${_dst_root}/${dstname}"
        COMMAND ${CMAKE_COMMAND} -E copy_directory "${srcdir}" "${_dst_root}/${dstname}"
        VERBATIM)
    endif()
  endforeach()
endif()
