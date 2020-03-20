#pragma once
#include <type_traits>
#include <cstdint>
#include <optional>
#include <array>
#include <concepts>

namespace bla
{
	//C++20 alternative
	//Should work but somehow doesn't
	//template<typename T>
	//concept Is_Scalar = requires std::is_arithmetic<T>::value;
	//replaces
	//typename = typename std::enable_if<std::is_arithmetic<Scalar>::value, Scalar>::type 
	template<typename Scalar,
		typename = typename std::enable_if<std::is_arithmetic<Scalar>::value, Scalar>::type >
	inline constexpr const Scalar& min(const Scalar& a, const Scalar& b) noexcept {
		if ((a <=> b) < 0)
			return a;
		return b;
	}
	template<typename Scalar,
		typename = typename std::enable_if<std::is_arithmetic<Scalar>::value, Scalar>::type >
	inline constexpr const Scalar& max(const Scalar& a, const Scalar& b) noexcept {
		//Three way comparison, why? because it's new
		//No really, no reason to do this
		if ((a <=> b) > 0)
			return a;
		return b;
	}
	template<typename Scalar,
		typename = typename std::enable_if<std::is_arithmetic<Scalar>::value, Scalar>::type >
	inline constexpr const bool close(const Scalar& a, const Scalar& b, const Scalar&eps = Scalar(1e-5)) noexcept {
		return ((a - eps) <= b) && ((a + eps) >= b);
	}

