#include <gtest/gtest.h>
#include "LinAlg.h"
#include <random>
namespace bla {
    TEST(Linalg, Close) {
        double a = 1.0;
        double b = 1.0;
        double c = 1.0001;
        EXPECT_TRUE(close(a, b));
        EXPECT_FALSE(close(a, c));
        EXPECT_TRUE(close(a, c, 0.1));
    }
    TEST(Linalg, Min) {
        double a = 1.0;
        double c = 1.0001;
        EXPECT_EQ(a, min(a, c));
    }
    TEST(Linalg, MAX) {
        double a = 1.0;
        double c = 1.0001;
        EXPECT_EQ(c, max(a, c));
    }
    TEST(Vectors, Assignment) {
        bla::vec4 test({ 0,0,0,0 });
        test.x() = 5;
        test.y() -= 4;
        test[2] = 3;
        test[3] = 4;
        EXPECT_EQ(test, vec4({5,-4,3,4}));
    }
    TEST(Vectors, Addition) {
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
    TEST(Vectors, Subtraction) {
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
    TEST(Vectors, Multiplication) {
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
    TEST(Vectors, Division) {
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
    TEST(Vectors, Pow) {
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

    TEST(Vectors, Dot) {
        bla::vec2 a2(0), b2(1), c2(2);
        bla::vec3 a3(0), b3(1), c3(2);
        bla::vec4 a4(0), b4(1), c4(2);

        EXPECT_EQ(dot(b2, b2), 2);
        EXPECT_EQ(dot(b3, b3), 3);
        EXPECT_EQ(dot(b4, b4), 4);

        EXPECT_EQ(dot(b2, c2), 4);
        EXPECT_EQ(dot(b3, c3), 6);
        EXPECT_EQ(dot(b4, c4), 8);

        EXPECT_EQ(dot(c2, c2), 8);
        EXPECT_EQ(dot(c3, c3), 12);
        EXPECT_EQ(dot(c4, c4), 16);

        EXPECT_EQ(b2.dot( b2), 2);
        EXPECT_EQ(b3.dot( b3), 3);
        EXPECT_EQ(b4.dot( b4), 4);

        EXPECT_EQ(b2.dot( c2), 4);
        EXPECT_EQ(b3.dot( c3), 6);
        EXPECT_EQ(b4.dot( c4), 8);

        EXPECT_EQ(c2.dot( c2), 8);
        EXPECT_EQ(c3.dot( c3), 12);
        EXPECT_EQ(c4.dot( c4), 16);
    }

    TEST(Vectors, Broadcast) {
        bla::vec2 a2(0), b2(1), c2(2);
        bla::vec3 a3(0), b3(1), c3(2);
        bla::vec4 a4(0), b4(1), c4(2);
        auto fun = [](float a) { return a * 2; };

        EXPECT_EQ(a2.apply_fun(fun), bla::vec2(0));
        EXPECT_EQ(b4.apply_fun(fun), bla::vec4(2));

        EXPECT_EQ(c4.apply_fun(fun), bla::vec4(4));
    }

    TEST(Vectors, Cross) {
        bla::vec3 a({ 0, 0, 1 }), b({ 1, 0, 0 }), c({0, 1, 0});

        EXPECT_EQ(cross(a, b), c);
        EXPECT_EQ(cross(c, a), b);
        EXPECT_EQ(cross(b, c), a);

        EXPECT_EQ(cross(b, a), -c);
        EXPECT_EQ(cross(a, c), -b);
        EXPECT_EQ(cross(c, b), -a);
    }
    TEST(Quaternions, Dot) {
        quat a, b;
        EXPECT_EQ(dot(a,b), 0);
        EXPECT_EQ(a.dot(b), 0);
    }
    TEST(Quaternions, Conversion) {
        quat a, b;
        mat33 t(a);
        mat44 t2(a);
    }
    TEST(Quaternions, VectorRotation) {
        quat a;
        vec3 t;
        vec4 b;
        t = a * t;
        b = a * b;
    }
    TEST(Quaternions, EulerAnglesBasic) {
        vec3 angles({90, 0, 0});
        quat q(angles);
        vec3 angles2 = q.to_euler_angles();
        EXPECT_TRUE(close(angles, angles2));
    }
    TEST(Quaternions, EulerAnglesExtensive) {
        /* Rotations aren't unique need comparison function
        static std::default_random_engine rng;

        std::uniform_real_distribution<float> dist(0.0, 360.0);

        for (int i = 0; i < 1000; i++) {
            vec3 angles({ dist(rng), 0, 0});
            quat q(angles);
            vec3 angles2 = q.to_euler_angles();
            for (int j = 0; j < 3; j++) {
                if (angles2[j] < 0) {
                    angles2[j] += 360;
                }
            }
            EXPECT_TRUE(close(angles, angles2, 0.001f)) << angles.to_string() << " not equal to " << angles2.to_string();
        }
        for (int i = 0; i < 1000; i++) {
            vec3 angles({ 0, dist(rng), 0 });
            quat q(angles);
            vec3 angles2 = q.to_euler_angles();
            for (int j = 0; j < 3; j++) {
                if (angles2[j] < 0) {
                    angles2[j] += 360;
                }
            }
            EXPECT_TRUE(close(angles, angles2, 0.001f)) << angles.to_string() << " not equal to " << angles2.to_string();
        }
        for (int i = 0; i < 1000; i++) {
            vec3 angles({ 0, 0, dist(rng) });
            quat q(angles);
            vec3 angles2 = q.to_euler_angles();
            for (int j = 0; j < 3; j++) {
                if (angles2[j] < 0) {
                    angles2[j] += 360;
                }
            }
            EXPECT_TRUE(close(angles, angles2, 0.001f)) << angles.to_string() << " not equal to " << angles2.to_string();
        }
        */
    }
    TEST(Quaternions, VectorRotations) {
        vec3 position({ 1,0,0 });
        quat q(vec3{0,0,90});
        vec3 result = q * position;
        vec3 expected_result({0, 1, 0});
        EXPECT_TRUE(close(result, expected_result)) << result.to_string() << " not equal to " << expected_result.to_string();

        q = quat(vec3{ 0,90,0 });
        result = q * position;
        expected_result = vec3({ 0, 0, -1 });
        EXPECT_TRUE(close(result, expected_result)) << result.to_string() << " not equal to " << expected_result.to_string();
    }
}