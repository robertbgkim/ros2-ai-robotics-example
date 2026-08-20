cmake_minimum_required(VERSION 3.10)
project(hello_ros2)

if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 17)
endif()

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)

add_executable(timer_node src/timer_node.cpp)
target_link_libraries(timer_node PRIVATE rclcpp::rclcpp)

add_executable(blocking_callback_node src/blocking_callback_node.cpp)
target_link_libraries(blocking_callback_node PRIVATE rclcpp::rclcpp)

install(TARGETS
  timer_node
  blocking_callback_node
  DESTINATION lib/${PROJECT_NAME})

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  ament_lint_auto_find_test_dependencies()
endif()

ament_package()
