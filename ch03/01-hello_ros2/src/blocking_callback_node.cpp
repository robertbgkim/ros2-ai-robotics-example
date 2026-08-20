#include <chrono>
#include <cstddef>
#include <functional>
#include <memory>
#include <string>
#include <thread>

#include "rclcpp/rclcpp.hpp"

using namespace std::chrono_literals;

class BlockingCallbackNode : public rclcpp::Node
{
public:
  explicit BlockingCallbackNode(bool separate_groups)
  : Node("blocking_callback_node"), fast_count_(0)
  {
    // 1. separate_groups일 때만 slow 콜백용 별도 그룹 생성(멤버 변수로 보관)
    if (separate_groups) {
      slow_group_ = this->create_callback_group(rclcpp::CallbackGroupType::MutuallyExclusive);
    }

    // 2. 100ms 주기 fast 타이머 등록(기본 그룹)
    fast_timer_ = this->create_wall_timer(
        100ms, std::bind(&BlockingCallbackNode::on_fast, this));

    // 3. 1000ms 주기 slow 타이머 등록(slow_group_ 지정 시 분리)
    slow_timer_ = this->create_wall_timer(
        1000ms, std::bind(&BlockingCallbackNode::on_slow, this), slow_group_);
  }

private:
  void on_fast()
  {
    // 1. 직전 호출과의 간격 계산
    auto now = std::chrono::steady_clock::now();
    if (fast_count_ > 0) {
      auto gap_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
          now - last_fast_).count();

      // 2. 간격 로그 출력
      RCLCPP_INFO(
          this->get_logger(), "fast tick #%zu (간격 %lldms)",
          fast_count_, static_cast<long long>(gap_ms));
    }

    // 3. 다음 호출을 위한 상태 갱신
    last_fast_ = now;
    fast_count_++;
  }

  void on_slow()
  {
    // 1. 슬로우 콜백 시작 로그
    RCLCPP_INFO(this->get_logger(), "slow 콜백 시작 (800ms 대기)");

    // 2. 무거운 작업 흉내(블로킹 대기)
    std::this_thread::sleep_for(800ms);

    // 3. 슬로우 콜백 종료 로그
    RCLCPP_INFO(this->get_logger(), "slow 콜백 종료");
  }

  rclcpp::TimerBase::SharedPtr fast_timer_;
  rclcpp::TimerBase::SharedPtr slow_timer_;
  rclcpp::CallbackGroup::SharedPtr slow_group_;
  size_t fast_count_;
  std::chrono::steady_clock::time_point last_fast_;
};

int main(int argc, char * argv[])
{
  // 1. rclcpp 컨텍스트 초기화
  rclcpp::init(argc, argv);

  // 2. 실행 모드 인자 파싱(single·multi_same_group·multi_separate_groups)
  std::string mode = argc > 1 ? argv[1] : "single";
  bool separate_groups = (mode == "multi_separate_groups");
  auto node = std::make_shared<BlockingCallbackNode>(separate_groups);

  // 3. 모드에 따라 Executor 선택
  if (mode == "single") {
    rclcpp::spin(node);
  } else {
    rclcpp::executors::MultiThreadedExecutor executor;
    executor.add_node(node);
    executor.spin();
  }

  // 4. 컨텍스트 정리
  rclcpp::shutdown();
  return 0;
}
