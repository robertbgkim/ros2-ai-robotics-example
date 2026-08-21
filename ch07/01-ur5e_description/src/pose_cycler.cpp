// Copyright 2026 makepluscode
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <chrono>
#include <cmath>
#include <cstddef>
#include <exception>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "rclcpp/rclcpp.hpp"
#include "sensor_msgs/msg/joint_state.hpp"

using namespace std::chrono_literals;

// 미리 정한 관절 자세를 번갈아 발행해 TF가 자세를 따라 바뀌는지 확인하는 노드
// joint_state_publisher_gui 없이도 같은 결과를 재현할 수 있게 한다
class PoseCycler : public rclcpp::Node
{
public:
  PoseCycler()
  : Node("pose_cycler")
  {
    // 1. 자세 유지 시간 파라미터 선언과 범위 검사
    this->declare_parameter<double>("hold_seconds", 3.0);
    const double hold = this->get_parameter("hold_seconds").as_double();
    if (!std::isfinite(hold) || hold <= 0.0) {
      throw std::invalid_argument("hold_seconds는 0보다 큰 유한한 값이어야 합니다");
    }

    // 2. 유지 시간을 타이머 주기로 변환하고 0으로 잘리는 값 거부
    const auto hold_period = std::chrono::duration_cast<std::chrono::nanoseconds>(
      std::chrono::duration<double>(hold));
    if (hold_period <= std::chrono::nanoseconds::zero()) {
      throw std::invalid_argument("hold_seconds가 너무 작아서 주기가 0이 됩니다");
    }

    // 3. 관절 상태 발행자 생성
    publisher_ = this->create_publisher<sensor_msgs::msg::JointState>("/joint_states", 10);

    // 4. 50ms마다 현재 자세를 되풀이 발행
    publish_timer_ = this->create_wall_timer(50ms, [this]() {on_publish();});

    // 5. 정해진 시간마다 다음 자세로 넘어감
    switch_timer_ = this->create_wall_timer(hold_period, [this]() {on_switch();});

    RCLCPP_INFO(this->get_logger(), "자세 순환 시작 (자세 %zu개)", poses_.size());
  }

private:
  using Pose = std::pair<std::string, std::vector<double>>;

  void on_publish()
  {
    // 1. 현재 시각을 헤더에 담아 발행
    //    stamp가 0이면 기본 설정에서 이동 조인트 TF가 발행되지 않음
    sensor_msgs::msg::JointState msg;
    msg.header.stamp = this->now();
    msg.name = joint_names_;
    msg.position = poses_[index_].second;

    publisher_->publish(msg);
  }

  void on_switch()
  {
    // 1. 다음 자세로 넘기고 이름을 기록
    index_ = (index_ + 1) % poses_.size();

    RCLCPP_INFO(this->get_logger(), "자세 전환: %s", poses_[index_].first.c_str());
  }

  rclcpp::Publisher<sensor_msgs::msg::JointState>::SharedPtr publisher_;
  rclcpp::TimerBase::SharedPtr publish_timer_;
  rclcpp::TimerBase::SharedPtr switch_timer_;
  const std::vector<std::string> joint_names_{
    "shoulder_pan_joint", "shoulder_lift_joint", "elbow_joint",
    "wrist_1_joint", "wrist_2_joint", "wrist_3_joint"};
  const std::vector<Pose> poses_{
    {"영점", {0.0, 0.0, 0.0, 0.0, 0.0, 0.0}},
    {"접힌 자세", {0.0, -1.5708, 1.5708, -1.5708, -1.5708, 0.0}},
    {"베이스 90도 회전", {1.5708, -1.5708, 1.5708, -1.5708, -1.5708, 0.0}}};
  std::size_t index_{0};
};

int main(int argc, char * argv[])
{
  // 1. rclcpp 컨텍스트 초기화
  rclcpp::init(argc, argv);

  int exit_code = 0;
  try {
    // 2. 자세 순환 노드 실행
    rclcpp::spin(std::make_shared<PoseCycler>());
  } catch (const std::exception & error) {
    RCLCPP_FATAL(rclcpp::get_logger("pose_cycler"), "노드 실행 실패: %s", error.what());
    exit_code = 1;
  }

  // 3. 컨텍스트 정리
  rclcpp::shutdown();
  return exit_code;
}
