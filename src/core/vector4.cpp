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

#include "vector4.h"

#if defined(A2E_EXPORT)
template class vector4<float>;
template class vector4<unsigned int>;
template class vector4<int>;
template class vector4<short>;
template class vector4<char>;
template class vector4<unsigned short>;
template class vector4<unsigned char>;
template class vector4<bool>;
template class vector4<size_t>;
template class vector4<ssize_t>;
#endif
