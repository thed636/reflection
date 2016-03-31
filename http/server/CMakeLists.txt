
# Add test executable target

file(GLOB server_SRC "*.cpp")

add_executable(server ${server_SRC})
target_link_libraries(server ${Boost_LIBRARIES} ${yajl_LIBRARIES})
