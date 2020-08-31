set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DLOCALEDIR=\\\"${CMAKE_INSTALL_PREFIX}/share/local\\\"")

configure_file(${CMAKE_SOURCE_DIR}/CMake/config.h.in
  ${CMAKE_CURRENT_BINARY_DIR}/config.h)

add_custom_target(version ALL)

add_custom_command(TARGET version
  PRE_BUILD
  COMMAND ${CMAKE_SOURCE_DIR}/src/mkversionhpp.sh
  COMMENT "Generating version.hpp")

set(Ui_FLUID
  fluid/dlg_bulkdl.fl
  fluid/dlg_editselection.fl
  fluid/dlg_eleprofile.fl
  fluid/dlg_garmindl.fl
  fluid/dlg_garminul.fl
  fluid/dlg_search.fl
  fluid/dlg_settings.fl
  fluid/dlg_tileserver.fl
  fluid/dlg_txtdisp.fl
  fluid/dlg_ui.fl)

set(Ui_SOURCES
  dlg_bulkdl_ex.cpp
  dlg_editselection_ex.cpp
  dlg_eleprofile_ex.cpp
  dlg_garmindl_ex.cpp
  dlg_garminul_ex.cpp
  dlg_search_ex.cpp
  dlg_settings_ex.cpp
  dlg_tileserver_ex.cpp
  dlg_txtdisp_ex.cpp
  dlg_ui_ex.cpp)

set(Ui_INCLUDE_DIRS
  ${PROJECT_SOURCE_DIR}/src
  ${Boost_INCLUDE_DIRS}
  ${CURL_INCLUDE_DIRS}
  ${FLTK_INCLUDE_DIRS}
  ${Intl_INCLUDE_DIRS}
  ${TinyXML2_INCLUDE_DIRS})

fltk_wrap_ui(Ui ${Ui_FLUID})
add_library(Ui ${Ui_SOURCES} ${Ui_FLTK_UI_SRCS})
add_dependencies(Ui version)
target_include_directories(Ui PUBLIC ${Ui_INCLUDE_DIRS})

set(florb_SOURCES
  areaselectlayer.cpp
  cache.cpp
  downloader.cpp
  event.cpp
  gfx.cpp
  gpsdclient.cpp
  gpsdlayer.cpp
  layer.cpp
  markerlayer.cpp
  osmlayer.cpp
  scalelayer.cpp
  settings.cpp
  shell.cpp
  tracklayer.cpp
  unit.cpp
  utils.cpp
  viewport.cpp
  wgt_eleprofile.cpp
  wgt_map.cpp)

set(florb_INCLUDE_DIRS
  ${Boost_INCLUDE_DIRS}
  ${CURL_INCLUDE_DIRS}
  ${FLTK_INCLUDE_DIRS}
  ${Intl_INCLUDE_DIRS}
  ${TinyXML2_INCLUDE_DIRS})

add_executable(florb ${florb_SOURCES})
add_dependencies(florb Ui version)

target_include_directories(florb PUBLIC ${florb_INCLUDE_DIRS})

target_link_directories(florb PRIVATE
  ${Boost_LIBRARY_DIRS}
  ${CURL_LIBRARY_DIRS})

target_link_libraries(florb PRIVATE Ui)
target_link_libraries(florb PRIVATE ${FLTK_LIBRARIES})
target_link_libraries(florb PRIVATE ${Boost_LIBRARIES})
target_link_libraries(florb PRIVATE ${TinyXML2_LIBRARIES})
target_link_libraries(florb PRIVATE yaml-cpp)
target_link_libraries(florb PRIVATE ${CURL_LIBRARIES})
target_link_libraries(florb PRIVATE ${Intl_LIBRARIES})
target_link_libraries(florb PRIVATE ${libgps_LIBRARIES})

add_subdirectory(i18n)
