include_directories("${CMAKE_CURRENT_SOURCE_DIR}/request_handlers")

set(yreflection_single_reply_SRC
  "yreflection_single_reply_main.cpp"
)

add_executable(yreflection_single_reply ${yreflection_single_reply_SRC})
target_link_libraries(yreflection_single_reply
  ${Boost_LIBRARIES}
  ${yajl_LIBRARIES}
  server
)

set(handwritten_single_reply_SRC
  "handwritten_single_reply_main.cpp"
)

add_executable(handwritten_single_reply ${handwritten_single_reply_SRC})
target_link_libraries(handwritten_single_reply
  ${Boost_LIBRARIES}
  server
)

set(yajl_single_reply_SRC
  "yajl_single_reply_main.cpp"
)

add_executable(yajl_single_reply ${yajl_single_reply_SRC})
target_link_libraries(yajl_single_reply
  ${Boost_LIBRARIES}
  ${yajl_LIBRARIES}
  server
)
