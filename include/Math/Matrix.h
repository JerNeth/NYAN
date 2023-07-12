#ifndef MATRIX_H
#define MATRIX_H
#pragma once
#include "Util.h"
#include "Constants.h"
#include <string>
#include <vector>
#include <optional>
#include <cmath>
#include <cassert>

namespace Math {
	template<ScalarT Scalar, size_t Size> class Vec;
	template<ScalarT Scalar> class Quaternion;

	//Type, rows, columns, column_major
	template<
		ScalarT Scalar,
		size_t Size_y, //height, row_count, n rows
		size_t Size_x, //width, column_count, n columns
		bool column_major = true> 
	class Mat
	{
	public:
		using value_type = Scalar;
		constexpr Mat();
		constexpr explicit Mat(const Scalar& scalar);
		constexpr explicit Mat(const Quaternion<Scalar>& quaternion);
		constexpr explicit Mat(const Vec<Scalar, Size_x>& vec);
		template<typename T>
		constexpr explicit Mat(const T(&list)[Size_x*Size_y]) : m_data() {
			for (size_t y = 0; y < Size_y; y++) {
				for (size_t x = 0; x < Size_x; x++) {
					//Memory layout differs from user expectation
					if constexpr (column_major)
						at(x, y) = Scalar(list[Math::at<Size_y, column_major>(x, y)]);
					else
						at(x, y) = Scalar(list[Math::at<Size_x, column_major>(x, y)]);
				}
			}
		}
		template<typename T>
		constexpr explicit Mat(const std::vector<T> & vec) : m_data() {
			for (size_t y = 0; y < Size_y; y++) {
				for (size_t x = 0; x < Size_x; x++) {
					//Memory layout differs from user expectation
					if constexpr (column_major)
						at(x, y) = Scalar(vec[Math::at<Size_y, column_major>(x, y)]);
					else
						at(x, y) = Scalar(vec[Math::at<Size_x, column_major>(x, y)]);
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
				std::array<Scalar, sizeof...(Args)> list = {static_cast<Scalar>(args)... };
				for (size_t y = 0; y < Size_y; y++) {
					for (size_t x = 0; x < Size_x; x++) {
						at(x, y) = Scalar{ list[Math::at<Size_x, false>(x, y)] };
					}
				}
			}
		}
		template<typename ScalarOther, size_t Size_x_other, size_t Size_y_other, bool column_major_other>
		constexpr explicit Mat(const Mat<ScalarOther, Size_x_other, Size_y_other, column_major_other>& other) : m_data() {
			for (size_t y = 0; y < min(Size_y, Size_y_other); y++) {
				for (size_t x = 0; x < min(Size_x, Size_x_other); x++) {
					at(x, y) = other.at(x, y);
				}
			}
		}
		constexpr friend inline Mat operator+(const Mat& lhs, const Mat& rhs) noexcept {
			Mat result;
			for (size_t i = 0; i < Size_x * Size_y; i++)
				result.m_data[i] = lhs.m_data[i] + rhs.m_data[i];
			return result;
		}
		constexpr friend inline Mat operator-(const Mat& lhs, const Mat& rhs) noexcept {
			Mat result;
			for (size_t i = 0; i < Size_x * Size_y; i++)
				result.m_data[i] = lhs.m_data[i] - rhs.m_data[i];
			return result;
		}
		constexpr friend inline Mat operator+(const Mat& lhs, const Scalar& rhs) noexcept {
			Mat result;
			for (size_t i = 0; i < Size_x * Size_y; i++)
				result.m_data[i] = lhs.m_data[i] + rhs;
			return result;
		}
		constexpr friend inline Mat operator-(const Mat& lhs, const Scalar& rhs) noexcept {
			Mat result;
			for (size_t i = 0; i < Size_x * Size_y; i++)
				result.m_data[i] = lhs.m_data[i] - rhs;
			return result;
		}
		constexpr friend inline Mat operator*(const Mat& lhs, const Scalar& rhs) noexcept {
			Mat result;
			for (size_t i = 0; i < Size_x * Size_y; i++)
				result.m_data[i] = lhs.m_data[i] * rhs;
			return result;
		}
		constexpr friend inline Mat operator*(const Scalar& lhs, const Mat& rhs) noexcept {
			return rhs * lhs;
		}
		constexpr friend inline Mat operator/(const Mat& lhs, const Scalar& rhs) noexcept {
			Mat result;
			for (size_t i = 0; i < Size_x * Size_y; i++)
				result.m_data[i] = lhs.m_data[i] / rhs;
			return result;
		}
		constexpr friend inline Vec<Scalar, Size_y> operator*(const Mat& lhs, const Vec<Scalar, Size_x>& rhs) noexcept {
			Vec<Scalar, Size_y> result;
			for (size_t y = 0; y < Size_y; y++) {
				Scalar tmp = Scalar(0);
				for (size_t x = 0; x < Size_x; x++)
					tmp += lhs( x, y) * rhs[x];
				result[y] = tmp;
			}
			return result;
		}
		constexpr friend inline bool operator==(const Mat& lhs, const Mat& rhs) noexcept {
			for (size_t i = 0; i < Size_x * Size_y; i++)
				if (lhs.m_data[i] != rhs.m_data[i])
					return false;
			return true;
		}
		constexpr friend inline bool operator==(const Mat& lhs, const Scalar& rhs) noexcept {
			for (size_t i = 0; i < Size_x * Size_y; i++)
				if (lhs.m_data[i] != rhs)
					return false;
			return true;
		}
		constexpr friend inline bool close(const Mat& lhs, const Scalar& rhs, const Scalar& eps = Scalar(1e-5)) noexcept {
			for (size_t i = 0; i < Size_x * Size_y; i++)
				if (!close(lhs.m_data[i], rhs, eps))
					return false;
			return true;
		}
		constexpr inline std::string convert_to_string() const;
		constexpr inline Mat& operator=(const Scalar& rhs);
		constexpr inline Mat& operator=(Scalar& rhs);
		constexpr inline Mat& operator+=(const Mat& rhs);
		constexpr inline Mat& operator-=(const Mat& rhs);
		//constexpr inline Mat& operator*=(const Mat& rhs);
		constexpr inline Mat& operator+=(const Scalar& rhs);
		constexpr inline Mat& operator-=(const Scalar& rhs);
		constexpr inline Mat& operator*=(const Scalar& rhs);
		constexpr inline Mat& operator/=(const Scalar& rhs);
		constexpr inline Vec<Scalar, Size_x> row(const size_t index) const;
		constexpr inline Vec<Scalar, Size_y> col(const size_t index) const;
		constexpr inline const Scalar& operator()(const size_t col, const size_t  row) const {
			if constexpr (column_major) {
				assert(Size_x * Size_y > col * Size_y + row);
			}
			else {
				assert(Size_x * Size_y > row * Size_x + col);
			}
			//Just pass through, User responsible for bounds
			//if (Size_x * Size_y <= index)
			//	throw std::out_of_range("Index out of range");
			return at(col, row);
		}
		constexpr inline Scalar& operator()(const size_t col, const size_t row) {
			if constexpr (column_major) {
				assert(Size_x * Size_y > col * Size_y + row);
			}
			else {
				assert(Size_x * Size_y > row * Size_x + col);
			}
			//Just pass through, User responsible for bounds
			//if (Size_x * Size_y <= index)
			//	throw std::out_of_range("Index out of range");
			return at(col, row);
		}
		constexpr inline Mat& transposed();
		constexpr inline Mat transpose() const;
		constexpr inline Scalar determinante() const;
		constexpr inline bool inverse(Mat& res) const;
		constexpr inline Vec<Scalar, 3> euler() const;

		constexpr static Mat eye(const Scalar& val) 
		{
			Mat ret {0};
			for (size_t i = 0; i < min(Size_x, Size_y); i++)
				ret.at(i, i) = val;
			return ret;
		}
		constexpr inline static Mat identity() 
		{
			return Mat::eye(1);
		}
		template<size_t Size>
		constexpr void set_col(Vec<Scalar, Size> column, size_t colNum = 0, size_t offset = 0) {
			assert(colNum < Size_x);
			assert(offset + Size <= Size_y);
			for (size_t i = offset; i < Size + offset; i++)
				at(colNum, i) = column[i];
		}
		template<size_t Size>
		constexpr void set_row(Vec<Scalar, Size> row, size_t rowNum = 0, size_t offset = 0) {
			assert(rowNum < Size_y);
			assert(offset + Size <= Size_x);
			for (size_t i = offset; i < Size + offset; i++)
				at(i, rowNum) = row[i];
		}
		constexpr void set_to_identity();
		/*	Rotates first around Z, then around Y and then around Z
		*	yaw (Z) , pitch (Y), roll (X) in degrees
		*/
		static Mat<Scalar, 3, 3, column_major> rotation_matrix(Scalar roll, Scalar pitch, Scalar yaw);
		static Mat<Scalar, 4, 4, column_major> affine_transformation_matrix(Vec<Scalar, 3>  roll_pitch_yaw, Vec<Scalar, 3>  translation_vector);
		static Mat<Scalar, 4, 4, column_major> affine_transformation_matrix(Vec<Scalar, 3>  roll_pitch_yaw, Vec<Scalar, 3>  translation_vector, Vec<Scalar, 3>  scale);
		static Mat<Scalar, 4, 4, column_major> affine_transformation_matrix(Quaternion<Scalar>  orientation, Vec<Scalar, 3>  translation_vector);
		static Mat<Scalar, 4, 4, column_major> affine_transformation_matrix(Quaternion<Scalar>  orientation, Vec<Scalar, 3>  translation_vector, Vec<Scalar, 3>  scale);
		inline Mat<Scalar, Size_y, Size_x, column_major> inverse_affine_transformation_matrix();
		static Mat<Scalar, 3, 3, column_major> rotation_matrix(Vec<Scalar, 3>  roll_pitch_yaw);
		static Mat<Scalar, 4, 4, column_major> translation_matrix(Vec<Scalar, 3>  translation_vector);
		static Mat<Scalar, 4, 4, column_major> look_at(Vec<Scalar, 3> eye, Vec<Scalar, 3> at, Vec<Scalar, 3> up);
		static Mat<Scalar, 4, 4, column_major> first_person(Vec<Scalar, 3> eye, Vec<Scalar, 3> direction, Vec<Scalar, 3> up, Vec<Scalar, 3> right);
		static Mat<Scalar, 4, 4, column_major> perspectiveY(Scalar nearPlane, Scalar farPlane, Scalar fovY, Scalar aspectRatio);
		/// <summary>
		/// Converts  +y|_+x   into vulkan space +z/_+x
		///			 +z/					      |+y
		/// I.e. is left-handed
		/// Also maps onto z [0, 1]
		/// </summary>
		/// <param name="nearPlane"></param>
		/// <param name="farPlane"></param>
		/// <param name="fovX"></param>
		/// <param name="aspectRatio"></param>
		/// <returns></returns>
		static Mat<Scalar, 4, 4, column_major> perspectiveInverseDepthFovXLH(Scalar nearPlane, Scalar farPlane, Scalar fovX, Scalar aspectRatio);
		static Mat<Scalar, 4, 4, column_major> perspectiveFovXLH(Scalar nearPlane, Scalar farPlane, Scalar fovX, Scalar aspectRatio);
		static Mat<Scalar, 4, 4, column_major> perspectiveInverseDepthFovXLH(Scalar nearPlane, Scalar fovX, Scalar aspectRatio);
		static Mat<Scalar, 4, 4, column_major> perspectiveXY(Scalar nearPlane, Scalar farPlane, Scalar fovX, Scalar fovY);

		constexpr inline Scalar& at(size_t col, size_t row) {
			if constexpr (column_major) {
				return m_data[col * Size_y + row];
			}
			else {
				return m_data[row * Size_x + col];
			}
		}

		[[nodiscard]] constexpr inline const Scalar& at(size_t col, size_t row) const {
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
			std::array<Vec<Scalar, column_major ?Size_y : Size_x>, column_major ? Size_x : Size_y> m_vectors;
			//Columns if columnmajor, rows otherwise
		};
	};


