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

#include <gtest/gtest.h>

#include <limits>

#include "ops_demo/temperature_grade.hpp"

using ops_demo::Grade;
using ops_demo::grade_temperature;

// 임계값 아래는 정상
TEST(TemperatureGrade, BelowWarnIsOk)
{
  EXPECT_EQ(grade_temperature(40.0, 55.0, 70.0), Grade::Ok);
  EXPECT_EQ(grade_temperature(54.9, 55.0, 70.0), Grade::Ok);
}

// 경계값은 경고 구간에 포함
TEST(TemperatureGrade, WarnBoundaryIsInclusive)
{
  EXPECT_EQ(grade_temperature(55.0, 55.0, 70.0), Grade::Warn);
  EXPECT_EQ(grade_temperature(69.9, 55.0, 70.0), Grade::Warn);
}

// 오류 임계값 이상은 오류
TEST(TemperatureGrade, AtOrAboveErrorIsError)
{
  EXPECT_EQ(grade_temperature(70.0, 55.0, 70.0), Grade::Error);
  EXPECT_EQ(grade_temperature(120.0, 55.0, 70.0), Grade::Error);
}

// 임계값 순서가 뒤집히면 판정하지 않음
TEST(TemperatureGrade, InvertedThresholdIsRejected)
{
  EXPECT_EQ(grade_temperature(60.0, 90.0, 70.0), Grade::InvalidThreshold);
  EXPECT_EQ(grade_temperature(60.0, 70.0, 70.0), Grade::InvalidThreshold);
}

// NaN과 무한대 온도는 정상 등급으로 분류하지 않음
TEST(TemperatureGrade, NonFiniteTemperatureIsRejected)
{
  EXPECT_EQ(
    grade_temperature(std::numeric_limits<double>::quiet_NaN(), 55.0, 70.0),
    Grade::InvalidTemperature);
  EXPECT_EQ(
    grade_temperature(std::numeric_limits<double>::infinity(), 55.0, 70.0),
    Grade::InvalidTemperature);
}

// NaN과 무한대 임계값은 임계값 설정 오류로 분류
TEST(TemperatureGrade, NonFiniteThresholdIsRejected)
{
  EXPECT_EQ(
    grade_temperature(60.0, std::numeric_limits<double>::quiet_NaN(), 70.0),
    Grade::InvalidThreshold);
  EXPECT_EQ(
    grade_temperature(60.0, 55.0, std::numeric_limits<double>::infinity()),
    Grade::InvalidThreshold);
}
