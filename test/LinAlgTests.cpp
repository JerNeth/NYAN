#include <gtest/gtest.h>
#include "LinAlg.h"
#include <random>
namespace Math {
    TEST(Linalg, Close) {
        double a = 1.0;
        double b = 1.0;
        double c = 1.0001;
        double d = -1.0;
        double e = -1.0001;
        EXPECT_TRUE(close(a, b));
        EXPECT_FALSE(close(a, c));
        EXPECT_TRUE(close(a, c, 0.1));
        EXPECT_FALSE(close(d, e));
        EXPECT_TRUE(close(d, e, 0.1));
        
    }
    TEST(Linalg, OperatorConversion) {
        std::array<float, 3> t;
        t = Math::vec3{};
        Math::vec3 v;
        auto [x, y, z] = t;
        x = 1;
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
        Math::vec4 test({ 0,0,0,0 });
        test.x() = 5;
        test.y() -= 4;
        test[2] = 3;
        test[3] = 4;
        EXPECT_EQ(test, vec4({5,-4,3,4}));
    }
    TEST(Vectors, Addition) {
       
        Math::vec2 a2(0), b2(1), c2(2);
        Math::vec3 a3(0), b3(1), c3(2);
        Math::vec4 a4(0), b4(1), c4(2);
        EXPECT_EQ(a2 + b2, b2);
        EXPECT_EQ(a3 + b3, b3);
        EXPECT_EQ(a4 + b4, b4);

        EXPECT_EQ(b2 + b2, c2);
        EXPECT_EQ(b3 + b3, c3);
        EXPECT_EQ(b4 + b4, c4);
    }
    TEST(Vectors, Subtraction) {
        Math::vec2 a2(0), b2(1), c2(2);
        Math::vec3 a3(0), b3(1), c3(2);
        Math::vec4 a4(0), b4(1), c4(2);

        EXPECT_EQ(b2 - a2, b2);
        EXPECT_EQ(b3 - a3, b3);
        EXPECT_EQ(b4 - a4, b4);

        EXPECT_EQ(b2 - b2, a2);
        EXPECT_EQ(b3 - b3, a3);
        EXPECT_EQ(b4 - b4, a4);
    }
    TEST(Vectors, Multiplication) {
        Math::vec2 a2(0), b2(1), c2(2);
        Math::vec3 a3(0), b3(1), c3(2);
        Math::vec4 a4(0), b4(1), c4(2);

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
        Math::vec2 a2(0), b2(1), c2(2);
        Math::vec3 a3(0), b3(1), c3(2);
        Math::vec4 a4(0), b4(1), c4(2);

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
        Math::vec2 a2(0), b2(1), c2(2);
        Math::vec3 a3(0), b3(1), c3(2);
        Math::vec4 a4(0), b4(1), c4(2);

        EXPECT_EQ(b2 ^ 2, b2);
        EXPECT_EQ(b3 ^ 2, b3);
        EXPECT_EQ(b4 ^ 2, b4);

        EXPECT_EQ(b2 ^ 0, b2);
        EXPECT_EQ(b3 ^ 0, b3);
        EXPECT_EQ(b4 ^ 0, b4);

        EXPECT_EQ(c4 ^ c4, Math::vec4(4));

        EXPECT_EQ(a2 ^ 0, b2);
        EXPECT_EQ(a3 ^ 0, b3);
        EXPECT_EQ(a4 ^ 0, b4);

        EXPECT_EQ(c2 ^ 0, b2);
        EXPECT_EQ(c3 ^ 0, b3);
        EXPECT_EQ(c4 ^ 0, b4);

        EXPECT_EQ(c2 ^ 1, c2);
        EXPECT_EQ(c3 ^ 1, c3);
        EXPECT_EQ(c4 ^ 1, c4);

        EXPECT_EQ(c2 ^ 2, Math::vec2(4));
        EXPECT_EQ(c3 ^ 2, Math::vec3(4));
        EXPECT_EQ(c4 ^ 2, Math::vec4(4));
    }

