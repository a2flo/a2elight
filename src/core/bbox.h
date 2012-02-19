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

#ifndef __BBOX_H__
#define __BBOX_H__

#include "core/cpp_headers.h"
#include "core/vector3.h"
#include "core/ray.h"

#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wnon-virtual-dtor"
#elif defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wnon-virtual-dtor"
#endif

class A2E_API __attribute__((packed, aligned(4))) bbox {
public:
	float3 min;
	float3 max;
	
	bbox() : min(__FLT_MAX__), max(-__FLT_MAX__) {}
	bbox(const bbox& box) : min(box.min), max(box.max) {}
	bbox(const float3& bmin, const float3& bmax) : min(bmin), max(bmax) {}
	~bbox() {}

	void extend(const float3& v) {
		min.min(v);
		max.max(v);
	}
	
	void extend(const bbox& box) {
		min.min(box.min);
		max.max(box.max);
	}

	static bbox empty() {
		bbox ret;
		ret.min = float3(__FLT_MAX__);
		ret.max = float3(-__FLT_MAX__);
		return ret;
	}
	
	float3 diagonal() const {
		return max - min;
	}
	
	float3 center() const {
		return (min + max) * 0.5f;
	}
	
	bbox& operator=(const bbox& box) {
		min = box.min;
		max = box.max;
		return *this;
	}
	
	friend ostream& operator<<(ostream& output, const bbox& box) {
		output << "(Min: " << box.min << ", Max: " << box.max << ")";
		return output;
	}
	
	virtual void intersect(pair<float, float>& ret, const ray& r) const {
		float3 v1 = min, v2 = max;
		const float3 bbox_eps(0.0000001f);
		
		float3 div = r.direction;
		div.set_if(div.abs() < bbox_eps, bbox_eps);
		
		float3 t1 = (v1 - r.origin) / div;
		float3 t2 = (v2 - r.origin) / div;
		
		float3 tmin = float3::min(t1, t2);
		float3 tmax = float3::max(t1, t2);
		
		ret.first = std::max(std::max(tmin.x, tmin.y), tmin.z);
		ret.second = std::min(std::min(tmax.x, tmax.y), tmax.z);
	}
	virtual bool is_intersection(const ray& r) const {
		pair<float, float> ret;
		intersect(ret, r);
		return (ret.first <= ret.second);
	}
	
	virtual bool contains(const float3& p) const {
		if(((p.x >= min.x && p.x <= max.x) || (p.x <= min.x && p.x >= max.x)) &&
		   ((p.y >= min.y && p.y <= max.y) || (p.y <= min.y && p.y >= max.y)) &&
		   ((p.z >= min.z && p.z <= max.z) || (p.z <= min.z && p.z >= max.z))) {
			return true;
		}
		return false;
	}

};

//! extended bound box (including position and model view matrix)
class A2E_API __attribute__((packed)) extbbox : public bbox {
public:
	float3 pos;
	matrix4f mview;
	
	extbbox() : pos(), mview() {}
	extbbox(const extbbox& ebox) : pos(ebox.pos), mview(ebox.mview) {}
	extbbox(const float3& bmin, const float3& bmax, const float3& bpos, const matrix4f& bmview) : bbox(bmin, bmax), pos(bpos), mview(bmview) {}
	~extbbox() {}
	
	extbbox& operator=(const extbbox& box) {
		min = box.min;
		max = box.max;
		pos = box.pos;
		mview = box.mview;
		return *this;
	}
	
	virtual bool contains(const float3& p) const {
		float3 tp(p);
		tp -= pos;
		tp *= mview;
		return bbox::contains(tp);
	}
	
	virtual void intersect(pair<float, float>& ret, const ray& r) const {
		ray tr(r);
		tr.origin -= pos;
		tr.origin *= mview;
		tr.origin += pos;
		tr.direction *= mview;
		tr.direction.normalize();
		
		bbox::intersect(ret, tr);
	}
	
	virtual bool is_intersection(const extbbox& box) const {
		// TODO: implement this at some point:
		// http://en.wikipedia.org/wiki/Separating_axis_theorem
		// http://www.gamasutra.com/view/feature/3383/simple_intersection_tests_for_games.php?page=5
		return false;
	}
	
	virtual bool is_intersection(const ray& r) const {
		ray tr(r);
		tr.origin -= pos;
		tr.origin *= mview;
		tr.origin += pos;
		tr.direction *= mview;
		tr.direction.normalize();
		
		return bbox::is_intersection(tr);
	}
	
	static extbbox empty() {
		extbbox ret;
		ret.min = float3(__FLT_MAX__);
		ret.max = float3(-__FLT_MAX__);
		ret.mview.identity();
		ret.pos = float3(0.0f);
		return ret;
	}
	
	friend ostream& operator<<(ostream& output, const extbbox& box) {
		output << "(Min: " << box.min << ", Max: " << box.max;
		output << ", Pos: " << box.pos << ")" << endl;
		output << box.mview << endl;
		return output;
	}
	
};

#if defined(__clang__)
#pragma clang diagnostic pop
#elif defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

#endif

