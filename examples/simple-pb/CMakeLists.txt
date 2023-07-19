cmake_minimum_required(VERSION 3.5.1)

project(cyclone_simple_pb)

set(TARGET_NAME ${PROJECT_NAME})

file(GLOB PROTO_SRC
  "${CMAKE_CURRENT_SOURCE_DIR}/*.pb.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/*.pb.h"
)

add_executable(${TARGET_NAME}_runner
  ${PROTO_SRC}
  main.cc
)

target_link_libraries(${TARGET_NAME}_runner
  ${Protobuf_LIBRARIES}
  cyclone
)
