add_executable(v8-phi test-with-v8.cpp)

target_link_libraries(v8-phi PRIVATE phi)

find_library(WEE8 wee8 HINTS /home/tal/Documents/Studium/TUM/Kurse/7-WS21-22/BA/v8/v8/out/wee8/obj/)
target_link_libraries(v8-phi PRIVATE ${WEE8})

find_path (V8_INCLUDE wasm.hh HINTS /home/tal/Documents/Studium/TUM/Kurse/7-WS21-22/BA/v8/v8/third_party/wasm-api/)
target_include_directories(v8-phi PRIVATE ${V8_INCLUDE})

# ----
add_executable(wasmer-phi test-with-wasmer.cpp Benchmark.cpp)

target_link_libraries(wasmer-phi PRIVATE phi)

find_library(WASMER_LIB wasmer HINTS /home/tal/.wasmer/lib)
target_link_libraries(wasmer-phi PRIVATE ${WASMER_LIB})

target_include_directories(wasmer-phi PRIVATE /home/tal/.wasmer/include/)

# ----
add_executable(wasmtime-phi test-with-wasmtime.cpp Benchmark.cpp)

target_link_libraries(wasmtime-phi PRIVATE phi)

find_library(WASMTIME_LIB wasmtime HINTS /home/tal/Documents/Studium/TUM/Kurse/7-WS21-22/BA/wasmtime/target/release/)
target_link_libraries(wasmtime-phi PRIVATE ${WASMTIME_LIB})

target_include_directories(wasmtime-phi PRIVATE /home/tal/Documents/Studium/TUM/Kurse/7-WS21-22/BA/wasmtime/crates/c-api/include/)
target_include_directories(wasmtime-phi PRIVATE /home/tal/Documents/Studium/TUM/Kurse/7-WS21-22/BA/wasmtime/crates/c-api/wasm-c-api/include/)

# ----

SET(V8_LINKER_FLAGS "-Wl,--no-as-needed -ldl")
SET(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} ${V8_LINKER_FLAGS}")

# ----

find_package (Threads REQUIRED)
