#ifndef MATRIX_H
#define MATRIX_H
#pragma once
#include "Util.h"
#include "Constants.h"
#include <string>
#include <optional>

namespace Math {
	template<typename Scalar, size_t Size> class Vec;
	template<typename Scalar> class Quaternion;

	template<
		typename Scalar,
		size_t Size_x, //width
		size_t Size_y> //height
	class Mat
	{
	public:
		Mat() : m_data() {
			for (size_t i = 0; i < Size_x * Size_y; i++)
				m_data[i] = Scalar();
		}
		explicit Mat(const Scalar& scalar) : m_data() {
			for (size_t i = 0; i < Size_x * Size_y; i++)
				m_data[i] = scalar;
		}
		explicit Mat(const Quaternion<Scalar>& quaternion) {
			static_assert((Size_x == 3 || Size_x == 4) && (Size_y == 3 || Size_y == 4), "Rotation matrices from Quaternions are inherently 3D or 3D homogeneous");
			Scalar s2 = Scalar(2) / quaternion.squared_norm();
			
			Scalar q0 = quaternion.m_real;
			Scalar q1 = quaternion.m_imaginary[0];
			Scalar q2 = quaternion.m_imaginary[1];
			Scalar q3 = quaternion.m_imaginary[2];

			m_data[at<Size_y>(0, 0)] = q0 * q0 + q1 * q1 - q2 * q2 - q3 * q3;
			m_data[at<Size_y>(0, 1)] = s2 * (q1 * q2 + q0 * q3);
			m_data[at<Size_y>(0, 2)] = s2 * (q1 * q3 - q0 * q2);

			m_data[at<Size_y>(1, 0)] = s2 * (q1 * q2 - q0 * q3);
			m_data[at<Size_y>(1, 1)] = q0 * q0 - q1 * q1 + q2 * q2 - q3 * q3;
			m_data[at<Size_y>(1, 2)] = s2 * (q0 * q1 + q2 * q3);

			m_data[at<Size_y>(2, 0)] = s2 * (q0 * q2 + q1 * q3);
			m_data[at<Size_y>(2, 1)] = s2 * (q2 * q3 - q0 * q1);
			m_data[at<Size_y>(2, 2)] = q0 * q0 - q1 * q1 - q2 * q2 + q3 * q3;
			/*			
			Scalar x2 = rotation.m_imaginary[0] + rotation.m_imaginary[0];
			Scalar y2 = rotation.m_imaginary[1] + rotation.m_imaginary[1];
			Scalar z2 = rotation.m_imaginary[2] + rotation.m_imaginary[2];
			Scalar xx = x2 * rotation.m_imaginary[0];
			Scalar xy = rotation.m_imaginary[0] * y2;
			Scalar xz = rotation.m_imaginary[0] * z2;
			Scalar yy = y2 * rotation.m_imaginary[1];
			Scalar yz = rotation.m_imaginary[1] * z2;
			Scalar zz = rotation.m_imaginary[2] * z2;
			Scalar wx = x2 * rotation.m_real;
			Scalar wy = y2 * rotation.m_real;
			Scalar wz = z2 * rotation.m_real;
			m_data[at<Size_x>(0, 0)] = Scalar(1) - (yy+zz);
			m_data[at<Size_x>(1, 0)] = xy-wz;
			m_data[at<Size_x>(2, 0)] = xz + wy;

			m_data[at<Size_x>(0, 1)] = xy + wz;
			m_data[at<Size_x>(1, 1)] = Scalar(1) - (xx+zz);
			m_data[at<Size_x>(2, 1)] = yz -wx;

			m_data[at<Size_x>(0, 2)] = xz - wy;
			m_data[at<Size_x>(1, 2)] = yz + wx;
			m_data[at<Size_x>(2, 2)] = Scalar(1) - (xx+yy);
			*/
			if constexpr (Size_x == 4) {
				m_data[at<Size_y>(3, 0)] = Scalar(0);
				m_data[at<Size_y>(3, 1)] = Scalar(0);
				m_data[at<Size_y>(3, 2)] = Scalar(0);
			}
			if constexpr (Size_y == 4) {
				m_data[at<Size_y>(0, 3)] = Scalar(0);
				m_data[at<Size_y>(1, 3)] = Scalar(0);
				m_data[at<Size_y>(2, 3)] = Scalar(0);
			}
			if constexpr (Size_x == 4 && Size_x == 4) {
				m_data[at<Size_y>(3, 3)] = Scalar(1);
			}
		}
		explicit Mat(const Vec<Scalar, Size_x>& vec) : m_data() {
			static_assert(Size_y == 1 || Size_x == 1);
			if constexpr (Size_y == 1) {
				for (size_t i = 0; i < Size_x; i++)
					m_data[i] = vec[i];
			}
			else if constexpr (Size_x == 1) {
				for (size_t i = 0; i < Size_y; i++)
					m_data[i] = vec[i];
			}
		}
		explicit Mat(const Scalar(&list)[Size_x*Size_y]) : m_data() {
			for (size_t y = 0; y < Size_y; y++) {
				for (size_t x = 0; x < Size_x; x++) {
					//Memory layout differs from user expectation
					m_data[at<Size_y>(x, y)] = list[at<Size_x>(y, x)];
				}
			}
		}
		template<typename ScalarOther, size_t Size_x_other, size_t Size_y_other>
		explicit Mat(const Mat<ScalarOther, Size_x_other, Size_y_other>& other) : m_data() {
			for (size_t y = 0; y < min(Size_y, Size_y_other); y++) {
				for (size_t x = 0; x < min(Size_x, Size_x_other); x++) {
					m_data[at<Size_y>(x, y)] = other[at<Size_y_other>(x, y)];
				}
			}
		}
		friend inline Mat operator+(const Mat& lhs, const Mat& rhs) noexcept {
			Mat result;
			for (size_t i = 0; i < Size_x * Size_y; i++)
				result.m_data[i] = lhs.m_data[i] + rhs.m_data[i];
			return result;
		}
		friend inline Mat operator-(const Mat& lhs, const Mat& rhs) noexcept {
			Mat result;
			for (size_t i = 0; i < Size_x * Size_y; i++)
				result.m_data[i] = lhs.m_data[i] - rhs.m_data[i];
			return result;
		}
		friend inline Mat operator+(const Mat& lhs, const Scalar& rhs) noexcept {
			Mat result;
			for (size_t i = 0; i < Size_x * Size_y; i++)
				result.m_data[i] = lhs.m_data[i] + rhs;
			return result;
		}
		friend inline Mat operator-(const Mat& lhs, const Scalar& rhs) noexcept {
			Mat result;
			for (size_t i = 0; i < Size_x * Size_y; i++)
				result.m_data[i] = lhs.m_data[i] - rhs;
			return result;
		}
		friend inline Mat operator*(const Mat& lhs, const Scalar& rhs) noexcept {
			Mat result;
			for (size_t i = 0; i < Size_x * Size_y; i++)
				result.m_data[i] = lhs.m_data[i] * rhs;
			return result;
		}
		friend inline Mat operator/(const Mat& lhs, const Scalar& rhs) noexcept {
			Mat result;
			for (size_t i = 0; i < Size_x * Size_y; i++)
				result.m_data[i] = lhs.m_data[i] / rhs;
			return result;
		}
		inline std::string convert_to_string() {
			std::string result("(");
			for (size_t y = 0; y < (Size_y - 1); y++) {
				for (size_t x = 0; x < Size_x; x++)
					result += std::to_string(m_data[at<Size_y>(x, y)]) + ", ";
				result += "\n";
			}
			for (size_t x = 0; x < Size_x-1; x++)
				result += std::to_string(m_data[at<Size_y>(x , Size_y - 1)]) + ", ";
			result += std::to_string(m_data[Size_x*Size_y - 1]) + ")";
			return result;
		}
		friend inline Vec<Scalar, Size_y> operator*(const Mat& lhs, const Vec<Scalar, Size_y>& rhs) noexcept {
			Vec<Scalar, Size_y> result;
			for (size_t y = 0; y < Size_y; y++) {
				Scalar tmp = Scalar(0);
				for (size_t x = 0; x < Size_x; x++)
					tmp += lhs.m_data[at<Size_y>( x, y)] * rhs[x];
				result[y] = tmp;
			}
			return result;
		}
		friend inline bool operator==(const Mat& lhs, const Mat& rhs) noexcept {
			for (size_t i = 0; i < Size_x * Size_y; i++)
				if (lhs.m_data[i] != rhs.m_data[i])
					return false;
			return true;
		}
		friend inline bool operator==(const Mat& lhs, const Scalar& rhs) noexcept {
			for (size_t i = 0; i < Size_x * Size_y; i++)
				if (lhs.m_data[i] != rhs)
					return false;
			return true;
		}
		friend inline bool close(const Mat& lhs, const Scalar& rhs, const Scalar& eps = Scalar(1e-5)) noexcept {
			for (size_t i = 0; i < Size_x * Size_y; i++)
				if (!close(lhs.m_data[i], rhs, eps))
					return false;
			return true;
		}
		inline Mat& operator=(const Scalar& rhs)
		{
			for (size_t i = 0; i < Size_x * Size_y; i++)
				m_data[i] += rhs;
			return *this;
		}
		inline Mat& operator=(Scalar& rhs)
		{
			for (size_t i = 0; i < Size_x * Size_y; i++)
				m_data[i] = rhs;
			return *this;
		}
		inline Mat& operator+=(const Mat& rhs)
		{
			for (size_t i = 0; i < Size_x * Size_y; i++)
				m_data[i] += rhs.m_data[i];
			return *this;
		}
		inline Mat& operator-=(const Mat& rhs)
		{
			for (size_t i = 0; i < Size_x * Size_y; i++)
				m_data[i] -= rhs.m_data[i];
			return *this;
		}
		inline Mat& operator*=(const Mat& rhs)
		{
			for (size_t y = 0; y < Size_y; y++) {
				Vec<Scalar, Size_x> old_row = row(y);
				for (size_t x = 0; x < Size_x; x++) {
					m_data[at<Size_y>(x, y)] = old_row.dot(rhs.col(x));
				}
			}
			return *this;
		}
		inline Mat& operator+=(const Scalar& rhs)
		{
			for (size_t i = 0; i < Size_x * Size_y; i++)
				m_data[i] += rhs;
			return *this;
		}
		inline Mat& operator-=(const Scalar& rhs)
		{
			for (size_t i = 0; i < Size_x * Size_y; i++)
				m_data[i] -= rhs;
			return *this;
		}
		inline Mat& operator*=(const Scalar& rhs)
		{
			for (size_t i = 0; i < Size_x * Size_y; i++)
				m_data[i] *= rhs;
			return *this;
		}
		inline Mat& operator/=(const Scalar& rhs)
		{
			for (size_t i = 0; i < Size_x * Size_y; i++)
				m_data[i] /= rhs;
			return *this;
		}
		inline Vec<Scalar, Size_x> row(const size_t index) const {
			Vec<Scalar, Size_x> ret;
			if (Size_y <= index)
				throw std::out_of_range("Index out of range");
			for (size_t x = 0; x < Size_x; x++)
				ret[x] = m_data[at<Size_y>(static_cast<int>(x), static_cast<int>(index))];
			return ret;
		}
		inline Vec<Scalar, Size_y> col(const size_t index) const {
			Vec<Scalar, Size_y> ret;
			if (Size_x <= index)
				throw std::out_of_range("Index out of range");
			for (size_t y = 0; y < Size_y; y++)
				ret[y] = m_data[at<Size_y>(static_cast<int>(index), static_cast<int>(y))];
			return ret;
		}
		inline const Scalar& operator[] (const size_t index) const {
			//Just pass through, User responsible for bounds
			//if (Size_x * Size_y <= index)
			//	throw std::out_of_range("Index out of range");
			return m_data[index];
		}
		inline constexpr Scalar& operator[](const size_t index) {
			//Just pass through, User responsible for bounds
			//if (Size_x * Size_y <= index)
			//	throw std::out_of_range("Index out of range");
			return m_data[index];
		}
		inline const Scalar& operator()(const size_t x, const size_t y) const {
			//Just pass through, User responsible for bounds
			//if (Size_x * Size_y <= index)
			//	throw std::out_of_range("Index out of range");
			return m_data[at<Size_y>(y, x)];
		}
		inline constexpr Scalar& operator()(const size_t x, const size_t y) {
			//Just pass through, User responsible for bounds
			//if (Size_x * Size_y <= index)
			//	throw std::out_of_range("Index out of range");
			return m_data[at<Size_y>(y, x)];
		}
		//TODO, this is ugly
		Mat<Scalar, Size_x - 1, Size_y - 1> cofactor(const size_t i, const size_t j) const {
			Mat<Scalar, Size_x - 1, Size_y - 1> ret;
			size_t x_offset = 0;
			for (size_t x = 0; x < Size_x - 1; x++) {
				if (x == i) x_offset = 1;
				size_t y_offset = 0;
				for (size_t y = x; y < Size_y - 1; y++) {
					if (y == j) y_offset = 1;
					ret(x, y) = this->operator()(x + x_offset, y + y_offset);
				}
			}
			return ret;
		}
		Mat& transposed() {
			static_assert(Size_x == Size_y);
			for (size_t y = 0; y < Size_y; y++) {
				for (size_t x = y+1; x < Size_x; x++) {
					std::swap(m_data[at<Size_y>(x, y)],m_data[at<Size_y>(y, x)]);
				}
			}
			return *this;
		}
		Mat<Scalar, Size_y, Size_x> transpose() const {
			Mat<Scalar, Size_y, Size_x> ret;
			for (size_t y = 0; y < Size_y; y++) {
				for (size_t x = y; x < Size_x; x++) {
					ret.m_data[at<Size_x>(x,y)] = m_data[at<Size_y>(y, x)];
					ret.m_data[at<Size_x>(y,x)] = m_data[at<Size_y>(x, y)];
				}
			}
			return ret;
		}
		Scalar determinante() const {
			static_assert(Size_x == Size_y);
			if constexpr (Size_x == 2) {
				return (m_data[0] * m_data[3] - m_data[1] * m_data[2]);
			}
			else if constexpr (Size_x == 3) {
				Scalar determinante = m_data[0] * (m_data[4] * m_data[8] - m_data[5] * m_data[7]); //a * (ei - fh)
				determinante += m_data[1] * (m_data[5] * m_data[6] - m_data[3] * m_data[8]); //+ b * (fg - di)
				determinante += m_data[2] * (m_data[3] * m_data[7] - m_data[4] * m_data[6]); //+ c * (dh - eg)
				return determinante;
			}
			else if constexpr (Size_x == 4) {
				/*
				* [11 12 13 14] [ 0  1  2  3]
				* [21 22 23 24] [ 4  5  6  7]
				* [31 32 33 34] [ 8  9 10 11]
				* [41 42 43 44] [12 13 14 15]
				*/
				//                      a11 *     (a22 *     (a33        a44        - a34        a43       ) + a42*       (a23        a34        - a24       a33        ) + a32 *     (a24      a43         - a23       a44))
				Scalar determinante = m_data[0] * (m_data[5] * (m_data[10] * m_data[15] - m_data[11] * m_data[14]) + m_data[13] * (m_data[6] * m_data[11] - m_data[7] * m_data[10]) + m_data[9] * (m_data[7] * m_data[14] - m_data[6] * m_data[15]));
				//		        a21 *       ( a33* (a12a44 - a14a42) +										  a13 (a34a42 - a32a44)										    + a43*(a14a32  - a12a34))
				determinante -= m_data[4] * (m_data[10] * (m_data[1] * m_data[15] - m_data[3] * m_data[13]) + m_data[2] * (m_data[11] * m_data[13] - m_data[9] * m_data[15]) + m_data[14] * (m_data[3] * m_data[9] - m_data[1] * m_data[11]));
				//				a31*		(a23		(a12a44- a14a42) +										 a13 (a24a42  - a22a44)										 + a43(a14a22 - a12a24))
				determinante += m_data[8] * (m_data[6] * (m_data[1] * m_data[15] - m_data[3] * m_data[13]) + m_data[2] * (m_data[7] * m_data[13] - m_data[5] * m_data[15]) + m_data[14] * (m_data[3] * m_data[5] - m_data[1] * m_data[7]));
				//				-a41		(a23		(a12a34 - a14a32) +										 a13 (a24a32 -  a22a34)										 + a33(a14a22 - a12a24))
				determinante -= m_data[8] * (m_data[6] * (m_data[1] * m_data[11] - m_data[3] * m_data[9]) + m_data[2] * (m_data[7] * m_data[9] - m_data[5] * m_data[11]) + m_data[10] * (m_data[3] * m_data[5] - m_data[1] * m_data[7]));
				return determinante;
			}
			else {
				return Scalar(0);
			}
		}

