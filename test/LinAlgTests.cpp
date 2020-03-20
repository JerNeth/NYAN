#include <gtest/gtest.h>
#include "LinAlg.h"

namespace bla {
    TEST(BasicLinearAlgebra, Assignment) {
        bla::vec4 test({ 0,0,0,0 });
        test.x() = 5;
        test.y() -= 4;
        EXPECT_EQ(test.x(), 5);
        EXPECT_EQ(test.y(), -4);
    }
    TEST(BasicLinearAlgebra, Addition) {
        bla::vec2 a2(0), b2(1), c2(2);
        bla::vec3 a3(0), b3(1), c3(2);
        bla::vec4 a4(0), b4(1), c4(2);

        EXPECT_EQ(a2 + b2, b2);
        EXPECT_EQ(a3 + b3, b3);
        EXPECT_EQ(a4 + b4, b4);

        EXPECT_EQ(b2 + b2, c2);
        EXPECT_EQ(b3 + b3, c3);
        EXPECT_EQ(b4 + b4, c4);
    }
    TEST(BasicLinearAlgebra, Subtraction) {
        bla::vec2 a2(0), b2(1), c2(2);
        bla::vec3 a3(0), b3(1), c3(2);
        bla::vec4 a4(0), b4(1), c4(2);

        EXPECT_EQ(b2 - a2, b2);
        EXPECT_EQ(b3 - a3, b3);
        EXPECT_EQ(b4 - a4, b4);

        EXPECT_EQ(b2 - b2, a2);
        EXPECT_EQ(b3 - b3, a3);
        EXPECT_EQ(b4 - b4, a4);
    }
    TEST(BasicLinearAlgebra, Multiplication) {
        bla::vec2 a2(0), b2(1), c2(2);
        bla::vec3 a3(0), b3(1), c3(2);
        bla::vec4 a4(0), b4(1), c4(2);

        EXPECT_EQ(a2 * b2, a2);
        EXPECT_EQ(a3 * b3, a3);
        EXPECT_EQ(a4 * b4, a4);

        EXPECT_EQ(b2 * b2, b2);
        EXPECT_EQ(b3 * b3, b3);
        EXPECT_EQ(b4 * b4, b4);

        EXPECT_EQ(b2 * c2, c2);
        EXPECT_EQ(b3 * c3, c3);
        EXPECT_EQ(b4 * c4, c4);
    }
    TEST(BasicLinearAlgebra, Division) {
        bla::vec2 a2(0), b2(1), c2(2);
        bla::vec3 a3(0), b3(1), c3(2);
        bla::vec4 a4(0), b4(1), c4(2);

        EXPECT_EQ(c2 / 1, c2);
        EXPECT_EQ(c3 / 1, c3);
        EXPECT_EQ(c4 / 1, c4);

        EXPECT_EQ(b2 / 1, b2);
        EXPECT_EQ(b3 / 1, b3);
        EXPECT_EQ(b4 / 1, b4);

        EXPECT_EQ(c2 / 2, b2);
        EXPECT_EQ(c3 / 2, b3);
        EXPECT_EQ(c4 / 2, b4);
    }
    TEST(BasicLinearAlgebra, Pow) {
        bla::vec2 a2(0), b2(1), c2(2);
        bla::vec3 a3(0), b3(1), c3(2);
        bla::vec4 a4(0), b4(1), c4(2);

        EXPECT_EQ(b2 ^ 2, b2);
        EXPECT_EQ(b3 ^ 2, b3);
        EXPECT_EQ(b4 ^ 2, b4);

        EXPECT_EQ(b2 ^ 0, b2);
        EXPECT_EQ(b3 ^ 0, b3);
        EXPECT_EQ(b4 ^ 0, b4);

        EXPECT_EQ(c4 ^ c4, bla::vec4(4));

        EXPECT_EQ(a2 ^ 0, b2);
        EXPECT_EQ(a3 ^ 0, b3);
        EXPECT_EQ(a4 ^ 0, b4);

        EXPECT_EQ(c2 ^ 0, b2);
        EXPECT_EQ(c3 ^ 0, b3);
        EXPECT_EQ(c4 ^ 0, b4);

        EXPECT_EQ(c2 ^ 1, c2);
        EXPECT_EQ(c3 ^ 1, c3);
        EXPECT_EQ(c4 ^ 1, c4);

        EXPECT_EQ(c2 ^ 2, bla::vec2(4));
        EXPECT_EQ(c3 ^ 2, bla::vec3(4));
        EXPECT_EQ(c4 ^ 2, bla::vec4(4));
    }

    TEST(BasicLinearAlgebra, Dot) {
        bla::vec2 a2(0), b2(1), c2(2);
        bla::vec3 a3(0), b3(1), c3(2);
        bla::vec4 a4(0), b4(1), c4(2);

        EXPECT_EQ(bla::dot(b2, b2), 2);
        EXPECT_EQ(bla::dot(b3, b3), 3);
        EXPECT_EQ(bla::dot(b4, b4), 4);

        EXPECT_EQ(bla::dot(b2, c2), 4);
        EXPECT_EQ(bla::dot(b3, c3), 6);
        EXPECT_EQ(bla::dot(b4, c4), 8);

        EXPECT_EQ(bla::dot(c2, c2), 8);
        EXPECT_EQ(bla::dot(c3, c3), 12);
        EXPECT_EQ(bla::dot(c4, c4), 16);
    }

    TEST(BasicLinearAlgebra, Broadcast) {
        bla::vec2 a2(0), b2(1), c2(2);
        bla::vec3 a3(0), b3(1), c3(2);
        bla::vec4 a4(0), b4(1), c4(2);
        auto fun = [](float a) { return a * 2; };

        EXPECT_EQ(a2.apply_fun(fun), bla::vec2(0));
        EXPECT_EQ(b4.apply_fun(fun), bla::vec4(2));

        EXPECT_EQ(c4.apply_fun(fun), bla::vec4(4));
    }
}