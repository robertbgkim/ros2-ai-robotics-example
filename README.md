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

## 3장 예제 실행

현재 구현된 예제는 `ch03/hello_ros2`입니다.

```bash
cd ~/ros2_ws
colcon build --symlink-install --packages-select hello_ros2
source install/setup.bash

ros2 run hello_ros2 timer_node
ros2 run hello_ros2 blocking_callback_node single
ros2 run hello_ros2 blocking_callback_node multi_same_group
ros2 run hello_ros2 blocking_callback_node multi_separate_groups
```

## 장별 구성

| 장 | 디렉터리 | 내용 | 상태 |
|---|---|---|---|
| 3장 | `ch03/` | ROS2 개발 환경과 노드·실행 모델 | 구현 |
| 4장 | `ch04/` | 통신과 인터페이스 설계 | 예정 |
| 5장 | `ch05/` | 시스템 운영 도구 | 예정 |
| 6장 | `ch06/` | 테스트·미들웨어 튜닝·보안 | 예정 |
| 7장 | `ch07/` | URDF·TF와 UR5e 모델링 | 예정 |
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
