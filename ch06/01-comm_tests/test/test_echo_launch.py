# Copyright 2026 makepluscode
# SPDX-License-Identifier: Apache-2.0

import time
import unittest

import launch
import launch_ros.actions
import launch_testing.actions
import launch_testing.markers
import pytest
import rclpy
from rclpy.node import Node
from std_msgs.msg import String


@pytest.mark.launch_test
@launch_testing.markers.keep_alive
def generate_test_description():
    # 1. 테스트 대상 노드 정의
    echo_node = launch_ros.actions.Node(
        package='comm_tests',
        executable='echo_node',
        name='echo_node',
        parameters=[{'prefix': 'test: '}])

    # 2. 노드를 띄운 뒤 테스트를 시작하도록 선언
    return launch.LaunchDescription([
        echo_node,
        launch_testing.actions.ReadyToTest(),
    ]), {'echo_node': echo_node}


class TestEchoNode(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        rclpy.init()

    @classmethod
    def tearDownClass(cls):
        rclpy.shutdown()

    def setUp(self):
        self.node = Node('echo_test_client')
        self.received = []
        self.subscription = self.node.create_subscription(
            String, '/echo_out', lambda msg: self.received.append(msg.data), 10)
        self.publisher = self.node.create_publisher(String, '/echo_in', 10)

    def tearDown(self):
        self.node.destroy_node()

    def test_prefix_is_applied(self):
        # 1. 양방향 매칭이 끝날 때까지 대기
        #    디스커버리는 비동기라 한쪽만 확인하면 첫 발행을 놓칠 수 있음
        #    ROS 시간은 시뮬레이션 시각으로 바뀔 수 있으므로 단조 시계를 씀
        end = time.monotonic_ns() + 10_000_000_000
        while (self.publisher.get_subscription_count() == 0
               or self.subscription.get_publisher_count() == 0):
            rclpy.spin_once(self.node, timeout_sec=0.1)
            self.assertLess(time.monotonic_ns(), end, '에코 노드와 매칭되지 않음')

        # 2. 응답이 올 때까지 되풀이해 발행
        #    매칭 직후에도 첫 건이 유실될 수 있으므로 한 번만 보내지 않음
        message = String()
        message.data = 'hello'
        end = time.monotonic_ns() + 10_000_000_000
        while not self.received:
            self.publisher.publish(message)
            rclpy.spin_once(self.node, timeout_sec=0.2)
            self.assertLess(time.monotonic_ns(), end, '응답을 받지 못함')

        # 3. 접두사가 붙었는지 확인
        self.assertEqual(self.received[0], 'test: hello')


@launch_testing.post_shutdown_test()
class TestEchoNodeShutdown(unittest.TestCase):

    def test_exit_codes(self, proc_info):
        # 1. 노드가 비정상 종료하지 않았는지 확인
        launch_testing.asserts.assertExitCodes(proc_info)
