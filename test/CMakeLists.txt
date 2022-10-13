

L_add_depend_gtest()

file(GLOB source LIST_DIRECTORIES false src/*)
add_executable(primitive-test ${source})
target_link_libraries(primitive-test PRIVATE LGL::Primitive GTest::gtest)
L_target_set_cpp(primitive-test 20)