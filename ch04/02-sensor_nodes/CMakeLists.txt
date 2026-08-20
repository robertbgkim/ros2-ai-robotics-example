cmake_minimum_required(VERSION 3.10)
project(sensor_nodes)

if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 17)
endif()

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(sensor_msgs REQUIRED)
find_package(robot_interfaces REQUIRED)

add_executable(camera_node src/camera_node.cpp)
target_link_libraries(camera_node PRIVATE
  rclcpp::rclcpp
  ${sensor_msgs_TARGETS})

add_executable(joint_state_relay src/joint_state_relay.cpp)
target_link_libraries(joint_state_relay PRIVATE
  rclcpp::rclcpp
  ${sensor_msgs_TARGETS}
  ${robot_interfaces_TARGETS})

install(TARGETS
  camera_node
  joint_state_relay
  DESTINATION lib/${PROJECT_NAME})

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  ament_lint_auto_find_test_dependencies()
endif()

ament_package()
