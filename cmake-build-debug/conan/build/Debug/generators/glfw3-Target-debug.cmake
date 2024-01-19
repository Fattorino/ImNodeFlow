# Avoid multiple calls to find_package to append duplicated properties to the targets
include_guard()########### VARIABLES #######################################################################
#############################################################################################
set(glfw_FRAMEWORKS_FOUND_DEBUG "") # Will be filled later
conan_find_apple_frameworks(glfw_FRAMEWORKS_FOUND_DEBUG "${glfw_FRAMEWORKS_DEBUG}" "${glfw_FRAMEWORK_DIRS_DEBUG}")

set(glfw_LIBRARIES_TARGETS "") # Will be filled later


######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
if(NOT TARGET glfw_DEPS_TARGET)
    add_library(glfw_DEPS_TARGET INTERFACE IMPORTED)
endif()

set_property(TARGET glfw_DEPS_TARGET
             PROPERTY INTERFACE_LINK_LIBRARIES
             $<$<CONFIG:Debug>:${glfw_FRAMEWORKS_FOUND_DEBUG}>
             $<$<CONFIG:Debug>:${glfw_SYSTEM_LIBS_DEBUG}>
             $<$<CONFIG:Debug>:opengl::opengl>
             APPEND)

####### Find the libraries declared in cpp_info.libs, create an IMPORTED target for each one and link the
####### glfw_DEPS_TARGET to all of them
conan_package_library_targets("${glfw_LIBS_DEBUG}"    # libraries
                              "${glfw_LIB_DIRS_DEBUG}" # package_libdir
                              "${glfw_BIN_DIRS_DEBUG}" # package_bindir
                              "${glfw_LIBRARY_TYPE_DEBUG}"
                              "${glfw_IS_HOST_WINDOWS_DEBUG}"
                              glfw_DEPS_TARGET
                              glfw_LIBRARIES_TARGETS  # out_libraries_targets
                              "_DEBUG"
                              "glfw"    # package_name
                              "${glfw_NO_SONAME_MODE_DEBUG}")  # soname

# FIXME: What is the result of this for multi-config? All configs adding themselves to path?
set(CMAKE_MODULE_PATH ${glfw_BUILD_DIRS_DEBUG} ${CMAKE_MODULE_PATH})

########## GLOBAL TARGET PROPERTIES Debug ########################################
    set_property(TARGET glfw
                 PROPERTY INTERFACE_LINK_LIBRARIES
                 $<$<CONFIG:Debug>:${glfw_OBJECTS_DEBUG}>
                 $<$<CONFIG:Debug>:${glfw_LIBRARIES_TARGETS}>
                 APPEND)

    if("${glfw_LIBS_DEBUG}" STREQUAL "")
        # If the package is not declaring any "cpp_info.libs" the package deps, system libs,
        # frameworks etc are not linked to the imported targets and we need to do it to the
        # global target
        set_property(TARGET glfw
                     PROPERTY INTERFACE_LINK_LIBRARIES
                     glfw_DEPS_TARGET
                     APPEND)
    endif()

    set_property(TARGET glfw
                 PROPERTY INTERFACE_LINK_OPTIONS
                 $<$<CONFIG:Debug>:${glfw_LINKER_FLAGS_DEBUG}> APPEND)
    set_property(TARGET glfw
                 PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                 $<$<CONFIG:Debug>:${glfw_INCLUDE_DIRS_DEBUG}> APPEND)
    # Necessary to find LINK shared libraries in Linux
    set_property(TARGET glfw
                 PROPERTY INTERFACE_LINK_DIRECTORIES
                 $<$<CONFIG:Debug>:${glfw_LIB_DIRS_DEBUG}> APPEND)
    set_property(TARGET glfw
                 PROPERTY INTERFACE_COMPILE_DEFINITIONS
                 $<$<CONFIG:Debug>:${glfw_COMPILE_DEFINITIONS_DEBUG}> APPEND)
    set_property(TARGET glfw
                 PROPERTY INTERFACE_COMPILE_OPTIONS
                 $<$<CONFIG:Debug>:${glfw_COMPILE_OPTIONS_DEBUG}> APPEND)

########## For the modules (FindXXX)
set(glfw_LIBRARIES_DEBUG glfw)
