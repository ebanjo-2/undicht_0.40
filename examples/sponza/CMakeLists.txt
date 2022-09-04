add_executable(sponza src/main.cpp)

target_link_libraries(sponza core graphics tools)

add_custom_target(run_sponza COMMAND gnome-terminal -- ${PROJECT_SOURCE_DIR}/build/examples/sponza/sponza)
