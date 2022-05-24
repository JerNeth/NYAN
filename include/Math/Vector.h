#ifndef VECTOR_H
#define VECTOR_H
#pragma once
#include <string>
#include <cmath>
#include "Util.h"

namespace Math {
	template<ScalarT Scalar, size_t Size_x, size_t Size_y, bool column_major> class Mat;
	template<ScalarT Scalar> class Quaternion;

	template<
		ScalarT Scalar,
		size_t Size>
		class Vec
	{

	public:
		using value_type = Scalar;
		constexpr Vec() noexcept : m_data() {
			for (size_t i = 0; i < Size; i++)
				m_data[i] = Scalar();
		}
		// I think this one is unnecessary
		// Although you could also argue that the other one is unnecessary and copy is better
		//explicit Vec(Scalar scalar) : m_data() {
		//	for (size_t i = 0; i < Size; i++)
		//		m_data[i] = scalar;
		//}
		//Without explicit: Vec<int, 2> t = 2; would compile
		//constexpr explicit Vec(Scalar const& scalar) noexcept : m_data() {
		//	for (size_t i = 0; i < Size; i++)
		//		m_data[i] = scalar;
		//}
		template<typename... Args>
		constexpr explicit Vec(Args... args) {
			static_assert(sizeof...(Args) <= Size);
			if constexpr (sizeof...(Args) == 1) {
				for (size_t i = 0; i < Size; i++)
				{
					std::array<std::common_type_t<Args...>, 1> list = {std::forward<Args>(args)...};
					m_data[i] = static_cast<Scalar>(list[0]);
				}
			}
			else {
				m_data = { static_cast<Scalar>(std::forward<Args>(args))... };
			}
		}
		template<ScalarT OtherScalar,size_t Size_other>
		constexpr Vec(Vec<OtherScalar, Size_other> other) noexcept : m_data() {
			for (size_t i = 0; i < min(Size, Size_other); i++)
				m_data[i] = other[i];
			for (size_t i = min(Size, Size_other); i < Size; i++)
				m_data[i] = Scalar(0);
		}
		constexpr explicit Vec(const Scalar(&list)[Size]) : m_data() {
			for (size_t i = 0; i < Size; i++)
				m_data[i] = list[i];
		}
		constexpr friend inline bool operator==(const Vec& lhs, const Vec& rhs) noexcept {
			for (size_t i = 0; i < Size; i++)
				if (lhs.m_data[i] != rhs.m_data[i])
					return false;
			return true;
		}
		constexpr friend inline Vec operator+(const Vec& lhs, const Vec& rhs) noexcept {
			Vec result;
			for (size_t i = 0; i < Size; i++)
				result.m_data[i] = lhs.m_data[i] + rhs.m_data[i];
			return result;
		}
		constexpr friend inline Vec operator-(const Vec& lhs, const Vec& rhs) noexcept {
			Vec result;
			for (size_t i = 0; i < Size; i++)
				result.m_data[i] = lhs.m_data[i] - rhs.m_data[i];
			return result;
		}
		constexpr friend inline Vec operator|(const Vec& lhs, const Vec& rhs) noexcept {
			Vec result;
			for (size_t i = 0; i < Size; i++)
				result.m_data[i] = lhs.m_data[i] | rhs.m_data[i];
			return result;
		}
		constexpr friend inline Vec operator&(const Vec& lhs, const Vec& rhs) noexcept {
			Vec result;
			for (size_t i = 0; i < Size; i++)
				result.m_data[i] = lhs.m_data[i] & rhs.m_data[i];
			return result;
		}
		template<ScalarT OtherScalar>
		constexpr operator Vec<OtherScalar, Size>() {
			Vec<OtherScalar, Size> result;
			for (size_t i = 0; i < Size; i++)
				result.m_data[i] = static_cast<OtherScalar>(m_data[i]);
			return result;
		}
		/*
		*	elementwise multiplication
		*/
		constexpr friend inline Vec operator*(const Vec& lhs, const Vec& rhs) noexcept {
			Vec result;
			for (size_t i = 0; i < Size; i++)
				result.m_data[i] = lhs.m_data[i] * rhs.m_data[i];
			return result;
		}
		inline std::string convert_to_string() {
			std::string result("(");
			for (size_t i = 0; i < (Size-1); i++)
				result += std::to_string(m_data[i]) + ", ";
			result += std::to_string(m_data[Size - 1]) + ")";
			return result;
		}
		constexpr friend inline bool close(const Vec& lhs, const Vec& rhs, const Scalar& eps = Scalar(1e-5)) noexcept {
			for (size_t i = 0; i < Size; i++)
				if (!close(lhs.m_data[i], rhs.m_data[i], eps))
					return false;
			return true;
		}
		constexpr friend inline bool operator==(const Vec& lhs, const Scalar& rhs) noexcept {
			for (size_t i = 0; i < Size; i++)
				if (lhs.m_data[i] != rhs)
					return false;
			return true;
		}
		constexpr friend inline bool operator>(const Vec& lhs, const Scalar& rhs) noexcept {
			for (size_t i = 0; i < Size; i++)
				if (lhs.m_data[i] <= rhs)
					return false;
			return true;
		}
		constexpr friend inline bool operator<(const Vec& lhs, const Scalar& rhs) noexcept {
			for (size_t i = 0; i < Size; i++)
				if (lhs.m_data[i] >= rhs)
					return false;
			return true;
		}
		constexpr friend inline Vec operator+(const Vec& lhs, const Scalar& rhs) noexcept {
			Vec result;
			for (size_t i = 0; i < Size; i++)
				result.m_data[i] = lhs.m_data[i] + rhs;
			return result;
		}
		constexpr friend inline Vec operator+(const Scalar& lhs, const Vec& rhs) noexcept {
			Vec result;
			for (size_t i = 0; i < Size; i++)
				result.m_data[i] = lhs + rhs.m_data[i];
			return result;
		}
		constexpr friend inline Vec operator-(const Vec& lhs, const Scalar& rhs) noexcept {
			Vec result;
			for (size_t i = 0; i < Size; i++)
				result.m_data[i] = lhs.m_data[i] - rhs;
			return result;
		}
		constexpr friend inline Vec operator-(const Scalar& lhs, const Vec& rhs) noexcept {
			Vec result;
			for (size_t i = 0; i < Size; i++)
				result.m_data[i] = lhs - rhs.m_data[i];
			return result;
		}
		constexpr friend inline Vec operator*(const Vec& lhs, const Scalar& rhs) noexcept {
			Vec result;
			for (size_t i = 0; i < Size; i++)
				result.m_data[i] = lhs.m_data[i] * rhs;
			return result;
		}

