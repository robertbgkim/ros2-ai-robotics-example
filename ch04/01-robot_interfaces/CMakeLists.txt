cmake_minimum_required(VERSION 3.10)
project(robot_interfaces)

find_package(ament_cmake REQUIRED)
find_package(rosidl_default_generators REQUIRED)

rosidl_generate_interfaces(${PROJECT_NAME}
  "msg/JointCommand.msg"
  "srv/SetGripper.srv"
  "action/MoveJoint.action")

if(BUILD_TESTING)
  # SPDX 한 줄 표기를 쓰므로 라이선스 전문을 요구하는 copyright 린터는 제외
  set(ament_cmake_copyright_FOUND TRUE)

  find_package(ament_lint_auto REQUIRED)
  ament_lint_auto_find_test_dependencies()
endif()

ament_package()
