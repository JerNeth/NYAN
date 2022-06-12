#include <gtest/gtest.h>
#include "Util"
#include <array>
#include <chrono>
#include <list>
#include <random>
namespace Utility {
    TEST(Utility, bitwidth) {
        EXPECT_EQ(bit_width(0x0), 0);
        EXPECT_EQ(bit_width(0x1), 1);
        EXPECT_EQ(bit_width(0x2), 2);
        EXPECT_EQ(bit_width(0x3), 2);
        EXPECT_EQ(bit_width(0x4), 3);
        EXPECT_EQ(bit_width(0x5), 3);
        for (uint64_t i = 0; i < 63; i++) {
            EXPECT_EQ(bit_width(1ull << i), i+1);
        }
    }
    TEST(Utility, bitwidth_perf) {
        auto t1 = std::chrono::steady_clock::now();
        size_t res1{ 0 };
        {
            std::default_random_engine rng;
            std::uniform_int_distribution<uint64_t> dist(0, ~0);
            for (size_t i{ 0 }; i < 1000000; i++) {
                res1 += std::bit_width(i);
            }
        }
        auto t2 = std::chrono::steady_clock::now();
        size_t res2{ 0 };
        {
            std::default_random_engine rng;
            std::uniform_int_distribution<uint64_t> dist(0, ~0);
            for (size_t i{ 0 }; i < 1000000; i++) {
                res2 += bit_width(i);
            }
        }
        auto t3 = std::chrono::steady_clock::now();
        std::cout << "Delta t, std: " << (t2 - t1).count() << " util: " << (t3 - t2).count() << "\n";
        EXPECT_EQ(res1, res2);
    }
    TEST(Utility, bitstuff) {
        for (size_t i{ 1 }; i < 1000000; i++) {
            auto res1 = std::bit_width(i) - 1;
            auto res2 = fast_log2(i);
            EXPECT_EQ(res1, res2);
        }
    }
    TEST(Utility, hash) {
        std::array<char, 10> ar { (char)0x07,(char)0x1e, (char)0x62,(char)0x37,(char)0x2c,(char)0x02,(char)0x40, (char)0x42,(char)0x14,(char)0x69 };
        size_t h = Hash<std::array<char, 10>>()(ar);
        EXPECT_EQ(h, 0);
    }
    TEST(Utility, FastLog2) {
        uint32_t num = 10;

        EXPECT_EQ(fast_log2(num), 3);
        EXPECT_EQ(fast_log2(8), 3);
        EXPECT_EQ(fast_log2(7), 2);
        EXPECT_EQ(fast_log2(6), 2);
        EXPECT_EQ(fast_log2(5), 2);
        for (int i = 0; i < 1000000; i++) {
            EXPECT_EQ(fast_log2(i),static_cast<uint32_t>(std::log2(i)));
        }
        int iters = 1000000;
        uint64_t result = 0;
        {
            auto start = std::chrono::steady_clock::now();
            for (size_t i = 0; i < iters; i++) {
                result += fast_log2(i);
            }
            auto end = std::chrono::steady_clock::now();
            std::cout << "Fast log2 took: " << std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() << "microseconds\n";
        }
        uint64_t result2 = 0;
        {
            auto start = std::chrono::steady_clock::now();
            for (size_t i = 0; i < iters; i++) {
                result2 += static_cast<uint32_t>(std::log2(i));
            }
            auto end = std::chrono::steady_clock::now();
            std::cout << "log2 took: " << std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() << "microseconds\n";
        }
        uint64_t result3 = 0;
        {
            auto start = std::chrono::steady_clock::now();
            for (size_t i = 1; i < iters; i++) {
                result3 += static_cast<uint32_t>(std::bit_width(i) - 1);
            }
            auto end = std::chrono::steady_clock::now();
            std::cout << "bitwidth took: " << std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() << "microseconds\n";
        }
        EXPECT_EQ(result, result3);
    }
    TEST(Utility, bitset) {
        enum class Test : uint32_t{
            A,
            B,
            C,
            D,
            E,
            F,
            G,
            H,
            Size
        };
        using enum Test;
        Utility::bitset<static_cast<size_t>(Test::Size), Test> bitset;
        bitset.set(A);
        EXPECT_TRUE(bitset.test(A));
        EXPECT_FALSE(bitset.test(B));
        EXPECT_FALSE(bitset.test(C));
        auto r = bitset.get_and_clear(Test::A);
        //The following mustn't compile
        //auto l = bitset.get_and_clear(0);
        EXPECT_TRUE(r.test(A));
        EXPECT_FALSE(bitset.test(A));
        EXPECT_FALSE(bitset.test(B));
        EXPECT_FALSE(bitset.test(C));
        bitset.set(C);
        r = bitset.get_and_clear(A, B);
        EXPECT_FALSE(bitset.test(A));
        EXPECT_FALSE(bitset.test(B));
        EXPECT_TRUE(bitset.test(C));

        EXPECT_TRUE( bitset.any_of(B, C));

        bitset.set(H);

        EXPECT_TRUE(bitset.test(H));

    }
    TEST(Utility, Pool) {
        Utility::Pool<int> test;
    }

