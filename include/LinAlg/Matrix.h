#ifndef MATRIX_H
#define MATRIX_H
#pragma once
#include <optional>

namespace bla {
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
		Mat(Scalar scalar) : m_data() {
			for (size_t i = 0; i < Size_x * Size_y; i++)
				m_data[i] = scalar;
		}
		Mat(const Scalar& scalar) : m_data() {
			for (size_t i = 0; i < Size_x * Size_y; i++)
				m_data[i] = scalar;
		}
		explicit Mat(const Quaternion<Scalar>& rotation) {
			static_assert((Size_x == 3 && Size_y == 3) || (Size_x == 4 || Size_y == 4), "Rotation matrices from Quaternions are inherently 3D or 3D homogeneous");
			Scalar s2 = Scalar(2) / rotation.squared_norm();
			m_data[at<Size_x>(0, 0)] = Scalar(1) - s2 * (square(rotation.m_imaginary[1]) + square(rotation.m_imaginary[2]));
			m_data[at<Size_x>(1, 0)] = s2 * (rotation.m_imaginary[0] * rotation.m_imaginary[1] - rotation.m_imaginary[2] * rotation.m_real);
			m_data[at<Size_x>(2, 0)] = s2 * (rotation.m_imaginary[0] * rotation.m_imaginary[2] + rotation.m_imaginary[1] * rotation.m_real);

			m_data[at<Size_x>(0, 1)] = s2 * (rotation.m_imaginary[0] * rotation.m_imaginary[1] + rotation.m_imaginary[2] * rotation.m_real);
			m_data[at<Size_x>(1, 1)] = Scalar(1) - s2 * (square(rotation.m_imaginary[1]) + square(rotation.m_imaginary[2]));
			m_data[at<Size_x>(2, 1)] = s2 * (rotation.m_imaginary[1] * rotation.m_imaginary[2] - rotation.m_imaginary[0] * rotation.m_real);

			m_data[at<Size_x>(0, 2)] = s2 * (rotation.m_imaginary[0] * rotation.m_imaginary[2] - rotation.m_imaginary[1] * rotation.m_real);
			m_data[at<Size_x>(1, 2)] = s2 * (rotation.m_imaginary[1] * rotation.m_imaginary[2] + rotation.m_imaginary[0] * rotation.m_real);
			m_data[at<Size_x>(2, 2)] = Scalar(1) - s2 * (square(rotation.m_imaginary[1]) + square(rotation.m_imaginary[2]));
			if constexpr (Size_x == 4) {
				m_data[at<Size_x>(0, 3)] = Scalar(0);
				m_data[at<Size_x>(1, 3)] = Scalar(0);
				m_data[at<Size_x>(2, 3)] = Scalar(0);

				m_data[at<Size_x>(3, 0)] = Scalar(0);
				m_data[at<Size_x>(3, 1)] = Scalar(0);
				m_data[at<Size_x>(3, 2)] = Scalar(0);

				m_data[at<Size_x>(3, 3)] = Scalar(1);
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
		Mat(std::initializer_list<Scalar> scalars) : m_data() {
			if (Size_x * Size_y != scalars.size())
				throw std::out_of_range("Index out of range");
			size_t i = 0;
			for (Scalar s : scalars) {
				m_data[i] = s;
				i++;
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
		friend inline Vec<Scalar, Size_y> operator*(const Mat& lhs, const Vec<Scalar, Size_y>& rhs) noexcept {
			Vec<Scalar, Size_y> result;
			for (size_t y = 0; y < Size_y; y++) {
				result.m_data[y] = lhs.col(y).dot(rhs);
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
					m_data[y * Size_x + x] = old_row.dot(rhs.col(x));
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
				ret[x] = m_data[x + index * Size_x];
			return ret;
		}
		inline Vec<Scalar, Size_y> col(const size_t index) const {
			Vec<Scalar, Size_y> ret;
			if (Size_x <= index)
				throw std::out_of_range("Index out of range");
			for (size_t y = 0; y < Size_y; y++)
				ret[y] = m_data[index + y * Size_x];
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
			return m_data[y * Size_x + x];
		}
		inline constexpr Scalar& operator()(const size_t x, const size_t y) {
			//Just pass through, User responsible for bounds
			//if (Size_x * Size_y <= index)
			//	throw std::out_of_range("Index out of range");
			return m_data[y * Size_x + x];
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
			for (size_t x = 0; x < Size_x; x++) {
				for (size_t y = x; y < Size_y; y++) {
					Scalar tmp = m_data[x + y * Size_x];
					m_data[x + y * Size_x] = m_data[(x - Size_x) + (y - Size_y) * Size_x];
					m_data[(x - Size_x) + (y - Size_y) * Size_x] = tmp;
				}
			}
			return *this;
		}
		Mat transpose() const {
			Mat ret;
			//TODO do that for non square matrices
			static_assert(Size_x == Size_y);
			for (size_t x = 0; x < Size_x; x++) {
				for (size_t y = x; y < Size_y; y++) {
					ret.m_data[x + y * Size_x] = m_data[(x - Size_x) + (y - Size_y) * Size_x];
					ret.m_data[(x - Size_x) + (y - Size_y) * Size_x] = m_data[x + y * Size_x];
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
				ret.m_data[i + i * Size_x] = val;
			return ret;
		}
		static Mat identity() {
			return Mat::eye(1);
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
		for (size_t x = 0; x < rows; x++) {
			Vec<Scalar, equal> old_col = lhs.col(x);
			for (size_t y = 0; y < cols; y++) {
				result[y * rows + x] = old_col.dot(rhs.row(y));
			}
		}
		return result;
	}
	
}
#endif // !MATRIX_H