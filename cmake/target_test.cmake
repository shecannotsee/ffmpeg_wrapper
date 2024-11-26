if (build_tests STREQUAL ON)
    file(GLOB_RECURSE test_src "${CMAKE_SOURCE_DIR}/test/*")
    list(REMOVE_ITEM other_src "${CMAKE_SOURCE_DIR}/other/main.cpp")
    add_executable(${test_name} ${src} ${other_src} ${test_src})
    list(APPEND test_link_dependency ${src_link_dependency} ${other_link_dependency})
    target_link_libraries(${test_name} ${test_link_dependency})
endif ()

