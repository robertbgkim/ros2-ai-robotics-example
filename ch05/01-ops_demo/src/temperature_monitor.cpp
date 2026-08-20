// Copyright 2026 makepluscode
// SPDX-License-Identifier: Apache-2.0

#include <chrono>
#include <cmath>
#include <functional>
#include <stdexcept>
#include <vector>
#include <memory>
#include <string>

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
  : Node("temperature_monitor"), last_temperature_(0.0), received_(false)
  {
    // 1. 경고·오류 임계값 파라미터 선언
    this->declare_parameter<double>("warn_threshold", 55.0);
    this->declare_parameter<double>("error_threshold", 70.0);

    // 2. 온도 구독자 생성
    subscription_ = this->create_subscription<sensor_msgs::msg::Temperature>(
        "/motor_temperature", 10,
        std::bind(&TemperatureMonitor::on_temperature, this, std::placeholders::_1));

    // 3. 파라미터 변경을 받아들이기 전에 검증
    parameter_callback_ = this->add_on_set_parameters_callback(
      std::bind(&TemperatureMonitor::on_set_parameters, this, std::placeholders::_1));

    // 4. 초기 설정이 이미 잘못돼 있으면 즉시 중단
    if (!is_valid_pair(
        this->get_parameter("warn_threshold").as_double(),
        this->get_parameter("error_threshold").as_double()))
    {
      throw std::invalid_argument(
        "warn_threshold는 error_threshold보다 작은 유한한 값이어야 합니다");
    }

    // 5. 진단 갱신기 구성과 진단 항목 등록
    updater_ = std::make_shared<diagnostic_updater::Updater>(this);
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

  void on_temperature(const sensor_msgs::msg::Temperature::SharedPtr msg)
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
  std::shared_ptr<diagnostic_updater::Updater> updater_;
  rclcpp::node_interfaces::OnSetParametersCallbackHandle::SharedPtr parameter_callback_;
  double last_temperature_;
  std::chrono::steady_clock::time_point last_receive_time_;
  bool received_;
};

int main(int argc, char * argv[])
{
  // 1. rclcpp 컨텍스트 초기화
  rclcpp::init(argc, argv);

  // 2. 온도 감시 노드 실행
  rclcpp::spin(std::make_shared<TemperatureMonitor>());

  // 3. 컨텍스트 정리
  rclcpp::shutdown();
  return 0;
}
