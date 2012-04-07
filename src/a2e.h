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

#include "engine.h"
#include "global.h"

#include "cl/opencl.h"

#include "core/platform.h"
#include "core/core.h"
#include "core/file_io.h"
#include "core/matrix4.h"
#include "core/logger.h"
#include "core/xml.h"
#include "core/vector2.h"
#include "core/vector3.h"
#include "core/vector4.h"
#include "core/util.h"
#include "core/ray.h"
#include "core/bbox.h"
#include "core/functor.h"

#include "gui/gui.h"
#include "gui/event.h"
#include "gui/image.h"

#include "particle/particle.h"

#include "rendering/extensions.h"
#include "rendering/gfx2d.h"
#include "rendering/rtt.h"
#include "rendering/shader.h"
#include "rendering/texman.h"
#include "rendering/texture_object.h"
#if !defined(A2E_IOS)
#include "rendering/gl_funcs.h"
#else
#include "rendering/gles_compat.h"
#endif
#include "rendering/renderer/shader_base.h"
#include "rendering/renderer/shader_object.h"
#include "rendering/renderer/a2e_shader.h"
#if !defined(A2E_IOS)
#include "rendering/renderer/gl3/shader_gl3.h"
#else
#include "rendering/renderer/gles2/shader_gles2.h"
#endif

#include "scene/scene.h"
#include "scene/camera.h"
#include "scene/light.h"
#include "scene/model/a2ematerial.h"
#include "scene/model/a2estatic.h"
#include "scene/model/a2emodel.h"

