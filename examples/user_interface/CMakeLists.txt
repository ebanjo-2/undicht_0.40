add_executable(user_interface src/main.cpp)

target_link_libraries(user_interface core graphics tools)

add_custom_target(run_user_interface COMMAND gnome-terminal -- ${PROJECT_SOURCE_DIR}/build/examples/user_interface/user_interface)
