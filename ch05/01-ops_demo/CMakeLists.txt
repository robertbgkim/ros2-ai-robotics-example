cmake_minimum_required(VERSION 3.16)
project(ops_demo)

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
find_package(sensor_msgs REQUIRED)
find_package(diagnostic_msgs REQUIRED)
find_package(diagnostic_updater REQUIRED)

add_executable(sensor_sim src/sensor_sim.cpp)
target_compile_options(sensor_sim PRIVATE ${PROJECT_WARNING_FLAGS})
target_link_libraries(sensor_sim PRIVATE
  rclcpp::rclcpp
  ${sensor_msgs_TARGETS})

add_executable(temperature_monitor src/temperature_monitor.cpp)
target_include_directories(temperature_monitor PRIVATE include)
target_compile_options(temperature_monitor PRIVATE ${PROJECT_WARNING_FLAGS})
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

install(DIRECTORY include/
  DESTINATION include)

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  ament_lint_auto_find_test_dependencies()

  # 온도 판정 순수 함수 단위 테스트 — temperature_monitor가 쓰는 바로 그 함수
  find_package(ament_cmake_gtest REQUIRED)
  ament_add_gtest(test_temperature_grade test/test_temperature_grade.cpp)
  target_include_directories(test_temperature_grade PRIVATE include)
  target_compile_options(test_temperature_grade PRIVATE ${PROJECT_WARNING_FLAGS})
endif()

ament_package()