    TEST(Vectors, Dot) {
        Math::vec2 a2(0), b2(1), c2(2);
        Math::vec3 a3(0), b3(1), c3(2);
        Math::vec4 a4(0), b4(1), c4(2);

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
    TEST(Vectors, Lerp) {
        Math::vec2 a2(0), b2(1), c2(0.5);
        Math::vec3 a3(0), b3(1), c3(0.5);
        Math::vec4 a4(0), b4(1), c4(0.5);
        float t = 0.5f;
        EXPECT_EQ(Math::lerp(a2, b2, t), c2);
        EXPECT_EQ(Math::lerp(a3, b3, t), c3);
        EXPECT_EQ(Math::lerp(a4, b4, t), c4);

    }
    TEST(Vectors, Broadcast) {
        Math::vec2 a2(0), b2(1), c2(2);
        Math::vec3 a3(0), b3(1), c3(2);
        Math::vec4 a4(0), b4(1), c4(2);
        auto fun = [](float a) { return a * 2; };

        EXPECT_EQ(a2.apply_fun(fun), Math::vec2(0));
        EXPECT_EQ(b4.apply_fun(fun), Math::vec4(2));

        EXPECT_EQ(c4.apply_fun(fun), Math::vec4(4));
    }

    TEST(Vectors, Cross) {
        Math::vec3 a({ 0, 0, 1 }), b({ 1, 0, 0 }), c({0, 1, 0});

        EXPECT_EQ(cross(a, b), c);
        EXPECT_EQ(cross(c, a), b);
        EXPECT_EQ(cross(b, c), a);

        EXPECT_EQ(cross(b, a), -c);
        EXPECT_EQ(cross(a, c), -b);
        EXPECT_EQ(cross(c, b), -a);
    }
    TEST(Vectors, Close) {
        Math::vec3 a({ 0, 0, 1 }), b({ 0, 0, 1 }), c({ 0, 0, 1.001f }), d({ 0, 0, -1.0f }), e({0, 0, -1.001f});

        EXPECT_TRUE(close(a, a));
        EXPECT_TRUE(close(a, b));
        EXPECT_FALSE(close(a, c));
        EXPECT_TRUE(close(a, c, 0.01f));
        EXPECT_TRUE(close(d, d));
        EXPECT_FALSE(close(d, e));
        EXPECT_TRUE(close(d, e, 0.01f));
    }
    TEST(Matrices, Basic) {
        mat44 t;
        //EXPECT_EQ(a.dot(b), 0);
    }
    TEST(Matrices, MatrixVectorMultiplication) {
        mat33 test({2, 0, 0,  0, 1, 0, 0, 0, 1});
        vec3 testv({1,1,1});

        EXPECT_TRUE(close(test * testv, vec3({2,1,1})));

        mat33 test2({ 2, 2, 2,  0, 0, 0, 0, 0, 0 });
        vec3 testv2({ 1,1,1 });

        EXPECT_TRUE(close(test2 * testv2, vec3({ 6,0,0 })));

        mat44 t44 = mat44::identity();
        mat33 t33 = mat33::identity();
        static std::default_random_engine rng;

        std::uniform_real_distribution<float> dist_vec(-100000.0, 100000.0);

        for (int i = 0; i < 10000; i++) {
            vec4 a({ dist_vec(rng), dist_vec(rng), dist_vec(rng), dist_vec(rng) });
            vec4 result1 = t44 * a;

            vec3 b({ dist_vec(rng), dist_vec(rng), dist_vec(rng) });
            vec3 result2 = t33 * b;

            ASSERT_TRUE(close(a, result1, 0.0001f)) << a.convert_to_string() << " not close to " << result1.convert_to_string() << " at: i="<< i;
            ASSERT_TRUE(close(b, result2, 0.0001f)) << b.convert_to_string() << " not close to " << result2.convert_to_string() << " at: i=" << i;
        }
        //EXPECT_EQ(a.dot(b), 0);
    }
    TEST(Matrices, transpose) {

        static std::default_random_engine rng;

        std::uniform_real_distribution<float> dist_vec(-100000.0, 100000.0);

        for (int i = 0; i < 10000; i++) {
            mat44 t({dist_vec(rng), dist_vec(rng) , dist_vec(rng) ,dist_vec(rng),
                dist_vec(rng), dist_vec(rng) , dist_vec(rng) ,dist_vec(rng), 
                dist_vec(rng), dist_vec(rng) , dist_vec(rng) ,dist_vec(rng), 
                dist_vec(rng), dist_vec(rng) , dist_vec(rng) ,dist_vec(rng), });
            mat44 t_t = t.transpose();
            ASSERT_TRUE(close(t, t.transpose().transpose()));
            ASSERT_TRUE(close(t, t_t.transposed()));
        }
        //EXPECT_EQ(a.dot(b), 0);
    }
    TEST(Matrices, multiplication) {

        mat33 a({1, 0, 0,
                 0, 1, 0,
                 0, 1, 0});
        mat33 b({ 1, 0, 1,
                 0, 1, 0,
                 0, 0, 0 });
        mat33 c({ 1, 0, 1,
                 0, 1, 0,
                 0, 1, 0 });
        EXPECT_EQ(a*b, c);

        mat33 d({ 2, 0, 2,
                 0, 2, 0,
                 0, 2, 0 });
        mat33 e({ 1, 0, 1,
                 0, 1, 0,
                 1, 0, 0 });
        mat33 f({ 4, 0, 2,
                 0, 2, 0,
                 0, 2, 0 });
        mat33 g({ 2, 2, 2,
                 0, 2, 0,
                 2, 0, 2 });
        EXPECT_EQ(d * e, f);
        EXPECT_EQ(e * d, g);
    }
    TEST(Matrices, rotationMatrices) {
        static std::default_random_engine rng;

        std::uniform_real_distribution<float> dist_rot(0.0, 360.0);
        std::uniform_real_distribution<float> dist_rot_half(0.0, 180.0);
        std::uniform_real_distribution<float> dist_pos(-100.0, 100.0);
        int counter1_2 = 0;
        for (int i = 0; i < 100000; i++) {
            vec3 angles({ dist_rot(rng), dist_rot_half(rng), dist_rot(rng) });
            double yaw = angles[2], pitch = angles[1], roll = angles[0];
            float cy = (float)cos(yaw * deg_to_rad);
            float sy = (float)sin(yaw * deg_to_rad);
            float cp = (float)cos(pitch * deg_to_rad);
            float sp = (float)sin(pitch * deg_to_rad);
            float cr = (float)cos(roll * deg_to_rad);
            float sr = (float)sin(roll * deg_to_rad);
            mat33 roll_m({ 1, 0, 0,
                                        0, cr, -sr,
                                        0, sr, cr });
            mat33  pitch_m({ cp, 0, sp,
                                    0, 1, 0,
                                    -sp, 0, cp });
            mat33  yaw_m({ cy, -sy, 0,
                                    sy, cy, 0,
                                    0, 0, 1 });
            mat33 rot = (yaw_m * pitch_m) * roll_m;

            vec3 pos({ dist_pos(rng), dist_pos(rng) ,dist_pos(rng) });
            vec3 result1 = mat33::rotation_matrix(angles) * pos;
            vec3 result2 = rot * pos;
           
            if (!close(result1, result2, 0.01f))
                counter1_2++;
        }
        EXPECT_EQ(counter1_2, 0);
        
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
        ///* Rotations aren't unique need comparison function
        static std::default_random_engine rng;

        std::uniform_real_distribution<float> dist(0.0, 360.0);
        std::uniform_real_distribution<float> dist2(0.0, 179.0);


        for (int i = 0; i < 1000; i++) {
            vec3 angles({ dist(rng), 0, 0});
            quat q(angles);
            vec3 angles2 = q.to_euler_angles();
            for (int j = 0; j < 3; j++) {
                if (angles2[j] < 0) {
                    angles2[j] += 360;
                }
            }
            //EXPECT_TRUE(close(angles, angles2, 0.001f)) << angles.convert_to_string() << " not equal to " << angles2.convert_to_string();
        }
        for (int i = 0; i < 1000; i++) {
            vec3 angles({ 0, dist2(rng), 0 });
            quat q(angles);
            vec3 angles2 = q.to_euler_angles();
            for (int j = 0; j < 3; j++) {
                if (angles2[j] < 0) {
                    angles2[j] += 360;
                }
            }
            //EXPECT_TRUE(close(angles, angles2, 0.001f)) << angles.convert_to_string() << " not equal to " << angles2.convert_to_string();
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
            //EXPECT_TRUE(close(angles, angles2, 0.001f)) << angles.convert_to_string() << " not equal to " << angles2.convert_to_string();
        }
        //*/
    }
    TEST(Quaternions, VectorRotations) {
        vec3 position({ 1,0,0 });
        quat q(vec3({0,0,90}));
        vec3 result = q * position;
        vec3 expected_result({0, 1, 0});
        EXPECT_TRUE(close(result, expected_result)) << result.convert_to_string() << " not equal to " << expected_result.convert_to_string();

        q = quat(vec3({ 0,90,0 }));
        result = q * position;
        expected_result = vec3({ 0, 0, -1 });
        EXPECT_TRUE(close(result, expected_result)) << result.convert_to_string() << " not equal to " << expected_result.convert_to_string();
    }
    TEST(Quaternions, VectorRotationsAndMatrixRotationsBasic) {
        vec3 angles({ 0, 0, 90});
        quat q(angles);
        vec3 pos({ 1, 0, 0 });
        vec3 expected_result({ 0, 1, 0 });
        vec3 result1 = mat33(q) * pos;
        vec3 result2 = q * pos;
        vec3 result3 = mat33::rotation_matrix(angles) * pos;
        EXPECT_TRUE(close(result1, expected_result, 0.0001f)) << result1.convert_to_string() << " not equal to " << expected_result.convert_to_string() << "\n" << mat33(q).convert_to_string();
        EXPECT_TRUE(close(result2, expected_result, 0.0001f)) << result2.convert_to_string() << " not equal to " << expected_result.convert_to_string();
        EXPECT_TRUE(close(result3, expected_result, 0.0001f)) << result3.convert_to_string() << " not equal to " << expected_result.convert_to_string() << "\n" << mat33::rotation_matrix(angles).convert_to_string();

        angles = vec3({ 0.f,90.f,0.f });
        q = quat(angles);
        result1 = mat33(q) * pos;
        result2 = q * pos;
        expected_result = vec3({ 0, 0, -1 });
        result3 = mat33::rotation_matrix(angles) * pos;
        EXPECT_TRUE(close(result1, expected_result, 0.0001f)) << result1.convert_to_string() << " not equal to " << expected_result.convert_to_string() << "\n" << mat33(q).convert_to_string();
        EXPECT_TRUE(close(result2, expected_result, 0.0001f)) << result2.convert_to_string() << " not equal to " << expected_result.convert_to_string();
        EXPECT_TRUE(close(result3, expected_result, 0.0001f)) << result3.convert_to_string() << " not equal to " << expected_result.convert_to_string() << "\n" << mat33::rotation_matrix(angles).convert_to_string();

        angles = vec3({ 90,0,0 });
        pos = vec3({ 0, 1, 0 });
        q = quat(angles);
        result1 = mat33(q) * pos;
        result2 = q * pos;
        expected_result = vec3({ 0, 0, 1 });
        result3 = mat33::rotation_matrix(angles) * pos;
        EXPECT_TRUE(close(result1, expected_result, 0.0001f)) << result1.convert_to_string() << " not equal to " << expected_result.convert_to_string() << "\n" << mat33(q).convert_to_string();
        EXPECT_TRUE(close(result2, expected_result, 0.0001f)) << result2.convert_to_string() << " not equal to " << expected_result.convert_to_string();
        EXPECT_TRUE(close(result3, expected_result, 0.0001f)) << result3.convert_to_string() << " not equal to " << expected_result.convert_to_string() << "\n" << mat33::rotation_matrix(angles).convert_to_string();
    }
    TEST(Quaternions, VectorRotationsAndMatrixRotations) {
        static std::default_random_engine rng;

        std::uniform_real_distribution<float> dist_rot(0.0, 360.0);
        std::uniform_real_distribution<float> dist_rot_half(0.0, 180.0);
        std::uniform_real_distribution<float> dist_pos(-100.0, 100.0);
        int counter1_2 = 0;
        int counter2_3 = 0;
        int counter1_3 = 0;
        for (int i = 0; i < 100000; i++) {
            vec3 angles({ dist_rot(rng), dist_rot_half(rng), dist_rot(rng) });
            quat q(angles);
            vec3 pos({ dist_pos(rng), dist_pos(rng) ,dist_pos(rng) });
            vec3 result1 = mat33(q) * pos;
            vec3 result2 = q * pos;
            vec3 result3 = mat33::rotation_matrix(angles) * pos;
            //EXPECT_TRUE(close(result1.L2_norm(), result2.L2_norm())) << result1.L2_norm() << " != " << result2.L2_norm();
            //EXPECT_TRUE(close(result2.L2_norm(), result3.L2_norm())) << result2.L2_norm() << " != " << result3.L2_norm();
            //EXPECT_TRUE(close(result1, result2, 0.0001f));// << result1.convert_to_string() << " not equal to " << result2.convert_to_string() << " at: i=" << i << " pos: " << pos.convert_to_string() << " angles: " << angles.convert_to_string() << "\n" << mat33(q).convert_to_string();
            //EXPECT_TRUE(close(result2, result3, 0.0001f));// << result2.convert_to_string() << " not equal to " << result3.convert_to_string() << " at: i=" << i << " pos: " << pos.convert_to_string() << " angles: " << angles.convert_to_string() << "\n" << mat33::rotation_matrix(angles).convert_to_string() << "\n" << q.convert_to_string();
            //EXPECT_TRUE(close(result1, result3, 0.0001f));// << result1.convert_to_string() << " not equal to " << result3.convert_to_string() << " at: i=" << i << " pos: " << pos.convert_to_string() << " angles: " << angles.convert_to_string() << "\n" << mat33::rotation_matrix(angles).convert_to_string() << "\n" << mat33(q).convert_to_string();
            if (!close(result1, result2, 0.01f))
                counter1_2++;
            if (!close(result2, result3, 0.01f))
                counter2_3++;
            if (!close(result1, result3, 0.01f))
                counter1_3++;
        }
        EXPECT_EQ(counter1_2, 0);
        EXPECT_EQ(counter2_3, 0);
        EXPECT_EQ(counter1_3, 0);
    }
}