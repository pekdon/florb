cmake_minimum_required(VERSION 3.10)

project(florb)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(OpenGL_GL_PREFERENCE GLVND)

include(FindPkgConfig)

find_package(Boost REQUIRED COMPONENTS system filesystem thread)
find_package(CURL REQUIRED)
find_package(FLTK REQUIRED)
find_package(Gettext REQUIRED)
find_package(Intl REQUIRED)
find_package(TinyXML2)
if (TinyXML2_FOUND)
  set(TinyXML2_LIBRARIES tinyxml2::tinyxml2)
else (Tinyxml2_FOUND)
  pkg_check_modules(TinyXML2 REQUIRED tinyxml2)
endif (NOT TinyXML2_FOUND)
find_package(yaml-cpp REQUIRED)

# optional packages
find_package(libgps)
if (libgps_FOUND)
  set(HAVE_LIBGPS 1)
else (libgps_FOUND)
  pkg_check_modules(libgps libgps)
  if (libgps_FOUND)
    set(HAVE_LIBGPS 1)
  endif (libgps_FOUND)
endif (libgps_FOUND)

add_subdirectory(src)
