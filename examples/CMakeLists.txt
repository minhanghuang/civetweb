include_directories(
  ../src
)

find_package(Protobuf)

if(${Protobuf_FOUND})
  include_directories(
    ${Protobuf_INCLUDE_DIRS}
  )
  link_directories(
    ${Protobuf_LIBRARY_DIRS}
  )
  file(GLOB_RECURSE PROTO_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/*.proto"
  )
  LIST(APPEND PROTO_FLAGS
   -I${CMAKE_CURRENT_SOURCE_DIR}
  )
  FOREACH(FIL ${PROTO_FILES})
    GET_FILENAME_COMPONENT(FIL_WE ${FIL} NAME_WE)
    string(REGEX REPLACE ".+/(.+)\\..*" "\\1" FILE_NAME ${FIL})
    string(REGEX REPLACE "(.+)\\${FILE_NAME}.*" "\\1" FILE_PATH ${FIL})
    EXECUTE_PROCESS(
      COMMAND ${PROTOBUF_PROTOC_EXECUTABLE} -I${FILE_PATH} --cpp_out=${FILE_PATH} --python_out=${FILE_PATH} ${FIL}
    )
  ENDFOREACH()
  add_subdirectory(simple-pb)
endif(${Protobuf_FOUND})

add_subdirectory(simple)
add_subdirectory(callback)
add_subdirectory(websocket)
