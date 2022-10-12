add_executable(empty empty.cpp)
target_link_libraries(empty PRIVATE LPrimitiveInterface)
set_target_properties(empty PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
)