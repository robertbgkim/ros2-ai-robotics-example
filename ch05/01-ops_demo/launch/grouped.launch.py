# Copyright 2026 makepluscode
# SPDX-License-Identifier: Apache-2.0

from launch import LaunchDescription
from launch.actions import GroupAction
from launch_ros.actions import Node, PushRosNamespace


def generate_launch_description():
    # 1. 왼쪽 모터용 노드 묶음
    left = GroupAction([
        PushRosNamespace('left'),
        Node(package='ops_demo', executable='sensor_sim', name='sensor_sim',
             output='screen',
             parameters=[{'base_temperature': 40.0, 'frame_id': 'left_motor'}]),
    ])

    # 2. 오른쪽 모터용 노드 묶음
    right = GroupAction([
        PushRosNamespace('right'),
        Node(package='ops_demo', executable='sensor_sim', name='sensor_sim',
             output='screen',
             parameters=[{'base_temperature': 60.0, 'frame_id': 'right_motor'}]),
    ])

    # 3. 두 묶음을 함께 실행
    return LaunchDescription([left, right])
