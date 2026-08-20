cmake_minimum_required(VERSION 3.10)
project(comm_tests)

if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 17)
endif()

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(std_msgs REQUIRED)

add_executable(echo_node src/echo_node.cpp)
target_include_directories(echo_node PRIVATE include)
target_link_libraries(echo_node PRIVATE
  rclcpp::rclcpp
  ${std_msgs_TARGETS})

install(TARGETS
  echo_node
  DESTINATION lib/${PROJECT_NAME})

install(DIRECTORY include/
  DESTINATION include)

if(BUILD_TESTING)
  # SPDX 한 줄 표기를 쓰므로 라이선스 전문을 요구하는 copyright 린터는 제외
  set(ament_cmake_copyright_FOUND TRUE)

  find_package(ament_lint_auto REQUIRED)
  ament_lint_auto_find_test_dependencies()

  # 1. gtest 기반 단위 테스트 등록
  find_package(ament_cmake_gtest REQUIRED)
  ament_add_gtest(test_temperature_grade test/test_temperature_grade.cpp)
  target_include_directories(test_temperature_grade PRIVATE include)

  # 2. launch_testing 기반 통합 테스트 등록
  find_package(launch_testing_ament_cmake REQUIRED)
  add_launch_test(test/test_echo_launch.py)
endif()

ament_package()
