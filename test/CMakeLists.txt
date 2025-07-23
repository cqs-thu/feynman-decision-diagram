# GTest Setup
enable_testing()

add_executable(circuit_tests test.cpp)

target_link_libraries(circuit_tests PRIVATE "${CUDD_LIB_FILES}")
target_include_directories(circuit_tests PRIVATE "${CUDD_BUILD_DIR}/include")
target_include_directories(circuit_tests PRIVATE "${PROJECT_SOURCE_DIR}/external/cudd/cudd")
target_include_directories(circuit_tests PRIVATE "${PROJECT_SOURCE_DIR}/external/cudd/")
target_include_directories(circuit_tests PRIVATE "${PROJECT_SOURCE_DIR}/external/cudd/st")
target_include_directories(circuit_tests PRIVATE "${PROJECT_SOURCE_DIR}/external/cudd/mtr")
target_include_directories(circuit_tests PRIVATE "${PROJECT_SOURCE_DIR}/external/cudd/epd")

# target_link_libraries(circuit_tests PRIVATE Sylvan::Sylvan)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
target_link_libraries(circuit_tests PUBLIC gtest gtest_main feynman)
gtest_discover_tests(circuit_tests)
