# Avoid multiple calls to find_package to append duplicated properties to the targets
include_guard()########### VARIABLES #######################################################################
#############################################################################################
set(imgui_FRAMEWORKS_FOUND_DEBUG "") # Will be filled later
conan_find_apple_frameworks(imgui_FRAMEWORKS_FOUND_DEBUG "${imgui_FRAMEWORKS_DEBUG}" "${imgui_FRAMEWORK_DIRS_DEBUG}")

set(imgui_LIBRARIES_TARGETS "") # Will be filled later


######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
if(NOT TARGET imgui_DEPS_TARGET)
    add_library(imgui_DEPS_TARGET INTERFACE IMPORTED)
endif()

set_property(TARGET imgui_DEPS_TARGET
             PROPERTY INTERFACE_LINK_LIBRARIES
             $<$<CONFIG:Debug>:${imgui_FRAMEWORKS_FOUND_DEBUG}>
             $<$<CONFIG:Debug>:${imgui_SYSTEM_LIBS_DEBUG}>
             $<$<CONFIG:Debug>:>
             APPEND)

####### Find the libraries declared in cpp_info.libs, create an IMPORTED target for each one and link the
####### imgui_DEPS_TARGET to all of them
conan_package_library_targets("${imgui_LIBS_DEBUG}"    # libraries
                              "${imgui_LIB_DIRS_DEBUG}" # package_libdir
                              "${imgui_BIN_DIRS_DEBUG}" # package_bindir
                              "${imgui_LIBRARY_TYPE_DEBUG}"
                              "${imgui_IS_HOST_WINDOWS_DEBUG}"
                              imgui_DEPS_TARGET
                              imgui_LIBRARIES_TARGETS  # out_libraries_targets
                              "_DEBUG"
                              "imgui"    # package_name
                              "${imgui_NO_SONAME_MODE_DEBUG}")  # soname

# FIXME: What is the result of this for multi-config? All configs adding themselves to path?
set(CMAKE_MODULE_PATH ${imgui_BUILD_DIRS_DEBUG} ${CMAKE_MODULE_PATH})

########## GLOBAL TARGET PROPERTIES Debug ########################################
    set_property(TARGET imgui::imgui
                 PROPERTY INTERFACE_LINK_LIBRARIES
                 $<$<CONFIG:Debug>:${imgui_OBJECTS_DEBUG}>
                 $<$<CONFIG:Debug>:${imgui_LIBRARIES_TARGETS}>
                 APPEND)

    if("${imgui_LIBS_DEBUG}" STREQUAL "")
        # If the package is not declaring any "cpp_info.libs" the package deps, system libs,
        # frameworks etc are not linked to the imported targets and we need to do it to the
        # global target
        set_property(TARGET imgui::imgui
                     PROPERTY INTERFACE_LINK_LIBRARIES
                     imgui_DEPS_TARGET
                     APPEND)
    endif()

    set_property(TARGET imgui::imgui
                 PROPERTY INTERFACE_LINK_OPTIONS
                 $<$<CONFIG:Debug>:${imgui_LINKER_FLAGS_DEBUG}> APPEND)
    set_property(TARGET imgui::imgui
                 PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                 $<$<CONFIG:Debug>:${imgui_INCLUDE_DIRS_DEBUG}> APPEND)
    # Necessary to find LINK shared libraries in Linux
    set_property(TARGET imgui::imgui
                 PROPERTY INTERFACE_LINK_DIRECTORIES
                 $<$<CONFIG:Debug>:${imgui_LIB_DIRS_DEBUG}> APPEND)
    set_property(TARGET imgui::imgui
                 PROPERTY INTERFACE_COMPILE_DEFINITIONS
                 $<$<CONFIG:Debug>:${imgui_COMPILE_DEFINITIONS_DEBUG}> APPEND)
    set_property(TARGET imgui::imgui
                 PROPERTY INTERFACE_COMPILE_OPTIONS
                 $<$<CONFIG:Debug>:${imgui_COMPILE_OPTIONS_DEBUG}> APPEND)

########## For the modules (FindXXX)
set(imgui_LIBRARIES_DEBUG imgui::imgui)