		//Mat& inversed();

		std::optional<Mat> inverse() const {
			static_assert(Size_x == Size_y);
			Mat res;
			if constexpr (Size_x == 2) {
				Scalar determinante = this->determinante();
				if (close(determinante, Scalar(0)))
					return std::nullopt;
				determinante = Scalar(1) / determinante;
				res.m_data[0] = determinante * m_data[3];
				res.m_data[1] = -determinante * m_data[1];
				res.m_data[2] = -determinante * m_data[2];
				res.m_data[3] = determinante * m_data[0];
				return res;
			}
			else if constexpr (Size_x == 3) {
				Scalar determinante = this->determinante();
				if (close(determinante, Scalar(0)))
					return std::nullopt;
				determinante = Scalar(1) / determinante;
				/*
				* [ 0 1 2 ]
				* [ 3 4 5 ]
				* [ 6 7 8 ]
				*/
				res.m_data[0] = determinante * (m_data[4] * m_data[8] - m_data[5] * m_data[7]);
				res.m_data[3] = -determinante * (m_data[3] * m_data[8] - m_data[5] * m_data[6]);
				res.m_data[6] = determinante * (m_data[3] * m_data[7] - m_data[4] * m_data[6]);
				res.m_data[1] = -determinante * (m_data[1] * m_data[8] - m_data[7] * m_data[2]);
				res.m_data[4] = determinante * (m_data[0] * m_data[8] - m_data[2] * m_data[6]);
				res.m_data[7] = -determinante * (m_data[0] * m_data[7] - m_data[1] * m_data[6]);
				res.m_data[2] = determinante * (m_data[1] * m_data[5] - m_data[2] * m_data[4]);
				res.m_data[5] = -determinante * (m_data[0] * m_data[5] - m_data[2] * m_data[3]);
				res.m_data[8] = determinante * (m_data[0] * m_data[4] - m_data[1] * m_data[3]);
				return res;
			}
			else if constexpr (Size_x == 4) {
				Scalar determinante = this->determinante();
				if (close(determinante, Scalar(0)))
					return std::nullopt;
				determinante = Scalar(1) / determinante;
				//Currently not working
				for (size_t i = 0; i < Size_x; i++) {
					for (size_t j = 0; j < Size_y; j++) {
						res(i, j) = (((i + j) & 0x1) ? Scalar(-1) : Scalar(1)) * this->cofactor(i, j).determinante() * determinante;
					}
				}
				return res;
			}
			else {
				return std::nullopt;
			}
		}

