# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file LICENSE.rst or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION ${CMAKE_VERSION}) # this file comes with cmake

# If CMAKE_DISABLE_SOURCE_CHANGES is set to true and the source directory is an
# existing directory in our source tree, calling file(MAKE_DIRECTORY) on it
# would cause a fatal error, even though it would be a no-op.
if(NOT EXISTS "C:/DJROOFRAT/DJ-ROOFRAT/build-test-phase7a/_deps/glfw3-src")
  file(MAKE_DIRECTORY "C:/DJROOFRAT/DJ-ROOFRAT/build-test-phase7a/_deps/glfw3-src")
endif()
file(MAKE_DIRECTORY
  "C:/DJROOFRAT/DJ-ROOFRAT/build-test-phase7a/_deps/glfw3-build"
  "C:/DJROOFRAT/DJ-ROOFRAT/build-test-phase7a/_deps/glfw3-subbuild/glfw3-populate-prefix"
  "C:/DJROOFRAT/DJ-ROOFRAT/build-test-phase7a/_deps/glfw3-subbuild/glfw3-populate-prefix/tmp"
  "C:/DJROOFRAT/DJ-ROOFRAT/build-test-phase7a/_deps/glfw3-subbuild/glfw3-populate-prefix/src/glfw3-populate-stamp"
  "C:/DJROOFRAT/DJ-ROOFRAT/build-test-phase7a/_deps/glfw3-subbuild/glfw3-populate-prefix/src"
  "C:/DJROOFRAT/DJ-ROOFRAT/build-test-phase7a/_deps/glfw3-subbuild/glfw3-populate-prefix/src/glfw3-populate-stamp"
)

set(configSubDirs Debug)
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "C:/DJROOFRAT/DJ-ROOFRAT/build-test-phase7a/_deps/glfw3-subbuild/glfw3-populate-prefix/src/glfw3-populate-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "C:/DJROOFRAT/DJ-ROOFRAT/build-test-phase7a/_deps/glfw3-subbuild/glfw3-populate-prefix/src/glfw3-populate-stamp${cfgdir}") # cfgdir has leading slash
endif()
