set(MODULE model)

set(PROTO_SOURCES "model.proto")

PROTOBUF_GENERATE_CPP(PROTO_SRCS PROTO_HDRS ${PROTO_SOURCES})
message(STATUS "${MODULE} sources: ${PROTO_SRCS}")
message(STATUS "${MODULE} headers: ${PROTO_HDRS}")

add_library(${MODULE} STATIC ${PROTO_SRCS} ${PROTO_HDRS} ${PROTO_SOURCES})
target_link_libraries(${MODULE} ${PROTOBUF_LIBRARIES})

set(MODEL_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/${MODULE}/protobuf/")
file(MAKE_DIRECTORY ${MODEL_INCLUDE_DIR})
foreach(PROTO_HDR ${PROTO_HDRS})
    add_custom_command(TARGET model
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
            ${PROTO_HDR}
            ${MODEL_INCLUDE_DIR}
    )
endforeach()
