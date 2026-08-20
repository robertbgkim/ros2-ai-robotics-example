cmake_minimum_required(VERSION 3.10)
project(dds_benchmark)

if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 17)
endif()

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(rmw REQUIRED)
find_package(std_msgs REQUIRED)

add_executable(latency_ping src/latency_ping.cpp)
target_link_libraries(latency_ping PRIVATE
  rclcpp::rclcpp
  rmw::rmw
  ${std_msgs_TARGETS})

add_executable(latency_pong src/latency_pong.cpp)
target_link_libraries(latency_pong PRIVATE
  rclcpp::rclcpp
  ${std_msgs_TARGETS})

install(TARGETS
  latency_ping
  latency_pong
  DESTINATION lib/${PROJECT_NAME})

if(BUILD_TESTING)
  # SPDX 한 줄 표기를 쓰므로 라이선스 전문을 요구하는 copyright 린터는 제외
  set(ament_cmake_copyright_FOUND TRUE)

  find_package(ament_lint_auto REQUIRED)
  ament_lint_auto_find_test_dependencies()
endif()

ament_package()
