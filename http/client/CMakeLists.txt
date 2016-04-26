file(GLOB client_SRC "*.cpp")

add_executable(client ${client_SRC})
target_link_libraries(client
  ${Boost_LIBRARIES}
)
