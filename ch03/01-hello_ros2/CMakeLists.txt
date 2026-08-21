cmake_minimum_required(VERSION 3.16)
project(hello_ros2)

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

add_executable(timer_node src/timer_node.cpp)
target_compile_options(timer_node PRIVATE ${PROJECT_WARNING_FLAGS})
target_link_libraries(timer_node PRIVATE rclcpp::rclcpp)

add_executable(blocking_callback_node src/blocking_callback_node.cpp)
target_compile_options(blocking_callback_node PRIVATE ${PROJECT_WARNING_FLAGS})
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
