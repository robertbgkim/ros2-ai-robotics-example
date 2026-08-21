# CI 설정

실제 GitHub Actions 워크플로는 `.github/workflows/ci.yml`입니다. 모든 PR과 `main`
푸시에서 ROS 2 Lyrical 의존성을 설치하고, 자체 대상 엄격 경고 빌드, clang-tidy 정적 분석,
전체 ament 린트·테스트를 실행합니다. 자체 소스의 최소 기준은 C++17이지만 현재 rclcpp의
전이 요구에 따라 ROS 대상의 실제 컴파일은 C++20으로 상향될 수 있습니다.
`cppcheck` 2.19도 생략하지 않도록
`AMENT_CPPCHECK_ALLOW_SLOW_VERSIONS=1`을 명시합니다.
