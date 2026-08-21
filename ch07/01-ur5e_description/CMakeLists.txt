cmake_minimum_required(VERSION 3.16)
project(ur5e_description)

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(sensor_msgs REQUIRED)

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

add_executable(pose_cycler src/pose_cycler.cpp)
target_compile_options(pose_cycler PRIVATE ${PROJECT_WARNING_FLAGS})
target_link_libraries(pose_cycler PRIVATE
  rclcpp::rclcpp
  ${sensor_msgs_TARGETS})

install(TARGETS
  pose_cycler
  DESTINATION lib/${PROJECT_NAME})

install(DIRECTORY
  urdf
  launch
  rviz
  DESTINATION share/${PROJECT_NAME})

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  ament_lint_auto_find_test_dependencies()
endif()

ament_package()
