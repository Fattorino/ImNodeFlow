########### AGGREGATED COMPONENTS AND DEPENDENCIES FOR THE MULTI CONFIG #####################
#############################################################################################

set(glfw_COMPONENT_NAMES "")
list(APPEND glfw_FIND_DEPENDENCY_NAMES opengl_system)
list(REMOVE_DUPLICATES glfw_FIND_DEPENDENCY_NAMES)
set(opengl_system_FIND_MODE "NO_MODULE")

########### VARIABLES #######################################################################
#############################################################################################
set(glfw_PACKAGE_FOLDER_DEBUG "C:/Users/admin/.conan2/p/b/glfwda36766526193/p")
set(glfw_BUILD_MODULES_PATHS_DEBUG )


set(glfw_INCLUDE_DIRS_DEBUG "${glfw_PACKAGE_FOLDER_DEBUG}/include")
set(glfw_RES_DIRS_DEBUG )
set(glfw_DEFINITIONS_DEBUG )
set(glfw_SHARED_LINK_FLAGS_DEBUG )
set(glfw_EXE_LINK_FLAGS_DEBUG )
set(glfw_OBJECTS_DEBUG )
set(glfw_COMPILE_DEFINITIONS_DEBUG )
set(glfw_COMPILE_OPTIONS_C_DEBUG )
set(glfw_COMPILE_OPTIONS_CXX_DEBUG )
set(glfw_LIB_DIRS_DEBUG "${glfw_PACKAGE_FOLDER_DEBUG}/lib")
set(glfw_BIN_DIRS_DEBUG )
set(glfw_LIBRARY_TYPE_DEBUG STATIC)
set(glfw_IS_HOST_WINDOWS_DEBUG 1)
set(glfw_LIBS_DEBUG glfw3)
set(glfw_SYSTEM_LIBS_DEBUG gdi32)
set(glfw_FRAMEWORK_DIRS_DEBUG )
set(glfw_FRAMEWORKS_DEBUG )
set(glfw_BUILD_DIRS_DEBUG )
set(glfw_NO_SONAME_MODE_DEBUG FALSE)


# COMPOUND VARIABLES
set(glfw_COMPILE_OPTIONS_DEBUG
    "$<$<COMPILE_LANGUAGE:CXX>:${glfw_COMPILE_OPTIONS_CXX_DEBUG}>"
    "$<$<COMPILE_LANGUAGE:C>:${glfw_COMPILE_OPTIONS_C_DEBUG}>")
set(glfw_LINKER_FLAGS_DEBUG
    "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${glfw_SHARED_LINK_FLAGS_DEBUG}>"
    "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${glfw_SHARED_LINK_FLAGS_DEBUG}>"
    "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${glfw_EXE_LINK_FLAGS_DEBUG}>")


set(glfw_COMPONENTS_DEBUG )