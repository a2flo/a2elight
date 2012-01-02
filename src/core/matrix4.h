/*
 *  Albion 2 Engine "light"
 *  Copyright (C) 2004 - 2012 Florian Ziesche
 *  
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; version 2 of the License only.
 *  
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *  
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifndef __MATRIX4_H__
#define __MATRIX4_H__

#include "core/cpp_headers.h"
#include "core/basic_math.h"

template <typename T> class matrix4;
typedef matrix4<float> matrix4f;
typedef matrix4<double> matrix4d;
typedef matrix4<int> matrix4i;
typedef matrix4<unsigned int> matrix4ui;
typedef matrix4<bool> matrix4b;

template <typename T> class vector4;

template <typename T> class A2E_API __attribute__((packed)) matrix4 {
public:
	T data[16];
	
	matrix4() { identity();	}
	matrix4(const matrix4<T>& m4) { memcpy(&data[0], &m4.data[0], sizeof(T)*16); }
	matrix4(const matrix4<T>* m4) { memcpy(&data[0], &m4->data[0], sizeof(T)*16); }
	matrix4(const T& m0, const T& m1, const T& m2, const T& m3,
			const T& m4, const T& m5, const T& m6, const T& m7,
			const T& m8, const T& m9, const T& m10, const T& m11,
			const T& m12, const T& m13, const T& m14, const T& m15) {
		data[0] = m0; data[1] = m1; data[2] = m2; data[3] = m3;
		data[4] = m4; data[5] = m5; data[6] = m6; data[7] = m7;
		data[8] = m8; data[9] = m9; data[10] = m10; data[11] = m11;
		data[12] = m12; data[13] = m13; data[14] = m14; data[15] = m15;
	}
	template <typename U> matrix4(const matrix4<U>& mat4) {
		for(size_t mi = 0; mi < 16; mi++) {
			data[mi] = mat4.data[mi];
		}
	}
	~matrix4() {}
	
	matrix4& operator=(const matrix4& mat) {
		memcpy(&data[0], &mat.data[0], sizeof(T)*16);
		return *this;
	}
	
	friend ostream& operator<<(ostream& output, const matrix4<T>& m4) {
		output << "/" << m4.data[0] << "\t" << m4.data[4] << "\t" << m4.data[8] << "\t" << m4.data[12] << "\\" << endl;
		output << "|" << m4.data[1] << "\t" << m4.data[5] << "\t" << m4.data[9] << "\t" << m4.data[13] << "|" << endl;
		output << "|" << m4.data[2] << "\t" << m4.data[6] << "\t" << m4.data[10] << "\t" << m4.data[14] << "|" << endl;
		output << "\\" << m4.data[3] << "\t" << m4.data[7] << "\t" << m4.data[11] << "\t" << m4.data[15] << "/" << endl;
		return output;
	}
	
	T& operator[](const size_t index) { return data[index]; }
	const T& operator[](const size_t index) const { return data[index]; }
	
	matrix4 operator*(const matrix4& mat) const;
	matrix4& operator*=(const matrix4& mat);
	
	// basic functions
	matrix4& invert();
	matrix4& identity();
	matrix4& transpose();
	
	// transformations
	matrix4& translate(const T x, const T y, const T z);
	matrix4& set_translation(const T x, const T y, const T z);
	matrix4& scale(const T x, const T y, const T z);
	
	matrix4& rotate_x(const T x);
	matrix4& rotate_y(const T y);
	matrix4& rotate_z(const T z);
	
	//
	template<size_t col> vector4<T> column() const {
		return vector4<T>(data[col*4],
						  data[(col*4) + 1],
						  data[(col*4) + 2],
						  data[(col*4) + 3]);
	}
	
	// projection
	matrix4& perspective(const T fov, const T aspect, const T z_near, const T z_far);
	matrix4& ortho(const T left, const T right, const T bottom, const T top, const T z_near, const T z_far);
};

template<typename T> matrix4<T> matrix4<T>::operator*(const matrix4<T>& mat) const {
	matrix4 mul_mat;
	for(size_t mi = 0; mi < 4; mi++) { // column
		for(size_t mj = 0; mj < 4; mj++) { // row
			mul_mat.data[(mi*4) + mj] = ((T)0);
			for(size_t mk = 0; mk < 4; mk++) { // mul iteration
				mul_mat.data[(mi*4) + mj] += data[(mi*4) + mk] * mat.data[(mk*4) + mj];
			}
		}
	}
	return mul_mat;
}

template<typename T> matrix4<T>& matrix4<T>::operator*=(const matrix4<T>& mat) {
	*this = *this * mat;
	return *this;
}

template<typename T> matrix4<T>& matrix4<T>::invert() {
	matrix4<T> mat;
	
	float p00 = data[10] * data[15];
	float p01 = data[14] * data[11];
	float p02 = data[6] * data[15];
	float p03 = data[14] * data[7];
	float p04 = data[6] * data[11];
	float p05 = data[10] * data[7];
	float p06 = data[2] * data[15];
	float p07 = data[14] * data[3];
	float p08 = data[2] * data[11];
	float p09 = data[10] * data[3];
	float p10 = data[2] * data[7];
	float p11 = data[6] * data[3];
	
	mat.data[0] = (p00 * data[5] + p03 * data[9] + p04 * data[13]) - (p01 * data[5] + p02 * data[9] + p05 * data[13]);
	mat.data[1] = (p01 * data[1] + p06 * data[9] + p09 * data[13]) - (p00 * data[1] + p07 * data[9] + p08 * data[13]);
	mat.data[2] = (p02 * data[1] + p07 * data[5] + p10 * data[13]) - (p03 * data[1] + p06 * data[5] + p11 * data[13]);
	mat.data[3] = (p05 * data[1] + p08 * data[5] + p11 * data[9]) - (p04 * data[1] + p09 * data[5] + p10 * data[9]);
	mat.data[4] = (p01 * data[4] + p02 * data[8] + p05 * data[12]) - (p00 * data[4] + p03 * data[8] + p04 * data[12]);
	mat.data[5] = (p00 * data[0] + p07 * data[8] + p08 * data[12]) - (p01 * data[0] + p06 * data[8] + p09 * data[12]);
	mat.data[6] = (p03 * data[0] + p06 * data[4] + p11 * data[12]) - (p02 * data[0] + p07 * data[4] + p10 * data[12]);
	mat.data[7] = (p04 * data[0] + p09 * data[4] + p10 * data[8]) - (p05 * data[0] + p08 * data[4] + p11 * data[8]);
	
	float q00 = data[8] * data[13];
	float q01 = data[12] * data[9];
	float q02 = data[4] * data[13];
	float q03 = data[12] * data[5];
	float q04 = data[4] * data[9];
	float q05 = data[8] * data[5];
	float q06 = data[0] * data[13];
	float q07 = data[12] * data[1];
	float q08 = data[0] * data[9];
	float q09 = data[8] * data[1];
	float q10 = data[0] * data[5];
	float q11 = data[4] * data[1];
	
	mat.data[8] = (q00 * data[7] + q03 * data[11] + q04 * data[15]) - (q01 * data[7] + q02 * data[11] + q05 * data[15]);
	mat.data[9] = (q01 * data[3] + q06 * data[11] + q09 * data[15]) - (q00 * data[3] + q07 * data[11] + q08 * data[15]);
	mat.data[10] = (q02 * data[3] + q07 * data[7] + q10 * data[15]) - (q03 * data[3] + q06 * data[7] + q11 * data[15]);
	mat.data[11] = (q05 * data[3] + q08 * data[7] + q11 * data[11]) - (q04 * data[3] + q09 * data[7] + q10 * data[11]);
	mat.data[12] = (q02 * data[10] + q05 * data[14] + q01 * data[6]) - (q04 * data[14] + q00 * data[6] + q03 * data[10]);
	mat.data[13] = (q08 * data[14] + q00 * data[2] + q07 * data[10]) - (q06 * data[10] + q09 * data[14] + q01 * data[2]);
	mat.data[14] = (q06 * data[6] + q11 * data[14] + q03 * data[2]) - (q10 * data[14] + q02 * data[2] + q07 * data[6]);
	mat.data[15] = (q10 * data[10] + q04 * data[2] + q09 * data[6]) - (q08 * data[6] + q11 * data[10] + q05 * data[2]);
	
	float mx = 1.0f / (data[0] * mat.data[0] + data[4] * mat.data[1] + data[8] * mat.data[2] + data[12] * mat.data[3]);
	
	for(size_t mi = 0; mi < 4; mi++) {
		for(size_t mj = 0; mj < 4; mj++) {
			mat.data[(mi*4) + mj] *= mx;
		}
	}
	
	*this = mat;
	return *this;
}

template<typename T> matrix4<T>& matrix4<T>::identity() {
	data[0] = 1.0f;
	data[1] = 0.0f;
	data[2] = 0.0f;
	data[3] = 0.0f;
	
	data[4] = 0.0f;
	data[5] = 1.0f;
	data[6] = 0.0f;
	data[7] = 0.0f;
	
	data[8] = 0.0f;
	data[9] = 0.0f;
	data[10] = 1.0f;
	data[11] = 0.0f;
	
	data[12] = 0.0f;
	data[13] = 0.0f;
	data[14] = 0.0f;
	data[15] = 1.0f;
	
	return *this;
}

template<typename T> matrix4<T>& matrix4<T>::transpose() {
	matrix4 tmp = *this;
	for(size_t mi = 0; mi < 4; mi++) {
		for(size_t mj = 0; mj < 4; mj++) {
			data[(mi*4) + mj] = tmp.data[(mj*4) + mi];
		}
	}
	return *this;
}

// transformations
template<typename T> matrix4<T>& matrix4<T>::translate(const T x, const T y, const T z) {
	matrix4 trans_mat;
	trans_mat[12] = x;
	trans_mat[13] = y;
	trans_mat[14] = z;
	
	*this *= trans_mat;
	
	return *this;
}

template<typename T> matrix4<T>& matrix4<T>::set_translation(const T x, const T y, const T z) {
	data[12] = x;
	data[13] = y;
	data[14] = z;
	return *this;
}

template<typename T> matrix4<T>& matrix4<T>::scale(const T x, const T y, const T z) {
	matrix4 scale_mat;
	scale_mat[0] = x;
	scale_mat[5] = y;
	scale_mat[10] = z;
	
	*this *= scale_mat;
	
	return *this;
}

template<typename T> matrix4<T>& matrix4<T>::rotate_x(const T x) {
	T angle = DEG2RAD(x);
	T sinx, cosx;
	sinx = sin(angle);
	cosx = cos(angle);
	
	data[0] = 1.0f;
	data[1] = 0.0f;
	data[2] = 0.0f;
	data[3] = 0.0f;
	
	data[4] = 0.0f;
	data[5] = cosx;
	data[6] = sinx;
	data[7] = 0.0f;
	
	data[8] = 0.0f;
	data[9] = -sinx;
	data[10] = cosx;
	data[11] = 0.0f;
	
	data[12] = 0.0f;
	data[13] = 0.0f;
	data[14] = 0.0f;
	data[15] = 1.0f;
	
	return *this;
}

template<typename T> matrix4<T>& matrix4<T>::rotate_y(const T y) {
	T angle = DEG2RAD(y);
	T siny, cosy;
	siny = sin(angle);
	cosy = cos(angle);
	
	data[0] = cosy;
	data[1] = 0.0f;
	data[2] = -siny;
	data[3] = 0.0f;
	
	data[4] = 0.0f;
	data[5] = 1.0f;
	data[6] = 0.0f;
	data[7] = 0.0f;
	
	data[8] = siny;
	data[9] = 0.0f;
	data[10] = cosy;
	data[11] = 0.0f;
	
	data[12] = 0.0f;
	data[13] = 0.0f;
	data[14] = 0.0f;
	data[15] = 1.0f;
	
	return *this;
}

template<typename T> matrix4<T>& matrix4<T>::rotate_z(const T z) {
	T angle = DEG2RAD(z);
	T sinz, cosz;
	sinz = sin(angle);
	cosz = cos(angle);
	
	data[0] = cosz;
	data[1] = sinz;
	data[2] = 0.0f;
	data[3] = 0.0f;
	
	data[4] = -sinz;
	data[5] = cosz;
	data[6] = 0.0f;
	data[7] = 0.0f;
	
	data[8] = 0.0f;
	data[9] = 0.0f;
	data[10] = 1.0f;
	data[11] = 0.0f;
	
	data[12] = 0.0f;
	data[13] = 0.0f;
	data[14] = 0.0f;
	data[15] = 1.0f;
	
	return *this;
}

// projection
template<typename T> matrix4<T>& matrix4<T>::perspective(const T fov, const T aspect, const T z_near, const T z_far) {
	const float f = ((T)1) / tanf(fov * _PIDIV360);
	
	////
	data[0] = f / aspect;
	data[1] = (T)0;
	data[2] = (T)0;
	data[3] = (T)0;
	
	data[4] = (T)0;
	data[5] = f;
	data[6] = (T)0;
	data[7] = (T)0;
	
	data[8] = (T)0;
	data[9] = (T)0;
	data[10] = (z_far + z_near) / (z_near - z_far);
	data[11] = -1.0f;
	
	data[12] = (T)0;
	data[13] = (T)0;
	data[14] = (((T)2) * z_far * z_near) / (z_near - z_far);
	data[15] = (T)0;
	
	return *this;
}

template<typename T> matrix4<T>& matrix4<T>::ortho(const T left, const T right, const T bottom, const T top, const T z_near, const T z_far) {
	T r_l = right - left;
	T t_b = top - bottom;
	T f_n = z_far - z_near;
	
	data[0] = ((T)2) / r_l;
	data[1] = ((T)0);
	data[2] = ((T)0);
	data[3] = ((T)0);
	
	data[4] = ((T)0);
	data[5] = ((T)2) / t_b;
	data[6] = ((T)0);
	data[7] = ((T)0);
	
	data[8] = ((T)0);
	data[9] = ((T)0);
	data[10] = -((T)2) / f_n;
	data[11] = ((T)0);
	
	data[12] = -(right + left) / (right - left);
	data[13] = -(top + bottom) / (top - bottom);
	data[14] = -(z_far + z_near) / (z_far - z_near);
	data[15] = ((T)1);
	
	return *this;
}

#if defined(A2E_EXPORT)
// only instantiate this in the matrix4.cpp
extern template class matrix4<float>;
extern template class matrix4<double>;
extern template class matrix4<int>;
extern template class matrix4<unsigned int>;
extern template class matrix4<bool>;
#endif

#endif
