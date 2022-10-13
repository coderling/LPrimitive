add_executable(empty empty.cpp)
target_link_libraries(empty PRIVATE LGL::Primitive)
L_target_set_cpp(empty 20)