		static Mat eye(const Scalar& val) {
			Mat ret;
			for (int i = 0; i < min(Size_x, Size_y); i++)
				ret.m_data[at<Size_y>(i, i)] = val;
			return ret;
		}
		static Mat identity() {
			return Mat::eye(1);
		}
		static Mat<Scalar, 2, 2> rotation_matrix(Scalar angle) {
			Scalar cos_t = Scalar(cos(angle));
			Scalar sin_t = Scalar(sin(angle));
			return Mat<Scalar, 2, 2>({ cos_t, -sin_t, sin_t, cos_t});
		}
		template<size_t Size>
		void set_col(Vec<Scalar, Size> column, size_t colNum = 0, size_t offset = 0) {
			//assert(offset + Size < Size_y);
			for (size_t i = offset; i < Size + offset; i++) {
				m_data[at<Size_y>(colNum, i)] = column[i];
			}
		}
		template<size_t Size>
		void set_row(Vec<Scalar, Size> row, size_t rowNum = 0, size_t offset = 0) {
			//assert(offset + Size < Size_x);
			for (size_t i = offset; i < Size + offset; i++) {
				m_data[at<Size_y>(i, rowNum)] = row[i];
			}
		}
		Mat& set_to_identity() {
			for (int i = 0; i < Size_y* Size_x; i++) {
				m_data[i] = 0;
			}
			for (int i = 0; i < min(Size_x, Size_y); i++)
				m_data[at<Size_y>(i, i)] = 1;
			return *this;
		}
		/*	Rotates first around Z, then around Y and then around Z
		*	yaw (Z) , pitch (Y), roll (X) in degrees
		*/
		static Mat<Scalar, Size_x, Size_y> rotation_matrix(Scalar roll, Scalar pitch, Scalar yaw) {
			
			static_assert(Size_x == 3 || Size_x == 4);
			static_assert(Size_y == 3 || Size_y == 4);
			//TODO use doubles until return for more precision
			Scalar cy = Scalar(cos(yaw * deg_to_rad));
			Scalar sy = Scalar(sin(yaw * deg_to_rad));
			Scalar cp = Scalar(cos(pitch * deg_to_rad));
			Scalar sp = Scalar(sin(pitch * deg_to_rad));
			Scalar cr = Scalar(cos(roll * deg_to_rad));
			Scalar sr = Scalar(sin(roll * deg_to_rad));

			
			return Mat<Scalar, 3, 3>({ cy * cp,		cy * sp * sr - sy * cr,		cy * sp * cr + sy * sr,
										sy * cp,	sy * sp * sr + cr * cy,		sy * sp * cr - cy * sr,
										-sp,			cp * sr,							cp * cr });
			/*Scalar ca = Scalar(cos(yaw * deg_to_rad));
			Scalar sa = Scalar(sin(yaw * deg_to_rad));
			Scalar ch = Scalar(cos(pitch * deg_to_rad));
			Scalar sh = Scalar(sin(pitch * deg_to_rad));
			Scalar cb = Scalar(cos(roll * deg_to_rad));
			Scalar sb = Scalar(sin(roll * deg_to_rad));*/
			//return Mat<Scalar, 3, 3>({ ch * ca, -ch*sa*cb + sh*sb, ch*sa*sb + sh*cb,
			//							sa, ca*cb, -ca*sb,
			//							-sh*ca, sh*sa*cb + ch*sb, -sh*sa*sb + ch*cb});
			/*Scalar ca = Scalar(cos(yaw * deg_to_rad));
			Scalar sa = Scalar(sin(yaw * deg_to_rad));
			Scalar ch = Scalar(cos(pitch * deg_to_rad));
			Scalar sh = Scalar(sin(pitch * deg_to_rad));
			Scalar cb = Scalar(cos(roll * deg_to_rad));
			Scalar sb = Scalar(sin(roll * deg_to_rad));*/
			////https://en.wikipedia.org/wiki/Rotation_matrix
			///*return Mat<Scalar, 3, 3>({ cy * cp,		-sp,						cp*sy,
			//							cy*sp*cr+sr*sy,	cr*cp,		sp*cr*sy-sr*cy,
			//							sr*sp*cy-sy*cr, sr*cp, sp*sr*sy+cr*cy});*/
			////attitude = yaw, heading= pitch, bank = roll
			//
			//Mat<Scalar, 3, 3> ret;
			//ret.m_data[at<Size_y>(0, 0)] = ch * ca;
			//ret.m_data[at<Size_y>(1, 0)] = sh* sb - ch*sa*cb;
			//ret.m_data[at<Size_y>(2, 0)] = ch*sa*sb + sh*cb;

			//ret.m_data[at<Size_y>(0, 1)] = sa;
			//ret.m_data[at<Size_y>(1, 1)] = ca *cb;
			//ret.m_data[at<Size_y>(2, 1)] = -ca*sb;

			//ret.m_data[at<Size_y>(0, 2)] = -sh*ca;
			//ret.m_data[at<Size_y>(1, 2)] = sh*sa*cb + ch*sb;
			//ret.m_data[at<Size_y>(2, 2)] = -sh * sa *sb + ch*cb;

			//Mat<Scalar, 3, 3> temp2({ ch*ca, -ch*sa, sh,
			//						sa, ca, 0,
			//						-sh*ca, sh*sa, ch});
			//if constexpr (Size_x == 4) {
			//	ret.m_data[at<Size_y>(0, 3)] = Scalar(0);
			//	ret.m_data[at<Size_y>(1, 3)] = Scalar(0);
			//	ret.m_data[at<Size_y>(2, 3)] = Scalar(0);

			//	ret.m_data[at<Size_y>(3, 0)] = Scalar(0);
			//	ret.m_data[at<Size_y>(3, 1)] = Scalar(0);
			//	ret.m_data[at<Size_y>(3, 2)] = Scalar(0);

			//	ret.m_data[at<Size_y>(3, 3)] = Scalar(1);
			//}
			//return ret;
			//Mat<Scalar, 3, 3> temp3 = temp2;
			//temp3.transposed();
			//Mat<Scalar, 3, 3> roll_m({ 1, 0, 0,
			//						0, cr, -sr,
			//						0, sr, cr});
			//Mat<Scalar, 3, 3> pitch_m({ cp, 0, sp,
			//						0, 1, 0,
			//						-sp, 0, cp });
			//Mat<Scalar, 3, 3> yaw_m({ cy, -sy, 0,
			//						sy, cy, 0,
			//						0, 0, 1 });
			////Mat<Scalar, 3, 3> temp = (yaw_m * pitch_m);
			//Mat<Scalar, 3, 3> comb = (yaw_m * pitch_m) * roll_m;
			////for some reason this (probably wrong) matrix multiplications work 
			////std::cout << temp.convert_to_string() << '\n' << temp2.convert_to_string() << '\n' << temp3.convert_to_string() << std::endl;
			//return comb;
		}
		static inline Mat<Scalar, 3, 3> rotation_matrix(Vec<Scalar, 3>  roll_pitch_yaw) { // roll (x), pitch (y), yaw (z)
			return rotation_matrix(roll_pitch_yaw[0], roll_pitch_yaw[1], roll_pitch_yaw[2]);
		}
		static inline Mat<Scalar, 4, 4> translation_matrix(Vec<Scalar, 3>  translation_vector) { // roll (x), pitch (y), yaw (z)
			Mat<Scalar, 4, 4> matrix = Mat<Scalar, 4, 4>::identity();
			matrix.set_col(translation_vector, 3);
			return matrix;
		}
		static inline Mat<Scalar, 4, 4> look_at(Vec<Scalar, 3> eye, Vec<Scalar, 3> at, Vec<Scalar, 3> up) {
			Mat<Scalar, 4, 4> matrix = Mat<Scalar, 4, 4>::identity();
			Vec<Scalar, 3> zAxis = at - eye;
			zAxis.normalize();
			Vec<Scalar, 3> xAxis = zAxis.cross(up);
			xAxis.normalize();
			Vec<Scalar, 3> yAxis = zAxis.cross(xAxis);
			matrix.set_row(xAxis, 0);
			matrix.set_row(yAxis, 1);
			matrix.set_row(-zAxis, 2);
			//matrix[Math::at<Size_x>(0, 0)] = xAxis.x();
			//matrix[Math::at<Size_x>(1, 0)] = xAxis.y();
			//matrix[Math::at<Size_x>(2, 0)] = xAxis.z();
			//matrix[Math::at<Size_x>(0, 1)] = yAxis.x();
			//matrix[Math::at<Size_x>(1, 1)] = yAxis.y();
			//matrix[Math::at<Size_x>(2, 1)] = yAxis.z();
			//matrix[Math::at<Size_x>(0, 2)] = -(zAxis.x());
			//matrix[Math::at<Size_x>(1, 2)] = -(zAxis.y());
			//matrix[Math::at<Size_x>(2, 2)] = -(zAxis.z());
			matrix[Math::at<Size_y>(3, 0)] = -(xAxis.dot(eye));
			matrix[Math::at<Size_y>(3, 1)] = -(yAxis.dot(eye));
			matrix[Math::at<Size_y>(3, 2)] =  (zAxis.dot(eye));
			return matrix;
		}
		
