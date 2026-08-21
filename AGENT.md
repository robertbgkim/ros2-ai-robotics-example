# 주석 규칙

이 저장소의 C++/rclcpp 예제에 적용하는 주석 규칙입니다.

## 1. 기본 언어
- 모든 코드 주석은 한글 사용
- Apache-2.0 표준 라이선스 헤더의 영문 법률 문구는 예외
- 라이선스 헤더는 설명 주석이나 번호 주석으로 세지 않음

## 2. 문장 종결
- 주석은 서술어 없이 명사형 또는 구 형태로 작성
- 예시
  - 잘못된 예: `// 500ms 마다 콜백을 실행한다`
  - 올바른 예: `// 500ms 마다 콜백 실행`

## 3. 번호 주석 형식
- 단계성 주석은 번호 형식 사용
- 기본 형식: `// 1. 첫번째 ...`
- 최대 번호: `// 9. 아홉번째 ...`

## 4. 하위 번호 형식
- 필요한 경우 하위 번호 사용 가능
- 형식: `// 3-1. 세번째의 첫번째 ...`
- 형식: `// 3-2. 세번째의 두번째 ...`

## 5. 개수 제한
- 한 흐름(함수 하나) 안의 최상위 번호 주석은 10개를 넘기지 않음
- 10개를 넘길 것 같으면 단계 통합 또는 하위 번호로 조정
- 번호는 함수마다 1부터 새로 시작(생성자·콜백·main이 각각 독립된 흐름)

## 6. 적용 대상
- main() 같은 흐름 중심 예제 코드에 우선 적용
- 생성자·콜백처럼 복잡한 로직 설명이 필요한 함수에도 동일 규칙 적용

## 7. 작성 기준
- 동작 순서, 의도, 구간 구분 위주
- 코드만 보면 자명한 설명은 생략

## 8. 톤
- 짧고 단정한 표현
- 실험 기록, 감상, 장황한 배경 설명 제외

## 9. 디렉터리·패키지 네이밍

- 장별 디렉터리는 `chNN/` 형식
- 장 안의 예제는 `NN-패키지이름` 형식으로 번호를 붙여 순서를 드러냄
- 번호는 원고에 등장하는 순서를 따르고 01부터 시작
- 패키지 이름 자체(`package.xml`의 `<name>`)에는 번호를 넣지 않음 — 번호는 디렉터리에만 붙임
- 디렉터리 번호와 패키지 이름은 하이픈 하나로 구분하고, 패키지 이름은 스네이크 케이스 유지
- 예시

```text
code/
├── ch03/
│   └── 01-hello_ros2/        ← 패키지 이름은 hello_ros2
└── ch04/
    ├── 01-robot_interfaces/  ← 패키지 이름은 robot_interfaces
    └── 02-sensor_nodes/      ← 패키지 이름은 sensor_nodes
```

- `colcon`은 디렉터리 이름이 아니라 `package.xml`로 패키지를 찾으므로 `ros2 run`·`--packages-select`에는 번호 없는 패키지 이름을 씀

## 10. Git 커밋
- 커밋 제목 형식: `[예제] 한국어 명사형 본문`
- 서로 다른 장의 변경은 한 커밋에 혼합하지 않음
- `build/`, `install/`, `log/`와 자격 증명 파일은 커밋하지 않음

## 11. C++ 품질 기준

- 모든 자체 C++ 대상은 C++17 필수, 컴파일러 확장 문법 비활성화
- GCC/Clang 경고: `-Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wsign-conversion`
- 최종 검증과 CI에서는 `CMAKE_COMPILE_WARNING_AS_ERROR=ON`으로 경고를 오류 처리
- 소유하지 않은 ROS 생성 코드나 외부 헤더에는 저장소 경고 옵션을 강제로 적용하지 않음
- 읽기 전용 메시지 콜백은 `ConstSharedPtr`, 짧은 콜백 연결은 람다 우선
- 파라미터는 리소스 생성 전에 유한성·범위·관계 검증
- 생성자와 실행 중 예외는 `main()`에서 기록하고 0이 아닌 종료 코드 반환
- 표준 라이선스 헤더를 유지하고 copyright 린터 우회 금지

## 12. WSL 에이전트 빌드·테스트

- Windows 작업 트리는 `/mnt/<drive>/...` 경로로 읽고, 빌드 산출물은 WSL의
  `~/ros2_agent_ws/<작업명>` 아래에 격리
- 기존 `~/ros2_ws/src/ros2-ai-robotics-example` 복제본을 수정·빌드 대상으로 사용하지 않음
- `colcon` 장별 검증에도 반드시 `--base-paths "$SOURCE"` 지정
- `cppcheck` 2.19가 생략되지 않도록 테스트 전에
  `AMENT_CPPCHECK_ALLOW_SLOW_VERSIONS=1` 설정
- 최소 최종 게이트:

```bash
source /opt/ros/lyrical/setup.bash
export AMENT_CPPCHECK_ALLOW_SLOW_VERSIONS=1

colcon build --base-paths "$SOURCE" --symlink-install \
  --cmake-args \
    -DCMAKE_COMPILE_WARNING_AS_ERROR=ON \
    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
source install/setup.bash
colcon test --base-paths "$SOURCE" --event-handlers console_direct+
colcon test-result --verbose
```

- 런타임 예제는 별도 `ROS_DOMAIN_ID`를 사용해 남아 있는 DDS 참가자와 격리
- GUI 검증은 WSLg에서 직접 실행하며, RViz 창 핸들 오류 때만
  `QT_QPA_PLATFORM=xcb LIBGL_ALWAYS_SOFTWARE=1` 폴백 사용
