#include <gtest/gtest.h>
#include "LinAlg/LinAlg.h"

TEST(BasicLinearAlgebra, Addition) {
    bla::vec2 a2({ 0,0 }), b2({ 1, 1 });
    EXPECT_EQ(a2 + b2 , b2);
    bla::vec3 a3({ 0,0, 0 }), b3({ 1, 1, 1 });
    EXPECT_EQ(a3 + b3, b3);
    bla::vec4 a4({ 0,0, 0, 0 }), b4({ 1, 1, 1, 1 });
    EXPECT_EQ(a4 + b4, b4);
}
TEST(BasicLinearAlgebra, Subtraction) {
    bla::vec2 a2({ 0,0 }), b2({ 1, 1 });
    EXPECT_EQ(b2 - a2, b2);
    bla::vec3 a3({ 0,0, 0 }), b3({ 1, 1, 1 });
    EXPECT_EQ(b3 - a3, b3);
    bla::vec4 a4({ 0,0, 0 , 0}), b4({ 1, 1, 1, 1 });
    EXPECT_EQ(b4 - a4, b4);

    EXPECT_EQ(b2 - b2, a2);
    EXPECT_EQ(b3 - b3, a3);
    EXPECT_EQ(b4 - b4, a4);
}