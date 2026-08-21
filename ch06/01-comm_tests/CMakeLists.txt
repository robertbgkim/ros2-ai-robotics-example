cmake_minimum_required(VERSION 3.16)
project(comm_tests)

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
find_package(std_msgs REQUIRED)

add_executable(echo_node src/echo_node.cpp)
target_compile_options(echo_node PRIVATE ${PROJECT_WARNING_FLAGS})
target_link_libraries(echo_node PRIVATE
  rclcpp::rclcpp
  ${std_msgs_TARGETS})

install(TARGETS
  echo_node
  DESTINATION lib/${PROJECT_NAME})

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  ament_lint_auto_find_test_dependencies()

  # launch_testing 기반 통합 테스트 등록
  find_package(launch_testing_ament_cmake REQUIRED)
  add_launch_test(test/test_echo_launch.py)
endif()

ament_package()
