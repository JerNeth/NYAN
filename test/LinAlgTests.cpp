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
    TEST(Linalg, Half) {
        half a(3.141);
        EXPECT_EQ(a.data, 0x4248) << std::hex << a.data << " should be " << 0x4248;
        half b(std::numeric_limits<float>::quiet_NaN());
        EXPECT_EQ(b.data, 0x7E00) << std::hex << b.data << " should be " << 0x7E00;
        half c(std::numeric_limits<float>::infinity());
        EXPECT_EQ(c.data, 0x7C00) << std::hex << c.data << " should be " << 0x7C00;
        half d(-std::numeric_limits<float>::infinity());
        EXPECT_EQ(d.data, 0xFC00) << std::hex << d.data << " should be " << 0xFC00;
        half e(-65504);
        EXPECT_EQ(e.data, 0xFBFF) << std::hex << e.data << " should be " << 0xFBFF;
        half f(65504);
        EXPECT_EQ(f.data, 0x7BFF) << std::hex << f.data << " should be " << 0x7BFF;
    }
    TEST(Linalg, Half2) {
        for (int i = -1024; i < 1024; i++) {
            half a(i);
            float b = i;
            EXPECT_TRUE(close(float(a), b)) << float(a) << " should be " << b << "\n";
        }
    }
    TEST(Linalg, Half3) {
        for (float i = -1.0; i < 1.0; i += 0.0025) {
            half a(i);
            float b = i;
            EXPECT_TRUE(close(float(a), b, 5e-4f)) << float(a) << " should be " << b << "\n";
        }
    }
    TEST(Linalg, Half4) {
        for (int i = -65500; i < 65500; i += 10) {
            half a(i);
            float b = i;
            EXPECT_TRUE(close(float(a), b, 30.f)) << float(a) << " should be " << b << "\n";
        }
    }
    TEST(Linalg, HalfVecConversion) {
        vec3 b(0, 1, 3);
        hvec3 a{ b };
        EXPECT_TRUE(a== static_cast<hvec3>(b));
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
        Math::vec3 a({ 0, 0, 1 }), b({ 0, 0, 1 }), c({ 0, 0, 1.001f }), d({ 0, 0, -1.0f }), e({ 0, 0, -1.001f });

        EXPECT_TRUE(close(a, a));
        EXPECT_TRUE(close(a, b));
        EXPECT_FALSE(close(a, c));
        EXPECT_TRUE(close(a, c, 0.01f));
        EXPECT_TRUE(close(d, d));
        EXPECT_FALSE(close(d, e));
        EXPECT_TRUE(close(d, e, 0.01f));
    }
    Math::vec2 mix(Math::vec2 x, Math::vec2 y, Math::vec2 a) {
        return x * (Math::vec2(1.f) - a) + a * y;
    }
    Math::vec2 step(float step, float a) {
        if (a < step)
            return Math::vec2{ 0.f };
        else
            return Math::vec2{ 1.f };
    }

    Math::vec2 octahedral(Math::vec3 n) {
        n *= 1.f / Math::vec3(abs(n[0]), abs(n[1]), abs(n[2])).dot(Math::vec3{ 1.f });
        if (n.z() < 0) {
        }
        Math::vec2 tmp{ abs(n.y()), abs(n.x()) };
        tmp = 1.0f - tmp;
        if (n.x() < 0)
            tmp.x() *= -1;
        if (n.y() < 0)
            tmp.y() *= -1;
        //n.x() = tmp.x();
        //n.y() = tmp.y();
        
        //return n.xy();
        return mix( tmp, n.xy(), step(0.0f, n.z()));
    }
    Math::vec3 deoctahedral(Math::vec2 n) {
        Math::vec3 res{n[0], n[1], 1.f - abs(n[0]) - abs(n[1])};
        //if (res[2] < 0) {
        //    Math::vec2 tmp{ abs(res.y()), abs(res.x()) };
        //    tmp = 1.0f - tmp;
        //    if (n.x() < 0)
        //        tmp.x() *= -1;
        //    if (n.y() < 0)
        //        tmp.y() *= -1;
        //    res[0] = tmp[0];
        //    res[1] = tmp[1];
        //}
        Math::vec2 tmp{ abs(res.y()), abs(res.x()) };
        tmp = 1.0f - tmp;
        if (n.x() < 0)
            tmp.x() *= -1;
        if (n.y() < 0)
            tmp.y() *= -1;
        //res[0] = tmp[0];
        //res[1] = tmp[1];
        auto tmp2 = mix(tmp, res.xy(), step(0.0f, res.z()));
        res[0] = tmp2[0];
        res[1] = tmp2[1];
        return res.normalized();
    }

    TEST(Vectors, Octahedral) {
        std::array as{ Math::vec3{0, 0, 1}, Math::vec3{0, 0, 1}, Math::vec3{0, 0, 1.f}
        , Math::vec3{0.16, 0.02f, -0.6f}
        , Math::vec3{0, 0.01f, -0.99f} 
        , Math::vec3{1, 0.01f, 0.f} 
        , Math::vec3{0.7, 0.5f, -0.1f} 
        , Math::vec3{-0.4, -0.01f, -0.99f} 
        , Math::vec3{-0.5, 0.01f, -0.99f}
        , Math::vec3{0.5, 0.4f, 0.2f}
        , Math::vec3{0.2, 0.6f, 0.4f}
        , Math::vec3{0.1, 0.2f, 0.6f} };
        for (auto& a : as)
            a.normalize();

        for(const auto& a : as )
            EXPECT_TRUE(close(a, deoctahedral(octahedral(a)), 1e-5)) << a.convert_to_string() << "->" << octahedral(a).convert_to_string() << "->" << deoctahedral(octahedral(a)).convert_to_string() << "\n";
    }


    TEST(Matrices, ViewMatrix) {
        Math::vec3 a({ 0, 0, 1 }), b({ 0, 0, 1 }), c({ 0, 0, 1.001f }), d({ 0, 0, -1.0f }), e({ 0, 0, -1.001f });

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
            mat44 t({ dist_vec(rng), dist_vec(rng) , dist_vec(rng) ,dist_vec(rng),
                dist_vec(rng), dist_vec(rng) , dist_vec(rng) ,dist_vec(rng),
                dist_vec(rng), dist_vec(rng) , dist_vec(rng) ,dist_vec(rng),
                dist_vec(rng), dist_vec(rng) , dist_vec(rng) ,dist_vec(rng), });
            mat44 t_t = t.transpose();
            ASSERT_TRUE(close(t, t.transpose().transpose()));
            ASSERT_TRUE(close(t, t_t.transposed()));
        }
        //EXPECT_EQ(a.dot(b), 0);
    }
    TEST(Matrices, inverse) {

        static std::default_random_engine rng;

        std::uniform_real_distribution<float> dist_vec(-100000.0, 100000.0);
        auto counter = 0;
        for (int i = 0; i < 10000; i++) {
            mat44 t({ dist_vec(rng), dist_vec(rng) , dist_vec(rng) ,dist_vec(rng),
                dist_vec(rng), dist_vec(rng) , dist_vec(rng) ,dist_vec(rng),
                dist_vec(rng), dist_vec(rng) , dist_vec(rng) ,dist_vec(rng),
                dist_vec(rng), dist_vec(rng) , dist_vec(rng) ,dist_vec(rng), });
            float determinante;
            mat44 t_t;
            if (!t.inverse(t_t)) {
                counter++;
                continue;
            }
            ASSERT_TRUE(close(mat44::identity(), t * t_t, 1e-2f)) << t.convert_to_string() << "\n" << t_t.convert_to_string() << "\n" << (t * t_t).convert_to_string() << " should be:\n" << mat44::identity().convert_to_string() << "\n";
            ASSERT_TRUE(close(mat44::identity(),  t_t *t, 1e-2f));
        }
        std::cout << "Non invertible matrices: " << counter << "\n";
        //EXPECT_EQ(a.dot(b), 0);
    }
    TEST(Matrices, inverse2) {


        mat44 t(1.f, 1.f, 1.f, -1.f,
            1.f, 1.f, -1.f, 1.f,
            1.f, -1.f, 1.f, 1.f,
            -1.f, 1.f, 1.f, 1.f);
        mat44 t_t;
        if (!t.inverse(t_t)) {
        }
        ASSERT_TRUE(close(mat44::identity(), t * t_t)) << t.convert_to_string() << "\n" << t_t.convert_to_string() << "\n" << (t * t_t).convert_to_string() << " should be:\n" << mat44::identity().convert_to_string() << "\n";
        ASSERT_TRUE(close(mat44::identity(), t_t * t));

        //EXPECT_EQ(a.dot(b), 0);
    }
    TEST(Matrices, inverseRotation) {

        static std::default_random_engine rng;
        std::uniform_real_distribution<float> dist_rot(0.0, 360.0);
        std::uniform_real_distribution<float> dist_rot_half(0.0, 180.0);
        std::uniform_real_distribution<float> dist_pos(-100.0, 100.0);
        int counter1_2 = 0;
        int counter1_1 = 0;
        for (int i = 0; i < 100000; i++) {
            vec3 angles({ dist_rot(rng), dist_rot_half(rng), dist_rot(rng) });
            vec3 pos({ dist_pos(rng), dist_pos(rng) ,dist_pos(rng) });
            auto mat = mat44::affine_transformation_matrix(angles, pos);
            mat44 matInverse;
            if (!mat.inverse(matInverse)) {
                counter1_1++;
                continue;
            }
            auto rotMat = mat33::rotation_matrix(angles).transpose();
            mat44 mat2 = mat44(rotMat);
            mat2(3, 3) = 1;
            mat2.set_col(rotMat * -pos, 3ull);
            auto inv2 = mat.inverse_affine_transformation_matrix();
            ASSERT_TRUE(close(mat2, matInverse, 1e-3f)) << matInverse.convert_to_string() << " should be:\n" << mat2.convert_to_string() << "\n";
            ASSERT_TRUE(close(mat2, inv2, 1e-3f)) << inv2.convert_to_string() << " should be:\n" << mat2.convert_to_string() << "\n";

        }
        for (int i = 0; i < 100000; i++) {
            vec3 angles({ dist_rot(rng), dist_rot_half(rng), dist_rot(rng) });
            vec3 pos({ dist_pos(rng), dist_pos(rng) ,dist_pos(rng) });
            auto mat = mat34::affine_transformation_matrix(angles, pos);
            auto rotMat = mat33::rotation_matrix(angles).transpose();
            mat34 mat2 = mat34(rotMat);
            mat2.set_col(rotMat * -pos, 3ull);
            const auto inv2 = mat.inverse_affine_transformation_matrix();
            ASSERT_TRUE(close(mat2, static_cast<mat34>(inv2), 1e-3f)) << inv2.convert_to_string() << " should be:\n" << mat2.convert_to_string() << "\n";

        }
        EXPECT_EQ(counter1_1, 0) << counter1_1 << " trivially inversible matrices should have been inverted\n";
        EXPECT_EQ(counter1_2, 0);

        //EXPECT_EQ(a.dot(b), 0);
    }
    //TEST(Matrices, rowMajor) {

    //    Math::Mat<float, 3, 3, false> a(1, 1, 1,
    //        0, 0, 0,
    //        0, 0, 0
    //    );
    //    std::array<float, 9> b{ 1, 1, 1, 0, 0,0 ,0,0 ,0 };
    //    bool res = true;
    //    for (size_t i = 0; i < b.size(); i++) {
    //        res &= b[i] == static_cast<float>(a[i]);
    //    }

    //    EXPECT_TRUE(res);
    //    //EXPECT_EQ(a.dot(b), 0);
    //}
    //TEST(Matrices, colMajor) {

    //    Math::Mat<float, 3, 3, true> a(1, 1, 1,
    //        0, 0, 0,
    //        0, 0, 0
    //    );
    //    std::array<float, 9> b{ 1, 0, 0, 1, 0,0 ,1,0 ,0 };
    //    bool res = true;
    //    for (size_t i = 0; i < b.size(); i++) {
    //        res &= b[i] == a[i];
    //    }

    //    EXPECT_TRUE(res);
    //    //EXPECT_EQ(a.dot(b), 0);
    //}
    TEST(Matrices, perspectiveInverseDepth) {
        auto near = 0.1f;
        auto far = 10000.f;
        auto fov = 90.f;
        auto aspect = 16.f / 9.f;
        auto mat = mat44::perspectiveInverseDepthFovXLH(near, far, fov, aspect);

        {
            auto nearVec = vec4(0, 0, -near, 1);
            auto farVec = vec4(0, 0, -far, 1);
            auto result0 = vec4(0, 0, 1, 1);
            auto result1 = vec4(0, 0, 0, 1);
            nearVec = mat * nearVec;
            nearVec /= nearVec.w();
            farVec = mat * farVec;
            farVec /= farVec.w();
            EXPECT_EQ(nearVec, result0);
            EXPECT_EQ(farVec, result1);
        }
        {
            auto nearVecLeft = vec4(-tan(fov * 0.5 * Math::deg_to_rad) * near, 0, -near, 1);
            auto nearVecRight = vec4(tan(fov * 0.5 * Math::deg_to_rad) * near, 0, -near, 1);
            auto result0 = vec4(-1, 0, 1, 1);
            auto result1 = vec4(1, 0, 1, 1);
            nearVecLeft = mat * nearVecLeft;
            nearVecLeft /= nearVecLeft.w();
            nearVecRight = mat * nearVecRight;
            nearVecRight /= nearVecRight.w();
            EXPECT_EQ(nearVecLeft, result0);
            EXPECT_EQ(nearVecRight, result1);
        }
        {
            auto nearVecTop = vec4(0, tan(fov * 0.5 * Math::deg_to_rad) * near / aspect, -near, 1);
            auto nearVecBot = vec4(0, -tan(fov * 0.5 * Math::deg_to_rad) * near / aspect, -near, 1);
            auto result0 = vec4(0, -1, 1, 1);
            auto result1 = vec4(0, 1, 1, 1);
            nearVecTop = mat * nearVecTop;
            nearVecTop /= nearVecTop.w();
            nearVecBot = mat * nearVecBot;
            nearVecBot /= nearVecBot.w();
            EXPECT_EQ(nearVecTop, result0);
            EXPECT_EQ(nearVecBot, result1);
        }
    }
    TEST(Matrices, perspectiveFov) {
        auto near = 0.1f;
        auto far = 10000.f;
        auto fov = 90.f;
        auto aspect = 16.f / 9.f;
        auto mat = mat44::perspectiveFovXLH(near, far, fov, aspect);

        {
            auto nearVec = vec4(0, 0, -near, 1);
            auto farVec = vec4(0, 0, -far, 1);
            auto result0 = vec4(0, 0, 0, 1);
            auto result1 = vec4(0, 0, 1, 1);
            nearVec = mat * nearVec;
            nearVec /= nearVec.w();
            farVec = mat * farVec;
            farVec /= farVec.w();
            EXPECT_TRUE(close(nearVec, result0));
            EXPECT_TRUE(close(farVec, result1));
        }
        {
            auto nearVecLeft = vec4(-tan(fov * 0.5 * Math::deg_to_rad) * near, 0, -near, 1);
            auto nearVecRight = vec4(tan(fov * 0.5 * Math::deg_to_rad) * near, 0, -near, 1);
            auto result0 = vec4(-1, 0, 0, 1);
            auto result1 = vec4(1, 0, 0, 1);
            nearVecLeft = mat * nearVecLeft;
            nearVecLeft /= nearVecLeft.w();
            nearVecRight = mat * nearVecRight;
            nearVecRight /= nearVecRight.w();
            EXPECT_TRUE(close(nearVecLeft, result0));
            EXPECT_TRUE(close(nearVecRight, result1));
        }
        {
            auto nearVecTop = vec4(0, tan(fov * 0.5 * Math::deg_to_rad) * near / aspect, -near, 1);
            auto nearVecBot = vec4(0, -tan(fov * 0.5 * Math::deg_to_rad) * near / aspect, -near, 1);
            auto result0 = vec4(0, -1, 0, 1);
            auto result1 = vec4(0, 1, 0, 1);
            nearVecTop = mat * nearVecTop;
            nearVecTop /= nearVecTop.w();
            nearVecBot = mat * nearVecBot;
            nearVecBot /= nearVecBot.w();
            EXPECT_TRUE(close(nearVecTop, result0));
            EXPECT_TRUE(close(nearVecBot, result1));
        }
    }
    TEST(Matrices, multiplication) {

        mat33 a({ 1, 0, 0,
                 0, 1, 0,
                 0, 1, 0 });
        mat33 b({ 1, 0, 1,
                 0, 1, 0,
                 0, 0, 0 });
        mat33 c({ 1, 0, 1,
                 0, 1, 0,
                 0, 1, 0 });
        auto res = a * b;
        EXPECT_TRUE(close(res, c)) << res.convert_to_string() << " not equal to:\n" << c.convert_to_string() << "\n";

        {
            mat33 a({ 1, 0, 0,
                     0, 1, 0,
                     0, 0, 1 });
            auto res = a * a;
            EXPECT_TRUE(close(res, a)) << res.convert_to_string() << " not equal to:\n" << a.convert_to_string() << "\n";
        }
        {
            mat33 a({ 1, 0, 0,
                     0, 1, 0,
                     0, 0, 1 });
            mat33 b({ 1, 1, 5,
                     3, 1, 2,
                     4, 2, 1 });
            auto res = a * b;
            EXPECT_TRUE(close(res, b)) << res.convert_to_string() << " not equal to:\n" << b.convert_to_string() << "\n";
        }
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
        res = d * e;
        EXPECT_TRUE(close(res, f)) << res.convert_to_string() << " not equal to:\n" << f.convert_to_string() << "\n";
        res = e * d;
        EXPECT_TRUE(close(res, g)) << res.convert_to_string() << " not equal to:\n" << g.convert_to_string() << "\n";
        {

            mat34 d({ 2, 0, 2, 0,
                     0, 2, 0, 0,
                     0, 2, 0, 0});
            mat43 e({ 1, 0, 1,
                     0, 1, 0,
                     1, 0, 0,
                     0, 0, 0});
            mat33 r({ 4, 0, 2,
                     0, 2, 0,
                     0, 2, 0 });
            mat44 r2({ 2, 2, 2, 0,
                     0, 2, 0, 0,
                     2, 0, 2, 0,
                     0, 0, 0, 0});
            auto res = d * e;
            auto res2 = e * d;
            EXPECT_TRUE(close(res, r)) << res.convert_to_string() << " should be:\n" << r.convert_to_string() << "\n";
            EXPECT_TRUE(close(res2, r2)) << res2.convert_to_string() << " should be:\n" << r2.convert_to_string() << "\n";
        }
        {

            Mat<float, 2, 4> d({ 2, 0, 2, 0,
                                0, 2, 0, 0 });
            Mat<float, 4, 2> e({ 1, 0,
                                0, 1,
                                1, 0,
                                0, 0, });
            mat22 r(4, 0,
                0, 2);
            mat44 r2({ 2, 0, 2, 0,
                     0, 2, 0, 0,
                     2, 0, 2, 0,
                     0, 0, 0, 0 });
            auto res = d * e;
            auto res2 = e * d;
            EXPECT_TRUE(close(res, r)) << res.convert_to_string() << " should be:\n" << r.convert_to_string() << "\n";
            EXPECT_TRUE(close(res2, r2)) << res2.convert_to_string() << " should be:\n" << r2.convert_to_string() << "\n";
        } 
        {

            Mat<float, 2, 4> d({ 2, 0, 2, 0,
                                0, 2, 0, 0 });
            Mat<float, 4, 3> e({ 1, 0, 1,
                                0, 1, 1,
                                1, 0, 3,
                                0, 0, 2 });
            Mat<float, 2, 3> r(4, 0,8,
                                0, 2, 2);
            auto res = d * e;
            EXPECT_TRUE(close(res, r)) << res.convert_to_string() << " should be:\n" << r.convert_to_string() << "\n";
        }
    }
    TEST(Matrices, rotationMatrices) {
         // TODO: This Test is flawed currently
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
            mat33 roll_m( 1, 0, 0,
                                        0, cr, -sr,
                                        0, sr, cr );
            mat33  pitch_m( cp, 0, sp,
                                    0, 1, 0,
                                    -sp, 0, cp );
            mat33  yaw_m( cy, -sy, 0,
                                    sy, cy, 0,
                                    0, 0, 1 );
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
        for (int i = 0; i < 10000; i++) {
            vec3 angles({ dist_rot(rng), dist_rot_half(rng), dist_rot(rng) });
            quat q(angles);
            vec3 pos({ dist_pos(rng), dist_pos(rng) ,dist_pos(rng) });
            vec3 result1 = mat33(q) * pos;
            vec3 result2 = q * pos;
            vec3 result3 = mat33::rotation_matrix(angles) * pos;
            //EXPECT_TRUE(close(result1.L2_norm(), result2.L2_norm(), 1e-5f)) << result1.L2_norm() << " != " << result2.L2_norm();
            //EXPECT_TRUE(close(result2.L2_norm(), result3.L2_norm(), 1e-5f)) << result2.L2_norm() << " != " << result3.L2_norm();
            EXPECT_TRUE(close(result1, result2, 0.0001f)) << result1.convert_to_string() << " not equal to " << result2.convert_to_string() << " at: i=" << i << " pos: " << pos.convert_to_string() << " angles: " << angles.convert_to_string() << "\n" << mat33(q).convert_to_string();
            EXPECT_TRUE(close(result2, result3, 0.0001f)) << result2.convert_to_string() << " not equal to " << result3.convert_to_string() << " at: i=" << i << " pos: " << pos.convert_to_string() << " angles: " << angles.convert_to_string() << "\n" << mat33::rotation_matrix(angles).convert_to_string() << "\n" << q.convert_to_string();
            EXPECT_TRUE(close(result1, result3, 0.0001f)) << result1.convert_to_string() << " not equal to " << result3.convert_to_string() << " at: i=" << i << " pos: " << pos.convert_to_string() << " angles: " << angles.convert_to_string() << "\n" << mat33::rotation_matrix(angles).convert_to_string() << "\n" << mat33(q).convert_to_string();
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