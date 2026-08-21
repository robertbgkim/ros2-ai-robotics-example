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
#include <cstddef>
#include <cstdint>
#include <exception>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

#include "rclcpp/rclcpp.hpp"
#include "sensor_msgs/msg/image.hpp"

using namespace std::chrono_literals;

// 카메라 프레임을 약 30Hz로 발행하는 노드
// 실행 인자로 QoS 신뢰성 정책을 골라 매칭 성패를 비교
class CameraNode : public rclcpp::Node
{
public:
  explicit CameraNode(const std::string & reliability)
  : Node("camera_node")
  {
    // 1. 센서 스트림 기본값인 best_effort QoS 구성
    rclcpp::QoS qos = rclcpp::SensorDataQoS();

    // 2. 허용하지 않는 인자는 거부
    if (reliability != "best_effort" && reliability != "reliable") {
      throw std::invalid_argument("신뢰성 인자는 best_effort 또는 reliable이어야 합니다");
    }

    // 3. 인자가 reliable이면 신뢰성만 교체
    if (reliability == "reliable") {
      qos.reliable();
    }

    // 4. 선택한 정책으로 발행자 생성
    publisher_ = this->create_publisher<sensor_msgs::msg::Image>("/camera/image_raw", qos);

    // 5. 약 30Hz에 해당하는 33ms 주기 타이머 등록
    timer_ = this->create_wall_timer(33ms, [this]() {on_timer();});

    RCLCPP_INFO(
        this->get_logger(), "카메라 노드 시작 (신뢰성=%s, 깊이=5)", reliability.c_str());
  }

private:
  void on_timer()
  {
    // 1. 최소 크기 영상 메시지 구성
    sensor_msgs::msg::Image msg;
    msg.header.stamp = this->now();
    msg.header.frame_id = "camera_optical_frame";
    msg.height = 4;
    msg.width = 4;
    msg.encoding = "mono8";
    msg.step = msg.width;
    const std::size_t pixel_count =
      static_cast<std::size_t>(msg.height) * static_cast<std::size_t>(msg.step);
    msg.data.assign(pixel_count, static_cast<uint8_t>(frame_count_ % 256U));

    // 2. 발행 후 프레임 번호 누적
    publisher_->publish(msg);
    ++frame_count_;

    // 3. 30프레임마다 한 번만 진행 상황 출력
    if (frame_count_ % 30U == 0U) {
      RCLCPP_INFO(this->get_logger(), "프레임 %zu 발행", frame_count_);
    }
  }

  rclcpp::Publisher<sensor_msgs::msg::Image>::SharedPtr publisher_;
  rclcpp::TimerBase::SharedPtr timer_;
  std::size_t frame_count_{0};
};

int main(int argc, char * argv[])
{
  // 1. rclcpp 컨텍스트 초기화
  rclcpp::init(argc, argv);

  // 2. 실행 인자에서 신뢰성 정책 결정(기본 best_effort)
  std::vector<std::string> args = rclcpp::remove_ros_arguments(argc, argv);
  const std::string reliability = (args.size() > 1) ? args[1] : "best_effort";

  int exit_code = 0;
  try {
    // 3. 노드 실행
    rclcpp::spin(std::make_shared<CameraNode>(reliability));
  } catch (const std::exception & error) {
    RCLCPP_FATAL(
      rclcpp::get_logger("camera_node"), "노드 실행 실패: %s", error.what());
    exit_code = 1;
  }

  // 4. 컨텍스트 정리
  rclcpp::shutdown();
  return exit_code;
}