	template<typename Scalar, size_t Size_x, size_t Size_y>
	constexpr inline bool close(const Mat<Scalar, Size_x, Size_y>& lhs, const Mat<Scalar, Size_x, Size_y>& rhs, const Scalar& eps = Scalar(1e-5)) {
		for (size_t i = 0; i < Size_x; i++)
			for (size_t j = 0; j < Size_y; j++)
				if (!close(lhs(i, j), rhs(i, j), eps))
					return false;
		return true;
	}

	template<typename Scalar, size_t rows, size_t equal, size_t cols, bool column_major>
	constexpr inline Mat<Scalar, rows, cols, column_major> operator*(const Mat<Scalar, rows, equal, column_major>& lhs, const Mat<Scalar, equal, cols, column_major>& rhs) {
		Mat<Scalar, rows, cols, column_major > result{};
		for (size_t col = 0; col < cols; col++) {
			for (size_t row = 0; row < rows; row++) {
				Scalar tmp{ 0 };
				for (size_t i = 0; i < equal; i++) {
					tmp += lhs(i, row) * rhs(col, i);
				}
				result(col, row) = tmp;
			}
		}
		return result;
	}
	template<typename Scalar, size_t rows, size_t equal, size_t cols>
	constexpr inline Mat<Scalar, rows, cols, false> operator*(const Mat<Scalar, rows, equal, false>& lhs, const Mat<Scalar, equal, cols, false>& rhs) {
		Mat<Scalar, rows, cols, false> result{};
		for (size_t row = 0; row < rows; row++) {
			for (size_t col = 0; col < cols; col++) {
				float tmp{ 0.f };
				for (size_t i = 0; i < equal; i++) {
					tmp += lhs(i, row) * rhs(col, i);
				}
				result(col, row) = tmp;
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