		constexpr friend inline Vec operator*(const Scalar& lhs, const Vec& rhs) noexcept {
			Vec result;
			for (size_t i = 0; i < Size; i++)
				result.m_data[i] = lhs * rhs.m_data[i];
			return result;
		}
		constexpr friend inline Vec operator/(const Vec& lhs, const Scalar& rhs) noexcept {
			Vec result;
			for (size_t i = 0; i < Size; i++)
				result.m_data[i] = lhs.m_data[i] / rhs;
			return result;
		}
		constexpr friend inline Vec operator/(const Scalar& lhs, const Vec& rhs) noexcept {
			Vec result;
			for (size_t i = 0; i < Size; i++)
				result.m_data[i] = lhs / rhs.m_data[i];
			return result;
		}
		constexpr friend inline Vec operator&(const Vec& lhs, const Scalar& rhs) noexcept {
			Vec result;
			for (size_t i = 0; i < Size; i++)
				result.m_data[i] = lhs.m_data[i] & rhs;
			return result;
		}
		constexpr friend inline Vec operator&(const Scalar& lhs, const Vec& rhs) noexcept {
			Vec result;
			for (size_t i = 0; i < Size; i++)
				result.m_data[i] = lhs & rhs.m_data[i];
			return result;
		}
		constexpr friend inline Vec operator|(const Scalar& lhs, const Vec& rhs) noexcept {
			Vec result;
			for (size_t i = 0; i < Size; i++)
				result.m_data[i] = lhs | rhs.m_data[i];
			return result;
		}
		constexpr friend inline Vec operator|(const Vec& lhs, const Scalar& rhs) noexcept {
			Vec result;
			for (size_t i = 0; i < Size; i++)
				result.m_data[i] = lhs.m_data[i] | rhs;
			return result;
		}
		constexpr inline Scalar L2_square() const noexcept {
			Scalar sum = Scalar(0);
			for (size_t i = 0; i < Size; i++)
				sum += m_data[i] * m_data[i];
			return sum;
		}
		inline Scalar L2_norm() const noexcept {
			return std::sqrt(L2_square());
		}
		inline Scalar L1_norm() const noexcept {
			Scalar sum = Scalar(0);
			for (size_t i = 0; i < Size; i++)
				sum += std::abs(m_data[i]);
			return sum;
		}
		constexpr inline Vec operator-() const noexcept {
			Vec result;
			for (size_t i = 0; i < Size; i++)
				result.m_data[i] = -m_data[i];
			return result;
		}
		constexpr inline Vec& operator=(const Scalar& rhs) noexcept {
			for (size_t i = 0; i < Size; i++)
				m_data[i] += rhs;
			return *this;
		}
		constexpr inline Vec& operator=(Scalar& rhs) noexcept {
			for (size_t i = 0; i < Size; i++)
				m_data[i] = rhs;
			return *this;
		}
		constexpr inline Vec& operator+=(const Vec& rhs) noexcept {
			for (size_t i = 0; i < Size; i++)
				m_data[i] += rhs.m_data[i];
			return *this;
		}
		constexpr inline Vec& operator-=(const Vec& rhs) noexcept {
			for (size_t i = 0; i < Size; i++)
				m_data[i] -= rhs.m_data[i];
			return *this;
		}
		constexpr inline Vec& operator*=(const Vec& rhs) noexcept {
			for (size_t i = 0; i < Size; i++)
				m_data[i] *= rhs.m_data[i];
			return *this;
		}
		constexpr inline Vec& operator+=(const Scalar& rhs) noexcept {
			for (size_t i = 0; i < Size; i++)
				m_data[i] += rhs;
			return *this;
		}
		constexpr inline Vec& operator-=(const Scalar& rhs) noexcept {
			for (size_t i = 0; i < Size; i++)
				m_data[i] -= rhs;
			return *this;
		}
		constexpr inline Vec& operator*=(const Scalar& rhs) noexcept {
			for (size_t i = 0; i < Size; i++)
				m_data[i] *= rhs;
			return *this;
		}
		constexpr inline Vec& operator/=(const Scalar& rhs) noexcept {
			for (size_t i = 0; i < Size; i++)
				m_data[i] /= rhs;
			return *this;
		}
		constexpr inline Vec& operator*=(const Quaternion<Scalar>& rhs) noexcept {
			static_assert(Size == 3 || Size == 4, "Only works for three dimensional vectors");
			*this = rhs * *this;
			return *this;
		}
		constexpr inline const Scalar& operator[] (const size_t index) const noexcept {
			//Just pass through, User responsible for bounds
			//if (Size <= index)
			//	throw std::out_of_range("Index out of range");
			return m_data[index];
		}
		constexpr inline Scalar& operator[](const size_t index) noexcept {
			//Just pass through, User responsible for bounds
			//if(Size <= index)
			//	throw std::out_of_range("Index out of range");
			return m_data[index];
		}
		constexpr inline operator std::array<Scalar, Size>() const noexcept{
			return m_data;
		}
		//inline Vec pow(Scalar exponent) {
		//	Vec result;
		//	for (size_t i = 0; i < Size; i++)
		//		result.m_data[i] = pow(m_data[i], exponent);
		//	return result;
		//}
		/// I don't know if this is a good idea, or not. We'll see.
		constexpr friend inline Vec operator^(const Vec& lhs, const Scalar& rhs) noexcept {
			Vec result;
			for (size_t i = 0; i < Size; i++)
				result.m_data[i] = pow(lhs.m_data[i], rhs);
			return result;
		}
		/// Even less of a good idea than the one before
		constexpr friend inline Vec operator^(const Vec& lhs, const Vec& rhs) noexcept {
			Vec result;
			for (size_t i = 0; i < Size; i++)
				result.m_data[i] = pow(lhs.m_data[i], rhs.m_data[i]);
			return result;
		}
		constexpr inline Scalar dot(const Vec& rhs) const noexcept {
			// Not sure if this is better than = 0, but this way we correctly have a Scalar
			Scalar result = Scalar(0);
			for (size_t i = 0; i < Size; i++)
				result += m_data[i] * rhs.m_data[i];
			return result;
		}
		constexpr inline Vec cross(const Vec& other) const noexcept {
			static_assert(Size == 3, "Cross Product is only defined for three dimensions");
			return Vec({ m_data[1] * other.m_data[2] - m_data[2] * other.m_data[1],
						m_data[2] * other.m_data[0] - m_data[0] * other.m_data[2],
						m_data[0] * other.m_data[1] - m_data[1] * other.m_data[0] });
		}
		constexpr inline Vec& apply_fun(Scalar fun(Scalar)) noexcept {
			for (size_t i = 0; i < Size; i++)
				m_data[i] = fun(m_data[i]);
			return *this;
		}

