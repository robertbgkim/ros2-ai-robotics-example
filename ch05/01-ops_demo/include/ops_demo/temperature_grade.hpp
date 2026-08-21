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

#ifndef OPS_DEMO__TEMPERATURE_GRADE_HPP_
#define OPS_DEMO__TEMPERATURE_GRADE_HPP_

#include <cmath>

namespace ops_demo
{

// 온도 판정 등급
enum class Grade
{
  Ok,
  Warn,
  Error,
  InvalidTemperature,
  InvalidThreshold,
};

// 온도와 두 임계값으로 등급 결정
// ROS2에 의존하지 않는 순수 함수라 노드를 띄우지 않고 검증 가능
inline Grade grade_temperature(double temperature, double warn, double error)
{
  // 1. 임계값이 유한하지 않거나 순서가 뒤집혔으면 판정 불가
  if (!std::isfinite(warn) || !std::isfinite(error) || !(warn < error)) {
    return Grade::InvalidThreshold;
  }

  // 2. 온도가 유한하지 않으면 별도의 입력 오류로 판정
  if (!std::isfinite(temperature)) {
    return Grade::InvalidTemperature;
  }

  // 3. 위쪽 경계부터 차례로 비교
  if (temperature >= error) {
    return Grade::Error;
  }
  if (temperature >= warn) {
    return Grade::Warn;
  }
  return Grade::Ok;
}

}  // namespace ops_demo

#endif  // OPS_DEMO__TEMPERATURE_GRADE_HPP_
