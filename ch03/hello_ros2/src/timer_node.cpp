#include <chrono>
#include <functional>
#include <memory>

#include "rclcpp/rclcpp.hpp"

using namespace std::chrono_literals;

class TimerNode : public rclcpp::Node
{
public:
  TimerNode() : Node("timer_node"), count_(0)
  {
    // 1. 500ms 주기 타이머 콜백 등록
    timer_ = this->create_wall_timer(
        500ms, std::bind(&TimerNode::on_timer, this));
  }

private:
  void on_timer()
  {
    // 1. 호출 횟수 누적
    count_++;

    // 2. 콜백 실행 로그 출력
    RCLCPP_INFO(this->get_logger(), "타이머 콜백 실행 #%zu", count_);
  }

  rclcpp::TimerBase::SharedPtr timer_;
  size_t count_;
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
