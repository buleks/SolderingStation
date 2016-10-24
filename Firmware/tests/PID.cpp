#include "gtest/gtest.h"

#include "pid.h"

constexpr float eps = 1e-2;

TEST(PID, setup) {
    core::PID pid;
    EXPECT_NEAR(pid.target, 0, eps);
    EXPECT_NEAR(pid.Kp, 0, eps);
    EXPECT_NEAR(pid.Ki, 0, eps);
    EXPECT_NEAR(pid.Kd, 0, eps);
    EXPECT_NEAR(pid.Kp, 0, eps);
    EXPECT_NEAR(pid.integral, 0, eps);
    EXPECT_NEAR(pid.prevError, 0, eps);
    EXPECT_NEAR(pid.prevOutput, 0, eps);
}

TEST(PID, noOutput) {
    core::PID pid;
    EXPECT_NEAR(pid.tick(0), 0, eps);
    EXPECT_NEAR(pid.tick(100), 0, eps);
}


TEST(PID, proportional) {
    core::PID pid;
    pid.Kp = 1;
    pid.target = 0;
    pid.lowerLimit = -1e6f;
    pid.upperLimit = 1e6f;
    EXPECT_NEAR(pid.tick(0), 0, eps);
    for (float i = -500; i <= 500; i += 0.7) {
        EXPECT_NEAR(pid.tick(i), i, eps);
    }


    pid.Kp = -1.5f;
    pid.target = 10.0f;
    EXPECT_NEAR(pid.tick(10), 0, eps);
    for (float i = -500; i <= 500; i += 0.7) {
        EXPECT_NEAR(pid.tick(i), -1.5*(i-10), eps);
    }
}

TEST(PID, integral) {
    core::PID pid;
    pid.Ki = 1;
    pid.target = 0;
    pid.lowerLimit = -1e6f;
    pid.upperLimit = 1e6f;
    EXPECT_NEAR(pid.tick(0), 0, eps);
    for (int i = 1; i <= 500; ++i) {
        EXPECT_NEAR(pid.tick(1), i, eps);
    }

    pid.reset();
    pid.Ki = -1.5f;
    pid.target = 10.0f;
    EXPECT_NEAR(pid.tick(10), 0, eps);
    float sum = 0;
    for (float i = -500; i <= 500; i += 0.7) {
        sum += (i-10.0f);
        EXPECT_NEAR(pid.tick(i), -1.5*sum, 10*eps);
    }
}

TEST(PID, derivative) {
    core::PID pid;
    pid.Kd = 1;
    pid.target = 0;
    pid.lowerLimit = -1e6f;
    pid.upperLimit = 1e6f;
    EXPECT_NEAR(pid.tick(0), 0, eps);

    for (int i = 1; i <= 500; ++i) {
        EXPECT_NEAR(pid.tick(0), -(i-1), eps);
        EXPECT_NEAR(pid.tick(i), i, eps);
    }

    pid.reset();
    pid.Kd = -1.5f;
    pid.target = 10.0f;
    EXPECT_NEAR(pid.tick(10), 0, eps);

    for (float i = 10.7; i <= 500; i += 0.7) {
        EXPECT_NEAR(pid.tick(i), -1.5f*0.7f, eps);
    }
}

TEST(PID, limits) {
    core::PID pid;
    pid.Kp = 1;
    pid.lowerLimit = -107;
    pid.upperLimit = 257;
    EXPECT_NEAR(pid.tick(0), 0, eps);

    for (float i = -500; i <= 500; i += 0.7) {
        float expected = std::max(-107.0f, std::min(i, 257.0f));
        EXPECT_NEAR(pid.tick(i), expected, eps);
    }

    pid.lowerLimit = -17;
    pid.upperLimit = 13;
    EXPECT_NEAR(pid.tick(0), 0, eps);

    for (float i = -500; i <= 500; i += 0.7) {
        float expected = std::max(-17.0f, std::min(i, 13.0f));
        EXPECT_NEAR(pid.tick(i), expected, eps);
    }
}

TEST(PID, antiWindup) {
    core::PID pid;
    pid.Kp = -1;
    pid.Ki = -1;
    pid.target = 10;
    pid.lowerLimit = 0;
    pid.upperLimit = 100;

    float temp = 0;
    for (int i = 1; i <= 100; ++i) {
        float pwr = pid.tick(temp);
        EXPECT_LT(pid.integral, pid.upperLimit);
        EXPECT_LT(pid.integral, pid.lowerLimit);

        temp += 0.1;
    }
}