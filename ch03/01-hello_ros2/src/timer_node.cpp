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
#include <memory>

#include "rclcpp/rclcpp.hpp"

using namespace std::chrono_literals;

class TimerNode : public rclcpp::Node
{
public:
  TimerNode()
  : Node("timer_node")
  {
    // 1. 500ms 주기 타이머 콜백 등록
    timer_ = this->create_wall_timer(500ms, [this]() {on_timer();});
  }

private:
  void on_timer()
  {
    // 1. 호출 횟수 누적
    ++count_;

    // 2. 콜백 실행 로그 출력
    RCLCPP_INFO(this->get_logger(), "타이머 콜백 실행 #%zu", count_);
  }

  rclcpp::TimerBase::SharedPtr timer_;
  std::size_t count_{0};
};

int main(int argc, char * argv[])
{
  // 1. rclcpp 컨텍스트 초기화
  rclcpp::init(argc, argv);

  // 2. 기본 Executor로 노드 실행
  rclcpp::spin(std::make_shared<TimerNode>());

  // 3. 컨텍스트 정리
  rclcpp::shutdown();
  return 0;
}
