#!/usr/bin/env bash
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

set -euo pipefail

repository_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
build_root="$(cd "${1:-build}" && pwd)"
cxx_compiler="${CXX:-g++}"
gcc_major="$("${cxx_compiler}" -dumpversion | cut -d. -f1)"
gcc_triple="$("${cxx_compiler}" -dumpmachine)"

extra_arguments=()
for include_directory in \
  "/usr/include/c++/${gcc_major}" \
  "/usr/include/${gcc_triple}/c++/${gcc_major}" \
  "/usr/include/c++/${gcc_major}/backward"
do
  if [[ -d "${include_directory}" ]]; then
    extra_arguments+=("--extra-arg-before=-isystem${include_directory}")
  fi
done

translation_units=(
  "hello_ros2:ch03/01-hello_ros2/src/timer_node.cpp"
  "hello_ros2:ch03/01-hello_ros2/src/blocking_callback_node.cpp"
  "sensor_nodes:ch04/02-sensor_nodes/src/camera_node.cpp"
  "sensor_nodes:ch04/02-sensor_nodes/src/joint_state_relay.cpp"
  "ops_demo:ch05/01-ops_demo/src/sensor_sim.cpp"
  "ops_demo:ch05/01-ops_demo/src/temperature_monitor.cpp"
  "ops_demo:ch05/01-ops_demo/test/test_temperature_grade.cpp"
  "comm_tests:ch06/01-comm_tests/src/echo_node.cpp"
  "dds_benchmark:ch06/02-dds_benchmark/src/latency_ping.cpp"
  "dds_benchmark:ch06/02-dds_benchmark/src/latency_pong.cpp"
  "ur5e_description:ch07/01-ur5e_description/src/pose_cycler.cpp"
)

for entry in "${translation_units[@]}"; do
  package_name="${entry%%:*}"
  relative_source="${entry#*:}"
  compilation_database="${build_root}/${package_name}"

  if [[ ! -f "${compilation_database}/compile_commands.json" ]]; then
    echo "compile_commands.json not found: ${compilation_database}" >&2
    exit 1
  fi

  echo "clang-tidy: ${relative_source}"
  clang-tidy \
    "${repository_root}/${relative_source}" \
    -p "${compilation_database}" \
    --config-file="${repository_root}/.clang-tidy" \
    --quiet \
    --warnings-as-errors='*' \
    "${extra_arguments[@]}"
done