    TEST(Utility, PoolEmplace) {
        Utility::Pool<int> test;
        auto i0 = test.emplace(0);
        auto i1 = test.emplace(1);
        auto i2 = test.emplace(2);
        auto i3 = test.emplace(3);
        auto i4 = test.emplace(4);




        EXPECT_EQ(*i0, 0);
        EXPECT_EQ(*i1, 1);
        EXPECT_EQ(*i2, 2);
        EXPECT_EQ(*i3, 3);
        EXPECT_EQ(*i4, 4);

    }
    TEST(Utility, PoolDelete) {
        Utility::Pool<int> test;
        auto i0 = test.emplace(0);
        auto i1 = test.emplace(1);
        auto i2 = test.emplace(2);
        auto i3 = test.emplace(3);
        auto i4 = test.emplace(4);

        EXPECT_EQ(*i0, 0);
        EXPECT_EQ(*i1, 1);
        EXPECT_EQ(*i2, 2);
        EXPECT_EQ(*i3, 3);
        EXPECT_EQ(*i4, 4);

        i2.remove();

        EXPECT_EQ(*i0, 0);
        EXPECT_EQ(*i1, 1);
        EXPECT_ANY_THROW(*i2);
        EXPECT_EQ(*i3, 3);
        EXPECT_EQ(*i4, 4);
    }
    TEST(Utility, PoolDeleteAndReinsert) {
        Utility::Pool<int> test;
        auto i0 = test.emplace(0);
        auto i1 = test.emplace(1);
        auto i2 = test.emplace(2);
        auto i3 = test.emplace(3);
        auto i4 = test.emplace(4);

        EXPECT_EQ(*i0, 0);
        EXPECT_EQ(*i1, 1);
        EXPECT_EQ(*i2, 2);
        EXPECT_EQ(*i3, 3);
        EXPECT_EQ(*i4, 4);

        i2.remove();

        EXPECT_EQ(*i0, 0);
        EXPECT_EQ(*i1, 1);
        EXPECT_ANY_THROW(*i2);
        EXPECT_EQ(i2.operator->(), nullptr);
        EXPECT_EQ(*i3, 3);
        EXPECT_EQ(*i4, 4);

        auto i5 = test.emplace(5);

        EXPECT_EQ(*i0, 0);
        EXPECT_EQ(*i1, 1);
        EXPECT_EQ(*i5, 5);
        EXPECT_EQ(*i3, 3);
        EXPECT_EQ(*i4, 4);
    }
    TEST(Utility, PoolHandleCopy) {
        Utility::Pool<int> test;
        auto i0 = test.emplace(0);
        auto i1 = test.emplace(1);
        auto i1_copy = i1;

        EXPECT_EQ(*i0, 0);
        EXPECT_EQ(*i1, 1);
        EXPECT_EQ(*i1_copy, 1);
    }
    TEST(Utility, PoolExtreme) {
        Utility::Pool<int> test;
        {
            std::vector<ObjectHandle<int, Pool<int>>> handles;
            auto iters = 10000;
            for (int i = 0; i < iters; i++) {
                handles.emplace_back(test.emplace(i));
            }
            EXPECT_EQ(test.size(), iters);
            for (int i = 0; i < iters; i++) {
                EXPECT_EQ(*handles[i], i);
                //handles[i].remove();
            }
        }
        EXPECT_EQ(test.size(), 0);
    }
    TEST(Utility, OwningHashMap) {
        Utility::OwningHashMap<int> test;

    }
    TEST(Utility, OwningHashMapEmplace) {
        Utility::OwningHashMap<int> test;
        auto i0 = test.emplace(Hash<int>()(0), 0);
        auto i1 = test.emplace(Hash<int>()(1), 1);
        auto i2 = test.emplace(Hash<int>()(2), 2);
        auto i3 = test.emplace(Hash<int>()(3), 3);
        auto i4 = test.emplace(Hash<int>()(4), 4);


        EXPECT_EQ(*i0, 0);
        EXPECT_EQ(*i1, 1);
        EXPECT_EQ(*i2, 2);
        EXPECT_EQ(*i3, 3);
        EXPECT_EQ(*i4, 4);

    }

