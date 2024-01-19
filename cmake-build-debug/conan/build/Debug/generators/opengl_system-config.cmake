########## MACROS ###########################################################################
#############################################################################################

# Requires CMake > 3.15
if(${CMAKE_VERSION} VERSION_LESS "3.15")
    message(FATAL_ERROR "The 'CMakeDeps' generator only works with CMake >= 3.15")
endif()

if(opengl_system_FIND_QUIETLY)
    set(opengl_system_MESSAGE_MODE VERBOSE)
else()
    set(opengl_system_MESSAGE_MODE STATUS)
endif()

include(${CMAKE_CURRENT_LIST_DIR}/cmakedeps_macros.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/opengl_systemTargets.cmake)
include(CMakeFindDependencyMacro)

check_build_type_defined()

foreach(_DEPENDENCY ${opengl_FIND_DEPENDENCY_NAMES} )
    # Check that we have not already called a find_package with the transitive dependency
    if(NOT ${_DEPENDENCY}_FOUND)
        find_dependency(${_DEPENDENCY} REQUIRED ${${_DEPENDENCY}_FIND_MODE})
    endif()
endforeach()

set(opengl_system_VERSION_STRING "system")
set(opengl_system_INCLUDE_DIRS ${opengl_INCLUDE_DIRS_DEBUG} )
set(opengl_system_INCLUDE_DIR ${opengl_INCLUDE_DIRS_DEBUG} )
set(opengl_system_LIBRARIES ${opengl_LIBRARIES_DEBUG} )
set(opengl_system_DEFINITIONS ${opengl_DEFINITIONS_DEBUG} )

# Only the first installed configuration is included to avoid the collision
foreach(_BUILD_MODULE ${opengl_BUILD_MODULES_PATHS_DEBUG} )
    message(${opengl_system_MESSAGE_MODE} "Conan: Including build module from '${_BUILD_MODULE}'")
    include(${_BUILD_MODULE})
endforeach()


