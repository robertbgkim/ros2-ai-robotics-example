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

#include <exception>
#include <memory>
#include <string>

#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/string.hpp"

// 받은 문자열 앞에 접두사를 붙여 되돌려 보내는 노드
// 통합 테스트에서 입력과 출력을 양쪽에서 확인하는 대상
class EchoNode : public rclcpp::Node
{
public:
  EchoNode()
  : Node("echo_node")
  {
    // 1. 접두사 파라미터 선언과 읽기
    this->declare_parameter<std::string>("prefix", "echo: ");
    prefix_ = this->get_parameter("prefix").as_string();

    // 2. 응답 발행자 생성
    publisher_ = this->create_publisher<std_msgs::msg::String>("/echo_out", 10);

    // 3. 요청 구독자 생성
    subscription_ = this->create_subscription<std_msgs::msg::String>(
      "/echo_in", 10,
      [this](const std_msgs::msg::String::ConstSharedPtr & msg) {on_input(msg);});

    RCLCPP_INFO(this->get_logger(), "에코 노드 시작 (접두사=%s)", prefix_.c_str());
  }

private:
  void on_input(const std_msgs::msg::String::ConstSharedPtr & msg)
  {
    // 1. 접두사를 붙인 응답 구성
    std_msgs::msg::String out;
    out.data = prefix_ + msg->data;

    // 2. 응답 발행
    publisher_->publish(out);
  }

  rclcpp::Publisher<std_msgs::msg::String>::SharedPtr publisher_;
  rclcpp::Subscription<std_msgs::msg::String>::SharedPtr subscription_;
  std::string prefix_;
};

int main(int argc, char * argv[])
{
  // 1. rclcpp 컨텍스트 초기화
  rclcpp::init(argc, argv);

  int exit_code = 0;
  try {
    // 2. 에코 노드 실행
    rclcpp::spin(std::make_shared<EchoNode>());
  } catch (const std::exception & error) {
    RCLCPP_FATAL(rclcpp::get_logger("echo_node"), "노드 실행 실패: %s", error.what());
    exit_code = 1;
  }

  // 3. 컨텍스트 정리
  rclcpp::shutdown();
  return exit_code;
}
