# ROS2 AI Robotics 예제

「ROS2 피지컬 AI」의 장별 실습 코드 저장소입니다. ROS2 노드와 시스템 코드는 C++(rclcpp)을 우선하며, 학습 파이프라인에서만 파이썬을 사용합니다.

## 기준 환경

- Ubuntu 26.04 LTS
- ROS 2 Lyrical Luth
- C++17 이상
- colcon과 ament_cmake

## 워크스페이스 구성

```bash
mkdir -p ~/ros2_ws/src
cd ~/ros2_ws/src
git clone https://github.com/makepluscode/ros2-ai-robotics-example

cd ~/ros2_ws
source /opt/ros/lyrical/setup.bash
rosdep install --from-paths src --ignore-src --rosdistro lyrical -r -y
colcon build --symlink-install
source install/setup.bash
```

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

ros2 run dds_benchmark latency_pong &
ros2 run dds_benchmark latency_ping --ros-args -p sample_count:=2000
```

## 7장 예제 실행

`ur_description`이 필요합니다.

```bash
sudo apt install -y ros-lyrical-xacro ros-lyrical-ur-description   ros-lyrical-joint-state-publisher ros-lyrical-joint-state-publisher-gui   ros-lyrical-tf2-tools

cd ~/ros2_ws
colcon build --symlink-install --packages-select ur5e_description
source install/setup.bash

ros2 launch ur5e_description display.launch.py
ros2 launch ur5e_description display.launch.py joint_state_source:=cycler rviz:=false
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

각 ROS2 패키지의 `package.xml`에 선언된 라이선스를 따릅니다.
