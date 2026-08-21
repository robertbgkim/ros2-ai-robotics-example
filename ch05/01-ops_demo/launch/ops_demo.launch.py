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

import os

from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import Node


def generate_launch_description():
    # 1. 파라미터 파일 기본 경로 계산
    default_params = os.path.join(
        get_package_share_directory('ops_demo'), 'config', 'ops_demo.yaml')

    # 2. 실행 시 바꿀 수 있는 인자 선언
    params_arg = DeclareLaunchArgument(
        'params_file', default_value=default_params,
        description='노드 파라미터 YAML 경로')
    log_level_arg = DeclareLaunchArgument(
        'log_level', default_value='info',
        description='두 노드에 함께 적용할 로그 레벨')

    params_file = LaunchConfiguration('params_file')
    log_level = LaunchConfiguration('log_level')

    # 3. 모의 센서 노드 정의
    sensor_sim = Node(
        package='ops_demo',
        executable='sensor_sim',
        name='sensor_sim',
        output='screen',
        parameters=[params_file],
        ros_arguments=['--log-level', log_level])

    # 4. 온도 감시 노드 정의
    temperature_monitor = Node(
        package='ops_demo',
        executable='temperature_monitor',
        name='temperature_monitor',
        output='screen',
        parameters=[params_file],
        ros_arguments=['--log-level', log_level])

    # 5. 선언한 인자와 노드를 한 묶음으로 반환
    return LaunchDescription([
        params_arg,
        log_level_arg,
        sensor_sim,
        temperature_monitor,
    ])
