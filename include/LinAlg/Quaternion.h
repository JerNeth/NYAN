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
		explicit Quaternion(Scalar real, Vec<Scalar, 3> imaginary) : m_real(real), m_imaginary(imaginary) {

		}
		explicit Quaternion(Scalar real, Scalar imaginary1, Scalar imaginary2, Scalar imaginary3) : m_real(real), m_imaginary({ imaginary1 , imaginary2 , imaginary3 }) {

		}
		// Expects Angles in Degrees
		// Reason being: this is expected to be used as a human interface and not intended for internal use, this is what quaternions are for
		explicit Quaternion(Scalar x_roll, Scalar y_pitch, Scalar z_yaw)  {
			//Source https://en.wikipedia.org/wiki/Conversion_between_quaternions_and_Euler_angles
			//TODO currently expanded to double, maybe give the option to stick to given precision
			double cy = cos(z_yaw * 0.5 * deg_to_rad);
			double sy = sin(z_yaw * 0.5 * deg_to_rad);
			double cp = cos(y_pitch * 0.5 * deg_to_rad);
			double sp = sin(y_pitch * 0.5 * deg_to_rad);
			double cr = cos(x_roll * 0.5 * deg_to_rad);
			double sr = sin(x_roll * 0.5 * deg_to_rad);
			m_real =         Scalar(cy * cp * cr + sy * sp * sr);
			m_imaginary[0] = Scalar(cy * cp * sr - sy * sp * cr);
			m_imaginary[1] = Scalar(sy * cp * sr + cy * sp * cr);
			m_imaginary[2] = Scalar(sy * cp * cr - cy * sp * sr);
		}
		// Expects Angles in Degrees
		// Reason being: this is expected to be used as a human interface and not intended for internal use, this is what quaternions are for
		explicit Quaternion(Vec<Scalar, 3> euler_angles) {
			//Source https://en.wikipedia.org/wiki/Conversion_between_quaternions_and_Euler_angles
			//TODO currently expanded to double, maybe give the option to stick to given precision
			double cy = cos(euler_angles[2] * 0.5 * deg_to_rad);
			double sy = sin(euler_angles[2] * 0.5 * deg_to_rad);
			double cp = cos(euler_angles[1] * 0.5 * deg_to_rad);
			double sp = sin(euler_angles[1] * 0.5 * deg_to_rad);
			double cr = cos(euler_angles[0] * 0.5 * deg_to_rad);
			double sr = sin(euler_angles[0] * 0.5 * deg_to_rad);
			m_real = Scalar(cy * cp * cr + sy * sp * sr);
			m_imaginary[0] = Scalar(cy * cp * sr - sy * sp * cr);
			m_imaginary[1] = Scalar(sy * cp * sr + cy * sp * cr);
			m_imaginary[2] = Scalar(sy * cp * cr - cy * sp * sr);
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
			result.m_real = lhs.m_real * rhs.m_real - lhs.m_imaginary.dot(rhs.m_imaginary);
			result.m_imaginary = lhs.m_real * rhs.m_imaginary + rhs.m_real * lhs.m_imaginary + cross(lhs.m_imaginary, rhs.m_imaginary);
			return result;
		}
		friend inline Quaternion operator*(const Quaternion& lhs, const Scalar& rhs) noexcept {
			Quaternion result;
			result.m_real = lhs.m_real * rhs;
			result.m_imaginary = lhs.m_imaginary * rhs;
			return result;
		}
		inline std::string convert_to_string() {
			std::string result("(");
			result += std::to_string(m_real) + ", ";
			result += m_imaginary.convert_to_string() + ")";
			return result;
		}
		friend inline Vec<Scalar, 3> operator*(const Quaternion& lhs, const Vec<Scalar, 3>& rhs) noexcept {
			//Vec<Scalar, 3> result = Scalar(2) * dot(m_imaginary, rhs) * m_imaginary + (square(m_real) - dot(m_imaginary, m_imaginary)) * rhs + Scalar(2) * m_real * (cross(m_imaginary, rhs));
			return Scalar(2) * lhs.m_imaginary.dot(rhs) * lhs.m_imaginary + (square(lhs.m_real) - lhs.m_imaginary.dot(lhs.m_imaginary)) * rhs + Scalar(2) * lhs.m_real * lhs.m_imaginary.cross(rhs);
		}
		friend inline Vec<Scalar, 4> operator*(const Quaternion& lhs, const Vec<Scalar, 4>& rhs) noexcept {
			//Quaternion result = lhs * Quaternion(0, Vec<Scalar, 3>({ rhs[0], rhs[1], rhs[2] })) * lhs.inverse();
			//return Vec<Scalar, 4>({result.m_imaginary[0], result.m_imaginary[1], result.m_imaginary[2], Scalar(1)});
			auto tmp = Vec<Scalar, 3>(rhs);
			tmp = Scalar(2) * lhs.m_imaginary.dot(tmp) * lhs.m_imaginary + (square(lhs.m_real) - lhs.m_imaginary.dot(lhs.m_imaginary)) * tmp + Scalar(2) * lhs.m_real * lhs.m_imaginary.cross(tmp);
			return Vec<Scalar, 4>({ tmp.x(), tmp.y(), tmp.z(), rhs[3]});
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
			Scalar inverse_magnitude = Scalar(1) / norm();
			m_real *= inverse_magnitude;
			m_imaginary = -m_imaginary * inverse_magnitude;
			return *this;
		}
		inline Quaternion inverse() const {
			Quaternion result;
			Scalar inverse_magnitude = Scalar(1) / norm();
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
		// Order: roll(x), pitch(y), yaw(z)
		// Returns in Degrees
		inline Vec<Scalar, 3> to_euler_angles() {
			//Source: https://en.wikipedia.org/wiki/Conversion_between_quaternions_and_Euler_angles
			Vec<Scalar, 3> result;
			// roll (x)
			double sinr_cosp = 2.0 * (m_real * m_imaginary[0] + m_imaginary[1] * m_imaginary[2]);
			double cosr_cosp = 1.0 - 2.0 * (square(m_imaginary[0]) + square(m_imaginary[1]));
			result[0] = Scalar(std::atan2(sinr_cosp, cosr_cosp) * rad_to_deg);

			// pitch (y)

			double sinp = 2.0 * (m_real * m_imaginary[1] - m_imaginary[2] * m_imaginary[0]);
			if (std::abs(sinp) >= 1.0)
				result[1] = Scalar(std::copysign(pi_2, sinp) * rad_to_deg);
			else
				result[1] = Scalar(std::asin(sinp) * rad_to_deg);
			
			// yaw (z)

			double siny_cosp = 2.0 * (m_real * m_imaginary[2] + m_imaginary[1] * m_imaginary[0]);
			double cosy_cosp = 1.0 - 2.0 * (square(m_imaginary[1]) + square(m_imaginary[2]));
			result[2] = Scalar(std::atan2(siny_cosp, cosy_cosp) * rad_to_deg);

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