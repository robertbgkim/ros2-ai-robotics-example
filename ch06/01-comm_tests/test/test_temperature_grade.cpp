// Copyright 2026 makepluscode
// SPDX-License-Identifier: Apache-2.0

#include <gtest/gtest.h>

#include "comm_tests/temperature_grade.hpp"

using comm_tests::Grade;
using comm_tests::grade_temperature;

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