		static inline Mat<Scalar, 4, 4> perspective(Scalar nearPlane, Scalar farPlane, Scalar fovY, Scalar aspectRatio) {
			//Right handed, zero to one
			Mat<Scalar, 4, 4> matrix;
			const Scalar tanHalfFovx = static_cast<Scalar>(tan(static_cast<double>(fovY) * 0.5 * deg_to_rad));
			//matrix(0, 0) = Scalar(1) / ( aspect * tanHalfFovy);
			//matrix(1, 1) = Scalar(1) / (tanHalfFovy);
			//matrix(2, 2) = -(farPlane + nearPlane) / (farPlane - nearPlane);
			//matrix(2, 3) = -Scalar(1);
			//matrix(3, 2) = -(Scalar(2) * farPlane * nearPlane) / (farPlane - nearPlane);
			matrix[Math::at<Size_y>(0, 0)] = static_cast<Scalar>(1) / (tanHalfFovx);
			matrix[Math::at<Size_y>(1, 1)] = static_cast<Scalar>(1) / (aspectRatio * tanHalfFovx);
			matrix[Math::at<Size_y>(2, 2)] =  farPlane / (nearPlane  - farPlane);
			matrix[Math::at<Size_y>(2, 3)] = -Scalar(1);
			matrix[Math::at<Size_y>(3, 2)] = -(nearPlane * farPlane) / (farPlane - nearPlane);
			return matrix;
		}
	private:

