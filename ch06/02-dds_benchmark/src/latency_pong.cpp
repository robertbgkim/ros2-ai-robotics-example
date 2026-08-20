// Copyright 2026 makepluscode
// SPDX-License-Identifier: Apache-2.0

#include <functional>
#include <memory>

#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/u_int64.hpp"

// 받은 타임스탬프를 그대로 되돌려 보내는 노드
// 왕복 지연 측정의 반환점 역할
class LatencyPong : public rclcpp::Node
{
public:
  LatencyPong()
  : Node("latency_pong")
  {
    // 1. 지연 측정용 QoS 구성(재전송 없이 최신 한 건만)
    rclcpp::QoS qos(rclcpp::KeepLast(1));
    qos.best_effort();

    // 2. 반환 발행자 생성
    publisher_ = this->create_publisher<std_msgs::msg::UInt64>("/pong", qos);

    // 3. 요청 구독자 생성
    subscription_ = this->create_subscription<std_msgs::msg::UInt64>(
        "/ping", qos, std::bind(&LatencyPong::on_ping, this, std::placeholders::_1));

    RCLCPP_INFO(
        this->get_logger(), "지연 측정 반환 노드 시작 (RMW=%s)",
        rmw_get_implementation_identifier());
  }

private:
  void on_ping(const std_msgs::msg::UInt64::SharedPtr msg)
  {
    // 1. 받은 값을 그대로 되돌려 보냄
    publisher_->publish(*msg);
  }

  rclcpp::Publisher<std_msgs::msg::UInt64>::SharedPtr publisher_;
  rclcpp::Subscription<std_msgs::msg::UInt64>::SharedPtr subscription_;
};

int main(int argc, char * argv[])
{
  // 1. rclcpp 컨텍스트 초기화
  rclcpp::init(argc, argv);

  // 2. 반환 노드 실행
  rclcpp::spin(std::make_shared<LatencyPong>());

  // 3. 컨텍스트 정리
  rclcpp::shutdown();
  return 0;
}
