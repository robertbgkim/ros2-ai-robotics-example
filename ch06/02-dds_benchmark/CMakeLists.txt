cmake_minimum_required(VERSION 3.16)
project(dds_benchmark)

if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 17)
endif()
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(MSVC)
  set(PROJECT_WARNING_FLAGS /W4)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
  set(PROJECT_WARNING_FLAGS
    -Wall
    -Wextra
    -Wpedantic
    -Wshadow
    -Wconversion
    -Wsign-conversion)
endif()

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(rmw REQUIRED)
find_package(std_msgs REQUIRED)

add_executable(latency_ping src/latency_ping.cpp)
target_compile_options(latency_ping PRIVATE ${PROJECT_WARNING_FLAGS})
target_link_libraries(latency_ping PRIVATE
  rclcpp::rclcpp
  rmw::rmw
  ${std_msgs_TARGETS})

add_executable(latency_pong src/latency_pong.cpp)
target_compile_options(latency_pong PRIVATE ${PROJECT_WARNING_FLAGS})
target_link_libraries(latency_pong PRIVATE
  rclcpp::rclcpp
  rmw::rmw
  ${std_msgs_TARGETS})

install(TARGETS
  latency_ping
  latency_pong
  DESTINATION lib/${PROJECT_NAME})

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  ament_lint_auto_find_test_dependencies()
endif()

ament_package()
