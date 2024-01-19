########## MACROS ###########################################################################
#############################################################################################

# Requires CMake > 3.15
if(${CMAKE_VERSION} VERSION_LESS "3.15")
    message(FATAL_ERROR "The 'CMakeDeps' generator only works with CMake >= 3.15")
endif()

if(glfw3_FIND_QUIETLY)
    set(glfw3_MESSAGE_MODE VERBOSE)
else()
    set(glfw3_MESSAGE_MODE STATUS)
endif()

include(${CMAKE_CURRENT_LIST_DIR}/cmakedeps_macros.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/glfw3Targets.cmake)
include(CMakeFindDependencyMacro)

check_build_type_defined()

foreach(_DEPENDENCY ${glfw_FIND_DEPENDENCY_NAMES} )
    # Check that we have not already called a find_package with the transitive dependency
    if(NOT ${_DEPENDENCY}_FOUND)
        find_dependency(${_DEPENDENCY} REQUIRED ${${_DEPENDENCY}_FIND_MODE})
    endif()
endforeach()

set(glfw3_VERSION_STRING "3.3.8")
set(glfw3_INCLUDE_DIRS ${glfw_INCLUDE_DIRS_DEBUG} )
set(glfw3_INCLUDE_DIR ${glfw_INCLUDE_DIRS_DEBUG} )
set(glfw3_LIBRARIES ${glfw_LIBRARIES_DEBUG} )
set(glfw3_DEFINITIONS ${glfw_DEFINITIONS_DEBUG} )

# Only the first installed configuration is included to avoid the collision
foreach(_BUILD_MODULE ${glfw_BUILD_MODULES_PATHS_DEBUG} )
    message(${glfw3_MESSAGE_MODE} "Conan: Including build module from '${_BUILD_MODULE}'")
    include(${_BUILD_MODULE})
endforeach()


