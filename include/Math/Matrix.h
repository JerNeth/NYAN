#ifndef MATRIX_H
#define MATRIX_H
#pragma once
#include "Util.h"
#include "Constants.h"
#include <string>
#include <optional>
#include <cmath>

namespace Math {
	template<ScalarT Scalar, size_t Size> class Vec;
	template<ScalarT Scalar> class Quaternion;

	template<
		ScalarT Scalar,
		size_t Size_y, //height, row_count
		size_t Size_x, //width, column_count 
		bool column_major = true> 
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

			at(0, 0) = q0 * q0 + q1 * q1 - q2 * q2 - q3 * q3;
			at(0, 1) = s2 * (q1 * q2 + q0 * q3);
			at(0, 2) = s2 * (q1 * q3 - q0 * q2);

			at(1, 0) = s2 * (q1 * q2 - q0 * q3);
			at(1, 1) = q0 * q0 - q1 * q1 + q2 * q2 - q3 * q3;
			at(1, 2) = s2 * (q0 * q1 + q2 * q3);

			at(2, 0) = s2 * (q0 * q2 + q1 * q3);
			at(2, 1) = s2 * (q2 * q3 - q0 * q1);
			at(2, 2) = q0 * q0 - q1 * q1 - q2 * q2 + q3 * q3;
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
			at(0, 0) = Scalar(1) - (yy+zz);
			at(1, 0) = xy-wz;
			at(2, 0) = xz + wy;

			at(0, 1) = xy + wz;
			at(1, 1) = Scalar(1) - (xx+zz);
			at(2, 1) = yz -wx;

