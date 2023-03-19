cmake_minimum_required (VERSION 3.3.0)

project(cyclone_callback)

add_executable(${PROJECT_NAME}_runner main.cc)

target_link_libraries(${PROJECT_NAME}_runner
  ${TARGET_NAME}
)
