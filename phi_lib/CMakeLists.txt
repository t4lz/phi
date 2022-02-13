add_library(phi phi.cpp phi.hpp)
find_library(BINARYEN_LIB binaryen HINTS /home/tal/Documents/Studium/TUM/Kurse/7-WS21-22/BA/binaryen/lib)
target_link_libraries(phi PRIVATE ${BINARYEN_LIB})
find_path (BINARYEN_C_H_DIR binaryen-c.h HINTS /home/tal/Documents/Studium/TUM/Kurse/7-WS21-22/BA/binaryen/src/)
target_include_directories(phi PRIVATE ${BINARYEN_C_H_DIR})

find_package (Threads REQUIRED)
target_link_libraries(phi PRIVATE ${CMAKE_THREAD_LIBS_INIT})


