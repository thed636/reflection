include_directories("${CMAKE_CURRENT_SOURCE_DIR}/request_handlers")

set(yreflection_single_reply_SRC
  "yreflection_single_reply_main.cpp"
  "request_handlers/yreflection_single_reply.cpp"
)

add_executable(yreflection_single_reply ${yreflection_single_reply_SRC})
target_link_libraries(yreflection_single_reply
  ${Boost_LIBRARIES}
  ${yajl_LIBRARIES}
  server
)
