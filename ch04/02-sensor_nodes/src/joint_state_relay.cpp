#include <functional>
#include <memory>
#include <string>

#include "rclcpp/rclcpp.hpp"
#include "robot_interfaces/msg/joint_command.hpp"
#include "robot_interfaces/srv/set_gripper.hpp"
#include "sensor_msgs/msg/joint_state.hpp"

// 관절 상태를 구독해 목표 명령으로 바꿔 발행하고, 그리퍼 폭 설정 서비스를 제공하는 노드
class JointStateRelay : public rclcpp::Node
{
public:
  JointStateRelay()
  : Node("joint_state_relay"), gripper_width_(0.0)
  {
    // 1. 드문 목표 설정값을 위한 신뢰성 있는 QoS 구성
    rclcpp::QoS command_qos(rclcpp::KeepLast(10));
    command_qos.reliable();
    command_qos.durability_volatile();

    // 2. 관절 상태 구독(센서 스트림이므로 best_effort)
    subscription_ = this->create_subscription<sensor_msgs::msg::JointState>(
        "/joint_states", rclcpp::SensorDataQoS(),
        std::bind(&JointStateRelay::on_joint_state, this, std::placeholders::_1));

    // 3. 목표 명령 발행자 생성
    publisher_ = this->create_publisher<robot_interfaces::msg::JointCommand>(
        "/joint_command", command_qos);

    // 4. 그리퍼 폭 설정 서비스 서버 생성
    service_ = this->create_service<robot_interfaces::srv::SetGripper>(
        "/set_gripper",
        std::bind(&JointStateRelay::on_set_gripper, this,
                  std::placeholders::_1, std::placeholders::_2));

    RCLCPP_INFO(this->get_logger(), "관절 상태 릴레이 시작");
  }

private:
  void on_joint_state(const sensor_msgs::msg::JointState::SharedPtr msg)
  {
    // 1. 이름이 비어 있는 메시지는 무시
    if (msg->name.empty() || msg->position.empty()) {
      RCLCPP_WARN(this->get_logger(), "이름 또는 위치가 비어 있는 관절 상태 수신");
      return;
    }

    // 2. 첫 관절만 골라 목표 명령으로 변환
    robot_interfaces::msg::JointCommand command;
    command.joint_name = msg->name.front();
    command.position = msg->position.front();
    command.max_velocity = 1.0;

    // 3. 변환한 명령 발행
    publisher_->publish(command);

    RCLCPP_INFO(
        this->get_logger(), "관절 %s 상태 %.3f -> 명령 발행",
        command.joint_name.c_str(), command.position);
  }

  void on_set_gripper(
      const std::shared_ptr<robot_interfaces::srv::SetGripper::Request> request,
      std::shared_ptr<robot_interfaces::srv::SetGripper::Response> response)
  {
    // 1. 물리적으로 가능한 범위 검사
    if (request->width < 0.0 || request->width > 0.085) {
      response->success = false;
      response->message = "그리퍼 폭은 0.0에서 0.085 사이여야 합니다";
      RCLCPP_WARN(this->get_logger(), "그리퍼 폭 %.3f 거부", request->width);
      return;
    }

    // 2. 유효한 값이면 상태 갱신 후 성공 응답
    gripper_width_ = request->width;
    response->success = true;
    response->message = "그리퍼 폭 설정 완료";

    RCLCPP_INFO(this->get_logger(), "그리퍼 폭 %.3f 적용", gripper_width_);
  }

  rclcpp::Subscription<sensor_msgs::msg::JointState>::SharedPtr subscription_;
  rclcpp::Publisher<robot_interfaces::msg::JointCommand>::SharedPtr publisher_;
  rclcpp::Service<robot_interfaces::srv::SetGripper>::SharedPtr service_;
  double gripper_width_;
};

int main(int argc, char * argv[])
{
  // 1. rclcpp 컨텍스트 초기화
  rclcpp::init(argc, argv);

  // 2. 릴레이 노드 실행
  rclcpp::spin(std::make_shared<JointStateRelay>());

  // 3. 컨텍스트 정리
  rclcpp::shutdown();
  return 0;
}
