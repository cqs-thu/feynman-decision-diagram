macro(add_feynman_dd NAME)
  add_library(feynman tensor_network.cpp)

  # Adiar
  # add_executable(adiar_${NAME}_bdd adiar/${NAME}_bdd.cpp)
  # target_link_libraries(adiar_${NAME}_bdd PRIVATE adiar feynman)
  # if (BDD_BENCHMARK_STATS)
  #   target_compile_definitions(adiar_${NAME}_bdd PRIVATE BDD_BENCHMARK_STATS)
  # endif()

  # BuDDy
  # add_executable(buddy_${NAME}_bdd buddy/${NAME}_bdd.cpp)
  # target_include_directories(buddy_${NAME}_bdd PRIVATE "${BDD_SOURCE_DIR}")
  # target_link_libraries(buddy_${NAME}_bdd PRIVATE buddy feynman)
  # if (BDD_BENCHMARK_STATS)
  #   target_compile_definitions(buddy_${NAME}_bdd PRIVATE BDD_BENCHMARK_STATS)
  # endif()

  # CAL
  # add_executable(cal_${NAME}_bdd cal/${NAME}_bdd.cpp)
  # target_link_libraries(cal_${NAME}_bdd PRIVATE cal feynman)
  # if (BDD_BENCHMARK_STATS)
  #   target_compile_definitions(cal_${NAME}_bdd PRIVATE BDD_BENCHMARK_STATS)
  # endif()

  find_library(GMP_LIB gmp /usr/local/lib)
  if (NOT GMP_LIB)
    message(FATAL_ERROR "Could not find GMP library")
  else ()
    message(STATUS "Found GMP library: ${GMP_LIB}")
  endif()
  target_link_libraries(feynman PRIVATE ${GMP_LIB})

  # CUDD
  add_executable(cudd_${NAME}_bdd cudd/${NAME}_bdd.cpp)
  target_link_libraries(cudd_${NAME}_bdd PRIVATE "${CUDD_LIB_FILES}" feynman)
  target_include_directories(cudd_${NAME}_bdd PRIVATE "${CUDD_BUILD_DIR}/include")
  target_include_directories(cudd_${NAME}_bdd PRIVATE "${PROJECT_SOURCE_DIR}/external/cudd/cudd")
  target_include_directories(cudd_${NAME}_bdd PRIVATE "${PROJECT_SOURCE_DIR}/external/cudd/")
  target_include_directories(cudd_${NAME}_bdd PRIVATE "${PROJECT_SOURCE_DIR}/external/cudd/st")
  target_include_directories(cudd_${NAME}_bdd PRIVATE "${PROJECT_SOURCE_DIR}/external/cudd/mtr")
  target_include_directories(cudd_${NAME}_bdd PRIVATE "${PROJECT_SOURCE_DIR}/external/cudd/epd")
  if (BDD_BENCHMARK_STATS)
    target_compile_definitions(cudd_${NAME}_bdd PRIVATE BDD_BENCHMARK_STATS)
  endif()

  # Sylvan
  # add_executable(sylvan_${NAME}_bdd sylvan/${NAME}_bdd.cpp)
  # target_link_libraries(sylvan_${NAME}_bdd PRIVATE Sylvan::Sylvan feynman)
  # if (BDD_BENCHMARK_STATS)
  #   target_compile_definitions(sylvan_${NAME}_bdd PRIVATE BDD_BENCHMARK_STATS)
  # endif()
endmacro(add_feynman_dd)

macro(link_bdd_extra NAME LIB_NAME)
  # target_link_libraries(adiar_${NAME}_bdd  PRIVATE ${LIB_NAME})
  # target_link_libraries(buddy_${NAME}_bdd  PRIVATE ${LIB_NAME})
  # target_link_libraries(cal_${NAME}_bdd    PRIVATE ${LIB_NAME})
  target_link_libraries(cudd_${NAME}_bdd   PRIVATE ${LIB_NAME})
  # target_link_libraries(sylvan_${NAME}_bdd PRIVATE ${LIB_NAME})
endmacro(link_bdd_extra)

add_feynman_dd(circuit)

