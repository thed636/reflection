file(GLOB utest_SRC "*.cc")

add_executable(utest ${utest_SRC})
target_link_libraries(utest ${Boost_LIBRARIES} gtest gmock ${yajl_LIBRARIES} ${LIBXML2_LIBRARIES})

add_custom_target( check
    COMMAND utest )

