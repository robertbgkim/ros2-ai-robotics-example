cmake_minimum_required(VERSION 3.10)
project(ops_demo)

if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 17)
endif()

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(sensor_msgs REQUIRED)
find_package(diagnostic_msgs REQUIRED)
find_package(diagnostic_updater REQUIRED)

add_executable(sensor_sim src/sensor_sim.cpp)
target_link_libraries(sensor_sim PRIVATE
  rclcpp::rclcpp
  ${sensor_msgs_TARGETS})

add_executable(temperature_monitor src/temperature_monitor.cpp)
target_link_libraries(temperature_monitor PRIVATE
  rclcpp::rclcpp
  ${sensor_msgs_TARGETS}
  ${diagnostic_msgs_TARGETS}
  diagnostic_updater::diagnostic_updater)

install(TARGETS
  sensor_sim
  temperature_monitor
  DESTINATION lib/${PROJECT_NAME})

install(DIRECTORY
  launch
  config
  DESTINATION share/${PROJECT_NAME})

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  ament_lint_auto_find_test_dependencies()
endif()

ament_package()
