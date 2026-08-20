// Copyright 2026 makepluscode
// SPDX-License-Identifier: Apache-2.0

#include <algorithm>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
#include <vector>

#include "rclcpp/rclcpp.hpp"
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
    // 1. 측정 횟수 파라미터 선언
    this->declare_parameter<int>("sample_count", 1000);
    sample_count_ = static_cast<std::size_t>(this->get_parameter("sample_count").as_int());
    samples_.reserve(sample_count_);

    // 2. 반환 노드와 같은 QoS 구성
    rclcpp::QoS qos(rclcpp::KeepLast(1));
    qos.best_effort();

    // 3. 요청 발행자와 반환 구독자 생성
    publisher_ = this->create_publisher<std_msgs::msg::UInt64>("/ping", qos);
    subscription_ = this->create_subscription<std_msgs::msg::UInt64>(
        "/pong", qos, std::bind(&LatencyPing::on_pong, this, std::placeholders::_1));

    // 4. 1ms 간격으로 한 건씩 발행
    timer_ = this->create_wall_timer(1ms, std::bind(&LatencyPing::on_timer, this));

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

  void on_pong(const std_msgs::msg::UInt64::SharedPtr msg)
  {
    // 1. 이미 필요한 표본을 다 모았으면 무시
    if (samples_.size() >= sample_count_) {
      return;
    }

    // 2. 왕복 지연을 마이크로초 단위로 누적
    const double round_trip_us = static_cast<double>(now_ns() - msg->data) / 1000.0;
    samples_.push_back(round_trip_us);

    // 3. 표본이 다 모이면 통계를 출력하고 종료
    if (samples_.size() >= sample_count_) {
      report();
      rclcpp::shutdown();
    }
  }

  void report()
  {
    // 1. 백분위수를 얻기 위해 정렬
    std::sort(samples_.begin(), samples_.end());

    // 2. 평균 계산
    double sum = 0.0;
    for (double v : samples_) {
      sum += v;
    }
    const double mean = sum / static_cast<double>(samples_.size());

    // 3. 대표 통계 출력
    RCLCPP_INFO(
        this->get_logger(),
        "RMW=%s 표본=%zu 평균=%.1fus 중앙값=%.1fus p99=%.1fus 최대=%.1fus",
        rmw_get_implementation_identifier(), samples_.size(), mean,
        samples_[samples_.size() / 2],
        samples_[static_cast<std::size_t>(static_cast<double>(samples_.size()) * 0.99)],
        samples_.back());
  }

  rclcpp::Publisher<std_msgs::msg::UInt64>::SharedPtr publisher_;
  rclcpp::Subscription<std_msgs::msg::UInt64>::SharedPtr subscription_;
  rclcpp::TimerBase::SharedPtr timer_;
  std::vector<double> samples_;
  std::size_t sample_count_;
};

int main(int argc, char * argv[])
{
  // 1. rclcpp 컨텍스트 초기화
  rclcpp::init(argc, argv);

  // 2. 측정 노드 실행
  rclcpp::spin(std::make_shared<LatencyPing>());

  // 3. 컨텍스트 정리
  rclcpp::shutdown();
  return 0;
}
