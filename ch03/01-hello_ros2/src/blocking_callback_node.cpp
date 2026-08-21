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
#include <cinttypes>
#include <cstddef>
#include <cstdint>
#include <exception>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <thread>
#include <vector>

#include "rclcpp/rclcpp.hpp"

using namespace std::chrono_literals;

namespace
{

enum class ExecutorMode : std::uint8_t
{
  Single,
  MultiSameGroup,
  MultiSeparateGroups,
};

std::optional<ExecutorMode> parse_executor_mode(std::string_view mode)
{
  if (mode == "single") {
    return ExecutorMode::Single;
  }
  if (mode == "multi_same_group") {
    return ExecutorMode::MultiSameGroup;
  }
  if (mode == "multi_separate_groups") {
    return ExecutorMode::MultiSeparateGroups;
  }
  return std::nullopt;
}

}  // namespace

class BlockingCallbackNode : public rclcpp::Node
{
public:
  explicit BlockingCallbackNode(ExecutorMode mode)
  : Node("blocking_callback_node")
  {
    // 1. separate_groups일 때만 slow 콜백용 별도 그룹 생성(멤버 변수로 보관)
    if (mode == ExecutorMode::MultiSeparateGroups) {
      slow_group_ = this->create_callback_group(rclcpp::CallbackGroupType::MutuallyExclusive);
    }

    // 2. 100ms 주기 fast 타이머 등록(기본 그룹)
    fast_timer_ = this->create_wall_timer(100ms, [this]() {on_fast();});

    // 3. 1000ms 주기 slow 타이머 등록(slow_group_ 지정 시 분리)
    slow_timer_ = this->create_wall_timer(1000ms, [this]() {on_slow();}, slow_group_);
  }

private:
  void on_fast()
  {
    // 1. 직전 호출과의 간격 계산
    const auto now = std::chrono::steady_clock::now();
    if (fast_count_ > 0) {
      const auto gap_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
          now - last_fast_).count();

      // 2. 간격 로그 출력
      RCLCPP_INFO(
          this->get_logger(), "fast tick #%zu (간격 %" PRId64 "ms)",
          fast_count_, static_cast<int64_t>(gap_ms));
    }

    // 3. 다음 호출을 위한 상태 갱신
    last_fast_ = now;
    ++fast_count_;
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
  std::size_t fast_count_{0};
  std::chrono::steady_clock::time_point last_fast_;
};

int main(int argc, char * argv[])
{
  // 1. rclcpp 컨텍스트 초기화
  rclcpp::init(argc, argv);

  int exit_code = 0;
  try {
    // 2. ROS 인자를 제외한 실행 모드 파싱
    const std::vector<std::string> args = rclcpp::remove_ros_arguments(argc, argv);
    if (args.size() > 2) {
      RCLCPP_ERROR(
        rclcpp::get_logger("blocking_callback_node"),
        "실행 모드 인자는 하나만 지정할 수 있습니다");
      rclcpp::shutdown();
      return 1;
    }

    const std::string mode_name = args.size() == 2 ? args[1] : "single";
    const auto mode = parse_executor_mode(mode_name);
    if (!mode) {
      RCLCPP_ERROR(
        rclcpp::get_logger("blocking_callback_node"),
        "알 수 없는 실행 모드: %s", mode_name.c_str());
      rclcpp::shutdown();
      return 1;
    }

    const auto node = std::make_shared<BlockingCallbackNode>(*mode);

    // 3. 모드에 따라 Executor 선택
    if (*mode == ExecutorMode::Single) {
      rclcpp::spin(node);
    } else {
      rclcpp::executors::MultiThreadedExecutor executor;
      executor.add_node(node);
      executor.spin();
    }
  } catch (const std::exception & error) {
    RCLCPP_FATAL(
      rclcpp::get_logger("blocking_callback_node"),
      "노드 실행 실패: %s", error.what());
    exit_code = 1;
  }

  // 4. 컨텍스트 정리
  rclcpp::shutdown();
  return exit_code;
}
