# Copyright 2026 makepluscode
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

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
