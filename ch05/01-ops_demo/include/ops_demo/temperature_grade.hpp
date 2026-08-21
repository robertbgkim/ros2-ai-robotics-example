// Copyright 2026 makepluscode
// SPDX-License-Identifier: Apache-2.0

#ifndef OPS_DEMO__TEMPERATURE_GRADE_HPP_
#define OPS_DEMO__TEMPERATURE_GRADE_HPP_

namespace ops_demo
{

// 온도 판정 등급
enum class Grade
{
  Ok,
  Warn,
  Error,
  InvalidThreshold,
};

// 온도와 두 임계값으로 등급 결정
// ROS2에 의존하지 않는 순수 함수라 노드를 띄우지 않고 검증 가능
inline Grade grade_temperature(double temperature, double warn, double error)
{
  // 1. 임계값 순서가 뒤집혔으면 판정 불가
  if (!(warn < error)) {
    return Grade::InvalidThreshold;
  }

  // 2. 위쪽 경계부터 차례로 비교
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