		std::array<Scalar, Size_x* Size_y> m_data;
		// I would really prefer C++20 concepts instead of this
		static_assert(std::is_arithmetic<Scalar>::value, "Scalar must be numeric");
	};
	
	template<typename Scalar, size_t Size_x, size_t Size_y>
	inline bool close(const Mat<Scalar, Size_x, Size_y>& lhs, const Mat<Scalar, Size_x, Size_y>& rhs, const Scalar& eps = Scalar(1e-5)) {
		for (size_t i = 0; i < Size_x * Size_y; i++)
			if (!close(lhs[i], rhs[i], eps))
				return false;
		return true;
	}
	
	template<typename Scalar, size_t rows, size_t equal, size_t cols>
	inline Mat<Scalar, rows, cols> operator*(const Mat<Scalar, rows, equal>& lhs, const Mat<Scalar, equal, cols>& rhs) {
		Mat<Scalar, rows, cols> result;
		for (size_t y = 0; y < cols; y++) {
			Vec<Scalar, equal> old_col = lhs.row(y);
			for (size_t x = 0; x < rows; x++) {
				result[at<cols>(x, y)] = old_col.dot(rhs.col(x));
			}
		}
		return result;
	}
	/*
	template<typename Scalar, size_t rows, size_t equal, size_t cols>
	inline Mat<Scalar, rows, cols> operator*(const Mat<Scalar, rows, equal>& lhs, const Mat<Scalar, equal, cols>& rhs) {
		Mat<Scalar, rows, cols> result;
		for (size_t y = 0; y < cols; y++) {
			for (size_t x = 0; x < rows; x++) {
				Scalar tmp = Scalar(0);
				for (size_t i = 0; i < equal; i++) {
					tmp += lhs[at<rows>(y, i)] * rhs[at<equal>(i, x)];
				}
				result[at<rows>(y, x)] = tmp;
			}
		}
		return result;
	}
	*/
}
#endif // !MATRIX_H