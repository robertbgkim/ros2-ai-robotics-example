# ROS2 AI Robotics 예제

「ROS2 피지컬 AI」의 장별 실습 코드 저장소입니다. ROS2 노드와 시스템 코드는 C++(rclcpp)을 우선하며, 학습 파이프라인에서만 파이썬을 사용합니다.

## 기준 환경

- Ubuntu 26.04 LTS
- ROS 2 Lyrical Luth
- 자체 소스 최소 기준 C++17(확장 문법 비활성화)
- ROS 2 Lyrical의 현재 rclcpp 전이 요구에 따라 ROS 대상의 실제 컴파일은 C++20
- colcon과 ament_cmake

## 워크스페이스 구성

```bash
mkdir -p ~/ros2_ws/src
cd ~/ros2_ws/src
git clone https://github.com/makepluscode/ros2-ai-robotics-example

cd ~/ros2_ws
source /opt/ros/lyrical/setup.bash
rosdep install --from-paths src --ignore-src --rosdistro lyrical -r -y
colcon build --symlink-install \
  --cmake-args -DROS2_EXAMPLE_WARNINGS_AS_ERRORS=ON
source install/setup.bash
```

## 에이전트·격리 작업 트리 검증

Codex나 Claude Code가 Windows 작업 트리를 WSL에서 빌드할 때는 소스 안에 `build/`,
`install/`, `log/`를 만들지 않습니다. `/mnt/<drive>/...`의 소스는 `--base-paths`로
지정하고 결과는 WSL의 별도 디렉터리에 둡니다.

```bash
SOURCE=/mnt/e/ros2-ai-robotics-book/code
BUILD_ROOT=~/ros2_agent_ws/example-quality

mkdir -p "$BUILD_ROOT"
cd "$BUILD_ROOT"
source /opt/ros/lyrical/setup.bash
export AMENT_CPPCHECK_ALLOW_SLOW_VERSIONS=1

colcon build --base-paths "$SOURCE" --symlink-install \
  --cmake-args \
    -DROS2_EXAMPLE_WARNINGS_AS_ERRORS=ON \
    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
source install/setup.bash
colcon test --base-paths "$SOURCE" --event-handlers console_direct+
colcon test-result --verbose
```

자체 소스는 C++17 문법 범위를 최소 기준으로 유지하고
`-Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wsign-conversion`을 사용합니다.
Lyrical의 현재 rclcpp는 C++20 전이 요구를 내보내므로 ROS 대상의 실제 컴파일
명령은 C++20으로 상향될 수 있습니다. CI는 자체 대상의 경고를 오류로 처리하고
clang-tidy, copyright, cppcheck, cpplint, uncrustify, CMake/XML/Python 린트와 기능
테스트를 모두 실행합니다.

## 디렉터리 이름 규칙

장별 폴더는 `chNN/`, 장 안의 예제는 `NN-패키지이름` 형식입니다. 번호는 원고에 나오는 순서를 나타내며 패키지 이름 자체에는 들어가지 않습니다. colcon은 `package.xml`로 패키지를 찾으므로 빌드·실행 명령에는 번호 없는 이름을 씁니다. 자세한 규칙은 `AGENT.md`를 참고합니다.

## 3장 예제 실행

```bash
cd ~/ros2_ws
colcon build --symlink-install --packages-select hello_ros2
source install/setup.bash

ros2 run hello_ros2 timer_node
ros2 run hello_ros2 blocking_callback_node single
ros2 run hello_ros2 blocking_callback_node multi_same_group
ros2 run hello_ros2 blocking_callback_node multi_separate_groups
```

## 4장 예제 실행

```bash
cd ~/ros2_ws
colcon build --symlink-install --packages-select robot_interfaces sensor_nodes
source install/setup.bash

ros2 run sensor_nodes camera_node best_effort
ros2 run sensor_nodes joint_state_relay
```

## 5장 예제 실행

```bash
cd ~/ros2_ws
colcon build --symlink-install --packages-select ops_demo
source install/setup.bash

ros2 launch ops_demo ops_demo.launch.py
ros2 launch ops_demo ops_demo.launch.py log_level:=debug
```

## 6장 예제 실행

```bash
cd ~/ros2_ws
colcon build --symlink-install --packages-select comm_tests dds_benchmark
source install/setup.bash

colcon test --packages-select comm_tests
colcon test-result --test-result-base build/comm_tests --verbose

RMW_IMPLEMENTATION=rmw_cyclonedds_cpp ros2 run dds_benchmark latency_pong &
RMW_IMPLEMENTATION=rmw_cyclonedds_cpp ros2 run dds_benchmark latency_ping \
  --ros-args -p sample_count:=2000
```

## 7장 예제 실행

`ur_description`이 필요합니다.

```bash
sudo apt install -y \
  ros-lyrical-xacro \
  ros-lyrical-ur-description \
  ros-lyrical-joint-state-publisher \
  ros-lyrical-joint-state-publisher-gui \
  ros-lyrical-tf2-tools

cd ~/ros2_ws
colcon build --symlink-install --packages-select ur5e_description
source install/setup.bash

ros2 launch ur5e_description display.launch.py
ros2 launch ur5e_description display.launch.py joint_state_source:=cycler rviz:=false
```

WSLg에서 RViz가 `Invalid parentWindowHandle`로 종료되면 소프트웨어 렌더링으로 실행합니다.

```bash
QT_QPA_PLATFORM=xcb LIBGL_ALWAYS_SOFTWARE=1 \
  ros2 launch ur5e_description display.launch.py
```

## 장별 구성

| 장 | 디렉터리 | 내용 | 상태 |
|---|---|---|---|
| 3장 | `ch03/01-hello_ros2` | ROS2 개발 환경과 노드·실행 모델 | 구현 |
| 4장 | `ch04/01-robot_interfaces`, `ch04/02-sensor_nodes` | 통신과 인터페이스 설계 | 구현 |
| 5장 | `ch05/01-ops_demo` | 시스템 운영 도구 | 구현 |
| 6장 | `ch06/01-comm_tests`, `ch06/02-dds_benchmark` | 테스트·미들웨어 튜닝·보안 | 구현 |
| 7장 | `ch07/01-ur5e_description` | URDF·TF와 UR5e 모델링 | 구현 |
| 8장 | `ch08/` | ros2_control과 C++ 하드웨어 인터페이스 | 예정 |
| 9장 | `ch09/` | 실시간 제어와 안전 계층 | 예정 |
| 10장 | `ch10/` | MuJoCo 시뮬레이션 환경 구축 | 예정 |
| 11장 | `ch11/` | MoveIt 2와 모션 플래닝 | 예정 |
| 12장 | — | 계획 기반 제어의 한계 | 코드 없음 |
| 13장 | `ch13/` | 피지컬 AI(VLA)와 데이터 수집 | 예정 |
| 14장 | `ch14/` | SmolVLA 파인튜닝 | 예정 |
| 15장 | `ch15/` | ONNX 변환과 C++ 추론 노드 | 예정 |
| 16장 | `ch16/` | 시뮬레이션 평가와 책 마무리 | 예정 |

## 라이선스

저장소 전체는 `LICENSE`의 Apache License 2.0을 따르며, 각 ROS2 패키지의
`package.xml`에도 같은 라이선스를 선언합니다.
