#ifndef VECTOR_H
#define VECTOR_H
#pragma once
#include <string>
#include <vector>
#include <cmath>
#include <cassert>
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
			//static_assert(sizeof...(Args) <= Size);
			assert(sizeof...(Args) <= Size);
			if constexpr (sizeof...(Args) == 1) {
				std::array<std::common_type_t<Args...>, 1> list = { std::forward<Args>(args)... };
				for (size_t i = 0; i < Size; i++)
				{
					m_data[i] = static_cast<Scalar>(list[0]);
				}
			}
			else if constexpr (sizeof...(Args) == Size) {
				std::array<Scalar, sizeof...(Args)> list = { static_cast<Scalar>(std::forward<Args>(args))... };
				m_data = std::move(list);
			}
			else {
				//std::array<std::common_type_t<Args...>, sizeof...(Args)> list = { std::forward<Args>(args)... };
				std::array<Scalar, sizeof...(Args)> list = { static_cast<Scalar>(std::forward<Args>(args))... };
				for (size_t i = 0; i < Size; i++)
				{
					m_data[i] = static_cast<Scalar>(list[i]);
				}
			}
		}
		template<ScalarT OtherScalar, size_t Size_other>
		constexpr explicit Vec(Vec<OtherScalar, Size_other> other) noexcept : m_data() {
			//Breaks stuff when using const ref
			for (size_t i = 0; i < min(Size, Size_other); i++)
				m_data[i] = other[i];
			for (size_t i = min(Size, Size_other); i < Size; i++)
				m_data[i] = Scalar(0);
		}
		constexpr explicit Vec(const Scalar(&list)[Size]) : m_data() {
			for (size_t i = 0; i < Size; i++)
				m_data[i] = list[i];
		}
		template<ScalarT OtherScalar>
		constexpr explicit Vec(const std::vector<OtherScalar>& data) noexcept : m_data() {
			for (size_t i = 0; i < min(Size, data.size()); i++)
				m_data[i] = data[i];
			for (size_t i = min(Size, data.size()); i < Size; i++)
				m_data[i] = Scalar(0);
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
		//constexpr friend inline Vec operator|(const Vec& lhs, const Vec& rhs) noexcept {
		//	Vec result;
		//	for (size_t i = 0; i < Size; i++)
		//		result.m_data[i] = lhs.m_data[i] | rhs.m_data[i];
		//	return result;
		//}
		//constexpr friend inline Vec operator&(const Vec& lhs, const Vec& rhs) noexcept {
		//	Vec result;
		//	for (size_t i = 0; i < Size; i++)
		//		result.m_data[i] = lhs.m_data[i] & rhs.m_data[i];
		//	return result;
		//}
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
		inline std::string convert_to_string() const {
			std::string result("(");
			for (size_t i = 0; i < (Size-1); i++)
				result += std::to_string(m_data[i]) + ", ";
			result += std::to_string(m_data[Size - 1]) + ")";
			return result;
		}
		constexpr friend inline bool close(const Vec& lhs, const Vec& rhs, const Scalar& eps = static_cast<Scalar>(1e-5f)) noexcept {
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
		//constexpr friend inline Vec operator&(const Vec& lhs, const Scalar& rhs) noexcept {
		//	Vec result;
		//	for (size_t i = 0; i < Size; i++)
		//		result.m_data[i] = lhs.m_data[i] & rhs;
		//	return result;
		//}
		//constexpr friend inline Vec operator&(const Scalar& lhs, const Vec& rhs) noexcept {
		//	Vec result;
		//	for (size_t i = 0; i < Size; i++)
		//		result.m_data[i] = lhs & rhs.m_data[i];
		//	return result;
		//}
		//constexpr friend inline Vec operator|(const Scalar& lhs, const Vec& rhs) noexcept {
		//	Vec result;
		//	for (size_t i = 0; i < Size; i++)
		//		result.m_data[i] = lhs | rhs.m_data[i];
		//	return result;
		//}
		//constexpr friend inline Vec operator|(const Vec& lhs, const Scalar& rhs) noexcept {
		//	Vec result;
		//	for (size_t i = 0; i < Size; i++)
		//		result.m_data[i] = lhs.m_data[i] | rhs;
		//	return result;
		//}
		constexpr inline Scalar L2_square() const noexcept {
			Scalar sum = Scalar(0.);
			for (size_t i = 0; i < Size; i++)
				sum = sum +  m_data[i] * m_data[i];
			return sum;
		}
		inline Scalar L2_norm() const noexcept {
			return static_cast<Scalar>(std::sqrt(L2_square()));
		}
		inline Scalar L1_norm() const noexcept {
			Scalar sum = Scalar(0.);
			//TODO use custom abs
			for (size_t i = 0; i < Size; i++)
				sum = sum + static_cast<Scalar>(std::abs(static_cast<double>(m_data[i])));
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
				m_data[i] = m_data[i] + rhs;
			return *this;
		}
		constexpr inline Vec& operator=(Scalar& rhs) noexcept {
			for (size_t i = 0; i < Size; i++)
				m_data[i] = rhs;
			return *this;
		}
		constexpr inline Vec& operator+=(const Vec& rhs) noexcept {
			for (size_t i = 0; i < Size; i++)
				m_data[i] = m_data[i] + rhs.m_data[i];
			return *this;
		}
		constexpr inline Vec& operator-=(const Vec& rhs) noexcept {
			for (size_t i = 0; i < Size; i++)
				m_data[i] = m_data[i] - rhs.m_data[i];
			return *this;
		}
		constexpr inline Vec& operator*=(const Vec& rhs) noexcept {
			for (size_t i = 0; i < Size; i++)
				m_data[i] = m_data[i] * rhs.m_data[i];
			return *this;
		}
		constexpr inline Vec& operator+=(const Scalar& rhs) noexcept {
			for (size_t i = 0; i < Size; i++)
				m_data[i] = m_data[i] + rhs;
			return *this;
		}
		constexpr inline Vec& operator-=(const Scalar& rhs) noexcept {
			for (size_t i = 0; i < Size; i++)
				m_data[i] = m_data[i] - rhs;
			return *this;
		}
		constexpr inline Vec& operator*=(const Scalar& rhs) noexcept {
			for (size_t i = 0; i < Size; i++)
				m_data[i] = m_data[i] * rhs;
			return *this;
		}
		constexpr inline Vec& operator/=(const Scalar& rhs) noexcept {
			for (size_t i = 0; i < Size; i++)
				m_data[i] = m_data[i] / rhs;
			return *this;
		}
		constexpr inline Vec& operator*=(const Quaternion<Scalar>& rhs) noexcept {
			//assert(("Only works for three dimensional vectors", Size == 3 || Size == 4));
			assert(Size == 3 && "Only works for three dimensional vectors");
			if constexpr (Size == 3) {
				*this = rhs * *this;
			}
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
				result.m_data[i] = static_cast<Scalar>(pow(lhs.m_data[i], rhs));
			return result;
		}
		/// Even less of a good idea than the one before
		constexpr friend inline Vec operator^(const Vec& lhs, const Vec& rhs) noexcept {
			Vec result;
			for (size_t i = 0; i < Size; i++)
				result.m_data[i] = static_cast<Scalar>(pow(lhs.m_data[i], rhs.m_data[i]));
			return result;
		}
		constexpr inline Scalar dot(const Vec& rhs) const noexcept {
			// Not sure if this is better than = 0, but this way we correctly have a Scalar
			Scalar result = static_cast<Scalar>(0.);
			for (size_t i = 0; i < Size; i++)
				result = result + m_data[i] * rhs.m_data[i];
			return result;
		}
		constexpr inline Vec cross(const Vec& other) const noexcept {
			assert("Cross Product is only defined for three dimensions" && Size == 3);
			return Vec( m_data[1] * other.m_data[2] - m_data[2] * other.m_data[1],
						m_data[2] * other.m_data[0] - m_data[0] * other.m_data[2],
						m_data[0] * other.m_data[1] - m_data[1] * other.m_data[0] );
		}
		constexpr inline Vec& apply_fun(Scalar fun(Scalar)) noexcept {
			for (size_t i = 0; i < Size; i++)
				m_data[i] = fun(m_data[i]);
			return *this;
		}

		constexpr inline Vec<Scalar, 2> xy() const noexcept {
			assert(Size > 1 && "Vector too small");
			return Vec<Scalar, 2>(m_data[0], m_data[1]);
		}
		constexpr inline Vec<Scalar, 3> xyz() const noexcept {
			assert(Size > 2 && "Vector too small");
			return Vec<Scalar, 3>(m_data[0], m_data[1], m_data[2]);
		}
		constexpr inline Scalar& x() noexcept {
			assert(Size > 0 && "Vector too small");
			return m_data[0];
		}
		constexpr inline Scalar& y() noexcept {
			assert(Size > 1 && "Vector too small");
			return m_data[1];
		}
		constexpr inline Scalar& z() noexcept {
			assert(Size > 2 && "Vector too small");
			return m_data[2];
		}
		constexpr inline Scalar& w() noexcept {
			assert(Size > 3 && "Vector too small");
			return m_data[3];
		}
		constexpr inline Scalar& r() noexcept {
			assert(Size > 0 && "Vector too small");
			return m_data[0];
		}
		constexpr inline Scalar& g() noexcept {
			assert(Size > 1 && "Vector too small");
			return m_data[1];
		}
		constexpr inline Scalar& b() noexcept {
			assert(Size > 2 && "Vector too small");
			return m_data[2];
		}
		constexpr inline Scalar& a() noexcept {
			assert(Size > 3 && "Vector too small");
			return m_data[3];
		}
		constexpr inline const Scalar& x() const noexcept  {
			assert(Size > 0 && "Vector too small");
			return m_data[0];
		}
		constexpr inline const Scalar& y() const noexcept  {
			assert(Size > 1 && "Vector too small");
			return m_data[1];
		}
		constexpr inline const Scalar& z() const noexcept {
			assert(Size > 2 && "Vector too small");
			return m_data[2];
		}
		constexpr inline const Scalar& w() const noexcept {
			assert(Size > 3 && "Vector too small");
			return m_data[3];
		}
		constexpr inline const Scalar& r() const noexcept {
			assert(Size > 0 && "Vector too small");
			return m_data[0];
		}
		constexpr inline const Scalar& g() const noexcept {
			assert(Size > 1 && "Vector too small");
			return m_data[1];
		}
		constexpr inline const Scalar& b() const noexcept {
			assert(Size > 2 && "Vector too small");
			return m_data[2];
		}
		constexpr inline const Scalar& a() const noexcept {
			assert(Size > 3 && "Vector too small");
			return m_data[3];
		}
		//inline Vec& normalize() noexcept {
		//	Scalar inverseNorm = static_cast<Scalar>(1.0) / L2_norm();
		//	for (size_t i = 0; i < m_data.size(); i++) {
		//		m_data[i] = m_data[i] * inverseNorm;
		//	}
		//	return *this;
		//}
		inline Vec& normalize() noexcept {
			Scalar inverseNorm = static_cast<Scalar>(1.0) / L2_norm();
			for (size_t i = 0; i < m_data.size(); i++) {
				m_data[i] = m_data[i] * inverseNorm;
			}
			return *this;
		}
		//inline Vec&& normalize() && noexcept {
		//	Scalar inverseNorm = static_cast<Scalar>(1.0) / L2_norm();
		//	for (size_t i = 0; i < m_data.size(); i++) {
		//		m_data[i] = m_data[i] * inverseNorm;
		//	}
		//	return std::move({ m_data });
		//}
		inline Vec normalized() const noexcept  {
			Vec ret {};
			Scalar inverseNorm = static_cast<Scalar>(1.0) / L2_norm();
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
	template<ScalarT Scalar,
		size_t Size >
	constexpr Vec<Scalar, Size> lerp(Vec<Scalar, Size> a, Vec<Scalar, Size> b, Scalar alpha) noexcept {
		return a * alpha + (1 - alpha) * b;
	}
	template<ScalarT Scalar,
		size_t Size >
	constexpr Vec<Scalar, Size> lerp(Vec<Scalar, Size> a, Vec<Scalar, Size> b, Vec<Scalar, Size> alpha) noexcept {
		return a * alpha + (1 - alpha) * b;
	}
	template<ScalarT Scalar,
		size_t Size >
	constexpr Vec<Scalar, Size> pow(Vec<Scalar, Size> base, Vec<Scalar, Size> exp) noexcept {
		Vec<Scalar, Size> ret;
		for (size_t i{ 0 }; i < Size; ++i)
			ret[i] = pow(base[i], exp[i]);
		return ret;
	}
	template<ScalarT Scalar,
		size_t Size >
	constexpr Vec<Scalar, Size> pow(Vec<Scalar, Size> base, Scalar exp) noexcept {
		Vec<Scalar, Size> ret;
		for (size_t i{ 0 }; i < Size; ++i)
			ret[i] = pow(base[i], exp);
		return ret;
	}

	template<typename... Args>
	constexpr Vec<std::common_type_t<Args...>, sizeof...(Args)> make_vector(Args... args) {
		return Vec<std::common_type_t<Args...>, sizeof...(Args)>(std::forward<Args>(args)...);
	}

	template<ScalarT Scalar, size_t size>
	void PrintTo(const Vec<Scalar, size>& t, std::ostream* os) {
		*os << t.convert_to_string();
	}
}
#endif // !VECTOR_H