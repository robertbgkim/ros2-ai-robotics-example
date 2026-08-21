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

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <exception>
#include <memory>
#include <numeric>
#include <stdexcept>
#include <vector>

#include "rclcpp/rclcpp.hpp"
#include "rmw/rmw.h"
#include "std_msgs/msg/u_int64.hpp"

using namespace std::chrono_literals;

// 타임스탬프를 실어 보내고 되돌아온 시각과의 차이로 왕복 지연을 측정하는 노드
// 미들웨어 구현체를 바꿔 가며 같은 조건에서 비교하는 데 사용
class LatencyPing : public rclcpp::Node
{
public:
  LatencyPing()
  : Node("latency_ping")
  {
    // 1. 측정 횟수 파라미터 선언과 범위 검사
    this->declare_parameter<int>("sample_count", 1000);
    const int64_t requested = this->get_parameter("sample_count").as_int();
    if (requested <= 0 || requested > kMaxSampleCount) {
      throw std::invalid_argument("sample_count는 1에서 1000000 사이여야 합니다");
    }
    sample_count_ = static_cast<std::size_t>(requested);
    samples_.reserve(sample_count_);

    // 2. 반환 노드와 같은 QoS 구성
    rclcpp::QoS qos(rclcpp::KeepLast(1));
    qos.best_effort();

    // 3. 요청 발행자와 반환 구독자 생성
    publisher_ = this->create_publisher<std_msgs::msg::UInt64>("/ping", qos);
    subscription_ = this->create_subscription<std_msgs::msg::UInt64>(
      "/pong", qos,
      [this](std_msgs::msg::UInt64::ConstSharedPtr msg) {on_pong(msg);});

    // 4. 1ms 간격으로 한 건씩 발행
    timer_ = this->create_wall_timer(1ms, [this]() {on_timer();});

    RCLCPP_INFO(
      this->get_logger(), "지연 측정 시작 (표본 %zu건, RMW=%s)",
      sample_count_, rmw_get_implementation_identifier());
  }

private:
  static uint64_t now_ns()
  {
    return static_cast<uint64_t>(
      std::chrono::duration_cast<std::chrono::nanoseconds>(
        std::chrono::steady_clock::now().time_since_epoch()).count());
  }

  void on_timer()
  {
    // 1. 현재 시각을 실어 발행
    std_msgs::msg::UInt64 msg;
    msg.data = now_ns();
    publisher_->publish(msg);
  }

  void on_pong(std_msgs::msg::UInt64::ConstSharedPtr msg)
  {
    // 1. 이미 필요한 표본을 다 모았으면 무시
    if (samples_.size() >= sample_count_) {
      return;
    }

    // 2. 미래 시각은 부호 없는 정수 뺄셈 전에 거부
    const uint64_t received_at = now_ns();
    if (msg->data > received_at) {
      RCLCPP_WARN(this->get_logger(), "현재보다 미래인 타임스탬프 수신");
      return;
    }

    // 3. 왕복 지연을 마이크로초 단위로 누적
    const uint64_t round_trip_ns = received_at - msg->data;
    const double round_trip_us = static_cast<double>(round_trip_ns) / 1000.0;
    samples_.push_back(round_trip_us);

    // 4. 표본이 다 모이면 발행을 멈추고 통계를 출력한 뒤 종료
    if (samples_.size() >= sample_count_) {
      timer_->cancel();
      report();
      rclcpp::shutdown();
    }
  }

  void report()
  {
    // 1. 백분위수를 얻기 위해 정렬
    std::sort(samples_.begin(), samples_.end());

    // 2. 평균과 중앙값 계산
    const double sum = std::accumulate(samples_.begin(), samples_.end(), 0.0);
    const double mean = sum / static_cast<double>(samples_.size());
    const std::size_t middle = samples_.size() / 2;
    const double median = samples_.size() % 2 == 0 ?
      (samples_[middle - 1] + samples_[middle]) / 2.0 : samples_[middle];

    // 3. 99번째 백분위수를 nearest-rank 방식으로 선택
    const double rank = std::ceil(static_cast<double>(samples_.size()) * 0.99);
    const std::size_t p99_index = static_cast<std::size_t>(rank) - 1;

    // 4. 대표 통계 출력
    RCLCPP_INFO(
      this->get_logger(),
      "RMW=%s 표본=%zu 평균=%.1fus 중앙값=%.1fus p99=%.1fus 최대=%.1fus",
      rmw_get_implementation_identifier(), samples_.size(), mean,
      median, samples_[p99_index], samples_.back());
  }

  rclcpp::Publisher<std_msgs::msg::UInt64>::SharedPtr publisher_;
  rclcpp::Subscription<std_msgs::msg::UInt64>::SharedPtr subscription_;
  rclcpp::TimerBase::SharedPtr timer_;
  std::vector<double> samples_;
  std::size_t sample_count_{0};

  static constexpr int64_t kMaxSampleCount = 1'000'000;
};

int main(int argc, char * argv[])
{
  // 1. rclcpp 컨텍스트 초기화
  rclcpp::init(argc, argv);

  int exit_code = 0;
  try {
    // 2. 측정 노드 실행
    rclcpp::spin(std::make_shared<LatencyPing>());
  } catch (const std::exception & error) {
    RCLCPP_FATAL(rclcpp::get_logger("latency_ping"), "노드 실행 실패: %s", error.what());
    exit_code = 1;
  }

  // 3. 컨텍스트 정리
  rclcpp::shutdown();
  return exit_code;
}
