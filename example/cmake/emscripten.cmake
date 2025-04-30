message("^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^")
message("^^^^^^^^^^ Enabling emscripten compile ^^^^^^^^^^^")
message("^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^")

set(CMAKE_EXECUTABLE_SUFFIX ".html")
target_compile_options(example PUBLIC -sUSE_SDL=2 -fwasm-exceptions)
target_compile_definitions(example PUBLIC "-DIMGUI_DISABLE_FILE_FUNCTIONS -Wall -Wformat -Os")
target_link_options(example PUBLIC -sUSE_SDL=2 -fwasm-exceptions -sWASM=1 -sALLOW_MEMORY_GROWTH=1
                              -sNO_EXIT_RUNTIME=0 -sASSERTIONS=1 -sNO_FILESYSTEM=1
                              --no-heap-copy --shell-file ${CMAKE_SOURCE_DIR}/html/shell_min.html
                              --llvm-lto -O2 -Oz -s ELIMINATE_DUPLICATE_FUNCTIONS=1)
target_include_directories(example PRIVATE ${IMGUI_DIR}/examples/libs)

