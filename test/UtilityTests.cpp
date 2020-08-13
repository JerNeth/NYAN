#include <gtest/gtest.h>
#include "Utility.h"
#include <array>
#include <chrono>

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
        int iters = 100000;
        uint32_t result = 0;
        {
            auto start = std::chrono::steady_clock::now();
            for (int i = 0; i < iters; i++) {
                result = fast_log2(i);
            }
            auto end = std::chrono::steady_clock::now();
            std::cout << "Fast log2 took: " << std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() << "microseconds\n";
        }
        uint32_t result2 = 0;
        {
            auto start = std::chrono::steady_clock::now();
            for (int i = 0; i < iters; i++) {
                result2 = static_cast<uint32_t>(std::log2(i));
            }
            auto end = std::chrono::steady_clock::now();
            std::cout << "log2 took: " << std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() << "microseconds\n";
        }
        EXPECT_EQ(result, result2);
    }
    TEST(Utility, linkedBucketList) {
        LinkedBucketList<uint32_t> l;
        auto id = l.insert(0);
        EXPECT_EQ(id, 0);
        EXPECT_EQ(0, *l.get(id));
        auto id2 = l.insert(55);
        EXPECT_EQ(id2, 1);
        EXPECT_EQ(55, *l.get(id2));
        for (int i = 0; i < 55; i++) {
            l.insert(i);
        }
        constexpr int i = sizeof(ListBucket<uint32_t, 16>);
        auto id3 = l.emplace(696969);
        auto id4 = l.emplace(6969699);
        //l.print();
        EXPECT_EQ(696969, *l.get(id3));
        EXPECT_EQ(6969699, *l.get(id4));
        l.delete_object(id3);
        EXPECT_EQ(nullptr, l.get(id3));
        EXPECT_EQ(6969699, *l.get(id4));
        auto id5 = l.emplace(6969);
        EXPECT_EQ(6969, *l.get(id5));
        //l.print();
        int iters = 1000;
        LinkedBucketList<uint32_t> other = std::move(l);
        
        {
            auto start = std::chrono::steady_clock::now();
            LinkedBucketList<uint64_t> p;
            for (int i = 0; i < iters; i++) {
                p.emplace(i);
            }
            auto end = std::chrono::steady_clock::now();
            std::cout << "List emplace took: " << std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() << "microseconds\n";
        }
        
        {
            auto start = std::chrono::steady_clock::now();
            LinkedBucketList<uint64_t> p;
            for (int i = 0; i < iters; i++) {
                p.insert(i);
            }
            auto end = std::chrono::steady_clock::now();
            std::cout << "List insert took: " << std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() << "microseconds\n";
            start = std::chrono::steady_clock::now();
            for (int i = 0; i < iters; i++) {
                p.delete_object(i);
            }
            end = std::chrono::steady_clock::now();
            std::cout << "List delete took: " << std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() << "microseconds\n";
            start = std::chrono::steady_clock::now();
            for (int i = 0; i < iters; i++) {
                p.insert(i);
            }
            end = std::chrono::steady_clock::now();
            std::cout << "List reinsert took: " << std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() << "microseconds\n";
        }
        {
            auto start = std::chrono::steady_clock::now();
            std::vector<uint64_t> p;
            for (int i = 0; i < iters; i++) {
                p.push_back(i);
            }
            auto end = std::chrono::steady_clock::now();
            std::cout << "Vector push back took: " << std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() << "microseconds\n";
        }
        {
            auto start = std::chrono::steady_clock::now();
            std::list<uint64_t> p;
            for (int i = 0; i < iters; i++) {
                p.push_back(i);
            }
            auto end = std::chrono::steady_clock::now();
            std::cout << "List push back took: " << std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() << "microseconds\n";
        }
    }
}