		constexpr inline Scalar& x() noexcept {
			static_assert(Size > 0, "Vector too small");
			return m_data[0];
		}
		constexpr inline Scalar& y() noexcept {
			static_assert(Size > 1, "Vector too small");
			return m_data[1];
		}
		constexpr inline Scalar& z() noexcept {
			static_assert(Size > 2, "Vector too small");
			return m_data[2];
		}
		constexpr inline Scalar& w() noexcept {
			static_assert(Size > 3, "Vector too small");
			return m_data[3];
		}
		constexpr inline Scalar& r() noexcept {
			static_assert(Size > 0, "Vector too small");
			return m_data[0];
		}
		constexpr inline Scalar& g() noexcept {
			static_assert(Size > 1, "Vector too small");
			return m_data[1];
		}
		constexpr inline Scalar& b() noexcept {
			static_assert(Size > 2, "Vector too small");
			return m_data[2];
		}
		constexpr inline Scalar& a() noexcept {
			static_assert(Size > 3, "Vector too small");
			return m_data[3];
		}
		inline Vec& normalize() noexcept {
			Scalar inverseNorm = Scalar(1)/L2_norm();
			for (size_t i = 0; i < m_data.size(); i++) {
				m_data[i] *= inverseNorm;
			}
			return *this;
		}
		inline Vec normalized() const noexcept  {
			Vec ret {};
			Scalar inverseNorm = Scalar(1) / L2_norm();
			for (int i = 0; i < m_data.size(); i++) {
				ret.m_data[i] = m_data[i]*inverseNorm;
			}
			return ret;
		}
		constexpr friend inline Scalar dot(Vec const& lhs, Vec const& rhs) {
			return lhs.dot(rhs);
		}
		constexpr friend inline Vec cross(Vec const& lhs, Vec const& rhs) {
			return lhs.cross(rhs);
		}
	private:
		std::array<Scalar, Size> m_data;
	};

	template<typename... Args>
	constexpr Vec<std::common_type_t<Args...>, sizeof...(Args)> make_vector(Args... args) {
		return Vec<std::common_type_t<Args...>, sizeof...(Args)>(std::forward<Args>(args)...);
	}

}
#endif // !VECTOR_H