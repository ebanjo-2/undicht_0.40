add_executable(hello_world src/main.cpp)

target_link_libraries(hello_world core graphics tools)

add_custom_target(run_hello_world COMMAND gnome-terminal -- ${PROJECT_SOURCE_DIR}/build/examples/hello_world/hello_world)