			at(0, 2) = xz - wy;
			at(1, 2) = yz + wx;
			at(2, 2) = Scalar(1) - (xx+yy);
			*/
			if constexpr (Size_x == 4) {
				at(3, 0) = Scalar(0);
				at(3, 1) = Scalar(0);
				at(3, 2) = Scalar(0);
			}
			if constexpr (Size_y == 4) {
				at(0, 3) = Scalar(0);
				at(1, 3) = Scalar(0);
				at(2, 3) = Scalar(0);
			}
			if constexpr (Size_x == 4 && Size_x == 4) {
				at(3, 3) = Scalar(1);
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
		template<typename T>
		explicit Mat(const T(&list)[Size_x*Size_y]) : m_data() {
			for (size_t y = 0; y < Size_y; y++) {
				for (size_t x = 0; x < Size_x; x++) {
					//Memory layout differs from user expectation
					at(x, y) = Scalar(list[Math::at<Size_x, false>(x, y)]);
				}
			}
		}
		template<ScalarT... Args>
		constexpr explicit Mat(Args... args) {
			static_assert(sizeof...(Args) == (Size_x * Size_y)|| sizeof...(Args) == 1, "Invalid amount of parameters for this mat");
			if constexpr (sizeof...(Args) == 1) {
				for (size_t i = 0; i < (Size_x * Size_y); i++)
				{
					std::array<std::common_type_t<Args...>, 1> list = { std::forward<Args>(args)... };
					m_data[i] = static_cast<Scalar>(list[0]);
				}
			}
			else {
				std::array<std::common_type_t<Args...>, sizeof...(Args)> list = { std::forward<Args>(args)... };
				for (size_t y = 0; y < Size_y; y++) {
					for (size_t x = 0; x < Size_x; x++) {
						at(x, y) = Scalar(list[Math::at<Size_x, false>(x, y)]);
					}
				}
			}
		}
		template<typename ScalarOther, size_t Size_x_other, size_t Size_y_other>
		explicit Mat(const Mat<ScalarOther, Size_x_other, Size_y_other>& other) : m_data() {
			for (size_t y = 0; y < min(Size_y, Size_y_other); y++) {
				for (size_t x = 0; x < min(Size_x, Size_x_other); x++) {
					at(x, y) = other.at(x, y);
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
		friend inline Mat operator*(const Scalar& lhs, const Mat& rhs) noexcept {
			return rhs * lhs;
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
					result += std::to_string(at(x, y)) + ", ";
				result += "\n";
			}
			for (size_t x = 0; x < Size_x-1; x++)
				result += std::to_string(at(x , Size_y - 1)) + ", ";
			result += std::to_string(m_data[Size_x*Size_y - 1]) + ")";
			return result;
		}
		friend inline Vec<Scalar, Size_y> operator*(const Mat& lhs, const Vec<Scalar, Size_x>& rhs) noexcept {
			Vec<Scalar, Size_y> result;
			for (size_t y = 0; y < Size_y; y++) {
				Scalar tmp = Scalar(0);
				for (size_t x = 0; x < Size_x; x++)
					tmp += lhs( x, y) * rhs[x];
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
					at(x, y) = old_row.dot(rhs.col(x));
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
			assert(Size_y > index);
			//if (Size_y <= index)
			//	throw std::out_of_range("Index out of range");
			if constexpr (!column_major) {
				return m_vectors[index];
			}
			else {
				Vec<Scalar, Size_x> ret;
				for (size_t x = 0; x < Size_x; x++)
					ret[x] =at(x, index);
				return ret;
			}
		}
		inline Vec<Scalar, Size_y> col(const size_t index) const {
			assert(Size_x > index);
			//if (Size_x <= index)
			//	throw std::out_of_range("Index out of range");
			if constexpr (column_major) {
				return m_vectors[index];
			}
			else {
				Vec<Scalar, Size_y> ret;
				for (size_t y = 0; y < Size_y; y++)
					ret[y] = at(index, y);
				return ret;
			}
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
		inline const Scalar& operator()(const size_t col, const size_t  row) const {
			//Just pass through, User responsible for bounds
			//if (Size_x * Size_y <= index)
			//	throw std::out_of_range("Index out of range");
			return at(col, row);
		}
		inline constexpr Scalar& operator()(const size_t col, const size_t row) {
			//Just pass through, User responsible for bounds
			//if (Size_x * Size_y <= index)
			//	throw std::out_of_range("Index out of range");
			return at(col, row);
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
					std::swap(at(x, y),at(y, x));
				}
			}
			return *this;
		}
		Mat<Scalar, Size_y, Size_x> transpose() const {
			Mat<Scalar, Size_y, Size_x> ret;
			for (size_t y = 0; y < Size_y; y++) {
				for (size_t x = y; x < Size_x; x++) {
					ret(x,y) = at(y, x);
					ret(y,x) = at(x, y);
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
				ret.at(i, i) = val;
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
				at(colNum, i) = column[i];
			}
		}
		template<size_t Size>
		void set_row(Vec<Scalar, Size> row, size_t rowNum = 0, size_t offset = 0) {
			//assert(offset + Size < Size_x);
			for (size_t i = offset; i < Size + offset; i++) {
				at(i, rowNum) = row[i];
			}
		}
		Mat& set_to_identity() {
			for (int i = 0; i < Size_y* Size_x; i++) {
				m_data[i] = 0;
			}
			for (int i = 0; i < min(Size_x, Size_y); i++)
				at(i, i) = 1;
			return *this;
		}
		/*	Rotates first around Z, then around Y and then around Z
		*	yaw (Z) , pitch (Y), roll (X) in degrees
		*/
		static Mat<Scalar, Size_y, Size_x, column_major> rotation_matrix(Scalar roll, Scalar pitch, Scalar yaw) {
			
			static_assert(Size_x == 3 || Size_x == 4);
			static_assert(Size_y == 3 || Size_y == 4);
			//TODO use doubles until return for more precision
			auto cy = cos(yaw * deg_to_rad);
			auto sy = sin(yaw * deg_to_rad);
			auto cp = cos(pitch * deg_to_rad);
			auto sp = sin(pitch * deg_to_rad);
			auto cr = cos(roll * deg_to_rad);
			auto sr = sin(roll * deg_to_rad);

			
			return Mat<Scalar, 3, 3, column_major>({ cy * cp,		cy * sp * sr - sy * cr,		cy * sp * cr + sy * sr,
										sy * cp,	sy * sp * sr + cr * cy,		sy * sp * cr - cy * sr,
										-sp,			cp * sr,							cp * cr });
		}

		static inline Mat<Scalar, Size_y, Size_x, column_major> affine_transformation_matrix(Vec<Scalar, 3>  roll_pitch_yaw, Vec<Scalar, 3>  translation_vector) { // roll (x), pitch (y), yaw (z)
			static_assert(Size_x == 4);
			static_assert(Size_y == 4 || Size_y == 3);
			Mat<Scalar, Size_y, Size_x, column_major> mat{ Mat<Scalar, 3,3, column_major>::rotation_matrix(roll_pitch_yaw[0], roll_pitch_yaw[1], roll_pitch_yaw[2]) };
			mat.set_col(translation_vector, 3);
			if constexpr(Size_y == 4)
				mat(3, 3) = 1;
			return mat;
		}
		static inline Mat<Scalar, 3, 3, column_major> rotation_matrix(Vec<Scalar, 3>  roll_pitch_yaw) { // roll (x), pitch (y), yaw (z)
			return rotation_matrix(roll_pitch_yaw[0], roll_pitch_yaw[1], roll_pitch_yaw[2]);
		}
		static inline Mat<Scalar, 4, 4, column_major> translation_matrix(Vec<Scalar, 3>  translation_vector) { // roll (x), pitch (y), yaw (z)
			Mat<Scalar, 4, 4, column_major> matrix = Mat<Scalar, 4, 4>::identity();
			matrix.set_col(translation_vector, 3);
			return matrix;
		}
		static inline Mat<Scalar, Size_y , Size_x, column_major> look_at(Vec<Scalar, 3> eye, Vec<Scalar, 3> at, Vec<Scalar, 3> up) {
			static_assert(Size_x == 4 && (Size_y == 3 || Size_y == 4));
			Mat<Scalar, Size_x, Size_y, column_major> matrix = Mat<Scalar, Size_x, Size_y>::identity();
			Vec<Scalar, 3> zAxis = at - eye;
			zAxis.normalize();
			Vec<Scalar, 3> xAxis = zAxis.cross(up);
			xAxis.normalize();
			Vec<Scalar, 3> yAxis = zAxis.cross(xAxis);
			matrix.set_row(xAxis, 0);
			matrix.set_row(yAxis, 1);
			matrix.set_row(-zAxis, 2);
			//matrix.at(0, 0) = xAxis.x();
			//matrix.at(1, 0) = xAxis.y();
			//matrix.at(2, 0) = xAxis.z();
			//matrix.at(0, 1) = yAxis.x();
			//matrix.at(1, 1) = yAxis.y();
			//matrix.at(2, 1) = yAxis.z();
			//matrix.at(0, 2) = -(zAxis.x());
			//matrix.at(1, 2) = -(zAxis.y());
			//matrix.at(2, 2) = -(zAxis.z());
			matrix.at(3, 0) = -(xAxis.dot(eye));
			matrix.at(3, 1) = -(yAxis.dot(eye));
			matrix.at(3, 2) =  (zAxis.dot(eye));
			return matrix;
		}
		
		static inline Mat<Scalar, 4, 4, column_major> perspectiveY(Scalar nearPlane, Scalar farPlane, Scalar fovY, Scalar aspectRatio) {
			//Right handed, zero to one
			Mat<Scalar, 4, 4, column_major> matrix;
			const Scalar tanHalfFovy = static_cast<Scalar>(tan(static_cast<double>(fovY) * 0.5 * deg_to_rad));
			//matrix(0, 0) = Scalar(1) / ( aspect * tanHalfFovy);
			//matrix(1, 1) = Scalar(1) / (tanHalfFovy);
			//matrix(2, 2) = -(farPlane + nearPlane) / (farPlane - nearPlane);
			//matrix(2, 3) = -Scalar(1);
			//matrix(3, 2) = -(Scalar(2) * farPlane * nearPlane) / (farPlane - nearPlane);
			matrix.at(0, 0) = static_cast<Scalar>(1) / (aspectRatio * tanHalfFovy);
			matrix.at(1, 1) = static_cast<Scalar>(1) / (tanHalfFovy);
			matrix.at(2, 2) =  farPlane / (nearPlane  - farPlane);
			matrix.at(2, 3) = -Scalar(1);
			matrix.at(3, 2) = -(nearPlane * farPlane) / (farPlane - nearPlane);
			return matrix;
		}
		static inline Mat<Scalar, 4, 4, column_major> perspectiveX(Scalar nearPlane, Scalar farPlane, Scalar fovX, Scalar aspectRatio) {
			//Right handed, zero to one
			Mat<Scalar, 4, 4, column_major> matrix;
			const Scalar tanHalfFovx = static_cast<Scalar>(tan(static_cast<double>(fovX) * 0.5 * deg_to_rad));
			//matrix(0, 0) = Scalar(1) / ( aspect * tanHalfFovy);
			//matrix(1, 1) = Scalar(1) / (tanHalfFovy);
			//matrix(2, 2) = -(farPlane + nearPlane) / (farPlane - nearPlane);
			//matrix(2, 3) = -Scalar(1);
			//matrix(3, 2) = -(Scalar(2) * farPlane * nearPlane) / (farPlane - nearPlane);
			matrix(0, 0) = static_cast<Scalar>(1) / ( tanHalfFovx);
			matrix(1, 1) = static_cast<Scalar>(1) / (aspectRatio * tanHalfFovx);
			matrix(2, 2) = farPlane / (nearPlane - farPlane);
			matrix(2, 3) = -Scalar(1);
			matrix(3, 2) = -(nearPlane * farPlane) / (farPlane - nearPlane);
			return matrix;
		}
		static inline Mat<Scalar, 4, 4, column_major> perspectiveXY(Scalar nearPlane, Scalar farPlane, Scalar fovX, Scalar fovY) {
			//Right handed, zero to one
			Mat<Scalar, 4, 4, column_major> matrix;
			const Scalar tanHalfFovx = static_cast<Scalar>(tan(static_cast<double>(fovX) * 0.5 * deg_to_rad));
			const Scalar tanHalfFovy = static_cast<Scalar>(tan(static_cast<double>(fovY) * 0.5 * deg_to_rad));
			//matrix(0, 0) = Scalar(1) / ( aspect * tanHalfFovy);
			//matrix(1, 1) = Scalar(1) / (tanHalfFovy);
			//matrix(2, 2) = -(farPlane + nearPlane) / (farPlane - nearPlane);
			//matrix(2, 3) = -Scalar(1);
			//matrix(3, 2) = -(Scalar(2) * farPlane * nearPlane) / (farPlane - nearPlane);
			matrix(0, 0) = static_cast<Scalar>(1) / (tanHalfFovx);
			matrix(1, 1) = static_cast<Scalar>(1) / (tanHalfFovy);
			matrix(2, 2) = farPlane / (nearPlane - farPlane);
			matrix(2, 3) = -Scalar(1);
			matrix(3, 2) = -(nearPlane * farPlane) / (farPlane - nearPlane);
			return matrix;
		}

		inline constexpr Scalar& at(size_t col, size_t row) {
			if constexpr (column_major) {
				return m_data[col * Size_y + row];
			}
			else {
				return m_data[row * Size_x + col];
			}
		}
		inline constexpr const Scalar& at(size_t col, size_t row) const {
			if constexpr (column_major) {
				return m_data[col * Size_y + row];
			}
			else {
				return m_data[row * Size_x + col];
			}
		}
	private:
		union {
			std::array<Scalar, Size_x* Size_y> m_data;
			std::array<Vec<Scalar, column_major ?Size_y : Size_x>, column_major ? Size_x : Size_y> m_vectors;//Columns if columnmajor, rows otherwise
		};
	};
	
	template<typename Scalar, size_t Size_x, size_t Size_y>
	inline bool close(const Mat<Scalar, Size_x, Size_y>& lhs, const Mat<Scalar, Size_x, Size_y>& rhs, const Scalar& eps = Scalar(1e-5)) {
		for (size_t i = 0; i < Size_x * Size_y; i++)
			if (!close(lhs[i], rhs[i], eps))
				return false;
		return true;
	}
	
	template<typename Scalar, size_t rows, size_t equal, size_t cols, bool column_major>
	inline Mat<Scalar, rows, cols, column_major> operator*(const Mat<Scalar, rows, equal, column_major>& lhs, const Mat<Scalar, equal, cols, column_major>& rhs) {
		Mat<Scalar, rows, cols> result;
		for (size_t col = 0; col < cols; col++) {
			//Vec<Scalar, equal> old_row = lhs.row(col);
			for (size_t row = 0; row < rows; row++) {
				//result(col, row) = old_row.dot(rhs.col(row));
				Scalar tmp{ 0 };
				for (size_t i = 0; i < equal; i++) {
					tmp += lhs(i, row) * rhs(col, i);
				}
				result(col, row) = tmp;
				//result[at<column_major ? cols : rows, column_major>(x, y)] = tmp;
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