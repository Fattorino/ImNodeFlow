find_package(OpenGL REQUIRED)
find_package(SDL2 REQUIRED)
if (UNIX)
    if (NOT APPLE)
        find_package(Threads REQUIRED)
        find_package(X11 REQUIRED)
        target_link_libraries(nd PRIVATE
                ${CMAKE_THREAD_LIBS_INIT} ${X11_LIBRARIES} ${CMAKE_DL_LIBS})
    endif()
endif()

# Fix for GNU libstdfs
# target_link_libraries(nd PUBLIC "$<$<CXX_COMPILER_ID:CXX,GNU>:stdc++fs>")
target_link_libraries(nd PUBLIC OpenGL::GL SDL2::SDL2)

