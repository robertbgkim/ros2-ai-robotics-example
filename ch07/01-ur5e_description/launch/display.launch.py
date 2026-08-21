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
from launch.conditions import IfCondition
from launch.substitutions import Command, EqualsSubstitution, LaunchConfiguration
from launch_ros.actions import Node
from launch_ros.parameter_descriptions import ParameterValue


def generate_launch_description():
    # 1. 패키지 안의 xacro와 RViz 설정 경로 계산
    share = get_package_share_directory('ur5e_description')
    model_path = os.path.join(share, 'urdf', 'ur5e_with_camera.urdf.xacro')
    rviz_path = os.path.join(share, 'rviz', 'ur5e.rviz')

    # 2. 실행 시 바꿀 수 있는 인자 선언
    model_arg = DeclareLaunchArgument(
        'model', default_value=model_path, description='표시할 xacro 경로')
    source_arg = DeclareLaunchArgument(
        'joint_state_source', default_value='gui',
        description='관절 각도를 만들 노드: gui | zero | cycler',
        choices=['gui', 'zero', 'cycler'])
    rviz_arg = DeclareLaunchArgument(
        'rviz', default_value='true', description='RViz2 실행 여부')
    hold_arg = DeclareLaunchArgument(
        'hold_seconds', default_value='6.0',
        description='cycler를 쓸 때 자세 하나를 유지할 시간')

    model = LaunchConfiguration('model')
    rviz = LaunchConfiguration('rviz')
    hold_seconds = LaunchConfiguration('hold_seconds')

    # 3. xacro를 실행해 URDF 문자열을 얻고 파라미터로 전달
    robot_description = ParameterValue(
        Command(['xacro ', model]), value_type=str)

    # 4. URDF와 관절 각도로 TF를 발행하는 노드
    robot_state_publisher = Node(
        package='robot_state_publisher',
        executable='robot_state_publisher',
        output='screen',
        parameters=[{'robot_description': robot_description}])

    # 5. 관절 각도를 만드는 노드는 셋 중 정확히 하나만 실행
    #    같은 토픽에 둘이 발행하면 자세가 섞여 TF가 오락가락함
    joint_state_publisher_gui = Node(
        package='joint_state_publisher_gui',
        executable='joint_state_publisher_gui',
        condition=IfCondition(EqualsSubstitution(
            LaunchConfiguration('joint_state_source'), 'gui')))

    joint_state_publisher = Node(
        package='joint_state_publisher',
        executable='joint_state_publisher',
        condition=IfCondition(EqualsSubstitution(
            LaunchConfiguration('joint_state_source'), 'zero')))

    pose_cycler = Node(
        package='ur5e_description',
        executable='pose_cycler',
        output='screen',
        parameters=[{'hold_seconds': ParameterValue(hold_seconds, value_type=float)}],
        condition=IfCondition(EqualsSubstitution(
            LaunchConfiguration('joint_state_source'), 'cycler')))

    # 6. 시각화 도구
    rviz_node = Node(
        package='rviz2',
        executable='rviz2',
        arguments=['-d', rviz_path],
        condition=IfCondition(rviz))

    # 7. 선언한 인자와 노드를 한 묶음으로 반환
    return LaunchDescription([
        model_arg,
        source_arg,
        rviz_arg,
        hold_arg,
        robot_state_publisher,
        joint_state_publisher_gui,
        joint_state_publisher,
        pose_cycler,
        rviz_node,
    ])