	template<
		typename Scalar,
		size_t Size,
		typename = typename std::enable_if<std::is_arithmetic<Scalar>::value, Scalar>::type>
	class Vec
	{
	private:
		std::array<Scalar, Size> m_data;
	public:
		Vec() noexcept : m_data() {
			for (size_t i = 0; i < Size; i++)
				m_data[i] = 0;
		}
		// I think this one is unnecessary
		// Although you could also argue that the other one is unnecessary and copy is better
		//explicit Vec(Scalar scalar) : m_data() {
		//	for (size_t i = 0; i < Size; i++)
		//		m_data[i] = scalar;
		//}
		//Without explicit: Vec<int, 2> t = 2; would compile
		explicit Vec(Scalar const& scalar) noexcept : m_data()  {
			for (size_t i = 0; i < Size; i++)
				m_data[i] = scalar;
		}
		// For now this only confuses, sticking to initializer lists
		//Vec(Scalar scalar, Scalar scalar...) : m_data() {
		//	if constexpr (Size != sizeof(scalar...))
		//		throw std::out_of_range("Too many parameters");
		//}
		Vec(std::initializer_list<Scalar> scalars) : m_data() {
			if (Size != scalars.size())
				throw std::out_of_range("Too many parameters");
			size_t i = 0;
			for (Scalar s : scalars) {
				m_data[i] = s;
				i++;
			}
		}
		friend inline bool operator==(const Vec& lhs, const Vec& rhs) noexcept {
			for (size_t i = 0; i < Size; i++)
				if (lhs.m_data[i] != rhs.m_data[i]) 
					return false;
			return true;
		}
		friend inline Vec operator+(const Vec& lhs, const Vec& rhs) noexcept {
			Vec result;
			for (size_t i = 0; i < Size; i++)
				result.m_data[i] = lhs.m_data[i] + rhs.m_data[i];
			return result;
		}
		friend inline Vec operator-(const Vec& lhs, const Vec& rhs) noexcept {
			Vec result;
			for (size_t i = 0; i < Size; i++)
				result.m_data[i] = lhs.m_data[i] - rhs.m_data[i];
			return result;
		}
		/*
		*	elementwise multiplication
		*/
		friend inline Vec operator*(const Vec& lhs, const Vec& rhs) noexcept {
			Vec result;
			for (size_t i = 0; i < Size; i++)
				result.m_data[i] = lhs.m_data[i] * rhs.m_data[i];
			return result;
		}
		friend inline bool operator==(const Vec& lhs, const Scalar& rhs) noexcept {
			for (size_t i = 0; i < Size; i++)
				if (lhs.m_data[i] != rhs)
					return false;
			return true;
		}
		friend inline Vec operator+(const Vec& lhs, const Scalar& rhs) noexcept {
			Vec result;
			for (size_t i = 0; i < Size; i++)
				result.m_data[i] = lhs.m_data[i] + rhs;
			return result;
		}
		friend inline Vec operator+(const Scalar& lhs, const Vec& rhs) noexcept {
			Vec result;
			for (size_t i = 0; i < Size; i++)
				result.m_data[i] = lhs + rhs.m_data[i];
			return result;
		}
		friend inline Vec operator-(const Vec& lhs, const Scalar& rhs) noexcept {
			Vec result;
			for (size_t i = 0; i < Size; i++)
				result.m_data[i] = lhs.m_data[i] - rhs;
			return result;
		}
		friend inline Vec operator-(const Scalar& lhs, const Vec& rhs) noexcept {
			Vec result;
			for (size_t i = 0; i < Size; i++)
				result.m_data[i] = lhs - rhs.m_data[i];
			return result;
		}
		friend inline Vec operator*(const Vec& lhs, const Scalar& rhs) noexcept {
			Vec result;
			for (size_t i = 0; i < Size; i++)
				result.m_data[i] = lhs.m_data[i] * rhs;
			return result;
		}
		friend inline Vec operator*(const Scalar& lhs, const Vec& rhs) noexcept {
			Vec result;
			for (size_t i = 0; i < Size; i++)
				result.m_data[i] = lhs * rhs.m_data[i];
			return result;
		}
		friend inline Vec operator/(const Vec& lhs, const Scalar& rhs) noexcept {
			Vec result;
			for (size_t i = 0; i < Size; i++)
				result.m_data[i] = lhs.m_data[i] / rhs;
			return result;
		}
		friend inline Vec operator/(const Scalar& lhs, const Vec& rhs) noexcept {
			Vec result;
			for (size_t i = 0; i < Size; i++)
				result.m_data[i] = lhs / rhs.m_data[i];
			return result;
		}
		inline Scalar L2_square() const noexcept {
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
		inline Vec& operator=(const Scalar& rhs) noexcept {
			for (size_t i = 0; i < Size; i++)
				m_data[i] += rhs;
			return *this;
		}
		inline Vec& operator=(Scalar& rhs) noexcept {
			for (size_t i = 0; i < Size; i++)
				m_data[i] = rhs;
			return *this;
		}
		inline Vec& operator+=(const Vec& rhs) noexcept {
			for(size_t i = 0; i<Size; i++)
				m_data[i] += rhs.m_data[i];
			return *this;
		}
		inline Vec& operator-=(const Vec& rhs) noexcept {
			for (size_t i = 0; i < Size; i++)
				m_data[i] -= rhs.m_data[i];
			return *this;
		}
		inline Vec& operator*=(const Vec& rhs) noexcept {
			for (size_t i = 0; i < Size; i++)
				m_data[i] *= rhs.m_data[i];
			return *this;
		}
		inline Vec& operator+=(const Scalar& rhs) noexcept {
			for (size_t i = 0; i < Size; i++)
				m_data[i] += rhs;
			return *this;
		}
		inline Vec& operator-=(const Scalar& rhs) noexcept {
			for (size_t i = 0; i < Size; i++)
				m_data[i] -= rhs;
			return *this;
		}
		inline Vec& operator*=(const Scalar& rhs) noexcept {
			for (size_t i = 0; i < Size; i++)
				m_data[i] *= rhs;
			return *this;
		}
		inline Vec& operator/=(const Scalar& rhs) noexcept {
			for (size_t i = 0; i < Size; i++)
				m_data[i] /= rhs;
			return *this;
		}
		inline const Scalar& operator[] (const size_t index) const noexcept {
			//Just pass through, User responsible for bounds
			//if (Size <= index)
			//	throw std::out_of_range("Index out of range");
			return m_data[index];
		}
		inline Scalar& operator[](const size_t index) noexcept {
			//Just pass through, User responsible for bounds
			//if(Size <= index)
			//	throw std::out_of_range("Index out of range");
			return m_data[index];
		}
		//inline Vec pow(Scalar exponent) {
		//	Vec result;
		//	for (size_t i = 0; i < Size; i++)
		//		result.m_data[i] = pow(m_data[i], exponent);
		//	return result;
		//}
		/// I don't know if this is a good idea, or not. We'll see.
		friend inline Vec operator^(const Vec& lhs, const Scalar& rhs) noexcept {
			Vec result;
			for (size_t i = 0; i < Size; i++)
				result.m_data[i] = pow(lhs.m_data[i], rhs);
			return result;
		}
		/// Even less of a good idea than the one before
		friend inline Vec operator^(const Vec& lhs, const Vec& rhs) noexcept {
			Vec result;
			for (size_t i = 0; i < Size; i++)
				result.m_data[i] = pow(lhs.m_data[i], rhs.m_data[i]);
			return result;
		}
		inline Scalar dot(const Vec& rhs) const noexcept {
			// Not sure if this is better than = 0, but this way we correctly have a Scalar
			Scalar result = Scalar();
			for (size_t i = 0; i < Size; i++)
				result += m_data[i] * rhs.m_data[i];
			return result;
		}
		inline Vec& apply_fun( Scalar fun(Scalar)) noexcept {
			for (size_t i = 0; i < Size; i++)
				m_data[i] = fun(m_data[i]);
			return *this;
		}
		
		inline Scalar& x() noexcept {
			static_assert(Size > 0);
			return m_data[0];
		}
		inline Scalar& y() noexcept {
			static_assert(Size > 1);
			return m_data[1];
		}
		inline Scalar& z() noexcept {
			static_assert(Size > 2);
			return m_data[2];
		}
		inline Scalar& w() noexcept {
			static_assert(Size > 3);
			return m_data[3];
		}
		inline Scalar& r() noexcept {
			static_assert(Size > 0);
			return m_data[0];
		}
		inline Scalar& g() noexcept {
			static_assert(Size > 1);
			return m_data[1];
		}
		inline Scalar& b() noexcept {
			static_assert(Size > 2);
			return m_data[2];
		}
		inline Scalar& a() noexcept {
			static_assert(Size > 3);
			return m_data[3];
		}
	};
	template<typename Scalar,
		size_t Size,
		typename = typename std::enable_if<std::is_arithmetic<Scalar>::value, Scalar>::type >
	inline Scalar dot(Vec<Scalar, Size>& lhs, Vec<Scalar, Size>& rhs) {
		// Not sure if this is better than = 0, but this way we correctly have a Scalar
		Scalar result = Scalar();
		for (size_t i = 0; i < Size; i++)
			result += lhs[i] * rhs[i];
		return result;
	}
	template<
		typename Scalar,
		size_t Size_x,
		size_t Size_y,
		typename = typename std::enable_if<std::is_arithmetic<Scalar>::value, Scalar>::type>
	class Mat
	{
	private:
		std::array<Scalar, Size_x* Size_y> m_data;
	public:
		Mat() : m_data() {
			for (size_t i = 0; i < Size_x * Size_y; i++)
				m_data[i] = 0;
		}
		Mat(Scalar scalar) : m_data() {
			for (size_t i = 0; i < Size_x * Size_y; i++)
				m_data[i] = scalar;
		}
		Mat(const Scalar& scalar) : m_data() {
			for (size_t i = 0; i < Size_x * Size_y; i++)
				m_data[i] = scalar;
		}
		explicit Mat(Vec<Scalar,Size_x>& vec) : m_data() {
			static_assert(Size_y == 1);
			for (size_t i = 0; i < Size_x; i++)
				m_data[i] = vec[i];
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
		
		friend inline Mat operator+(const Mat& lhs, const Mat& rhs) {
			Mat result;
			for (size_t i = 0; i < Size_x * Size_y; i++)
				result.m_data[i] = lhs.m_data[i] + rhs.m_data[i];
			return result;
		}
		friend inline Mat operator-(const Mat& lhs, const Mat& rhs) {
			Mat result;
			for (size_t i = 0; i < Size_x * Size_y; i++)
				result.m_data[i] = lhs.m_data[i] - rhs.m_data[i];
			return result;
		}
		friend inline Mat operator+(const Mat& lhs, const Scalar& rhs) {
			Mat result;
			for (size_t i = 0; i < Size_x * Size_y; i++)
				result.m_data[i] = lhs.m_data[i] + rhs;
			return result;
		}
		friend inline Mat operator-(const Mat& lhs, const Scalar& rhs) {
			Mat result;
			for (size_t i = 0; i < Size_x * Size_y; i++)
				result.m_data[i] = lhs.m_data[i] - rhs;
			return result;
		}
		friend inline Mat operator*(const Mat& lhs, const Scalar& rhs) {
			Mat result;
			for (size_t i = 0; i < Size_x * Size_y; i++)
				result.m_data[i] = lhs.m_data[i] * rhs;
			return result;
		}
		friend inline Mat operator/(const Mat& lhs, const Scalar& rhs) {
			Mat result;
			for (size_t i = 0; i < Size_x * Size_y; i++)
				result.m_data[i] = lhs.m_data[i] / rhs;
			return result;
		}
		friend inline Vec<Scalar, Size_y> operator*(const Mat& lhs, const Vec<Scalar, Size_y>& rhs) {
			Vec<Scalar, Size_y> result;
			for (size_t y = 0; y < Size_y; y++) {
				result.m_data[y] = lhs.col(y).dot(rhs);
			}
			return result;
		}
		friend inline bool operator==(const Mat& lhs, const Mat& rhs) {
			for (size_t i = 0; i < Size_x * Size_y; i++)
				if (lhs.m_data[i] != rhs.m_data[i])
					return false;
			return true;
		}
		friend inline bool operator==(const Mat& lhs, const Scalar& rhs) {
			for (size_t i = 0; i < Size_x * Size_y; i++)
				if (lhs.m_data[i] != rhs)
					return false;
			return true;
		}
		friend inline bool close(const Mat& lhs, const Scalar& rhs, const Scalar& eps = Scalar(1e-5)) {
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
					m_data[y*Size_x + x] = old_row.dot(rhs.col(x));
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
				ret[x] = m_data[x+index*Size_x];
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
			return m_data[y*Size_x + x];
		}
		//TODO, this is ugly
		Mat<Scalar, Size_x -1, Size_y -1> cofactor(const size_t i, const size_t j) const {
			Mat<Scalar, Size_x - 1, Size_y - 1> ret;
			size_t x_offset = 0;
			for (size_t x = 0; x < Size_x-1; x++) {
				if (x == i) x_offset = 1;
				size_t y_offset = 0;
				for (size_t y = x; y < Size_y-1; y++) {
					if (y == j) y_offset = 1;
					ret(x, y) = this->operator()(x+x_offset,y+y_offset);
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
				Scalar determinante = m_data[0] * (m_data[5]*(m_data[10]*m_data[15] - m_data[11]*m_data[14]) + m_data[13]*(m_data[6]* m_data[11] - m_data[7]* m_data[10]) + m_data[9]*(m_data[7]*m_data[14] - m_data[6]* m_data[15]));
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
				if (close(determinante,Scalar(0)))
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
						res(i, j) = (((i + j) & 0x1) ? Scalar(-1) : Scalar(1) )* this->cofactor(i,j).determinante()*determinante;
					}
				}
				return res;
			}
			else  {
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
	};
	template<typename Scalar, size_t Size_x, size_t Size_y, typename = typename std::enable_if<std::is_arithmetic<Scalar>::value, Scalar>::type >
	inline bool close(const Mat<Scalar, Size_x, Size_y>& lhs, const Mat<Scalar, Size_x, Size_y>& rhs, const Scalar& eps = Scalar(1e-5)) {
		for (size_t i = 0; i < Size_x * Size_y; i++)
			if (!close(lhs[i], rhs[i], eps))
				return false;
		return true;
	}
	template<typename Scalar, size_t rows, size_t equal, size_t cols, typename = typename std::enable_if<std::is_arithmetic<Scalar>::value, Scalar>::type >
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
	template<typename Scalar, typename = typename std::enable_if<std::is_arithmetic<Scalar>::value, Scalar>::type >
	class Quaternion {
	private:
		Scalar m_real;
		Vec<Scalar, 3> m_imaginary;
	};

	typedef Vec<float, 2> vec2;
	typedef Vec<float, 3> vec3;
	typedef Vec<float, 4> vec4;
	typedef Vec<double, 2> vec2_d;
	typedef Vec<double, 3> vec3_d;
	typedef Vec<double, 4> vec4_d;
	typedef Vec<int32_t, 2> vec2_i;
	typedef Vec<int32_t, 3> vec3_i;
	typedef Vec<int32_t, 4> vec4_i;
	typedef Vec<int64_t, 2> vec2_l;
	typedef Vec<int64_t, 3> vec3_l;
	typedef Vec<int64_t, 4> vec4_l;
	typedef Vec<int8_t, 3> vec2_b;
	typedef Vec<int8_t, 3> vec3_b;
	typedef Vec<int8_t, 4> vec4_b;

	/*
	typedef Vec<uint32_t, 2> vec2_ui;
	typedef Vec<uint32_t, 3> vec3_ui;
	typedef Vec<uint32_t, 4> vec4_ui;
	typedef Vec<uint64_t, 2> vec2_ul;
	typedef Vec<uint64_t, 3> vec3_ul;
	typedef Vec<uint64_t, 4> vec4_ul;
	typedef Vec<uint8_t, 2> vec2_ub;
	typedef Vec<uint8_t, 3> vec3_ub;
	typedef Vec<uint8_t, 4> vec4_ub;
	*/



	typedef Mat<float, 2, 2> mat22;
	typedef Mat<float, 3, 3> mat33;
	typedef Mat<float, 4, 4> mat44;
	typedef Mat<double, 2, 2> mat22_d;
	typedef Mat<double, 3, 3> mat33_d;
	typedef Mat<double, 4, 4> mat44_d;
	typedef Mat<int32_t, 2, 2> mat22_i;
	typedef Mat<int32_t, 3, 3> mat33_i;
	typedef Mat<int32_t, 4, 4> mat44_i;
	typedef Mat<int64_t, 2, 2> mat22_l;
	typedef Mat<int64_t, 3, 3> mat33_l;
	typedef Mat<int64_t, 4, 4> mat44_l;
	typedef Mat<int8_t, 2, 2> mat22_b;
	typedef Mat<int8_t, 3, 3> mat33_b;
	typedef Mat<int8_t, 4, 4> mat44_b;
	/*
	typedef Mat<uint32_t, 2, 2> mat22_ui;
	typedef Mat<uint32_t, 3, 3> mat33_ui;
	typedef Mat<uint32_t, 4, 4> mat44_ui;
	typedef Mat<uint64_t, 2, 2> mat22_ul;
	typedef Mat<uint64_t, 3, 3> mat33_ul;
	typedef Mat<uint64_t, 4, 4> mat44_ul;
	typedef Mat<uint8_t, 2, 2> mat22_ub;
	typedef Mat<uint8_t, 3, 3> mat33_ub;
	typedef Mat<uint8_t, 4, 4> mat44_ub;
	*/

	typedef Quaternion<float>  quat;
	typedef Quaternion<double> quatd;


}