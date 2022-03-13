add_executable(v8-phi test-with-v8.cpp)
target_link_libraries(v8-phi PRIVATE phi)
find_library(WEE8 wee8 HINTS /home/tal/Documents/Studium/TUM/Kurse/7-WS21-22/BA/v8/v8/out/wee8/obj/)
target_link_libraries(v8-phi PRIVATE ${WEE8})
find_path (V8_INCLUDE wasm.hh HINTS /home/tal/Documents/Studium/TUM/Kurse/7-WS21-22/BA/v8/v8/third_party/wasm-api/)
target_include_directories(v8-phi PRIVATE ${V8_INCLUDE})

SET(V8_LINKER_FLAGS "-Wl,--no-as-needed -ldl")
SET(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} ${V8_LINKER_FLAGS}")

find_package (Threads REQUIRED)