    TEST(Utility, OwningHashMapHandle) {
        Utility::OwningHashMap<int> test;
        {
            auto i0 = test.emplace(Hash<int>()(0), 0);
            auto i1 = test.emplace(Hash<int>()(1), 1);
            auto i2 = test.emplace(Hash<int>()(2), 2);
            auto i3 = test.emplace(Hash<int>()(3), 3);
            auto i4 = test.emplace(Hash<int>()(4), 4);


            EXPECT_EQ(*i0, 0);
            EXPECT_EQ(*i1, 1);
            EXPECT_EQ(*i2, 2);
            EXPECT_EQ(*i3, 3);
            EXPECT_EQ(*i4, 4);
        }
        EXPECT_EQ(test.size(), 0);
    }
    TEST(Utility, OwningHashMapHandleCopy) {
        OwningHashMapHandle<int> a;
        Utility::OwningHashMap<int> test;
        {
            auto i0 = test.emplace(Hash<int>()(0), 0);
            auto i1 = test.emplace(Hash<int>()(1), 1);
            auto i2 = test.emplace(Hash<int>()(2), 2);
            auto i3 = test.emplace(Hash<int>()(3), 3);
            auto i4 = test.emplace(Hash<int>()(4), 4);


            EXPECT_EQ(*i0, 0);
            EXPECT_EQ(*i1, 1);
            EXPECT_EQ(*i2, 2);
            EXPECT_EQ(*i3, 3);
            EXPECT_EQ(*i4, 4);
            a = i1;
        }
        EXPECT_EQ(test.size(), 1);
        EXPECT_EQ(*a, 1);
    }
    TEST(Utility, NonInvalidatingMapTest) {
        Utility::NonInvalidatingMap<std::string, std::string> testMap;
        EXPECT_FALSE(testMap.contains("test"));
        testMap.emplace("test", "testval");
        EXPECT_TRUE(testMap.contains("test"));        
        EXPECT_EQ(testMap.get("test"), "testval");
        testMap.emplace("test", "testval2");
        EXPECT_TRUE(testMap.contains("test"));
        EXPECT_EQ(testMap.get("test"), "testval2");
        testMap.emplace("test1", "testval3");
        testMap.emplace("test2", "testval4");
        testMap.emplace("test3", "testval5");
        testMap.emplace("test4", "testval6");
        testMap.emplace("test5", "testval7");
        testMap.emplace("test6", "testval8");
        testMap.emplace("test7", "testval9");
        testMap.emplace("test8", "testval10");
        testMap.emplace("test9", "testval11");
        EXPECT_TRUE(testMap.contains("test"));
        EXPECT_TRUE(testMap.contains("test1"));
        EXPECT_TRUE(testMap.contains("test2"));
        EXPECT_TRUE(testMap.contains("test3"));
        EXPECT_TRUE(testMap.contains("test4"));
        EXPECT_TRUE(testMap.contains("test5"));
        EXPECT_TRUE(testMap.contains("test6"));
        EXPECT_TRUE(testMap.contains("test7"));
        EXPECT_TRUE(testMap.contains("test8"));
        EXPECT_TRUE(testMap.contains("test9"));
        EXPECT_EQ(testMap.get("test1"), "testval3");
        EXPECT_EQ(testMap.get("test2"), "testval4");
        EXPECT_EQ(testMap.get("test3"), "testval5");
        EXPECT_EQ(testMap.get("test4"), "testval6");
        EXPECT_EQ(testMap.get("test5"), "testval7");
        EXPECT_EQ(testMap.get("test6"), "testval8");
        EXPECT_EQ(testMap.get("test7"), "testval9");
        EXPECT_EQ(testMap.get("test8"), "testval10");
        EXPECT_EQ(testMap.get("test9"), "testval11");
    }
    TEST(Utility, OwningHashMapExtreme) {
        Utility::OwningHashMap<int> test;
        {
            std::vector<OwningHashMapHandle<int>> handles;
            auto iters = 10000;
            for (int i = 0; i < iters; i++) {
                handles.emplace_back(test.emplace(Hash<int>()(i),i));
            }
            EXPECT_EQ(test.size(), iters);
            for (int i = 0; i < iters; i++) {
                EXPECT_EQ(*handles[i], i);
                //handles[i].remove();
            }
        }
        EXPECT_EQ(test.size(), 0);
    }
    TEST(Utility, LinkedBucketListMove) {
        struct T {
            T() {

            }
            T(T&) = delete;
            T(T&&) noexcept {
                moved++;
            }
            int moved = 0;
        };
        LinkedBucketList<T, 8> l;
        T t2;
        auto id2 = l.insert(std::move(t2));
        EXPECT_EQ(l.get_ptr(id2)->moved, 1);
        auto id3 = l.emplace_intrusive(std::move(t2));
        EXPECT_EQ(l.get_ptr(id3)->moved, 1);
    }

