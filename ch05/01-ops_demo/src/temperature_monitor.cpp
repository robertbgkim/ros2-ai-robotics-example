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
#include <exception>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

#include "diagnostic_updater/diagnostic_updater.hpp"
#include "ops_demo/temperature_grade.hpp"
#include "rcl_interfaces/msg/set_parameters_result.hpp"
#include "rclcpp/rclcpp.hpp"
#include "sensor_msgs/msg/temperature.hpp"

using namespace std::chrono_literals;

// 온도를 구독해 임계값과 비교하고 진단 상태로 보고하는 노드
class TemperatureMonitor : public rclcpp::Node
{
public:
  TemperatureMonitor()
  : Node("temperature_monitor")
  {
    // 1. 경고·오류 임계값 파라미터 선언
    this->declare_parameter<double>("warn_threshold", 55.0);
    this->declare_parameter<double>("error_threshold", 70.0);

    // 2. 초기 설정이 이미 잘못돼 있으면 리소스 생성 전에 중단
    if (!is_valid_pair(
        this->get_parameter("warn_threshold").as_double(),
        this->get_parameter("error_threshold").as_double()))
    {
      throw std::invalid_argument(
              "warn_threshold는 error_threshold보다 작은 유한한 값이어야 합니다");
    }

    // 3. 온도 구독자와 파라미터 변경 콜백 생성
    subscription_ = this->create_subscription<sensor_msgs::msg::Temperature>(
      "/motor_temperature", 10,
      [this](const sensor_msgs::msg::Temperature::ConstSharedPtr & msg) {
        on_temperature(msg);
      });
    parameter_callback_ = this->add_on_set_parameters_callback(
      [this](const std::vector<rclcpp::Parameter> & parameters) {
        return on_set_parameters(parameters);
      });

    // 4. 진단 갱신기 구성과 진단 항목 등록
    updater_ = std::make_unique<diagnostic_updater::Updater>(this);
    updater_->setHardwareID("motor_case");
    updater_->add("모터 온도", this, &TemperatureMonitor::diagnose);

    RCLCPP_INFO(this->get_logger(), "온도 감시 노드 시작");
  }

private:
  // 두 임계값이 유한하고 순서가 맞는지 확인
  static bool is_valid_pair(double warn, double error)
  {
    return std::isfinite(warn) && std::isfinite(error) && warn < error;
  }

  rcl_interfaces::msg::SetParametersResult on_set_parameters(
    const std::vector<rclcpp::Parameter> & parameters)
  {
    // 1. 변경 후에 적용될 값을 미리 계산
    double warn = this->get_parameter("warn_threshold").as_double();
    double error = this->get_parameter("error_threshold").as_double();
    for (const auto & parameter : parameters) {
      if (parameter.get_name() == "warn_threshold") {
        warn = parameter.as_double();
      } else if (parameter.get_name() == "error_threshold") {
        error = parameter.as_double();
      }
    }

    // 2. 관계가 깨지는 조합이면 적용 전에 거부
    rcl_interfaces::msg::SetParametersResult result;
    result.successful = is_valid_pair(warn, error);
    if (!result.successful) {
      result.reason = "warn_threshold는 error_threshold보다 작은 유한한 값이어야 합니다";
    }
    return result;
  }

  void on_temperature(const sensor_msgs::msg::Temperature::ConstSharedPtr & msg)
  {
    // 1. 유한하지 않은 값은 받아들이지 않음
    if (!std::isfinite(msg->temperature)) {
      RCLCPP_WARN(this->get_logger(), "유한하지 않은 온도 수신");
      return;
    }

    // 2. 최신 온도와 수신 시각 보관
    last_temperature_ = msg->temperature;
    last_receive_time_ = std::chrono::steady_clock::now();
    received_ = true;

    // 3. 매 건은 디버그 레벨로만 기록
    RCLCPP_DEBUG(this->get_logger(), "온도 %.1f도 수신", last_temperature_);
  }

  void diagnose(diagnostic_updater::DiagnosticStatusWrapper & status)
  {
    // 1. 현재 임계값 읽기
    const double warn = this->get_parameter("warn_threshold").as_double();
    const double error = this->get_parameter("error_threshold").as_double();

    // 2. 아직 수신 전이면 판정 보류
    if (!received_) {
      status.summary(
        diagnostic_msgs::msg::DiagnosticStatus::WARN, "온도 데이터 수신 전");
      return;
    }

    // 3. 마지막 수신 이후 시간이 지나치게 벌어지면 단절로 판정
    if (std::chrono::steady_clock::now() - last_receive_time_ > 2s) {
      status.summary(
        diagnostic_msgs::msg::DiagnosticStatus::ERROR, "온도 데이터 수신 중단");
      status.add("마지막 온도", last_temperature_);
      return;
    }

    // 4. 등급 판정은 ROS2에 의존하지 않는 순수 함수에 맡김
    switch (ops_demo::grade_temperature(last_temperature_, warn, error)) {
      case ops_demo::Grade::Error:
        status.summary(diagnostic_msgs::msg::DiagnosticStatus::ERROR, "온도 상한 초과");
        break;
      case ops_demo::Grade::Warn:
        status.summary(diagnostic_msgs::msg::DiagnosticStatus::WARN, "온도 경고 구간");
        break;
      case ops_demo::Grade::Ok:
        status.summary(diagnostic_msgs::msg::DiagnosticStatus::OK, "온도 정상");
        break;
      case ops_demo::Grade::InvalidTemperature:
        status.summary(diagnostic_msgs::msg::DiagnosticStatus::ERROR, "온도 입력 오류");
        break;
      case ops_demo::Grade::InvalidThreshold:
        status.summary(diagnostic_msgs::msg::DiagnosticStatus::ERROR, "임계값 설정 오류");
        break;
    }

    // 5. 판정 근거가 된 값을 함께 실어 보냄
    status.add("현재 온도", last_temperature_);
    status.add("경고 임계값", warn);
    status.add("오류 임계값", error);
  }

  rclcpp::Subscription<sensor_msgs::msg::Temperature>::SharedPtr subscription_;
  std::unique_ptr<diagnostic_updater::Updater> updater_;
  rclcpp::node_interfaces::OnSetParametersCallbackHandle::SharedPtr parameter_callback_;
  double last_temperature_{0.0};
  std::chrono::steady_clock::time_point last_receive_time_;
  bool received_{false};
};

int main(int argc, char * argv[])
{
  // 1. rclcpp 컨텍스트 초기화
  rclcpp::init(argc, argv);

  int exit_code = 0;
  try {
    // 2. 온도 감시 노드 실행
    rclcpp::spin(std::make_shared<TemperatureMonitor>());
  } catch (const std::exception & error) {
    RCLCPP_FATAL(
      rclcpp::get_logger("temperature_monitor"), "노드 실행 실패: %s", error.what());
    exit_code = 1;
  }

  // 3. 컨텍스트 정리
  rclcpp::shutdown();
  return exit_code;
}
