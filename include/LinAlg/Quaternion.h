#ifndef QUATERNIONS_H
#define QUATERNIONS_H
#pragma once
namespace bla {
	template<typename Scalar, size_t Size_x, size_t Size_y> class Mat;
	template<typename Scalar, size_t Size> class Vec;

	template<typename Scalar>
	class Quaternion {
	public:
		Quaternion() : m_real(), m_imaginary() {

		}
		Quaternion(Scalar real, Vec<Scalar, 3> imaginary) : m_real(real), m_imaginary(imaginary) {

		}
		Quaternion(Scalar real, Scalar imaginary1, Scalar imaginary2, Scalar imaginary3) : m_real(real), m_imaginary({ imaginary1 , imaginary2 , imaginary3 }) {

		}
		friend inline bool operator==(const Quaternion& lhs, const Quaternion& rhs) noexcept {
			return (lhs.m_real == rhs.m_real) && (lhs.m_imaginary == rhs.m_imaginary);
		}
		friend inline bool close(const Quaternion& lhs, const Quaternion& rhs, const Scalar& eps = Scalar(1e-5)) noexcept {
			return close(lhs.m_real, rhs.m_real, eps) && close(lhs.m_imaginary, rhs.m_imaginary, eps);
		}
		friend inline Quaternion operator+(const Quaternion& lhs, const Quaternion& rhs)  noexcept {
			Quaternion result;
			result.m_real = lhs.m_real + rhs.m_real;
			result.m_imaginary = lhs.m_imaginary + rhs.m_imaginary;
			return result;
		}
		friend inline Quaternion operator-(const Quaternion& lhs, const Quaternion& rhs)  noexcept {
			Quaternion result;
			result.m_real = lhs.m_real - rhs.m_real;
			result.m_imaginary = lhs.m_imaginary - rhs.m_imaginary;
			return result;
		}
		friend inline Quaternion operator*(const Quaternion& lhs, const Quaternion& rhs) noexcept {
			Quaternion result;
			result.m_real = lhs.m_real * rhs.m_real - dot(lhs.m_imaginary, rhs.m_imaginary);
			result.m_imaginary = lhs.m_real * rhs.m_imaginary + rhs.m_real * lhs.m_imaginary + cross(lhs.m_imaginary, rhs.m_imaginary);
			return result;
		}
		friend inline Quaternion operator*(const Quaternion& lhs, const Scalar& rhs) noexcept {
			Quaternion result;
			result.m_real = lhs.m_real * rhs;
			result.m_imaginary = lhs.m_imaginary * rhs;
			return result;
		}
		friend inline Vec<Scalar, 3> operator*(const Quaternion& lhs, const Vec<Scalar, 3>& rhs) noexcept {
			Vec<Scalar, 3> result;
			static_assert(false, "TODO");
			return result;
		}
		friend inline Vec<Scalar, 4> operator*(const Quaternion& lhs, const Vec<Scalar, 4>& rhs) noexcept {
			Vec<Scalar, 4> result;
			static_assert(false, "TODO");
			return result;
		}
		friend inline Quaternion operator/(const Quaternion& lhs, const Scalar& rhs) noexcept {
			Quaternion result;
			result.m_real = lhs.m_real / rhs;
			result.m_imaginary = lhs.m_imaginary / rhs;
			return result;
		}
		inline Quaternion& operator+=(const Quaternion& rhs) noexcept {
			m_real += rhs.m_real;
			m_imaginary += rhs.m_imaginary;
			return *this;
		}
		inline Quaternion& operator-=(const Quaternion& rhs) noexcept {
			m_real -= rhs.m_real;
			m_imaginary -= rhs.m_imaginary;
			return *this;
		}
		inline Quaternion& operator*=(const Quaternion& rhs) noexcept {
			m_real = m_real * rhs.m_real - dot(m_imaginary, rhs.m_imaginary);
			m_imaginary = m_real * rhs.m_imaginary + rhs.m_real * m_imaginary + cross(m_imaginary, rhs.m_imaginary);
			return *this;
		}
		inline Quaternion& operator*=(const Scalar& rhs) noexcept {
			m_real *= rhs;
			m_imaginary *= rhs;
			return *this;
		}
		inline Quaternion& operator/=(const Scalar& rhs) noexcept {
			m_real /= rhs;
			m_imaginary /= rhs;
			return *this;
		}
		inline Scalar dot(const Quaternion& rhs) noexcept {
			return m_imaginary.dot(rhs.m_imaginary) + m_real * rhs.m_real;
		}
		friend inline Scalar dot(Quaternion& lhs, Quaternion& rhs) {
			//return lhs.m_imaginary.dot(rhs.m_imaginary) + lhs.m_real * rhs.m_real;
			return lhs.m_imaginary.dot(rhs.m_imaginary) + lhs.m_real * rhs.m_real;
		}
		inline Scalar squared_norm() const noexcept {
			return m_real * m_real + m_imaginary.L2_square();
		}
		inline Scalar norm() const noexcept {
			return sqrt(squared_norm());
		}
		inline Quaternion& inversed() {
			Scalar inverse_magnitude = Scalar(1) / norm;
			m_real *= inverse_magnitude;
			m_imaginary = -m_imaginary * inverse_magnitude;
			return *this;
		}
		inline Quaternion inverse() const {
			Quaternion result;
			Scalar inverse_magnitude = Scalar(1) / norm;
			result.m_real = m_real * inverse_magnitude;
			result.m_imaginary = -m_imaginary * inverse_magnitude;
			return result;
		}
		inline Quaternion& normalized() {
			Scalar inverse_magnitude = Scalar(1) / norm();
			m_real *= inverse_magnitude;
			m_imaginary *= inverse_magnitude;
			return *this;
		}
		inline Quaternion normalize() const {
			Quaternion result;
			Scalar inverse_magnitude = Scalar(1) / norm();
			result.m_real = m_real * inverse_magnitude;
			result.m_imaginary = m_imaginary * inverse_magnitude;
			return result;
		}
		
		// ============================================================================================================
		// Frens
		// ============================================================================================================
		template<typename Scalar,
			size_t Size_x,
			size_t Size_y>
			friend class Mat;
		template<typename Scalar,
			size_t Size>
			friend class Vec;
	private:
		Scalar m_real;
		Vec<Scalar, 3> m_imaginary;
		// I would really prefer C++20 concepts instead of this
		static_assert(std::is_arithmetic<Scalar>::value, "Scalar must be numeric");
	};
	//template<typename Scalar,
	//	typename = typename std::enable_if<std::is_arithmetic<Scalar>::value, Scalar>::type >
	//	inline Scalar dot(Quaternion<Scalar>& lhs, Quaternion<Scalar>& rhs) {
	//	return dot(lhs.m_imaginary, rhs.m_imaginary) + lhs.m_real * rhs.m_real;
	//}
}
#endif // !QUATERNIONS_H