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

#include "vector3.h"

#if defined(A2E_EXPORT)
// instantiate
A2E_API template class vector3<float>;
A2E_API template class vector3<double>;
A2E_API template class vector3<unsigned int>;
A2E_API template class vector3<int>;
A2E_API template class vector3<short>;
A2E_API template class vector3<unsigned short>;
A2E_API template class vector3<char>;
A2E_API template class vector3<unsigned char>;
//A2E_API template class vector3<bool>;
A2E_API template class vector3<size_t>;
A2E_API template class vector3<ssize_t>;
#endif

template<> A2E_API vector3<float> vector3<float>::abs() const {
	return vector3<float>(fabs(x), fabs(y), fabs(z));
}

template<> A2E_API vector3<bool> vector3<bool>::abs() const {
	return vector3<bool>(x, y, z);
}

template<> A2E_API bool vector3<bool>::any() const {
	return (x || y || z);
}

template<> A2E_API bool vector3<bool>::all() const {
	return (x && y && z);
}

template<> A2E_API bool3 vector3<bool>::operator^(const bool3& bv) const {
	return bool3(x ^ bv.x, y ^ bv.y, z ^ bv.z);
}

template<> A2E_API float3 vector3<float>::operator%(const float3& v) const {
	return float3(fmodf(x, v.x), fmodf(y, v.y), fmodf(z, v.z));
}

template<> A2E_API float3& vector3<float>::operator%=(const float3& v) {
	x = fmodf(x, v.x);
	y = fmodf(y, v.y);
	z = fmodf(z, v.z);
	return *this;
}

template<> A2E_API float3& vector3<float>::floor() {
	x = floorf(x);
	y = floorf(y);
	z = floorf(z);
	return *this;
}

template<> A2E_API float3& vector3<float>::ceil() {
	x = ceilf(x);
	y = ceilf(y);
	z = ceilf(z);
	return *this;
}

template<> A2E_API float3& vector3<float>::round() {
	x = roundf(x);
	y = roundf(y);
	z = roundf(z);
	return *this;
}

template<> A2E_API float3 vector3<float>::floored() const {
	return float3(floorf(x), floorf(y), floorf(z));
}

template<> A2E_API float3 vector3<float>::ceiled() const {
	return float3(ceilf(x), ceilf(y), ceilf(z));
}

template<> A2E_API float3 vector3<float>::rounded() const {
	return float3(roundf(x), roundf(y), roundf(z));
}

template<> A2E_API double3 vector3<double>::operator%(const double3& v) const {
	return double3(fmod(x, v.x), fmod(y, v.y), fmod(z, v.z));
}

template<> A2E_API double3& vector3<double>::operator%=(const double3& v) {
	x = fmod(x, v.x);
	y = fmod(y, v.y);
	z = fmod(z, v.z);
	return *this;
}
