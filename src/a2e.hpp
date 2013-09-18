/*
 *  Albion 2 Engine "light"
 *  Copyright (C) 2004 - 2013 Florian Ziesche
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

#include "floor/floor.hpp"

#include "engine.hpp"
#include "global.hpp"

#include "gui/gui.hpp"
#include "gui/image.hpp"
#include "gui/font_manager.hpp"
#include "gui/font.hpp"

#include "gui/objects/gui_object.hpp"
#include "gui/objects/gui_window.hpp"
#include "gui/objects/gui_button.hpp"
#include "gui/objects/gui_text.hpp"
#include "gui/objects/gui_input_box.hpp"

#include "gui/style/gui_theme.hpp"
#include "gui/style/gui_surface.hpp"

#include "particle/particle.hpp"

#include "rendering/extensions.hpp"
#include "rendering/gfx2d.hpp"
#include "rendering/rtt.hpp"
#include "rendering/shader.hpp"
#include "rendering/texman.hpp"
#include "rendering/texture_object.hpp"
#if !defined(A2E_IOS)
#include "rendering/gl_funcs.hpp"
#else
#include "rendering/gles_compat.hpp"
#endif
#include "rendering/renderer/shader_base.hpp"
#include "rendering/renderer/shader_object.hpp"
#include "rendering/renderer/a2e_shader.hpp"
#if !defined(A2E_IOS)
#include "rendering/renderer/gl3/shader_gl3.hpp"
#include "rendering/gl_timer.hpp"
#else
#include "rendering/renderer/gles2/shader_gles2.hpp"
#endif

#include "scene/scene.hpp"
#include "scene/camera.hpp"
#include "scene/light.hpp"
#include "scene/model/a2ematerial.hpp"
#include "scene/model/a2estatic.hpp"
#include "scene/model/a2emodel.hpp"
