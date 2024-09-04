find_package(OpenGL REQUIRED)
find_package(SDL2 REQUIRED)
if (UNIX)
    if (NOT APPLE)
        find_package(Threads REQUIRED)
        find_package(X11 REQUIRED)
        target_link_libraries(example PRIVATE
                ${CMAKE_THREAD_LIBS_INIT} ${X11_LIBRARIES} ${CMAKE_DL_LIBS})
    endif()
endif()

target_link_libraries(example PUBLIC OpenGL::GL SDL2::SDL2)
