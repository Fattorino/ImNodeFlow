# Avoid multiple calls to find_package to append duplicated properties to the targets
include_guard()########### VARIABLES #######################################################################
#############################################################################################
set(opengl_FRAMEWORKS_FOUND_DEBUG "") # Will be filled later
conan_find_apple_frameworks(opengl_FRAMEWORKS_FOUND_DEBUG "${opengl_FRAMEWORKS_DEBUG}" "${opengl_FRAMEWORK_DIRS_DEBUG}")

set(opengl_LIBRARIES_TARGETS "") # Will be filled later


######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
if(NOT TARGET opengl_DEPS_TARGET)
    add_library(opengl_DEPS_TARGET INTERFACE IMPORTED)
endif()

set_property(TARGET opengl_DEPS_TARGET
             PROPERTY INTERFACE_LINK_LIBRARIES
             $<$<CONFIG:Debug>:${opengl_FRAMEWORKS_FOUND_DEBUG}>
             $<$<CONFIG:Debug>:${opengl_SYSTEM_LIBS_DEBUG}>
             $<$<CONFIG:Debug>:>
             APPEND)

####### Find the libraries declared in cpp_info.libs, create an IMPORTED target for each one and link the
####### opengl_DEPS_TARGET to all of them
conan_package_library_targets("${opengl_LIBS_DEBUG}"    # libraries
                              "${opengl_LIB_DIRS_DEBUG}" # package_libdir
                              "${opengl_BIN_DIRS_DEBUG}" # package_bindir
                              "${opengl_LIBRARY_TYPE_DEBUG}"
                              "${opengl_IS_HOST_WINDOWS_DEBUG}"
                              opengl_DEPS_TARGET
                              opengl_LIBRARIES_TARGETS  # out_libraries_targets
                              "_DEBUG"
                              "opengl"    # package_name
                              "${opengl_NO_SONAME_MODE_DEBUG}")  # soname

# FIXME: What is the result of this for multi-config? All configs adding themselves to path?
set(CMAKE_MODULE_PATH ${opengl_BUILD_DIRS_DEBUG} ${CMAKE_MODULE_PATH})

########## GLOBAL TARGET PROPERTIES Debug ########################################
    set_property(TARGET opengl::opengl
                 PROPERTY INTERFACE_LINK_LIBRARIES
                 $<$<CONFIG:Debug>:${opengl_OBJECTS_DEBUG}>
                 $<$<CONFIG:Debug>:${opengl_LIBRARIES_TARGETS}>
                 APPEND)

    if("${opengl_LIBS_DEBUG}" STREQUAL "")
        # If the package is not declaring any "cpp_info.libs" the package deps, system libs,
        # frameworks etc are not linked to the imported targets and we need to do it to the
        # global target
        set_property(TARGET opengl::opengl
                     PROPERTY INTERFACE_LINK_LIBRARIES
                     opengl_DEPS_TARGET
                     APPEND)
    endif()

    set_property(TARGET opengl::opengl
                 PROPERTY INTERFACE_LINK_OPTIONS
                 $<$<CONFIG:Debug>:${opengl_LINKER_FLAGS_DEBUG}> APPEND)
    set_property(TARGET opengl::opengl
                 PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                 $<$<CONFIG:Debug>:${opengl_INCLUDE_DIRS_DEBUG}> APPEND)
    # Necessary to find LINK shared libraries in Linux
    set_property(TARGET opengl::opengl
                 PROPERTY INTERFACE_LINK_DIRECTORIES
                 $<$<CONFIG:Debug>:${opengl_LIB_DIRS_DEBUG}> APPEND)
    set_property(TARGET opengl::opengl
                 PROPERTY INTERFACE_COMPILE_DEFINITIONS
                 $<$<CONFIG:Debug>:${opengl_COMPILE_DEFINITIONS_DEBUG}> APPEND)
    set_property(TARGET opengl::opengl
                 PROPERTY INTERFACE_COMPILE_OPTIONS
                 $<$<CONFIG:Debug>:${opengl_COMPILE_OPTIONS_DEBUG}> APPEND)

########## For the modules (FindXXX)
set(opengl_LIBRARIES_DEBUG opengl::opengl)
