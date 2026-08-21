cmake_minimum_required(VERSION 3.16)
project(sensor_nodes)

if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 17)
endif()
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
option(ROS2_EXAMPLE_WARNINGS_AS_ERRORS "Treat project warnings as errors" OFF)

if(MSVC)
  set(PROJECT_WARNING_FLAGS /W4)
  if(ROS2_EXAMPLE_WARNINGS_AS_ERRORS)
    list(APPEND PROJECT_WARNING_FLAGS /WX)
  endif()
elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
  set(PROJECT_WARNING_FLAGS
    -Wall
    -Wextra
    -Wpedantic
    -Wshadow
    -Wconversion
    -Wsign-conversion)
  if(ROS2_EXAMPLE_WARNINGS_AS_ERRORS)
    list(APPEND PROJECT_WARNING_FLAGS -Werror)
  endif()
endif()

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(sensor_msgs REQUIRED)
find_package(robot_interfaces REQUIRED)

add_executable(camera_node src/camera_node.cpp)
target_compile_options(camera_node PRIVATE ${PROJECT_WARNING_FLAGS})
target_link_libraries(camera_node PRIVATE
  rclcpp::rclcpp
  ${sensor_msgs_TARGETS})

add_executable(joint_state_relay src/joint_state_relay.cpp)
target_compile_options(joint_state_relay PRIVATE ${PROJECT_WARNING_FLAGS})
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
