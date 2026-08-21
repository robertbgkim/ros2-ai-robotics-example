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

#include "rclcpp/rclcpp.hpp"
#include "sensor_msgs/msg/temperature.hpp"

// 파라미터로 발행 주기와 기준 온도를 받아 온도 값을 발행하는 모의 센서 노드
class SensorSim : public rclcpp::Node
{
public:
  SensorSim()
  : Node("sensor_sim")
  {
    // 1. 파라미터 선언과 기본값 지정
    this->declare_parameter<double>("publish_rate_hz", 5.0);
    this->declare_parameter<double>("base_temperature", 40.0);
    this->declare_parameter<std::string>("frame_id", "motor_case");

    // 2. 선언한 파라미터 읽기
    const double rate_hz = this->get_parameter("publish_rate_hz").as_double();
    base_temperature_ = this->get_parameter("base_temperature").as_double();
    frame_id_ = this->get_parameter("frame_id").as_string();

    // 3. 주기와 메시지 메타데이터 파라미터 검증
    if (!std::isfinite(rate_hz) || rate_hz <= 0.0) {
      throw std::invalid_argument("publish_rate_hz는 0보다 큰 유한한 값이어야 합니다");
    }
    if (!std::isfinite(base_temperature_)) {
      throw std::invalid_argument("base_temperature는 유한한 값이어야 합니다");
    }
    if (frame_id_.empty()) {
      throw std::invalid_argument("frame_id는 비어 있지 않아야 합니다");
    }

    // 4. 파라미터로 받은 주기를 나노초 단위로 환산
    const auto period = std::chrono::duration_cast<std::chrono::nanoseconds>(
      std::chrono::duration<double>(1.0 / rate_hz));

    // 5. 환산 결과가 타이머 주기로 유효한지 확인
    if (period <= std::chrono::nanoseconds::zero()) {
      throw std::invalid_argument("publish_rate_hz가 너무 커서 주기가 0이 됩니다");
    }

    // 6. 검증이 끝난 뒤 발행자와 타이머 생성
    publisher_ = this->create_publisher<sensor_msgs::msg::Temperature>("/motor_temperature", 10);
    timer_ = this->create_wall_timer(period, [this]() {on_timer();});

    RCLCPP_INFO(
      this->get_logger(), "모의 센서 시작 (주기=%.1fHz, 기준 온도=%.1f도, 프레임=%s)",
      rate_hz, base_temperature_, frame_id_.c_str());
  }

private:
  void on_timer()
  {
    // 1. 기준 온도에서 서서히 오르는 값 계산
    const double temperature = base_temperature_ + (static_cast<double>(tick_) * 0.5);

    // 2. 온도 메시지 구성
    sensor_msgs::msg::Temperature msg;
    msg.header.stamp = this->now();
    msg.header.frame_id = frame_id_;
    msg.temperature = temperature;
    msg.variance = 0.0;

    // 3. 발행 후 순번 누적
    publisher_->publish(msg);
    ++tick_;

    // 4. 매 건은 디버그 레벨 로그로만 기록
    RCLCPP_DEBUG(this->get_logger(), "온도 %.1f도 발행", temperature);
  }

  rclcpp::Publisher<sensor_msgs::msg::Temperature>::SharedPtr publisher_;
  rclcpp::TimerBase::SharedPtr timer_;
  double base_temperature_{0.0};
  std::string frame_id_;
  std::size_t tick_{0};
};

int main(int argc, char * argv[])
{
  // 1. rclcpp 컨텍스트 초기화
  rclcpp::init(argc, argv);

  int exit_code = 0;
  try {
    // 2. 모의 센서 노드 실행
    rclcpp::spin(std::make_shared<SensorSim>());
  } catch (const std::exception & error) {
    RCLCPP_FATAL(rclcpp::get_logger("sensor_sim"), "노드 실행 실패: %s", error.what());
    exit_code = 1;
  }

  // 3. 컨텍스트 정리
  rclcpp::shutdown();
  return exit_code;
}