    TEST(Utility, linkedBucketList) {
        LinkedBucketList<uint32_t, 8> l;
        auto id = l.insert(0);
        EXPECT_EQ(id, 0);
        EXPECT_EQ(0, *l.get_ptr(id));
        auto id2 = l.insert(55);
        EXPECT_EQ(id2, 1);
        EXPECT_EQ(55, *l.get_ptr(id2));
        for (int i = 0; i < 55; i++) {
            l.insert(i);
        }
        //constexpr int i = sizeof(ListBucket<uint32_t, 16>);
        auto id3 = l.emplace_intrusive(696969);
        auto id4 = l.emplace_intrusive(6969699);
        //l.print();
        EXPECT_EQ(696969, *l.get_ptr(id3));
        EXPECT_EQ(6969699, *l.get_ptr(id4));
        l.remove(id3);
        EXPECT_EQ(nullptr, l.get_ptr(id3));
        EXPECT_EQ(6969699, *l.get_ptr(id4));
        auto id5 = l.emplace_intrusive(6969);
        EXPECT_EQ(6969, *l.get_ptr(id5));
        //l.print();
        int iters = 100;
        LinkedBucketList<uint32_t, 8> other = std::move(l);
        
        {
            auto start = std::chrono::steady_clock::now();
            LinkedBucketList<uint64_t, 8> p;
            for (int i = 0; i < iters; i++) {
                p.emplace(i);
            }
            auto end = std::chrono::steady_clock::now();
            //std::cout << "List emplace took: " << std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() << "microseconds\n";
        }
        
        {
            auto start = std::chrono::steady_clock::now();
            LinkedBucketList<uint64_t, 8> p;
            for (int i = 0; i < iters; i++) {
                p.insert(i);
            }
            auto end = std::chrono::steady_clock::now();
            //std::cout << "List insert took: " << std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() << "microseconds\n";
            start = std::chrono::steady_clock::now();
            for (int i = 0; i < iters; i++) {
                p.remove(i);
            }
            end = std::chrono::steady_clock::now();
            //std::cout << "List delete took: " << std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() << "microseconds\n";
            start = std::chrono::steady_clock::now();
            for (int i = 0; i < iters; i++) {
                p.insert(i);
            }
            end = std::chrono::steady_clock::now();
            //std::cout << "List reinsert took: " << std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() << "microseconds\n";
        }
        {
            auto start = std::chrono::steady_clock::now();
            std::vector<uint64_t> p;
            for (int i = 0; i < iters; i++) {
                p.push_back(i);
            }
            auto end = std::chrono::steady_clock::now();
            //std::cout << "Vector push back took: " << std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() << "microseconds\n";
        }
        {
            auto start = std::chrono::steady_clock::now();
            std::list<uint64_t> p;
            for (int i = 0; i < iters; i++) {
                p.push_back(i);
            }
            auto end = std::chrono::steady_clock::now();
            //std::cout << "List push back took: " << std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() << "microseconds\n";
        }
    }
}
