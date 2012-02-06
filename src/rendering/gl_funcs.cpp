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

#include "global.h"
#include "gl_funcs.h"
 
#define A2E_EXTENSIONS_DONT_INCLUDE_GL_FUNCS
#include "rendering/extensions.h"
 
#if defined(__WINDOWS__) || defined(WIN_UNIXENV)
#define glGetProcAddress(x) wglGetProcAddress(x)
#define ProcType LPCSTR
#elif defined(__APPLE__)
#define glGetProcAddress(x) aglGetProcAddress(x)
#define ProcType GLubyte*
#else
#define glGetProcAddress(x) glXGetProcAddressARB(x)
#define ProcType GLubyte*
#endif

#ifndef __APPLE__
OGL_API PFNGLRENDERBUFFERSTORAGEMULTISAMPLECOVERAGENVPROC _glRenderbufferStorageMultisampleCoverageNV_ptr = nullptr; // NV_framebuffer_multisample_coverage

#if !defined(__LINUX__)
// OpenGL 1.3
OGL_API PFNGLACTIVETEXTUREPROC _glActiveTexture_ptr = nullptr;
OGL_API PFNGLSAMPLECOVERAGEPROC _glSampleCoverage_ptr = nullptr;
OGL_API PFNGLCOMPRESSEDTEXIMAGE3DPROC _glCompressedTexImage3D_ptr = nullptr;
OGL_API PFNGLCOMPRESSEDTEXIMAGE2DPROC _glCompressedTexImage2D_ptr = nullptr;
OGL_API PFNGLCOMPRESSEDTEXIMAGE1DPROC _glCompressedTexImage1D_ptr = nullptr;
OGL_API PFNGLCOMPRESSEDTEXSUBIMAGE3DPROC _glCompressedTexSubImage3D_ptr = nullptr;
OGL_API PFNGLCOMPRESSEDTEXSUBIMAGE2DPROC _glCompressedTexSubImage2D_ptr = nullptr;
OGL_API PFNGLCOMPRESSEDTEXSUBIMAGE1DPROC _glCompressedTexSubImage1D_ptr = nullptr;
OGL_API PFNGLGETCOMPRESSEDTEXIMAGEPROC _glGetCompressedTexImage_ptr = nullptr;
#endif

// OpenGL 1.4
OGL_API PFNGLBLENDFUNCSEPARATEPROC _glBlendFuncSeparate_ptr = nullptr;
OGL_API PFNGLMULTIDRAWARRAYSPROC _glMultiDrawArrays_ptr = nullptr;
OGL_API PFNGLMULTIDRAWELEMENTSPROC _glMultiDrawElements_ptr = nullptr;
OGL_API PFNGLPOINTPARAMETERFPROC _glPointParameterf_ptr = nullptr;
OGL_API PFNGLPOINTPARAMETERFVPROC _glPointParameterfv_ptr = nullptr;
OGL_API PFNGLPOINTPARAMETERIPROC _glPointParameteri_ptr = nullptr;
OGL_API PFNGLPOINTPARAMETERIVPROC _glPointParameteriv_ptr = nullptr;

// OpenGL 1.5
OGL_API PFNGLGENQUERIESPROC _glGenQueries_ptr = nullptr;
OGL_API PFNGLDELETEQUERIESPROC _glDeleteQueries_ptr = nullptr;
OGL_API PFNGLISQUERYPROC _glIsQuery_ptr = nullptr;
OGL_API PFNGLBEGINQUERYPROC _glBeginQuery_ptr = nullptr;
OGL_API PFNGLENDQUERYPROC _glEndQuery_ptr = nullptr;
OGL_API PFNGLGETQUERYIVPROC _glGetQueryiv_ptr = nullptr;
OGL_API PFNGLGETQUERYOBJECTIVPROC _glGetQueryObjectiv_ptr = nullptr;
OGL_API PFNGLGETQUERYOBJECTUIVPROC _glGetQueryObjectuiv_ptr = nullptr;
OGL_API PFNGLBINDBUFFERPROC _glBindBuffer_ptr = nullptr;
OGL_API PFNGLDELETEBUFFERSPROC _glDeleteBuffers_ptr = nullptr;
OGL_API PFNGLGENBUFFERSPROC _glGenBuffers_ptr = nullptr;
OGL_API PFNGLISBUFFERPROC _glIsBuffer_ptr = nullptr;
OGL_API PFNGLBUFFERDATAPROC _glBufferData_ptr = nullptr;
OGL_API PFNGLBUFFERSUBDATAPROC _glBufferSubData_ptr = nullptr;
OGL_API PFNGLGETBUFFERSUBDATAPROC _glGetBufferSubData_ptr = nullptr;
OGL_API PFNGLMAPBUFFERPROC _glMapBuffer_ptr = nullptr;
OGL_API PFNGLUNMAPBUFFERPROC _glUnmapBuffer_ptr = nullptr;
OGL_API PFNGLGETBUFFERPARAMETERIVPROC _glGetBufferParameteriv_ptr = nullptr;
OGL_API PFNGLGETBUFFERPOINTERVPROC _glGetBufferPointerv_ptr = nullptr;

// OpenGL 2.0
OGL_API PFNGLBLENDEQUATIONSEPARATEPROC _glBlendEquationSeparate_ptr = nullptr;
OGL_API PFNGLDRAWBUFFERSPROC _glDrawBuffers_ptr = nullptr;
OGL_API PFNGLSTENCILOPSEPARATEPROC _glStencilOpSeparate_ptr = nullptr;
OGL_API PFNGLSTENCILFUNCSEPARATEPROC _glStencilFuncSeparate_ptr = nullptr;
OGL_API PFNGLSTENCILMASKSEPARATEPROC _glStencilMaskSeparate_ptr = nullptr;
OGL_API PFNGLATTACHSHADERPROC _glAttachShader_ptr = nullptr;
OGL_API PFNGLBINDATTRIBLOCATIONPROC _glBindAttribLocation_ptr = nullptr;
OGL_API PFNGLCOMPILESHADERPROC _glCompileShader_ptr = nullptr;
OGL_API PFNGLCREATEPROGRAMPROC _glCreateProgram_ptr = nullptr;
OGL_API PFNGLCREATESHADERPROC _glCreateShader_ptr = nullptr;
OGL_API PFNGLDELETEPROGRAMPROC _glDeleteProgram_ptr = nullptr;
OGL_API PFNGLDELETESHADERPROC _glDeleteShader_ptr = nullptr;
OGL_API PFNGLDETACHSHADERPROC _glDetachShader_ptr = nullptr;
OGL_API PFNGLDISABLEVERTEXATTRIBARRAYPROC _glDisableVertexAttribArray_ptr = nullptr;
OGL_API PFNGLENABLEVERTEXATTRIBARRAYPROC _glEnableVertexAttribArray_ptr = nullptr;
OGL_API PFNGLGETACTIVEATTRIBPROC _glGetActiveAttrib_ptr = nullptr;
OGL_API PFNGLGETACTIVEUNIFORMPROC _glGetActiveUniform_ptr = nullptr;
OGL_API PFNGLGETATTACHEDSHADERSPROC _glGetAttachedShaders_ptr = nullptr;
OGL_API PFNGLGETATTRIBLOCATIONPROC _glGetAttribLocation_ptr = nullptr;
OGL_API PFNGLGETPROGRAMIVPROC _glGetProgramiv_ptr = nullptr;
OGL_API PFNGLGETPROGRAMINFOLOGPROC _glGetProgramInfoLog_ptr = nullptr;
OGL_API PFNGLGETSHADERIVPROC _glGetShaderiv_ptr = nullptr;
OGL_API PFNGLGETSHADERINFOLOGPROC _glGetShaderInfoLog_ptr = nullptr;
OGL_API PFNGLGETSHADERSOURCEPROC _glGetShaderSource_ptr = nullptr;
OGL_API PFNGLGETUNIFORMLOCATIONPROC _glGetUniformLocation_ptr = nullptr;
OGL_API PFNGLGETUNIFORMFVPROC _glGetUniformfv_ptr = nullptr;
OGL_API PFNGLGETUNIFORMIVPROC _glGetUniformiv_ptr = nullptr;
OGL_API PFNGLGETVERTEXATTRIBDVPROC _glGetVertexAttribdv_ptr = nullptr;
OGL_API PFNGLGETVERTEXATTRIBFVPROC _glGetVertexAttribfv_ptr = nullptr;
OGL_API PFNGLGETVERTEXATTRIBIVPROC _glGetVertexAttribiv_ptr = nullptr;
OGL_API PFNGLGETVERTEXATTRIBPOINTERVPROC _glGetVertexAttribPointerv_ptr = nullptr;
OGL_API PFNGLISPROGRAMPROC _glIsProgram_ptr = nullptr;
OGL_API PFNGLISSHADERPROC _glIsShader_ptr = nullptr;
OGL_API PFNGLLINKPROGRAMPROC _glLinkProgram_ptr = nullptr;
OGL_API PFNGLSHADERSOURCEPROC _glShaderSource_ptr = nullptr;
OGL_API PFNGLUSEPROGRAMPROC _glUseProgram_ptr = nullptr;
OGL_API PFNGLUNIFORM1FPROC _glUniform1f_ptr = nullptr;
OGL_API PFNGLUNIFORM2FPROC _glUniform2f_ptr = nullptr;
OGL_API PFNGLUNIFORM3FPROC _glUniform3f_ptr = nullptr;
OGL_API PFNGLUNIFORM4FPROC _glUniform4f_ptr = nullptr;
OGL_API PFNGLUNIFORM1IPROC _glUniform1i_ptr = nullptr;
OGL_API PFNGLUNIFORM2IPROC _glUniform2i_ptr = nullptr;
OGL_API PFNGLUNIFORM3IPROC _glUniform3i_ptr = nullptr;
OGL_API PFNGLUNIFORM4IPROC _glUniform4i_ptr = nullptr;
OGL_API PFNGLUNIFORM1FVPROC _glUniform1fv_ptr = nullptr;
OGL_API PFNGLUNIFORM2FVPROC _glUniform2fv_ptr = nullptr;
OGL_API PFNGLUNIFORM3FVPROC _glUniform3fv_ptr = nullptr;
OGL_API PFNGLUNIFORM4FVPROC _glUniform4fv_ptr = nullptr;
OGL_API PFNGLUNIFORM1IVPROC _glUniform1iv_ptr = nullptr;
OGL_API PFNGLUNIFORM2IVPROC _glUniform2iv_ptr = nullptr;
OGL_API PFNGLUNIFORM3IVPROC _glUniform3iv_ptr = nullptr;
OGL_API PFNGLUNIFORM4IVPROC _glUniform4iv_ptr = nullptr;
OGL_API PFNGLUNIFORMMATRIX2FVPROC _glUniformMatrix2fv_ptr = nullptr;
OGL_API PFNGLUNIFORMMATRIX3FVPROC _glUniformMatrix3fv_ptr = nullptr;
OGL_API PFNGLUNIFORMMATRIX4FVPROC _glUniformMatrix4fv_ptr = nullptr;
OGL_API PFNGLVALIDATEPROGRAMPROC _glValidateProgram_ptr = nullptr;
OGL_API PFNGLVERTEXATTRIB1DPROC _glVertexAttrib1d_ptr = nullptr;
OGL_API PFNGLVERTEXATTRIB1DVPROC _glVertexAttrib1dv_ptr = nullptr;
OGL_API PFNGLVERTEXATTRIB1FPROC _glVertexAttrib1f_ptr = nullptr;
OGL_API PFNGLVERTEXATTRIB1FVPROC _glVertexAttrib1fv_ptr = nullptr;
OGL_API PFNGLVERTEXATTRIB1SPROC _glVertexAttrib1s_ptr = nullptr;
OGL_API PFNGLVERTEXATTRIB1SVPROC _glVertexAttrib1sv_ptr = nullptr;
OGL_API PFNGLVERTEXATTRIB2DPROC _glVertexAttrib2d_ptr = nullptr;
OGL_API PFNGLVERTEXATTRIB2DVPROC _glVertexAttrib2dv_ptr = nullptr;
OGL_API PFNGLVERTEXATTRIB2FPROC _glVertexAttrib2f_ptr = nullptr;
OGL_API PFNGLVERTEXATTRIB2FVPROC _glVertexAttrib2fv_ptr = nullptr;
OGL_API PFNGLVERTEXATTRIB2SPROC _glVertexAttrib2s_ptr = nullptr;
OGL_API PFNGLVERTEXATTRIB2SVPROC _glVertexAttrib2sv_ptr = nullptr;
OGL_API PFNGLVERTEXATTRIB3DPROC _glVertexAttrib3d_ptr = nullptr;
OGL_API PFNGLVERTEXATTRIB3DVPROC _glVertexAttrib3dv_ptr = nullptr;
OGL_API PFNGLVERTEXATTRIB3FPROC _glVertexAttrib3f_ptr = nullptr;
OGL_API PFNGLVERTEXATTRIB3FVPROC _glVertexAttrib3fv_ptr = nullptr;
OGL_API PFNGLVERTEXATTRIB3SPROC _glVertexAttrib3s_ptr = nullptr;
OGL_API PFNGLVERTEXATTRIB3SVPROC _glVertexAttrib3sv_ptr = nullptr;
OGL_API PFNGLVERTEXATTRIB4NBVPROC _glVertexAttrib4Nbv_ptr = nullptr;
OGL_API PFNGLVERTEXATTRIB4NIVPROC _glVertexAttrib4Niv_ptr = nullptr;
OGL_API PFNGLVERTEXATTRIB4NSVPROC _glVertexAttrib4Nsv_ptr = nullptr;
OGL_API PFNGLVERTEXATTRIB4NUBPROC _glVertexAttrib4Nub_ptr = nullptr;
OGL_API PFNGLVERTEXATTRIB4NUBVPROC _glVertexAttrib4Nubv_ptr = nullptr;
OGL_API PFNGLVERTEXATTRIB4NUIVPROC _glVertexAttrib4Nuiv_ptr = nullptr;
OGL_API PFNGLVERTEXATTRIB4NUSVPROC _glVertexAttrib4Nusv_ptr = nullptr;
OGL_API PFNGLVERTEXATTRIB4BVPROC _glVertexAttrib4bv_ptr = nullptr;
OGL_API PFNGLVERTEXATTRIB4DPROC _glVertexAttrib4d_ptr = nullptr;
OGL_API PFNGLVERTEXATTRIB4DVPROC _glVertexAttrib4dv_ptr = nullptr;
OGL_API PFNGLVERTEXATTRIB4FPROC _glVertexAttrib4f_ptr = nullptr;
OGL_API PFNGLVERTEXATTRIB4FVPROC _glVertexAttrib4fv_ptr = nullptr;
OGL_API PFNGLVERTEXATTRIB4IVPROC _glVertexAttrib4iv_ptr = nullptr;
OGL_API PFNGLVERTEXATTRIB4SPROC _glVertexAttrib4s_ptr = nullptr;
OGL_API PFNGLVERTEXATTRIB4SVPROC _glVertexAttrib4sv_ptr = nullptr;
OGL_API PFNGLVERTEXATTRIB4UBVPROC _glVertexAttrib4ubv_ptr = nullptr;
OGL_API PFNGLVERTEXATTRIB4UIVPROC _glVertexAttrib4uiv_ptr = nullptr;
OGL_API PFNGLVERTEXATTRIB4USVPROC _glVertexAttrib4usv_ptr = nullptr;
OGL_API PFNGLVERTEXATTRIBPOINTERPROC _glVertexAttribPointer_ptr = nullptr;

// OpenGL 2.1
OGL_API PFNGLUNIFORMMATRIX2X3FVPROC _glUniformMatrix2x3fv_ptr = nullptr;
OGL_API PFNGLUNIFORMMATRIX3X2FVPROC _glUniformMatrix3x2fv_ptr = nullptr;
OGL_API PFNGLUNIFORMMATRIX2X4FVPROC _glUniformMatrix2x4fv_ptr = nullptr;
OGL_API PFNGLUNIFORMMATRIX4X2FVPROC _glUniformMatrix4x2fv_ptr = nullptr;
OGL_API PFNGLUNIFORMMATRIX3X4FVPROC _glUniformMatrix3x4fv_ptr = nullptr;
OGL_API PFNGLUNIFORMMATRIX4X3FVPROC _glUniformMatrix4x3fv_ptr = nullptr;

// OpenGL 3.0
OGL_API PFNGLCOLORMASKIPROC _glColorMaski_ptr = nullptr;
OGL_API PFNGLGETBOOLEANI_VPROC _glGetBooleani_v_ptr = nullptr;
OGL_API PFNGLGETINTEGERI_VPROC _glGetIntegeri_v_ptr = nullptr;
OGL_API PFNGLENABLEIPROC _glEnablei_ptr = nullptr;
OGL_API PFNGLDISABLEIPROC _glDisablei_ptr = nullptr;
OGL_API PFNGLISENABLEDIPROC _glIsEnabledi_ptr = nullptr;
OGL_API PFNGLBEGINTRANSFORMFEEDBACKPROC _glBeginTransformFeedback_ptr = nullptr;
OGL_API PFNGLENDTRANSFORMFEEDBACKPROC _glEndTransformFeedback_ptr = nullptr;
OGL_API PFNGLBINDBUFFERRANGEPROC _glBindBufferRange_ptr = nullptr;
OGL_API PFNGLBINDBUFFERBASEPROC _glBindBufferBase_ptr = nullptr;
OGL_API PFNGLTRANSFORMFEEDBACKVARYINGSPROC _glTransformFeedbackVaryings_ptr = nullptr;
OGL_API PFNGLGETTRANSFORMFEEDBACKVARYINGPROC _glGetTransformFeedbackVarying_ptr = nullptr;
OGL_API PFNGLCLAMPCOLORPROC _glClampColor_ptr = nullptr;
OGL_API PFNGLBEGINCONDITIONALRENDERPROC _glBeginConditionalRender_ptr = nullptr;
OGL_API PFNGLENDCONDITIONALRENDERPROC _glEndConditionalRender_ptr = nullptr;
OGL_API PFNGLVERTEXATTRIBIPOINTERPROC _glVertexAttribIPointer_ptr = nullptr;
OGL_API PFNGLGETVERTEXATTRIBIIVPROC _glGetVertexAttribIiv_ptr = nullptr;
OGL_API PFNGLGETVERTEXATTRIBIUIVPROC _glGetVertexAttribIuiv_ptr = nullptr;
OGL_API PFNGLVERTEXATTRIBI1IPROC _glVertexAttribI1i_ptr = nullptr;
OGL_API PFNGLVERTEXATTRIBI2IPROC _glVertexAttribI2i_ptr = nullptr;
OGL_API PFNGLVERTEXATTRIBI3IPROC _glVertexAttribI3i_ptr = nullptr;
OGL_API PFNGLVERTEXATTRIBI4IPROC _glVertexAttribI4i_ptr = nullptr;
OGL_API PFNGLVERTEXATTRIBI1UIPROC _glVertexAttribI1ui_ptr = nullptr;
OGL_API PFNGLVERTEXATTRIBI2UIPROC _glVertexAttribI2ui_ptr = nullptr;
OGL_API PFNGLVERTEXATTRIBI3UIPROC _glVertexAttribI3ui_ptr = nullptr;
OGL_API PFNGLVERTEXATTRIBI4UIPROC _glVertexAttribI4ui_ptr = nullptr;
OGL_API PFNGLVERTEXATTRIBI1IVPROC _glVertexAttribI1iv_ptr = nullptr;
OGL_API PFNGLVERTEXATTRIBI2IVPROC _glVertexAttribI2iv_ptr = nullptr;
OGL_API PFNGLVERTEXATTRIBI3IVPROC _glVertexAttribI3iv_ptr = nullptr;
OGL_API PFNGLVERTEXATTRIBI4IVPROC _glVertexAttribI4iv_ptr = nullptr;
OGL_API PFNGLVERTEXATTRIBI1UIVPROC _glVertexAttribI1uiv_ptr = nullptr;
OGL_API PFNGLVERTEXATTRIBI2UIVPROC _glVertexAttribI2uiv_ptr = nullptr;
OGL_API PFNGLVERTEXATTRIBI3UIVPROC _glVertexAttribI3uiv_ptr = nullptr;
OGL_API PFNGLVERTEXATTRIBI4UIVPROC _glVertexAttribI4uiv_ptr = nullptr;
OGL_API PFNGLVERTEXATTRIBI4BVPROC _glVertexAttribI4bv_ptr = nullptr;
OGL_API PFNGLVERTEXATTRIBI4SVPROC _glVertexAttribI4sv_ptr = nullptr;
OGL_API PFNGLVERTEXATTRIBI4UBVPROC _glVertexAttribI4ubv_ptr = nullptr;
OGL_API PFNGLVERTEXATTRIBI4USVPROC _glVertexAttribI4usv_ptr = nullptr;
OGL_API PFNGLGETUNIFORMUIVPROC _glGetUniformuiv_ptr = nullptr;
OGL_API PFNGLBINDFRAGDATALOCATIONPROC _glBindFragDataLocation_ptr = nullptr;
OGL_API PFNGLGETFRAGDATALOCATIONPROC _glGetFragDataLocation_ptr = nullptr;
OGL_API PFNGLUNIFORM1UIPROC _glUniform1ui_ptr = nullptr;
OGL_API PFNGLUNIFORM2UIPROC _glUniform2ui_ptr = nullptr;
OGL_API PFNGLUNIFORM3UIPROC _glUniform3ui_ptr = nullptr;
OGL_API PFNGLUNIFORM4UIPROC _glUniform4ui_ptr = nullptr;
OGL_API PFNGLUNIFORM1UIVPROC _glUniform1uiv_ptr = nullptr;
OGL_API PFNGLUNIFORM2UIVPROC _glUniform2uiv_ptr = nullptr;
OGL_API PFNGLUNIFORM3UIVPROC _glUniform3uiv_ptr = nullptr;
OGL_API PFNGLUNIFORM4UIVPROC _glUniform4uiv_ptr = nullptr;
OGL_API PFNGLTEXPARAMETERIIVPROC _glTexParameterIiv_ptr = nullptr;
OGL_API PFNGLTEXPARAMETERIUIVPROC _glTexParameterIuiv_ptr = nullptr;
OGL_API PFNGLGETTEXPARAMETERIIVPROC _glGetTexParameterIiv_ptr = nullptr;
OGL_API PFNGLGETTEXPARAMETERIUIVPROC _glGetTexParameterIuiv_ptr = nullptr;
OGL_API PFNGLCLEARBUFFERIVPROC _glClearBufferiv_ptr = nullptr;
OGL_API PFNGLCLEARBUFFERUIVPROC _glClearBufferuiv_ptr = nullptr;
OGL_API PFNGLCLEARBUFFERFVPROC _glClearBufferfv_ptr = nullptr;
OGL_API PFNGLCLEARBUFFERFIPROC _glClearBufferfi_ptr = nullptr;
OGL_API PFNGLGETSTRINGIPROC _glGetStringi_ptr = nullptr;
OGL_API PFNGLISRENDERBUFFERPROC _glIsRenderbuffer_ptr = nullptr; // ARB_framebuffer_object
OGL_API PFNGLBINDRENDERBUFFERPROC _glBindRenderbuffer_ptr = nullptr; // ARB_framebuffer_object
OGL_API PFNGLDELETERENDERBUFFERSPROC _glDeleteRenderbuffers_ptr = nullptr; // ARB_framebuffer_object
OGL_API PFNGLGENRENDERBUFFERSPROC _glGenRenderbuffers_ptr = nullptr; // ARB_framebuffer_object
OGL_API PFNGLRENDERBUFFERSTORAGEPROC _glRenderbufferStorage_ptr = nullptr; // ARB_framebuffer_object
OGL_API PFNGLGETRENDERBUFFERPARAMETERIVPROC _glGetRenderbufferParameteriv_ptr = nullptr; // ARB_framebuffer_object
OGL_API PFNGLISFRAMEBUFFERPROC _glIsFramebuffer_ptr = nullptr; // ARB_framebuffer_object
OGL_API PFNGLBINDFRAMEBUFFERPROC _glBindFramebuffer_ptr = nullptr; // ARB_framebuffer_object
OGL_API PFNGLDELETEFRAMEBUFFERSPROC _glDeleteFramebuffers_ptr = nullptr; // ARB_framebuffer_object
OGL_API PFNGLGENFRAMEBUFFERSPROC _glGenFramebuffers_ptr = nullptr; // ARB_framebuffer_object
OGL_API PFNGLCHECKFRAMEBUFFERSTATUSPROC _glCheckFramebufferStatus_ptr = nullptr; // ARB_framebuffer_object
OGL_API PFNGLFRAMEBUFFERTEXTURE1DPROC _glFramebufferTexture1D_ptr = nullptr; // ARB_framebuffer_object
OGL_API PFNGLFRAMEBUFFERTEXTURE2DPROC _glFramebufferTexture2D_ptr = nullptr; // ARB_framebuffer_object
OGL_API PFNGLFRAMEBUFFERTEXTURE3DPROC _glFramebufferTexture3D_ptr = nullptr; // ARB_framebuffer_object
OGL_API PFNGLFRAMEBUFFERRENDERBUFFERPROC _glFramebufferRenderbuffer_ptr = nullptr; // ARB_framebuffer_object
OGL_API PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVPROC _glGetFramebufferAttachmentParameteriv_ptr = nullptr; // ARB_framebuffer_object
OGL_API PFNGLGENERATEMIPMAPPROC _glGenerateMipmap_ptr = nullptr; // ARB_framebuffer_object
OGL_API PFNGLBLITFRAMEBUFFERPROC _glBlitFramebuffer_ptr = nullptr; // ARB_framebuffer_object
OGL_API PFNGLRENDERBUFFERSTORAGEMULTISAMPLEPROC _glRenderbufferStorageMultisample_ptr = nullptr; // ARB_framebuffer_object
OGL_API PFNGLFRAMEBUFFERTEXTURELAYERPROC _glFramebufferTextureLayer_ptr = nullptr; // ARB_framebuffer_object
OGL_API PFNGLMAPBUFFERRANGEPROC _glMapBufferRange_ptr = nullptr; // ARB_map_buffer_range
OGL_API PFNGLFLUSHMAPPEDBUFFERRANGEPROC _glFlushMappedBufferRange_ptr = nullptr; // ARB_map_buffer_range
OGL_API PFNGLBINDVERTEXARRAYPROC _glBindVertexArray_ptr = nullptr; // ARB_vertex_array_object
OGL_API PFNGLDELETEVERTEXARRAYSPROC _glDeleteVertexArrays_ptr = nullptr; // ARB_vertex_array_object
OGL_API PFNGLGENVERTEXARRAYSPROC _glGenVertexArrays_ptr = nullptr; // ARB_vertex_array_object
OGL_API PFNGLISVERTEXARRAYPROC _glIsVertexArray_ptr = nullptr; // ARB_vertex_array_object

// OpenGL 3.1
OGL_API PFNGLDRAWARRAYSINSTANCEDPROC _glDrawArraysInstanced_ptr = nullptr;
OGL_API PFNGLDRAWELEMENTSINSTANCEDPROC _glDrawElementsInstanced_ptr = nullptr;
OGL_API PFNGLTEXBUFFERPROC _glTexBuffer_ptr = nullptr;
OGL_API PFNGLPRIMITIVERESTARTINDEXPROC _glPrimitiveRestartIndex_ptr = nullptr;
OGL_API PFNGLCOPYBUFFERSUBDATAPROC _glCopyBufferSubData_ptr = nullptr; // ARB_copy_buffer
OGL_API PFNGLGETUNIFORMINDICESPROC _glGetUniformIndices_ptr = nullptr; // ARB_uniform_buffer_object
OGL_API PFNGLGETACTIVEUNIFORMSIVPROC _glGetActiveUniformsiv_ptr = nullptr; // ARB_uniform_buffer_object
OGL_API PFNGLGETACTIVEUNIFORMNAMEPROC _glGetActiveUniformName_ptr = nullptr; // ARB_uniform_buffer_object
OGL_API PFNGLGETUNIFORMBLOCKINDEXPROC _glGetUniformBlockIndex_ptr = nullptr; // ARB_uniform_buffer_object
OGL_API PFNGLGETACTIVEUNIFORMBLOCKIVPROC _glGetActiveUniformBlockiv_ptr = nullptr; // ARB_uniform_buffer_object
OGL_API PFNGLGETACTIVEUNIFORMBLOCKNAMEPROC _glGetActiveUniformBlockName_ptr = nullptr; // ARB_uniform_buffer_object
OGL_API PFNGLUNIFORMBLOCKBINDINGPROC _glUniformBlockBinding_ptr = nullptr; // ARB_uniform_buffer_object

// OpenGL 3.2
OGL_API PFNGLGETINTEGER64I_VPROC _glGetInteger64i_v_ptr = nullptr;
OGL_API PFNGLGETBUFFERPARAMETERI64VPROC _glGetBufferParameteri64v_ptr = nullptr;
OGL_API PFNGLFRAMEBUFFERTEXTUREPROC _glFramebufferTexture_ptr = nullptr;
OGL_API PFNGLDRAWELEMENTSBASEVERTEXPROC _glDrawElementsBaseVertex_ptr = nullptr; // ARB_draw_elements_base_vertex
OGL_API PFNGLDRAWRANGEELEMENTSBASEVERTEXPROC _glDrawRangeElementsBaseVertex_ptr = nullptr; // ARB_draw_elements_base_vertex
OGL_API PFNGLDRAWELEMENTSINSTANCEDBASEVERTEXPROC _glDrawElementsInstancedBaseVertex_ptr = nullptr; // ARB_draw_elements_base_vertex
OGL_API PFNGLMULTIDRAWELEMENTSBASEVERTEXPROC _glMultiDrawElementsBaseVertex_ptr = nullptr; // ARB_draw_elements_base_vertex
OGL_API PFNGLPROVOKINGVERTEXPROC _glProvokingVertex_ptr = nullptr; // ARB_provoking_vertex
OGL_API PFNGLFENCESYNCPROC _glFenceSync_ptr = nullptr; // ARB_sync
OGL_API PFNGLISSYNCPROC _glIsSync_ptr = nullptr; // ARB_sync
OGL_API PFNGLDELETESYNCPROC _glDeleteSync_ptr = nullptr; // ARB_sync
OGL_API PFNGLCLIENTWAITSYNCPROC _glClientWaitSync_ptr = nullptr; // ARB_sync
OGL_API PFNGLWAITSYNCPROC _glWaitSync_ptr = nullptr; // ARB_sync
OGL_API PFNGLGETINTEGER64VPROC _glGetInteger64v_ptr = nullptr; // ARB_sync
OGL_API PFNGLGETSYNCIVPROC _glGetSynciv_ptr = nullptr; // ARB_sync
OGL_API PFNGLTEXIMAGE2DMULTISAMPLEPROC _glTexImage2DMultisample_ptr = nullptr; // ARB_texture_multisample
OGL_API PFNGLTEXIMAGE3DMULTISAMPLEPROC _glTexImage3DMultisample_ptr = nullptr; // ARB_texture_multisample
OGL_API PFNGLGETMULTISAMPLEFVPROC _glGetMultisamplefv_ptr = nullptr; // ARB_texture_multisample
OGL_API PFNGLSAMPLEMASKIPROC _glSampleMaski_ptr = nullptr; // ARB_texture_multisample

// OpenGL 3.3
OGL_API PFNGLVERTEXATTRIBDIVISORPROC _glVertexAttribDivisor_ptr = nullptr;
OGL_API PFNGLBINDFRAGDATALOCATIONINDEXEDPROC _glBindFragDataLocationIndexed_ptr = nullptr; // ARB_blend_func_extended
OGL_API PFNGLGETFRAGDATAINDEXPROC _glGetFragDataIndex_ptr = nullptr; // ARB_blend_func_extended
OGL_API PFNGLGENSAMPLERSPROC _glGenSamplers_ptr = nullptr; // ARB_sampler_objects
OGL_API PFNGLDELETESAMPLERSPROC _glDeleteSamplers_ptr = nullptr; // ARB_sampler_objects
OGL_API PFNGLISSAMPLERPROC _glIsSampler_ptr = nullptr; // ARB_sampler_objects
OGL_API PFNGLBINDSAMPLERPROC _glBindSampler_ptr = nullptr; // ARB_sampler_objects
OGL_API PFNGLSAMPLERPARAMETERIPROC _glSamplerParameteri_ptr = nullptr; // ARB_sampler_objects
OGL_API PFNGLSAMPLERPARAMETERIVPROC _glSamplerParameteriv_ptr = nullptr; // ARB_sampler_objects
OGL_API PFNGLSAMPLERPARAMETERFPROC _glSamplerParameterf_ptr = nullptr; // ARB_sampler_objects
OGL_API PFNGLSAMPLERPARAMETERFVPROC _glSamplerParameterfv_ptr = nullptr; // ARB_sampler_objects
OGL_API PFNGLSAMPLERPARAMETERIIVPROC _glSamplerParameterIiv_ptr = nullptr; // ARB_sampler_objects
OGL_API PFNGLSAMPLERPARAMETERIUIVPROC _glSamplerParameterIuiv_ptr = nullptr; // ARB_sampler_objects
OGL_API PFNGLGETSAMPLERPARAMETERIVPROC _glGetSamplerParameteriv_ptr = nullptr; // ARB_sampler_objects
OGL_API PFNGLGETSAMPLERPARAMETERIIVPROC _glGetSamplerParameterIiv_ptr = nullptr; // ARB_sampler_objects
OGL_API PFNGLGETSAMPLERPARAMETERFVPROC _glGetSamplerParameterfv_ptr = nullptr; // ARB_sampler_objects
OGL_API PFNGLGETSAMPLERPARAMETERIUIVPROC _glGetSamplerParameterIuiv_ptr = nullptr; // ARB_sampler_objects
OGL_API PFNGLQUERYCOUNTERPROC _glQueryCounter_ptr = nullptr; // ARB_timer_query
OGL_API PFNGLGETQUERYOBJECTI64VPROC _glGetQueryObjecti64v_ptr = nullptr; // ARB_timer_query
OGL_API PFNGLGETQUERYOBJECTUI64VPROC _glGetQueryObjectui64v_ptr = nullptr; // ARB_timer_query
OGL_API PFNGLVERTEXP2UIPROC _glVertexP2ui_ptr = nullptr; // ARB_vertex_type_2_10_10_10_rev
OGL_API PFNGLVERTEXP2UIVPROC _glVertexP2uiv_ptr = nullptr; // ARB_vertex_type_2_10_10_10_rev
OGL_API PFNGLVERTEXP3UIPROC _glVertexP3ui_ptr = nullptr; // ARB_vertex_type_2_10_10_10_rev
OGL_API PFNGLVERTEXP3UIVPROC _glVertexP3uiv_ptr = nullptr; // ARB_vertex_type_2_10_10_10_rev
OGL_API PFNGLVERTEXP4UIPROC _glVertexP4ui_ptr = nullptr; // ARB_vertex_type_2_10_10_10_rev
OGL_API PFNGLVERTEXP4UIVPROC _glVertexP4uiv_ptr = nullptr; // ARB_vertex_type_2_10_10_10_rev
OGL_API PFNGLTEXCOORDP1UIPROC _glTexCoordP1ui_ptr = nullptr; // ARB_vertex_type_2_10_10_10_rev
OGL_API PFNGLTEXCOORDP1UIVPROC _glTexCoordP1uiv_ptr = nullptr; // ARB_vertex_type_2_10_10_10_rev
OGL_API PFNGLTEXCOORDP2UIPROC _glTexCoordP2ui_ptr = nullptr; // ARB_vertex_type_2_10_10_10_rev
OGL_API PFNGLTEXCOORDP2UIVPROC _glTexCoordP2uiv_ptr = nullptr; // ARB_vertex_type_2_10_10_10_rev
OGL_API PFNGLTEXCOORDP3UIPROC _glTexCoordP3ui_ptr = nullptr; // ARB_vertex_type_2_10_10_10_rev
OGL_API PFNGLTEXCOORDP3UIVPROC _glTexCoordP3uiv_ptr = nullptr; // ARB_vertex_type_2_10_10_10_rev
OGL_API PFNGLTEXCOORDP4UIPROC _glTexCoordP4ui_ptr = nullptr; // ARB_vertex_type_2_10_10_10_rev
OGL_API PFNGLTEXCOORDP4UIVPROC _glTexCoordP4uiv_ptr = nullptr; // ARB_vertex_type_2_10_10_10_rev
OGL_API PFNGLMULTITEXCOORDP1UIPROC _glMultiTexCoordP1ui_ptr = nullptr; // ARB_vertex_type_2_10_10_10_rev
OGL_API PFNGLMULTITEXCOORDP1UIVPROC _glMultiTexCoordP1uiv_ptr = nullptr; // ARB_vertex_type_2_10_10_10_rev
OGL_API PFNGLMULTITEXCOORDP2UIPROC _glMultiTexCoordP2ui_ptr = nullptr; // ARB_vertex_type_2_10_10_10_rev
OGL_API PFNGLMULTITEXCOORDP2UIVPROC _glMultiTexCoordP2uiv_ptr = nullptr; // ARB_vertex_type_2_10_10_10_rev
OGL_API PFNGLMULTITEXCOORDP3UIPROC _glMultiTexCoordP3ui_ptr = nullptr; // ARB_vertex_type_2_10_10_10_rev
OGL_API PFNGLMULTITEXCOORDP3UIVPROC _glMultiTexCoordP3uiv_ptr = nullptr; // ARB_vertex_type_2_10_10_10_rev
OGL_API PFNGLMULTITEXCOORDP4UIPROC _glMultiTexCoordP4ui_ptr = nullptr; // ARB_vertex_type_2_10_10_10_rev
OGL_API PFNGLMULTITEXCOORDP4UIVPROC _glMultiTexCoordP4uiv_ptr = nullptr; // ARB_vertex_type_2_10_10_10_rev
OGL_API PFNGLNORMALP3UIPROC _glNormalP3ui_ptr = nullptr; // ARB_vertex_type_2_10_10_10_rev
OGL_API PFNGLNORMALP3UIVPROC _glNormalP3uiv_ptr = nullptr; // ARB_vertex_type_2_10_10_10_rev
OGL_API PFNGLCOLORP3UIPROC _glColorP3ui_ptr = nullptr; // ARB_vertex_type_2_10_10_10_rev
OGL_API PFNGLCOLORP3UIVPROC _glColorP3uiv_ptr = nullptr; // ARB_vertex_type_2_10_10_10_rev
OGL_API PFNGLCOLORP4UIPROC _glColorP4ui_ptr = nullptr; // ARB_vertex_type_2_10_10_10_rev
OGL_API PFNGLCOLORP4UIVPROC _glColorP4uiv_ptr = nullptr; // ARB_vertex_type_2_10_10_10_rev
OGL_API PFNGLSECONDARYCOLORP3UIPROC _glSecondaryColorP3ui_ptr = nullptr; // ARB_vertex_type_2_10_10_10_rev
OGL_API PFNGLSECONDARYCOLORP3UIVPROC _glSecondaryColorP3uiv_ptr = nullptr; // ARB_vertex_type_2_10_10_10_rev
OGL_API PFNGLVERTEXATTRIBP1UIPROC _glVertexAttribP1ui_ptr = nullptr; // ARB_vertex_type_2_10_10_10_rev
OGL_API PFNGLVERTEXATTRIBP1UIVPROC _glVertexAttribP1uiv_ptr = nullptr; // ARB_vertex_type_2_10_10_10_rev
OGL_API PFNGLVERTEXATTRIBP2UIPROC _glVertexAttribP2ui_ptr = nullptr; // ARB_vertex_type_2_10_10_10_rev
OGL_API PFNGLVERTEXATTRIBP2UIVPROC _glVertexAttribP2uiv_ptr = nullptr; // ARB_vertex_type_2_10_10_10_rev
OGL_API PFNGLVERTEXATTRIBP3UIPROC _glVertexAttribP3ui_ptr = nullptr; // ARB_vertex_type_2_10_10_10_rev
OGL_API PFNGLVERTEXATTRIBP3UIVPROC _glVertexAttribP3uiv_ptr = nullptr; // ARB_vertex_type_2_10_10_10_rev
OGL_API PFNGLVERTEXATTRIBP4UIPROC _glVertexAttribP4ui_ptr = nullptr; // ARB_vertex_type_2_10_10_10_rev
OGL_API PFNGLVERTEXATTRIBP4UIVPROC _glVertexAttribP4uiv_ptr = nullptr; // ARB_vertex_type_2_10_10_10_rev

// OpenGL 4.0
OGL_API PFNGLMINSAMPLESHADINGPROC _glMinSampleShading_ptr = nullptr;
OGL_API PFNGLBLENDEQUATIONIPROC _glBlendEquationi_ptr = nullptr;
OGL_API PFNGLBLENDEQUATIONSEPARATEIPROC _glBlendEquationSeparatei_ptr = nullptr;
OGL_API PFNGLBLENDFUNCIPROC _glBlendFunci_ptr = nullptr;
OGL_API PFNGLBLENDFUNCSEPARATEIPROC _glBlendFuncSeparatei_ptr = nullptr;
OGL_API PFNGLDRAWARRAYSINDIRECTPROC _glDrawArraysIndirect_ptr = nullptr; // ARB_draw_indirect
OGL_API PFNGLDRAWELEMENTSINDIRECTPROC _glDrawElementsIndirect_ptr = nullptr; // ARB_draw_indirect
OGL_API PFNGLUNIFORM1DPROC _glUniform1d_ptr = nullptr; // ARB_gpu_shader_fp64
OGL_API PFNGLUNIFORM2DPROC _glUniform2d_ptr = nullptr; // ARB_gpu_shader_fp64
OGL_API PFNGLUNIFORM3DPROC _glUniform3d_ptr = nullptr; // ARB_gpu_shader_fp64
OGL_API PFNGLUNIFORM4DPROC _glUniform4d_ptr = nullptr; // ARB_gpu_shader_fp64
OGL_API PFNGLUNIFORM1DVPROC _glUniform1dv_ptr = nullptr; // ARB_gpu_shader_fp64
OGL_API PFNGLUNIFORM2DVPROC _glUniform2dv_ptr = nullptr; // ARB_gpu_shader_fp64
OGL_API PFNGLUNIFORM3DVPROC _glUniform3dv_ptr = nullptr; // ARB_gpu_shader_fp64
OGL_API PFNGLUNIFORM4DVPROC _glUniform4dv_ptr = nullptr; // ARB_gpu_shader_fp64
OGL_API PFNGLUNIFORMMATRIX2DVPROC _glUniformMatrix2dv_ptr = nullptr; // ARB_gpu_shader_fp64
OGL_API PFNGLUNIFORMMATRIX3DVPROC _glUniformMatrix3dv_ptr = nullptr; // ARB_gpu_shader_fp64
OGL_API PFNGLUNIFORMMATRIX4DVPROC _glUniformMatrix4dv_ptr = nullptr; // ARB_gpu_shader_fp64
OGL_API PFNGLUNIFORMMATRIX2X3DVPROC _glUniformMatrix2x3dv_ptr = nullptr; // ARB_gpu_shader_fp64
OGL_API PFNGLUNIFORMMATRIX2X4DVPROC _glUniformMatrix2x4dv_ptr = nullptr; // ARB_gpu_shader_fp64
OGL_API PFNGLUNIFORMMATRIX3X2DVPROC _glUniformMatrix3x2dv_ptr = nullptr; // ARB_gpu_shader_fp64
OGL_API PFNGLUNIFORMMATRIX3X4DVPROC _glUniformMatrix3x4dv_ptr = nullptr; // ARB_gpu_shader_fp64
OGL_API PFNGLUNIFORMMATRIX4X2DVPROC _glUniformMatrix4x2dv_ptr = nullptr; // ARB_gpu_shader_fp64
OGL_API PFNGLUNIFORMMATRIX4X3DVPROC _glUniformMatrix4x3dv_ptr = nullptr; // ARB_gpu_shader_fp64
OGL_API PFNGLGETUNIFORMDVPROC _glGetUniformdv_ptr = nullptr; // ARB_gpu_shader_fp64
OGL_API PFNGLGETSUBROUTINEUNIFORMLOCATIONPROC _glGetSubroutineUniformLocation_ptr = nullptr; // ARB_shader_subroutine
OGL_API PFNGLGETSUBROUTINEINDEXPROC _glGetSubroutineIndex_ptr = nullptr; // ARB_shader_subroutine
OGL_API PFNGLGETACTIVESUBROUTINEUNIFORMIVPROC _glGetActiveSubroutineUniformiv_ptr = nullptr; // ARB_shader_subroutine
OGL_API PFNGLGETACTIVESUBROUTINEUNIFORMNAMEPROC _glGetActiveSubroutineUniformName_ptr = nullptr; // ARB_shader_subroutine
OGL_API PFNGLGETACTIVESUBROUTINENAMEPROC _glGetActiveSubroutineName_ptr = nullptr; // ARB_shader_subroutine
OGL_API PFNGLUNIFORMSUBROUTINESUIVPROC _glUniformSubroutinesuiv_ptr = nullptr; // ARB_shader_subroutine
OGL_API PFNGLGETUNIFORMSUBROUTINEUIVPROC _glGetUniformSubroutineuiv_ptr = nullptr; // ARB_shader_subroutine
OGL_API PFNGLGETPROGRAMSTAGEIVPROC _glGetProgramStageiv_ptr = nullptr; // ARB_shader_subroutine
OGL_API PFNGLPATCHPARAMETERIPROC _glPatchParameteri_ptr = nullptr; // ARB_tessellation_shader
OGL_API PFNGLPATCHPARAMETERFVPROC _glPatchParameterfv_ptr = nullptr; // ARB_tessellation_shader
OGL_API PFNGLBINDTRANSFORMFEEDBACKPROC _glBindTransformFeedback_ptr = nullptr; // ARB_transform_feedback2
OGL_API PFNGLDELETETRANSFORMFEEDBACKSPROC _glDeleteTransformFeedbacks_ptr = nullptr; // ARB_transform_feedback2
OGL_API PFNGLGENTRANSFORMFEEDBACKSPROC _glGenTransformFeedbacks_ptr = nullptr; // ARB_transform_feedback2
OGL_API PFNGLISTRANSFORMFEEDBACKPROC _glIsTransformFeedback_ptr = nullptr; // ARB_transform_feedback2
OGL_API PFNGLPAUSETRANSFORMFEEDBACKPROC _glPauseTransformFeedback_ptr = nullptr; // ARB_transform_feedback2
OGL_API PFNGLRESUMETRANSFORMFEEDBACKPROC _glResumeTransformFeedback_ptr = nullptr; // ARB_transform_feedback2
OGL_API PFNGLDRAWTRANSFORMFEEDBACKPROC _glDrawTransformFeedback_ptr = nullptr; // ARB_transform_feedback2
OGL_API PFNGLDRAWTRANSFORMFEEDBACKSTREAMPROC _glDrawTransformFeedbackStream_ptr = nullptr; // ARB_transform_feedback3
OGL_API PFNGLBEGINQUERYINDEXEDPROC _glBeginQueryIndexed_ptr = nullptr; // ARB_transform_feedback3
OGL_API PFNGLENDQUERYINDEXEDPROC _glEndQueryIndexed_ptr = nullptr; // ARB_transform_feedback3
OGL_API PFNGLGETQUERYINDEXEDIVPROC _glGetQueryIndexediv_ptr = nullptr; // ARB_transform_feedback3

// OpenGL 4.1
OGL_API PFNGLRELEASESHADERCOMPILERPROC _glReleaseShaderCompiler_ptr = nullptr; // ARB_ES2_compatibility
OGL_API PFNGLSHADERBINARYPROC _glShaderBinary_ptr = nullptr; // ARB_ES2_compatibility
OGL_API PFNGLGETSHADERPRECISIONFORMATPROC _glGetShaderPrecisionFormat_ptr = nullptr; // ARB_ES2_compatibility
OGL_API PFNGLDEPTHRANGEFPROC _glDepthRangef_ptr = nullptr; // ARB_ES2_compatibility
OGL_API PFNGLCLEARDEPTHFPROC _glClearDepthf_ptr = nullptr; // ARB_ES2_compatibility
OGL_API PFNGLGETPROGRAMBINARYPROC _glGetProgramBinary_ptr = nullptr; // ARB_get_program_binary
OGL_API PFNGLPROGRAMBINARYPROC _glProgramBinary_ptr = nullptr; // ARB_get_program_binary
OGL_API PFNGLPROGRAMPARAMETERIPROC _glProgramParameteri_ptr = nullptr; // ARB_get_program_binary
OGL_API PFNGLUSEPROGRAMSTAGESPROC _glUseProgramStages_ptr = nullptr; // ARB_separate_shader_objects
OGL_API PFNGLACTIVESHADERPROGRAMPROC _glActiveShaderProgram_ptr = nullptr; // ARB_separate_shader_objects
OGL_API PFNGLCREATESHADERPROGRAMVPROC _glCreateShaderProgramv_ptr = nullptr; // ARB_separate_shader_objects
OGL_API PFNGLBINDPROGRAMPIPELINEPROC _glBindProgramPipeline_ptr = nullptr; // ARB_separate_shader_objects
OGL_API PFNGLDELETEPROGRAMPIPELINESPROC _glDeleteProgramPipelines_ptr = nullptr; // ARB_separate_shader_objects
OGL_API PFNGLGENPROGRAMPIPELINESPROC _glGenProgramPipelines_ptr = nullptr; // ARB_separate_shader_objects
OGL_API PFNGLISPROGRAMPIPELINEPROC _glIsProgramPipeline_ptr = nullptr; // ARB_separate_shader_objects
OGL_API PFNGLGETPROGRAMPIPELINEIVPROC _glGetProgramPipelineiv_ptr = nullptr; // ARB_separate_shader_objects
OGL_API PFNGLPROGRAMUNIFORM1IPROC _glProgramUniform1i_ptr = nullptr; // ARB_separate_shader_objects
OGL_API PFNGLPROGRAMUNIFORM1IVPROC _glProgramUniform1iv_ptr = nullptr; // ARB_separate_shader_objects
OGL_API PFNGLPROGRAMUNIFORM1FPROC _glProgramUniform1f_ptr = nullptr; // ARB_separate_shader_objects
OGL_API PFNGLPROGRAMUNIFORM1FVPROC _glProgramUniform1fv_ptr = nullptr; // ARB_separate_shader_objects
OGL_API PFNGLPROGRAMUNIFORM1DPROC _glProgramUniform1d_ptr = nullptr; // ARB_separate_shader_objects
OGL_API PFNGLPROGRAMUNIFORM1DVPROC _glProgramUniform1dv_ptr = nullptr; // ARB_separate_shader_objects
OGL_API PFNGLPROGRAMUNIFORM1UIPROC _glProgramUniform1ui_ptr = nullptr; // ARB_separate_shader_objects
OGL_API PFNGLPROGRAMUNIFORM1UIVPROC _glProgramUniform1uiv_ptr = nullptr; // ARB_separate_shader_objects
OGL_API PFNGLPROGRAMUNIFORM2IPROC _glProgramUniform2i_ptr = nullptr; // ARB_separate_shader_objects
OGL_API PFNGLPROGRAMUNIFORM2IVPROC _glProgramUniform2iv_ptr = nullptr; // ARB_separate_shader_objects
OGL_API PFNGLPROGRAMUNIFORM2FPROC _glProgramUniform2f_ptr = nullptr; // ARB_separate_shader_objects
OGL_API PFNGLPROGRAMUNIFORM2FVPROC _glProgramUniform2fv_ptr = nullptr; // ARB_separate_shader_objects
OGL_API PFNGLPROGRAMUNIFORM2DPROC _glProgramUniform2d_ptr = nullptr; // ARB_separate_shader_objects
OGL_API PFNGLPROGRAMUNIFORM2DVPROC _glProgramUniform2dv_ptr = nullptr; // ARB_separate_shader_objects
OGL_API PFNGLPROGRAMUNIFORM2UIPROC _glProgramUniform2ui_ptr = nullptr; // ARB_separate_shader_objects
OGL_API PFNGLPROGRAMUNIFORM2UIVPROC _glProgramUniform2uiv_ptr = nullptr; // ARB_separate_shader_objects
OGL_API PFNGLPROGRAMUNIFORM3IPROC _glProgramUniform3i_ptr = nullptr; // ARB_separate_shader_objects
OGL_API PFNGLPROGRAMUNIFORM3IVPROC _glProgramUniform3iv_ptr = nullptr; // ARB_separate_shader_objects
OGL_API PFNGLPROGRAMUNIFORM3FPROC _glProgramUniform3f_ptr = nullptr; // ARB_separate_shader_objects
OGL_API PFNGLPROGRAMUNIFORM3FVPROC _glProgramUniform3fv_ptr = nullptr; // ARB_separate_shader_objects
OGL_API PFNGLPROGRAMUNIFORM3DPROC _glProgramUniform3d_ptr = nullptr; // ARB_separate_shader_objects
OGL_API PFNGLPROGRAMUNIFORM3DVPROC _glProgramUniform3dv_ptr = nullptr; // ARB_separate_shader_objects
OGL_API PFNGLPROGRAMUNIFORM3UIPROC _glProgramUniform3ui_ptr = nullptr; // ARB_separate_shader_objects
OGL_API PFNGLPROGRAMUNIFORM3UIVPROC _glProgramUniform3uiv_ptr = nullptr; // ARB_separate_shader_objects
OGL_API PFNGLPROGRAMUNIFORM4IPROC _glProgramUniform4i_ptr = nullptr; // ARB_separate_shader_objects
OGL_API PFNGLPROGRAMUNIFORM4IVPROC _glProgramUniform4iv_ptr = nullptr; // ARB_separate_shader_objects
OGL_API PFNGLPROGRAMUNIFORM4FPROC _glProgramUniform4f_ptr = nullptr; // ARB_separate_shader_objects
OGL_API PFNGLPROGRAMUNIFORM4FVPROC _glProgramUniform4fv_ptr = nullptr; // ARB_separate_shader_objects
OGL_API PFNGLPROGRAMUNIFORM4DPROC _glProgramUniform4d_ptr = nullptr; // ARB_separate_shader_objects
OGL_API PFNGLPROGRAMUNIFORM4DVPROC _glProgramUniform4dv_ptr = nullptr; // ARB_separate_shader_objects
OGL_API PFNGLPROGRAMUNIFORM4UIPROC _glProgramUniform4ui_ptr = nullptr; // ARB_separate_shader_objects
OGL_API PFNGLPROGRAMUNIFORM4UIVPROC _glProgramUniform4uiv_ptr = nullptr; // ARB_separate_shader_objects
OGL_API PFNGLPROGRAMUNIFORMMATRIX2FVPROC _glProgramUniformMatrix2fv_ptr = nullptr; // ARB_separate_shader_objects
OGL_API PFNGLPROGRAMUNIFORMMATRIX3FVPROC _glProgramUniformMatrix3fv_ptr = nullptr; // ARB_separate_shader_objects
OGL_API PFNGLPROGRAMUNIFORMMATRIX4FVPROC _glProgramUniformMatrix4fv_ptr = nullptr; // ARB_separate_shader_objects
OGL_API PFNGLPROGRAMUNIFORMMATRIX2DVPROC _glProgramUniformMatrix2dv_ptr = nullptr; // ARB_separate_shader_objects
OGL_API PFNGLPROGRAMUNIFORMMATRIX3DVPROC _glProgramUniformMatrix3dv_ptr = nullptr; // ARB_separate_shader_objects
OGL_API PFNGLPROGRAMUNIFORMMATRIX4DVPROC _glProgramUniformMatrix4dv_ptr = nullptr; // ARB_separate_shader_objects
OGL_API PFNGLPROGRAMUNIFORMMATRIX2X3FVPROC _glProgramUniformMatrix2x3fv_ptr = nullptr; // ARB_separate_shader_objects
OGL_API PFNGLPROGRAMUNIFORMMATRIX3X2FVPROC _glProgramUniformMatrix3x2fv_ptr = nullptr; // ARB_separate_shader_objects
OGL_API PFNGLPROGRAMUNIFORMMATRIX2X4FVPROC _glProgramUniformMatrix2x4fv_ptr = nullptr; // ARB_separate_shader_objects
OGL_API PFNGLPROGRAMUNIFORMMATRIX4X2FVPROC _glProgramUniformMatrix4x2fv_ptr = nullptr; // ARB_separate_shader_objects
OGL_API PFNGLPROGRAMUNIFORMMATRIX3X4FVPROC _glProgramUniformMatrix3x4fv_ptr = nullptr; // ARB_separate_shader_objects
OGL_API PFNGLPROGRAMUNIFORMMATRIX4X3FVPROC _glProgramUniformMatrix4x3fv_ptr = nullptr; // ARB_separate_shader_objects
OGL_API PFNGLPROGRAMUNIFORMMATRIX2X3DVPROC _glProgramUniformMatrix2x3dv_ptr = nullptr; // ARB_separate_shader_objects
OGL_API PFNGLPROGRAMUNIFORMMATRIX3X2DVPROC _glProgramUniformMatrix3x2dv_ptr = nullptr; // ARB_separate_shader_objects
OGL_API PFNGLPROGRAMUNIFORMMATRIX2X4DVPROC _glProgramUniformMatrix2x4dv_ptr = nullptr; // ARB_separate_shader_objects
OGL_API PFNGLPROGRAMUNIFORMMATRIX4X2DVPROC _glProgramUniformMatrix4x2dv_ptr = nullptr; // ARB_separate_shader_objects
OGL_API PFNGLPROGRAMUNIFORMMATRIX3X4DVPROC _glProgramUniformMatrix3x4dv_ptr = nullptr; // ARB_separate_shader_objects
OGL_API PFNGLPROGRAMUNIFORMMATRIX4X3DVPROC _glProgramUniformMatrix4x3dv_ptr = nullptr; // ARB_separate_shader_objects
OGL_API PFNGLVALIDATEPROGRAMPIPELINEPROC _glValidateProgramPipeline_ptr = nullptr; // ARB_separate_shader_objects
OGL_API PFNGLGETPROGRAMPIPELINEINFOLOGPROC _glGetProgramPipelineInfoLog_ptr = nullptr; // ARB_separate_shader_objects
OGL_API PFNGLVERTEXATTRIBL1DPROC _glVertexAttribL1d_ptr = nullptr; // ARB_vertex_attrib_64bit
OGL_API PFNGLVERTEXATTRIBL2DPROC _glVertexAttribL2d_ptr = nullptr; // ARB_vertex_attrib_64bit
OGL_API PFNGLVERTEXATTRIBL3DPROC _glVertexAttribL3d_ptr = nullptr; // ARB_vertex_attrib_64bit
OGL_API PFNGLVERTEXATTRIBL4DPROC _glVertexAttribL4d_ptr = nullptr; // ARB_vertex_attrib_64bit
OGL_API PFNGLVERTEXATTRIBL1DVPROC _glVertexAttribL1dv_ptr = nullptr; // ARB_vertex_attrib_64bit
OGL_API PFNGLVERTEXATTRIBL2DVPROC _glVertexAttribL2dv_ptr = nullptr; // ARB_vertex_attrib_64bit
OGL_API PFNGLVERTEXATTRIBL3DVPROC _glVertexAttribL3dv_ptr = nullptr; // ARB_vertex_attrib_64bit
OGL_API PFNGLVERTEXATTRIBL4DVPROC _glVertexAttribL4dv_ptr = nullptr; // ARB_vertex_attrib_64bit
OGL_API PFNGLVERTEXATTRIBLPOINTERPROC _glVertexAttribLPointer_ptr = nullptr; // ARB_vertex_attrib_64bit
OGL_API PFNGLGETVERTEXATTRIBLDVPROC _glGetVertexAttribLdv_ptr = nullptr; // ARB_vertex_attrib_64bit
OGL_API PFNGLVIEWPORTARRAYVPROC _glViewportArrayv_ptr = nullptr; // ARB_viewport_array
OGL_API PFNGLVIEWPORTINDEXEDFPROC _glViewportIndexedf_ptr = nullptr; // ARB_viewport_array
OGL_API PFNGLVIEWPORTINDEXEDFVPROC _glViewportIndexedfv_ptr = nullptr; // ARB_viewport_array
OGL_API PFNGLSCISSORARRAYVPROC _glScissorArrayv_ptr = nullptr; // ARB_viewport_array
OGL_API PFNGLSCISSORINDEXEDPROC _glScissorIndexed_ptr = nullptr; // ARB_viewport_array
OGL_API PFNGLSCISSORINDEXEDVPROC _glScissorIndexedv_ptr = nullptr; // ARB_viewport_array
OGL_API PFNGLDEPTHRANGEARRAYVPROC _glDepthRangeArrayv_ptr = nullptr; // ARB_viewport_array
OGL_API PFNGLDEPTHRANGEINDEXEDPROC _glDepthRangeIndexed_ptr = nullptr; // ARB_viewport_array
OGL_API PFNGLGETFLOATI_VPROC _glGetFloati_v_ptr = nullptr; // ARB_viewport_array
OGL_API PFNGLGETDOUBLEI_VPROC _glGetDoublei_v_ptr = nullptr; // ARB_viewport_array
#elif defined(__APPLE__) // !__APPLE__
void glRenderbufferStorageMultisampleCoverageNV(GLenum arg1, GLsizei arg2, GLsizei arg3, GLenum arg4, GLsizei arg5, GLsizei arg6) {
	return;
}
#endif

// fallback, error recognition functions
template <size_t N> void a2e_null_func() {
	cout << "ERROR: gl function " << N << " points to nullptr!" << endl;
}

void ext::check_gl_funcs() {
#ifndef __APPLE__
#if !defined(__LINUX__) && !defined(WIN_UNIXENV) // TODO: make this work under linux/cygwin/gcc/clang (-> real dummy functions)
	if(_glRenderbufferStorageMultisampleCoverageNV_ptr == nullptr) _glRenderbufferStorageMultisampleCoverageNV_ptr = (PFNGLRENDERBUFFERSTORAGEMULTISAMPLECOVERAGENVPROC)a2e_null_func<0>;
	if(_glActiveTexture_ptr == nullptr) _glActiveTexture_ptr = (PFNGLACTIVETEXTUREPROC)a2e_null_func<13>;
	if(_glSampleCoverage_ptr == nullptr) _glSampleCoverage_ptr = (PFNGLSAMPLECOVERAGEPROC)a2e_null_func<14>;
	if(_glCompressedTexImage3D_ptr == nullptr) _glCompressedTexImage3D_ptr = (PFNGLCOMPRESSEDTEXIMAGE3DPROC)a2e_null_func<15>;
	if(_glCompressedTexImage2D_ptr == nullptr) _glCompressedTexImage2D_ptr = (PFNGLCOMPRESSEDTEXIMAGE2DPROC)a2e_null_func<16>;
	if(_glCompressedTexImage1D_ptr == nullptr) _glCompressedTexImage1D_ptr = (PFNGLCOMPRESSEDTEXIMAGE1DPROC)a2e_null_func<17>;
	if(_glCompressedTexSubImage3D_ptr == nullptr) _glCompressedTexSubImage3D_ptr = (PFNGLCOMPRESSEDTEXSUBIMAGE3DPROC)a2e_null_func<18>;
	if(_glCompressedTexSubImage2D_ptr == nullptr) _glCompressedTexSubImage2D_ptr = (PFNGLCOMPRESSEDTEXSUBIMAGE2DPROC)a2e_null_func<19>;
	if(_glCompressedTexSubImage1D_ptr == nullptr) _glCompressedTexSubImage1D_ptr = (PFNGLCOMPRESSEDTEXSUBIMAGE1DPROC)a2e_null_func<20>;
	if(_glGetCompressedTexImage_ptr == nullptr) _glGetCompressedTexImage_ptr = (PFNGLGETCOMPRESSEDTEXIMAGEPROC)a2e_null_func<21>;
	if(_glBlendFuncSeparate_ptr == nullptr) _glBlendFuncSeparate_ptr = (PFNGLBLENDFUNCSEPARATEPROC)a2e_null_func<59>;
	if(_glMultiDrawArrays_ptr == nullptr) _glMultiDrawArrays_ptr = (PFNGLMULTIDRAWARRAYSPROC)a2e_null_func<60>;
	if(_glMultiDrawElements_ptr == nullptr) _glMultiDrawElements_ptr = (PFNGLMULTIDRAWELEMENTSPROC)a2e_null_func<61>;
	if(_glPointParameterf_ptr == nullptr) _glPointParameterf_ptr = (PFNGLPOINTPARAMETERFPROC)a2e_null_func<62>;
	if(_glPointParameterfv_ptr == nullptr) _glPointParameterfv_ptr = (PFNGLPOINTPARAMETERFVPROC)a2e_null_func<63>;
	if(_glPointParameteri_ptr == nullptr) _glPointParameteri_ptr = (PFNGLPOINTPARAMETERIPROC)a2e_null_func<64>;
	if(_glPointParameteriv_ptr == nullptr) _glPointParameteriv_ptr = (PFNGLPOINTPARAMETERIVPROC)a2e_null_func<65>;
	if(_glGenQueries_ptr == nullptr) _glGenQueries_ptr = (PFNGLGENQUERIESPROC)a2e_null_func<104>;
	if(_glDeleteQueries_ptr == nullptr) _glDeleteQueries_ptr = (PFNGLDELETEQUERIESPROC)a2e_null_func<105>;
	if(_glIsQuery_ptr == nullptr) _glIsQuery_ptr = (PFNGLISQUERYPROC)a2e_null_func<106>;
	if(_glBeginQuery_ptr == nullptr) _glBeginQuery_ptr = (PFNGLBEGINQUERYPROC)a2e_null_func<107>;
	if(_glEndQuery_ptr == nullptr) _glEndQuery_ptr = (PFNGLENDQUERYPROC)a2e_null_func<108>;
	if(_glGetQueryiv_ptr == nullptr) _glGetQueryiv_ptr = (PFNGLGETQUERYIVPROC)a2e_null_func<109>;
	if(_glGetQueryObjectiv_ptr == nullptr) _glGetQueryObjectiv_ptr = (PFNGLGETQUERYOBJECTIVPROC)a2e_null_func<110>;
	if(_glGetQueryObjectuiv_ptr == nullptr) _glGetQueryObjectuiv_ptr = (PFNGLGETQUERYOBJECTUIVPROC)a2e_null_func<111>;
	if(_glBindBuffer_ptr == nullptr) _glBindBuffer_ptr = (PFNGLBINDBUFFERPROC)a2e_null_func<112>;
	if(_glDeleteBuffers_ptr == nullptr) _glDeleteBuffers_ptr = (PFNGLDELETEBUFFERSPROC)a2e_null_func<113>;
	if(_glGenBuffers_ptr == nullptr) _glGenBuffers_ptr = (PFNGLGENBUFFERSPROC)a2e_null_func<114>;
	if(_glIsBuffer_ptr == nullptr) _glIsBuffer_ptr = (PFNGLISBUFFERPROC)a2e_null_func<115>;
	if(_glBufferData_ptr == nullptr) _glBufferData_ptr = (PFNGLBUFFERDATAPROC)a2e_null_func<116>;
	if(_glBufferSubData_ptr == nullptr) _glBufferSubData_ptr = (PFNGLBUFFERSUBDATAPROC)a2e_null_func<117>;
	if(_glGetBufferSubData_ptr == nullptr) _glGetBufferSubData_ptr = (PFNGLGETBUFFERSUBDATAPROC)a2e_null_func<118>;
	if(_glMapBuffer_ptr == nullptr) _glMapBuffer_ptr = (PFNGLMAPBUFFERPROC)a2e_null_func<119>;
	if(_glUnmapBuffer_ptr == nullptr) _glUnmapBuffer_ptr = (PFNGLUNMAPBUFFERPROC)a2e_null_func<120>;
	if(_glGetBufferParameteriv_ptr == nullptr) _glGetBufferParameteriv_ptr = (PFNGLGETBUFFERPARAMETERIVPROC)a2e_null_func<121>;
	if(_glGetBufferPointerv_ptr == nullptr) _glGetBufferPointerv_ptr = (PFNGLGETBUFFERPOINTERVPROC)a2e_null_func<122>;
	if(_glBlendEquationSeparate_ptr == nullptr) _glBlendEquationSeparate_ptr = (PFNGLBLENDEQUATIONSEPARATEPROC)a2e_null_func<123>;
	if(_glDrawBuffers_ptr == nullptr) _glDrawBuffers_ptr = (PFNGLDRAWBUFFERSPROC)a2e_null_func<124>;
	if(_glStencilOpSeparate_ptr == nullptr) _glStencilOpSeparate_ptr = (PFNGLSTENCILOPSEPARATEPROC)a2e_null_func<125>;
	if(_glStencilFuncSeparate_ptr == nullptr) _glStencilFuncSeparate_ptr = (PFNGLSTENCILFUNCSEPARATEPROC)a2e_null_func<126>;
	if(_glStencilMaskSeparate_ptr == nullptr) _glStencilMaskSeparate_ptr = (PFNGLSTENCILMASKSEPARATEPROC)a2e_null_func<127>;
	if(_glAttachShader_ptr == nullptr) _glAttachShader_ptr = (PFNGLATTACHSHADERPROC)a2e_null_func<128>;
	if(_glBindAttribLocation_ptr == nullptr) _glBindAttribLocation_ptr = (PFNGLBINDATTRIBLOCATIONPROC)a2e_null_func<129>;
	if(_glCompileShader_ptr == nullptr) _glCompileShader_ptr = (PFNGLCOMPILESHADERPROC)a2e_null_func<130>;
	if(_glCreateProgram_ptr == nullptr) _glCreateProgram_ptr = (PFNGLCREATEPROGRAMPROC)a2e_null_func<131>;
	if(_glCreateShader_ptr == nullptr) _glCreateShader_ptr = (PFNGLCREATESHADERPROC)a2e_null_func<132>;
	if(_glDeleteProgram_ptr == nullptr) _glDeleteProgram_ptr = (PFNGLDELETEPROGRAMPROC)a2e_null_func<133>;
	if(_glDeleteShader_ptr == nullptr) _glDeleteShader_ptr = (PFNGLDELETESHADERPROC)a2e_null_func<134>;
	if(_glDetachShader_ptr == nullptr) _glDetachShader_ptr = (PFNGLDETACHSHADERPROC)a2e_null_func<135>;
	if(_glDisableVertexAttribArray_ptr == nullptr) _glDisableVertexAttribArray_ptr = (PFNGLDISABLEVERTEXATTRIBARRAYPROC)a2e_null_func<136>;
	if(_glEnableVertexAttribArray_ptr == nullptr) _glEnableVertexAttribArray_ptr = (PFNGLENABLEVERTEXATTRIBARRAYPROC)a2e_null_func<137>;
	if(_glGetActiveAttrib_ptr == nullptr) _glGetActiveAttrib_ptr = (PFNGLGETACTIVEATTRIBPROC)a2e_null_func<138>;
	if(_glGetActiveUniform_ptr == nullptr) _glGetActiveUniform_ptr = (PFNGLGETACTIVEUNIFORMPROC)a2e_null_func<139>;
	if(_glGetAttachedShaders_ptr == nullptr) _glGetAttachedShaders_ptr = (PFNGLGETATTACHEDSHADERSPROC)a2e_null_func<140>;
	if(_glGetAttribLocation_ptr == nullptr) _glGetAttribLocation_ptr = (PFNGLGETATTRIBLOCATIONPROC)a2e_null_func<141>;
	if(_glGetProgramiv_ptr == nullptr) _glGetProgramiv_ptr = (PFNGLGETPROGRAMIVPROC)a2e_null_func<142>;
	if(_glGetProgramInfoLog_ptr == nullptr) _glGetProgramInfoLog_ptr = (PFNGLGETPROGRAMINFOLOGPROC)a2e_null_func<143>;
	if(_glGetShaderiv_ptr == nullptr) _glGetShaderiv_ptr = (PFNGLGETSHADERIVPROC)a2e_null_func<144>;
	if(_glGetShaderInfoLog_ptr == nullptr) _glGetShaderInfoLog_ptr = (PFNGLGETSHADERINFOLOGPROC)a2e_null_func<145>;
	if(_glGetShaderSource_ptr == nullptr) _glGetShaderSource_ptr = (PFNGLGETSHADERSOURCEPROC)a2e_null_func<146>;
	if(_glGetUniformLocation_ptr == nullptr) _glGetUniformLocation_ptr = (PFNGLGETUNIFORMLOCATIONPROC)a2e_null_func<147>;
	if(_glGetUniformfv_ptr == nullptr) _glGetUniformfv_ptr = (PFNGLGETUNIFORMFVPROC)a2e_null_func<148>;
	if(_glGetUniformiv_ptr == nullptr) _glGetUniformiv_ptr = (PFNGLGETUNIFORMIVPROC)a2e_null_func<149>;
	if(_glGetVertexAttribdv_ptr == nullptr) _glGetVertexAttribdv_ptr = (PFNGLGETVERTEXATTRIBDVPROC)a2e_null_func<150>;
	if(_glGetVertexAttribfv_ptr == nullptr) _glGetVertexAttribfv_ptr = (PFNGLGETVERTEXATTRIBFVPROC)a2e_null_func<151>;
	if(_glGetVertexAttribiv_ptr == nullptr) _glGetVertexAttribiv_ptr = (PFNGLGETVERTEXATTRIBIVPROC)a2e_null_func<152>;
	if(_glGetVertexAttribPointerv_ptr == nullptr) _glGetVertexAttribPointerv_ptr = (PFNGLGETVERTEXATTRIBPOINTERVPROC)a2e_null_func<153>;
	if(_glIsProgram_ptr == nullptr) _glIsProgram_ptr = (PFNGLISPROGRAMPROC)a2e_null_func<154>;
	if(_glIsShader_ptr == nullptr) _glIsShader_ptr = (PFNGLISSHADERPROC)a2e_null_func<155>;
	if(_glLinkProgram_ptr == nullptr) _glLinkProgram_ptr = (PFNGLLINKPROGRAMPROC)a2e_null_func<156>;
	if(_glShaderSource_ptr == nullptr) _glShaderSource_ptr = (PFNGLSHADERSOURCEPROC)a2e_null_func<157>;
	if(_glUseProgram_ptr == nullptr) _glUseProgram_ptr = (PFNGLUSEPROGRAMPROC)a2e_null_func<158>;
	if(_glUniform1f_ptr == nullptr) _glUniform1f_ptr = (PFNGLUNIFORM1FPROC)a2e_null_func<159>;
	if(_glUniform2f_ptr == nullptr) _glUniform2f_ptr = (PFNGLUNIFORM2FPROC)a2e_null_func<160>;
	if(_glUniform3f_ptr == nullptr) _glUniform3f_ptr = (PFNGLUNIFORM3FPROC)a2e_null_func<161>;
	if(_glUniform4f_ptr == nullptr) _glUniform4f_ptr = (PFNGLUNIFORM4FPROC)a2e_null_func<162>;
	if(_glUniform1i_ptr == nullptr) _glUniform1i_ptr = (PFNGLUNIFORM1IPROC)a2e_null_func<163>;
	if(_glUniform2i_ptr == nullptr) _glUniform2i_ptr = (PFNGLUNIFORM2IPROC)a2e_null_func<164>;
	if(_glUniform3i_ptr == nullptr) _glUniform3i_ptr = (PFNGLUNIFORM3IPROC)a2e_null_func<165>;
	if(_glUniform4i_ptr == nullptr) _glUniform4i_ptr = (PFNGLUNIFORM4IPROC)a2e_null_func<166>;
	if(_glUniform1fv_ptr == nullptr) _glUniform1fv_ptr = (PFNGLUNIFORM1FVPROC)a2e_null_func<167>;
	if(_glUniform2fv_ptr == nullptr) _glUniform2fv_ptr = (PFNGLUNIFORM2FVPROC)a2e_null_func<168>;
	if(_glUniform3fv_ptr == nullptr) _glUniform3fv_ptr = (PFNGLUNIFORM3FVPROC)a2e_null_func<169>;
	if(_glUniform4fv_ptr == nullptr) _glUniform4fv_ptr = (PFNGLUNIFORM4FVPROC)a2e_null_func<170>;
	if(_glUniform1iv_ptr == nullptr) _glUniform1iv_ptr = (PFNGLUNIFORM1IVPROC)a2e_null_func<171>;
	if(_glUniform2iv_ptr == nullptr) _glUniform2iv_ptr = (PFNGLUNIFORM2IVPROC)a2e_null_func<172>;
	if(_glUniform3iv_ptr == nullptr) _glUniform3iv_ptr = (PFNGLUNIFORM3IVPROC)a2e_null_func<173>;
	if(_glUniform4iv_ptr == nullptr) _glUniform4iv_ptr = (PFNGLUNIFORM4IVPROC)a2e_null_func<174>;
	if(_glUniformMatrix2fv_ptr == nullptr) _glUniformMatrix2fv_ptr = (PFNGLUNIFORMMATRIX2FVPROC)a2e_null_func<175>;
	if(_glUniformMatrix3fv_ptr == nullptr) _glUniformMatrix3fv_ptr = (PFNGLUNIFORMMATRIX3FVPROC)a2e_null_func<176>;
	if(_glUniformMatrix4fv_ptr == nullptr) _glUniformMatrix4fv_ptr = (PFNGLUNIFORMMATRIX4FVPROC)a2e_null_func<177>;
	if(_glValidateProgram_ptr == nullptr) _glValidateProgram_ptr = (PFNGLVALIDATEPROGRAMPROC)a2e_null_func<178>;
	if(_glVertexAttrib1d_ptr == nullptr) _glVertexAttrib1d_ptr = (PFNGLVERTEXATTRIB1DPROC)a2e_null_func<179>;
	if(_glVertexAttrib1dv_ptr == nullptr) _glVertexAttrib1dv_ptr = (PFNGLVERTEXATTRIB1DVPROC)a2e_null_func<180>;
	if(_glVertexAttrib1f_ptr == nullptr) _glVertexAttrib1f_ptr = (PFNGLVERTEXATTRIB1FPROC)a2e_null_func<181>;
	if(_glVertexAttrib1fv_ptr == nullptr) _glVertexAttrib1fv_ptr = (PFNGLVERTEXATTRIB1FVPROC)a2e_null_func<182>;
	if(_glVertexAttrib1s_ptr == nullptr) _glVertexAttrib1s_ptr = (PFNGLVERTEXATTRIB1SPROC)a2e_null_func<183>;
	if(_glVertexAttrib1sv_ptr == nullptr) _glVertexAttrib1sv_ptr = (PFNGLVERTEXATTRIB1SVPROC)a2e_null_func<184>;
	if(_glVertexAttrib2d_ptr == nullptr) _glVertexAttrib2d_ptr = (PFNGLVERTEXATTRIB2DPROC)a2e_null_func<185>;
	if(_glVertexAttrib2dv_ptr == nullptr) _glVertexAttrib2dv_ptr = (PFNGLVERTEXATTRIB2DVPROC)a2e_null_func<186>;
	if(_glVertexAttrib2f_ptr == nullptr) _glVertexAttrib2f_ptr = (PFNGLVERTEXATTRIB2FPROC)a2e_null_func<187>;
	if(_glVertexAttrib2fv_ptr == nullptr) _glVertexAttrib2fv_ptr = (PFNGLVERTEXATTRIB2FVPROC)a2e_null_func<188>;
	if(_glVertexAttrib2s_ptr == nullptr) _glVertexAttrib2s_ptr = (PFNGLVERTEXATTRIB2SPROC)a2e_null_func<189>;
	if(_glVertexAttrib2sv_ptr == nullptr) _glVertexAttrib2sv_ptr = (PFNGLVERTEXATTRIB2SVPROC)a2e_null_func<190>;
	if(_glVertexAttrib3d_ptr == nullptr) _glVertexAttrib3d_ptr = (PFNGLVERTEXATTRIB3DPROC)a2e_null_func<191>;
	if(_glVertexAttrib3dv_ptr == nullptr) _glVertexAttrib3dv_ptr = (PFNGLVERTEXATTRIB3DVPROC)a2e_null_func<192>;
	if(_glVertexAttrib3f_ptr == nullptr) _glVertexAttrib3f_ptr = (PFNGLVERTEXATTRIB3FPROC)a2e_null_func<193>;
	if(_glVertexAttrib3fv_ptr == nullptr) _glVertexAttrib3fv_ptr = (PFNGLVERTEXATTRIB3FVPROC)a2e_null_func<194>;
	if(_glVertexAttrib3s_ptr == nullptr) _glVertexAttrib3s_ptr = (PFNGLVERTEXATTRIB3SPROC)a2e_null_func<195>;
	if(_glVertexAttrib3sv_ptr == nullptr) _glVertexAttrib3sv_ptr = (PFNGLVERTEXATTRIB3SVPROC)a2e_null_func<196>;
	if(_glVertexAttrib4Nbv_ptr == nullptr) _glVertexAttrib4Nbv_ptr = (PFNGLVERTEXATTRIB4NBVPROC)a2e_null_func<197>;
	if(_glVertexAttrib4Niv_ptr == nullptr) _glVertexAttrib4Niv_ptr = (PFNGLVERTEXATTRIB4NIVPROC)a2e_null_func<198>;
	if(_glVertexAttrib4Nsv_ptr == nullptr) _glVertexAttrib4Nsv_ptr = (PFNGLVERTEXATTRIB4NSVPROC)a2e_null_func<199>;
	if(_glVertexAttrib4Nub_ptr == nullptr) _glVertexAttrib4Nub_ptr = (PFNGLVERTEXATTRIB4NUBPROC)a2e_null_func<200>;
	if(_glVertexAttrib4Nubv_ptr == nullptr) _glVertexAttrib4Nubv_ptr = (PFNGLVERTEXATTRIB4NUBVPROC)a2e_null_func<201>;
	if(_glVertexAttrib4Nuiv_ptr == nullptr) _glVertexAttrib4Nuiv_ptr = (PFNGLVERTEXATTRIB4NUIVPROC)a2e_null_func<202>;
	if(_glVertexAttrib4Nusv_ptr == nullptr) _glVertexAttrib4Nusv_ptr = (PFNGLVERTEXATTRIB4NUSVPROC)a2e_null_func<203>;
	if(_glVertexAttrib4bv_ptr == nullptr) _glVertexAttrib4bv_ptr = (PFNGLVERTEXATTRIB4BVPROC)a2e_null_func<204>;
	if(_glVertexAttrib4d_ptr == nullptr) _glVertexAttrib4d_ptr = (PFNGLVERTEXATTRIB4DPROC)a2e_null_func<205>;
	if(_glVertexAttrib4dv_ptr == nullptr) _glVertexAttrib4dv_ptr = (PFNGLVERTEXATTRIB4DVPROC)a2e_null_func<206>;
	if(_glVertexAttrib4f_ptr == nullptr) _glVertexAttrib4f_ptr = (PFNGLVERTEXATTRIB4FPROC)a2e_null_func<207>;
	if(_glVertexAttrib4fv_ptr == nullptr) _glVertexAttrib4fv_ptr = (PFNGLVERTEXATTRIB4FVPROC)a2e_null_func<208>;
	if(_glVertexAttrib4iv_ptr == nullptr) _glVertexAttrib4iv_ptr = (PFNGLVERTEXATTRIB4IVPROC)a2e_null_func<209>;
	if(_glVertexAttrib4s_ptr == nullptr) _glVertexAttrib4s_ptr = (PFNGLVERTEXATTRIB4SPROC)a2e_null_func<210>;
	if(_glVertexAttrib4sv_ptr == nullptr) _glVertexAttrib4sv_ptr = (PFNGLVERTEXATTRIB4SVPROC)a2e_null_func<211>;
	if(_glVertexAttrib4ubv_ptr == nullptr) _glVertexAttrib4ubv_ptr = (PFNGLVERTEXATTRIB4UBVPROC)a2e_null_func<212>;
	if(_glVertexAttrib4uiv_ptr == nullptr) _glVertexAttrib4uiv_ptr = (PFNGLVERTEXATTRIB4UIVPROC)a2e_null_func<213>;
	if(_glVertexAttrib4usv_ptr == nullptr) _glVertexAttrib4usv_ptr = (PFNGLVERTEXATTRIB4USVPROC)a2e_null_func<214>;
	if(_glVertexAttribPointer_ptr == nullptr) _glVertexAttribPointer_ptr = (PFNGLVERTEXATTRIBPOINTERPROC)a2e_null_func<215>;
	if(_glUniformMatrix2x3fv_ptr == nullptr) _glUniformMatrix2x3fv_ptr = (PFNGLUNIFORMMATRIX2X3FVPROC)a2e_null_func<216>;
	if(_glUniformMatrix3x2fv_ptr == nullptr) _glUniformMatrix3x2fv_ptr = (PFNGLUNIFORMMATRIX3X2FVPROC)a2e_null_func<217>;
	if(_glUniformMatrix2x4fv_ptr == nullptr) _glUniformMatrix2x4fv_ptr = (PFNGLUNIFORMMATRIX2X4FVPROC)a2e_null_func<218>;
	if(_glUniformMatrix4x2fv_ptr == nullptr) _glUniformMatrix4x2fv_ptr = (PFNGLUNIFORMMATRIX4X2FVPROC)a2e_null_func<219>;
	if(_glUniformMatrix3x4fv_ptr == nullptr) _glUniformMatrix3x4fv_ptr = (PFNGLUNIFORMMATRIX3X4FVPROC)a2e_null_func<220>;
	if(_glUniformMatrix4x3fv_ptr == nullptr) _glUniformMatrix4x3fv_ptr = (PFNGLUNIFORMMATRIX4X3FVPROC)a2e_null_func<221>;
	if(_glColorMaski_ptr == nullptr) _glColorMaski_ptr = (PFNGLCOLORMASKIPROC)a2e_null_func<222>;
	if(_glGetBooleani_v_ptr == nullptr) _glGetBooleani_v_ptr = (PFNGLGETBOOLEANI_VPROC)a2e_null_func<223>;
	if(_glGetIntegeri_v_ptr == nullptr) _glGetIntegeri_v_ptr = (PFNGLGETINTEGERI_VPROC)a2e_null_func<224>;
	if(_glEnablei_ptr == nullptr) _glEnablei_ptr = (PFNGLENABLEIPROC)a2e_null_func<225>;
	if(_glDisablei_ptr == nullptr) _glDisablei_ptr = (PFNGLDISABLEIPROC)a2e_null_func<226>;
	if(_glIsEnabledi_ptr == nullptr) _glIsEnabledi_ptr = (PFNGLISENABLEDIPROC)a2e_null_func<227>;
	if(_glBeginTransformFeedback_ptr == nullptr) _glBeginTransformFeedback_ptr = (PFNGLBEGINTRANSFORMFEEDBACKPROC)a2e_null_func<228>;
	if(_glEndTransformFeedback_ptr == nullptr) _glEndTransformFeedback_ptr = (PFNGLENDTRANSFORMFEEDBACKPROC)a2e_null_func<229>;
	if(_glBindBufferRange_ptr == nullptr) _glBindBufferRange_ptr = (PFNGLBINDBUFFERRANGEPROC)a2e_null_func<230>;
	if(_glBindBufferBase_ptr == nullptr) _glBindBufferBase_ptr = (PFNGLBINDBUFFERBASEPROC)a2e_null_func<231>;
	if(_glTransformFeedbackVaryings_ptr == nullptr) _glTransformFeedbackVaryings_ptr = (PFNGLTRANSFORMFEEDBACKVARYINGSPROC)a2e_null_func<232>;
	if(_glGetTransformFeedbackVarying_ptr == nullptr) _glGetTransformFeedbackVarying_ptr = (PFNGLGETTRANSFORMFEEDBACKVARYINGPROC)a2e_null_func<233>;
	if(_glClampColor_ptr == nullptr) _glClampColor_ptr = (PFNGLCLAMPCOLORPROC)a2e_null_func<234>;
	if(_glBeginConditionalRender_ptr == nullptr) _glBeginConditionalRender_ptr = (PFNGLBEGINCONDITIONALRENDERPROC)a2e_null_func<235>;
	if(_glEndConditionalRender_ptr == nullptr) _glEndConditionalRender_ptr = (PFNGLENDCONDITIONALRENDERPROC)a2e_null_func<236>;
	if(_glVertexAttribIPointer_ptr == nullptr) _glVertexAttribIPointer_ptr = (PFNGLVERTEXATTRIBIPOINTERPROC)a2e_null_func<237>;
	if(_glGetVertexAttribIiv_ptr == nullptr) _glGetVertexAttribIiv_ptr = (PFNGLGETVERTEXATTRIBIIVPROC)a2e_null_func<238>;
	if(_glGetVertexAttribIuiv_ptr == nullptr) _glGetVertexAttribIuiv_ptr = (PFNGLGETVERTEXATTRIBIUIVPROC)a2e_null_func<239>;
	if(_glVertexAttribI1i_ptr == nullptr) _glVertexAttribI1i_ptr = (PFNGLVERTEXATTRIBI1IPROC)a2e_null_func<240>;
	if(_glVertexAttribI2i_ptr == nullptr) _glVertexAttribI2i_ptr = (PFNGLVERTEXATTRIBI2IPROC)a2e_null_func<241>;
	if(_glVertexAttribI3i_ptr == nullptr) _glVertexAttribI3i_ptr = (PFNGLVERTEXATTRIBI3IPROC)a2e_null_func<242>;
	if(_glVertexAttribI4i_ptr == nullptr) _glVertexAttribI4i_ptr = (PFNGLVERTEXATTRIBI4IPROC)a2e_null_func<243>;
	if(_glVertexAttribI1ui_ptr == nullptr) _glVertexAttribI1ui_ptr = (PFNGLVERTEXATTRIBI1UIPROC)a2e_null_func<244>;
	if(_glVertexAttribI2ui_ptr == nullptr) _glVertexAttribI2ui_ptr = (PFNGLVERTEXATTRIBI2UIPROC)a2e_null_func<245>;
	if(_glVertexAttribI3ui_ptr == nullptr) _glVertexAttribI3ui_ptr = (PFNGLVERTEXATTRIBI3UIPROC)a2e_null_func<246>;
	if(_glVertexAttribI4ui_ptr == nullptr) _glVertexAttribI4ui_ptr = (PFNGLVERTEXATTRIBI4UIPROC)a2e_null_func<247>;
	if(_glVertexAttribI1iv_ptr == nullptr) _glVertexAttribI1iv_ptr = (PFNGLVERTEXATTRIBI1IVPROC)a2e_null_func<248>;
	if(_glVertexAttribI2iv_ptr == nullptr) _glVertexAttribI2iv_ptr = (PFNGLVERTEXATTRIBI2IVPROC)a2e_null_func<249>;
	if(_glVertexAttribI3iv_ptr == nullptr) _glVertexAttribI3iv_ptr = (PFNGLVERTEXATTRIBI3IVPROC)a2e_null_func<250>;
	if(_glVertexAttribI4iv_ptr == nullptr) _glVertexAttribI4iv_ptr = (PFNGLVERTEXATTRIBI4IVPROC)a2e_null_func<251>;
	if(_glVertexAttribI1uiv_ptr == nullptr) _glVertexAttribI1uiv_ptr = (PFNGLVERTEXATTRIBI1UIVPROC)a2e_null_func<252>;
	if(_glVertexAttribI2uiv_ptr == nullptr) _glVertexAttribI2uiv_ptr = (PFNGLVERTEXATTRIBI2UIVPROC)a2e_null_func<253>;
	if(_glVertexAttribI3uiv_ptr == nullptr) _glVertexAttribI3uiv_ptr = (PFNGLVERTEXATTRIBI3UIVPROC)a2e_null_func<254>;
	if(_glVertexAttribI4uiv_ptr == nullptr) _glVertexAttribI4uiv_ptr = (PFNGLVERTEXATTRIBI4UIVPROC)a2e_null_func<255>;
	if(_glVertexAttribI4bv_ptr == nullptr) _glVertexAttribI4bv_ptr = (PFNGLVERTEXATTRIBI4BVPROC)a2e_null_func<256>;
	if(_glVertexAttribI4sv_ptr == nullptr) _glVertexAttribI4sv_ptr = (PFNGLVERTEXATTRIBI4SVPROC)a2e_null_func<257>;
	if(_glVertexAttribI4ubv_ptr == nullptr) _glVertexAttribI4ubv_ptr = (PFNGLVERTEXATTRIBI4UBVPROC)a2e_null_func<258>;
	if(_glVertexAttribI4usv_ptr == nullptr) _glVertexAttribI4usv_ptr = (PFNGLVERTEXATTRIBI4USVPROC)a2e_null_func<259>;
	if(_glGetUniformuiv_ptr == nullptr) _glGetUniformuiv_ptr = (PFNGLGETUNIFORMUIVPROC)a2e_null_func<260>;
	if(_glBindFragDataLocation_ptr == nullptr) _glBindFragDataLocation_ptr = (PFNGLBINDFRAGDATALOCATIONPROC)a2e_null_func<261>;
	if(_glGetFragDataLocation_ptr == nullptr) _glGetFragDataLocation_ptr = (PFNGLGETFRAGDATALOCATIONPROC)a2e_null_func<262>;
	if(_glUniform1ui_ptr == nullptr) _glUniform1ui_ptr = (PFNGLUNIFORM1UIPROC)a2e_null_func<263>;
	if(_glUniform2ui_ptr == nullptr) _glUniform2ui_ptr = (PFNGLUNIFORM2UIPROC)a2e_null_func<264>;
	if(_glUniform3ui_ptr == nullptr) _glUniform3ui_ptr = (PFNGLUNIFORM3UIPROC)a2e_null_func<265>;
	if(_glUniform4ui_ptr == nullptr) _glUniform4ui_ptr = (PFNGLUNIFORM4UIPROC)a2e_null_func<266>;
	if(_glUniform1uiv_ptr == nullptr) _glUniform1uiv_ptr = (PFNGLUNIFORM1UIVPROC)a2e_null_func<267>;
	if(_glUniform2uiv_ptr == nullptr) _glUniform2uiv_ptr = (PFNGLUNIFORM2UIVPROC)a2e_null_func<268>;
	if(_glUniform3uiv_ptr == nullptr) _glUniform3uiv_ptr = (PFNGLUNIFORM3UIVPROC)a2e_null_func<269>;
	if(_glUniform4uiv_ptr == nullptr) _glUniform4uiv_ptr = (PFNGLUNIFORM4UIVPROC)a2e_null_func<270>;
	if(_glTexParameterIiv_ptr == nullptr) _glTexParameterIiv_ptr = (PFNGLTEXPARAMETERIIVPROC)a2e_null_func<271>;
	if(_glTexParameterIuiv_ptr == nullptr) _glTexParameterIuiv_ptr = (PFNGLTEXPARAMETERIUIVPROC)a2e_null_func<272>;
	if(_glGetTexParameterIiv_ptr == nullptr) _glGetTexParameterIiv_ptr = (PFNGLGETTEXPARAMETERIIVPROC)a2e_null_func<273>;
	if(_glGetTexParameterIuiv_ptr == nullptr) _glGetTexParameterIuiv_ptr = (PFNGLGETTEXPARAMETERIUIVPROC)a2e_null_func<274>;
	if(_glClearBufferiv_ptr == nullptr) _glClearBufferiv_ptr = (PFNGLCLEARBUFFERIVPROC)a2e_null_func<275>;
	if(_glClearBufferuiv_ptr == nullptr) _glClearBufferuiv_ptr = (PFNGLCLEARBUFFERUIVPROC)a2e_null_func<276>;
	if(_glClearBufferfv_ptr == nullptr) _glClearBufferfv_ptr = (PFNGLCLEARBUFFERFVPROC)a2e_null_func<277>;
	if(_glClearBufferfi_ptr == nullptr) _glClearBufferfi_ptr = (PFNGLCLEARBUFFERFIPROC)a2e_null_func<278>;
	if(_glGetStringi_ptr == nullptr) _glGetStringi_ptr = (PFNGLGETSTRINGIPROC)a2e_null_func<279>;
	if(_glIsRenderbuffer_ptr == nullptr) _glIsRenderbuffer_ptr = (PFNGLISRENDERBUFFERPROC)a2e_null_func<280>;
	if(_glBindRenderbuffer_ptr == nullptr) _glBindRenderbuffer_ptr = (PFNGLBINDRENDERBUFFERPROC)a2e_null_func<281>;
	if(_glDeleteRenderbuffers_ptr == nullptr) _glDeleteRenderbuffers_ptr = (PFNGLDELETERENDERBUFFERSPROC)a2e_null_func<282>;
	if(_glGenRenderbuffers_ptr == nullptr) _glGenRenderbuffers_ptr = (PFNGLGENRENDERBUFFERSPROC)a2e_null_func<283>;
	if(_glRenderbufferStorage_ptr == nullptr) _glRenderbufferStorage_ptr = (PFNGLRENDERBUFFERSTORAGEPROC)a2e_null_func<284>;
	if(_glGetRenderbufferParameteriv_ptr == nullptr) _glGetRenderbufferParameteriv_ptr = (PFNGLGETRENDERBUFFERPARAMETERIVPROC)a2e_null_func<285>;
	if(_glIsFramebuffer_ptr == nullptr) _glIsFramebuffer_ptr = (PFNGLISFRAMEBUFFERPROC)a2e_null_func<286>;
	if(_glBindFramebuffer_ptr == nullptr) _glBindFramebuffer_ptr = (PFNGLBINDFRAMEBUFFERPROC)a2e_null_func<287>;
	if(_glDeleteFramebuffers_ptr == nullptr) _glDeleteFramebuffers_ptr = (PFNGLDELETEFRAMEBUFFERSPROC)a2e_null_func<288>;
	if(_glGenFramebuffers_ptr == nullptr) _glGenFramebuffers_ptr = (PFNGLGENFRAMEBUFFERSPROC)a2e_null_func<289>;
	if(_glCheckFramebufferStatus_ptr == nullptr) _glCheckFramebufferStatus_ptr = (PFNGLCHECKFRAMEBUFFERSTATUSPROC)a2e_null_func<290>;
	if(_glFramebufferTexture1D_ptr == nullptr) _glFramebufferTexture1D_ptr = (PFNGLFRAMEBUFFERTEXTURE1DPROC)a2e_null_func<291>;
	if(_glFramebufferTexture2D_ptr == nullptr) _glFramebufferTexture2D_ptr = (PFNGLFRAMEBUFFERTEXTURE2DPROC)a2e_null_func<292>;
	if(_glFramebufferTexture3D_ptr == nullptr) _glFramebufferTexture3D_ptr = (PFNGLFRAMEBUFFERTEXTURE3DPROC)a2e_null_func<293>;
	if(_glFramebufferRenderbuffer_ptr == nullptr) _glFramebufferRenderbuffer_ptr = (PFNGLFRAMEBUFFERRENDERBUFFERPROC)a2e_null_func<294>;
	if(_glGetFramebufferAttachmentParameteriv_ptr == nullptr) _glGetFramebufferAttachmentParameteriv_ptr = (PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVPROC)a2e_null_func<295>;
	if(_glGenerateMipmap_ptr == nullptr) _glGenerateMipmap_ptr = (PFNGLGENERATEMIPMAPPROC)a2e_null_func<296>;
	if(_glBlitFramebuffer_ptr == nullptr) _glBlitFramebuffer_ptr = (PFNGLBLITFRAMEBUFFERPROC)a2e_null_func<297>;
	if(_glRenderbufferStorageMultisample_ptr == nullptr) _glRenderbufferStorageMultisample_ptr = (PFNGLRENDERBUFFERSTORAGEMULTISAMPLEPROC)a2e_null_func<298>;
	if(_glFramebufferTextureLayer_ptr == nullptr) _glFramebufferTextureLayer_ptr = (PFNGLFRAMEBUFFERTEXTURELAYERPROC)a2e_null_func<299>;
	if(_glMapBufferRange_ptr == nullptr) _glMapBufferRange_ptr = (PFNGLMAPBUFFERRANGEPROC)a2e_null_func<300>;
	if(_glFlushMappedBufferRange_ptr == nullptr) _glFlushMappedBufferRange_ptr = (PFNGLFLUSHMAPPEDBUFFERRANGEPROC)a2e_null_func<301>;
	if(_glBindVertexArray_ptr == nullptr) _glBindVertexArray_ptr = (PFNGLBINDVERTEXARRAYPROC)a2e_null_func<302>;
	if(_glDeleteVertexArrays_ptr == nullptr) _glDeleteVertexArrays_ptr = (PFNGLDELETEVERTEXARRAYSPROC)a2e_null_func<303>;
	if(_glGenVertexArrays_ptr == nullptr) _glGenVertexArrays_ptr = (PFNGLGENVERTEXARRAYSPROC)a2e_null_func<304>;
	if(_glIsVertexArray_ptr == nullptr) _glIsVertexArray_ptr = (PFNGLISVERTEXARRAYPROC)a2e_null_func<305>;
	if(_glDrawArraysInstanced_ptr == nullptr) _glDrawArraysInstanced_ptr = (PFNGLDRAWARRAYSINSTANCEDPROC)a2e_null_func<306>;
	if(_glDrawElementsInstanced_ptr == nullptr) _glDrawElementsInstanced_ptr = (PFNGLDRAWELEMENTSINSTANCEDPROC)a2e_null_func<307>;
	if(_glTexBuffer_ptr == nullptr) _glTexBuffer_ptr = (PFNGLTEXBUFFERPROC)a2e_null_func<308>;
	if(_glPrimitiveRestartIndex_ptr == nullptr) _glPrimitiveRestartIndex_ptr = (PFNGLPRIMITIVERESTARTINDEXPROC)a2e_null_func<309>;
	if(_glCopyBufferSubData_ptr == nullptr) _glCopyBufferSubData_ptr = (PFNGLCOPYBUFFERSUBDATAPROC)a2e_null_func<310>;
	if(_glGetUniformIndices_ptr == nullptr) _glGetUniformIndices_ptr = (PFNGLGETUNIFORMINDICESPROC)a2e_null_func<311>;
	if(_glGetActiveUniformsiv_ptr == nullptr) _glGetActiveUniformsiv_ptr = (PFNGLGETACTIVEUNIFORMSIVPROC)a2e_null_func<312>;
	if(_glGetActiveUniformName_ptr == nullptr) _glGetActiveUniformName_ptr = (PFNGLGETACTIVEUNIFORMNAMEPROC)a2e_null_func<313>;
	if(_glGetUniformBlockIndex_ptr == nullptr) _glGetUniformBlockIndex_ptr = (PFNGLGETUNIFORMBLOCKINDEXPROC)a2e_null_func<314>;
	if(_glGetActiveUniformBlockiv_ptr == nullptr) _glGetActiveUniformBlockiv_ptr = (PFNGLGETACTIVEUNIFORMBLOCKIVPROC)a2e_null_func<315>;
	if(_glGetActiveUniformBlockName_ptr == nullptr) _glGetActiveUniformBlockName_ptr = (PFNGLGETACTIVEUNIFORMBLOCKNAMEPROC)a2e_null_func<316>;
	if(_glUniformBlockBinding_ptr == nullptr) _glUniformBlockBinding_ptr = (PFNGLUNIFORMBLOCKBINDINGPROC)a2e_null_func<317>;
	if(_glGetInteger64i_v_ptr == nullptr) _glGetInteger64i_v_ptr = (PFNGLGETINTEGER64I_VPROC)a2e_null_func<318>;
	if(_glGetBufferParameteri64v_ptr == nullptr) _glGetBufferParameteri64v_ptr = (PFNGLGETBUFFERPARAMETERI64VPROC)a2e_null_func<319>;
	if(_glFramebufferTexture_ptr == nullptr) _glFramebufferTexture_ptr = (PFNGLFRAMEBUFFERTEXTUREPROC)a2e_null_func<320>;
	if(_glDrawElementsBaseVertex_ptr == nullptr) _glDrawElementsBaseVertex_ptr = (PFNGLDRAWELEMENTSBASEVERTEXPROC)a2e_null_func<321>;
	if(_glDrawRangeElementsBaseVertex_ptr == nullptr) _glDrawRangeElementsBaseVertex_ptr = (PFNGLDRAWRANGEELEMENTSBASEVERTEXPROC)a2e_null_func<322>;
	if(_glDrawElementsInstancedBaseVertex_ptr == nullptr) _glDrawElementsInstancedBaseVertex_ptr = (PFNGLDRAWELEMENTSINSTANCEDBASEVERTEXPROC)a2e_null_func<323>;
	if(_glMultiDrawElementsBaseVertex_ptr == nullptr) _glMultiDrawElementsBaseVertex_ptr = (PFNGLMULTIDRAWELEMENTSBASEVERTEXPROC)a2e_null_func<324>;
	if(_glProvokingVertex_ptr == nullptr) _glProvokingVertex_ptr = (PFNGLPROVOKINGVERTEXPROC)a2e_null_func<325>;
	if(_glFenceSync_ptr == nullptr) _glFenceSync_ptr = (PFNGLFENCESYNCPROC)a2e_null_func<326>;
	if(_glIsSync_ptr == nullptr) _glIsSync_ptr = (PFNGLISSYNCPROC)a2e_null_func<327>;
	if(_glDeleteSync_ptr == nullptr) _glDeleteSync_ptr = (PFNGLDELETESYNCPROC)a2e_null_func<328>;
	if(_glClientWaitSync_ptr == nullptr) _glClientWaitSync_ptr = (PFNGLCLIENTWAITSYNCPROC)a2e_null_func<329>;
	if(_glWaitSync_ptr == nullptr) _glWaitSync_ptr = (PFNGLWAITSYNCPROC)a2e_null_func<330>;
	if(_glGetInteger64v_ptr == nullptr) _glGetInteger64v_ptr = (PFNGLGETINTEGER64VPROC)a2e_null_func<331>;
	if(_glGetSynciv_ptr == nullptr) _glGetSynciv_ptr = (PFNGLGETSYNCIVPROC)a2e_null_func<332>;
	if(_glTexImage2DMultisample_ptr == nullptr) _glTexImage2DMultisample_ptr = (PFNGLTEXIMAGE2DMULTISAMPLEPROC)a2e_null_func<333>;
	if(_glTexImage3DMultisample_ptr == nullptr) _glTexImage3DMultisample_ptr = (PFNGLTEXIMAGE3DMULTISAMPLEPROC)a2e_null_func<334>;
	if(_glGetMultisamplefv_ptr == nullptr) _glGetMultisamplefv_ptr = (PFNGLGETMULTISAMPLEFVPROC)a2e_null_func<335>;
	if(_glSampleMaski_ptr == nullptr) _glSampleMaski_ptr = (PFNGLSAMPLEMASKIPROC)a2e_null_func<336>;
	if(_glVertexAttribDivisor_ptr == nullptr) _glVertexAttribDivisor_ptr = (PFNGLVERTEXATTRIBDIVISORPROC)a2e_null_func<337>;
	if(_glBindFragDataLocationIndexed_ptr == nullptr) _glBindFragDataLocationIndexed_ptr = (PFNGLBINDFRAGDATALOCATIONINDEXEDPROC)a2e_null_func<338>;
	if(_glGetFragDataIndex_ptr == nullptr) _glGetFragDataIndex_ptr = (PFNGLGETFRAGDATAINDEXPROC)a2e_null_func<339>;
	if(_glGenSamplers_ptr == nullptr) _glGenSamplers_ptr = (PFNGLGENSAMPLERSPROC)a2e_null_func<340>;
	if(_glDeleteSamplers_ptr == nullptr) _glDeleteSamplers_ptr = (PFNGLDELETESAMPLERSPROC)a2e_null_func<341>;
	if(_glIsSampler_ptr == nullptr) _glIsSampler_ptr = (PFNGLISSAMPLERPROC)a2e_null_func<342>;
	if(_glBindSampler_ptr == nullptr) _glBindSampler_ptr = (PFNGLBINDSAMPLERPROC)a2e_null_func<343>;
	if(_glSamplerParameteri_ptr == nullptr) _glSamplerParameteri_ptr = (PFNGLSAMPLERPARAMETERIPROC)a2e_null_func<344>;
	if(_glSamplerParameteriv_ptr == nullptr) _glSamplerParameteriv_ptr = (PFNGLSAMPLERPARAMETERIVPROC)a2e_null_func<345>;
	if(_glSamplerParameterf_ptr == nullptr) _glSamplerParameterf_ptr = (PFNGLSAMPLERPARAMETERFPROC)a2e_null_func<346>;
	if(_glSamplerParameterfv_ptr == nullptr) _glSamplerParameterfv_ptr = (PFNGLSAMPLERPARAMETERFVPROC)a2e_null_func<347>;
	if(_glSamplerParameterIiv_ptr == nullptr) _glSamplerParameterIiv_ptr = (PFNGLSAMPLERPARAMETERIIVPROC)a2e_null_func<348>;
	if(_glSamplerParameterIuiv_ptr == nullptr) _glSamplerParameterIuiv_ptr = (PFNGLSAMPLERPARAMETERIUIVPROC)a2e_null_func<349>;
	if(_glGetSamplerParameteriv_ptr == nullptr) _glGetSamplerParameteriv_ptr = (PFNGLGETSAMPLERPARAMETERIVPROC)a2e_null_func<350>;
	if(_glGetSamplerParameterIiv_ptr == nullptr) _glGetSamplerParameterIiv_ptr = (PFNGLGETSAMPLERPARAMETERIIVPROC)a2e_null_func<351>;
	if(_glGetSamplerParameterfv_ptr == nullptr) _glGetSamplerParameterfv_ptr = (PFNGLGETSAMPLERPARAMETERFVPROC)a2e_null_func<352>;
	if(_glGetSamplerParameterIuiv_ptr == nullptr) _glGetSamplerParameterIuiv_ptr = (PFNGLGETSAMPLERPARAMETERIUIVPROC)a2e_null_func<353>;
	if(_glQueryCounter_ptr == nullptr) _glQueryCounter_ptr = (PFNGLQUERYCOUNTERPROC)a2e_null_func<354>;
	if(_glGetQueryObjecti64v_ptr == nullptr) _glGetQueryObjecti64v_ptr = (PFNGLGETQUERYOBJECTI64VPROC)a2e_null_func<355>;
	if(_glGetQueryObjectui64v_ptr == nullptr) _glGetQueryObjectui64v_ptr = (PFNGLGETQUERYOBJECTUI64VPROC)a2e_null_func<356>;
	if(_glVertexP2ui_ptr == nullptr) _glVertexP2ui_ptr = (PFNGLVERTEXP2UIPROC)a2e_null_func<357>;
	if(_glVertexP2uiv_ptr == nullptr) _glVertexP2uiv_ptr = (PFNGLVERTEXP2UIVPROC)a2e_null_func<358>;
	if(_glVertexP3ui_ptr == nullptr) _glVertexP3ui_ptr = (PFNGLVERTEXP3UIPROC)a2e_null_func<359>;
	if(_glVertexP3uiv_ptr == nullptr) _glVertexP3uiv_ptr = (PFNGLVERTEXP3UIVPROC)a2e_null_func<360>;
	if(_glVertexP4ui_ptr == nullptr) _glVertexP4ui_ptr = (PFNGLVERTEXP4UIPROC)a2e_null_func<361>;
	if(_glVertexP4uiv_ptr == nullptr) _glVertexP4uiv_ptr = (PFNGLVERTEXP4UIVPROC)a2e_null_func<362>;
	if(_glTexCoordP1ui_ptr == nullptr) _glTexCoordP1ui_ptr = (PFNGLTEXCOORDP1UIPROC)a2e_null_func<363>;
	if(_glTexCoordP1uiv_ptr == nullptr) _glTexCoordP1uiv_ptr = (PFNGLTEXCOORDP1UIVPROC)a2e_null_func<364>;
	if(_glTexCoordP2ui_ptr == nullptr) _glTexCoordP2ui_ptr = (PFNGLTEXCOORDP2UIPROC)a2e_null_func<365>;
	if(_glTexCoordP2uiv_ptr == nullptr) _glTexCoordP2uiv_ptr = (PFNGLTEXCOORDP2UIVPROC)a2e_null_func<366>;
	if(_glTexCoordP3ui_ptr == nullptr) _glTexCoordP3ui_ptr = (PFNGLTEXCOORDP3UIPROC)a2e_null_func<367>;
	if(_glTexCoordP3uiv_ptr == nullptr) _glTexCoordP3uiv_ptr = (PFNGLTEXCOORDP3UIVPROC)a2e_null_func<368>;
	if(_glTexCoordP4ui_ptr == nullptr) _glTexCoordP4ui_ptr = (PFNGLTEXCOORDP4UIPROC)a2e_null_func<369>;
	if(_glTexCoordP4uiv_ptr == nullptr) _glTexCoordP4uiv_ptr = (PFNGLTEXCOORDP4UIVPROC)a2e_null_func<370>;
	if(_glMultiTexCoordP1ui_ptr == nullptr) _glMultiTexCoordP1ui_ptr = (PFNGLMULTITEXCOORDP1UIPROC)a2e_null_func<371>;
	if(_glMultiTexCoordP1uiv_ptr == nullptr) _glMultiTexCoordP1uiv_ptr = (PFNGLMULTITEXCOORDP1UIVPROC)a2e_null_func<372>;
	if(_glMultiTexCoordP2ui_ptr == nullptr) _glMultiTexCoordP2ui_ptr = (PFNGLMULTITEXCOORDP2UIPROC)a2e_null_func<373>;
	if(_glMultiTexCoordP2uiv_ptr == nullptr) _glMultiTexCoordP2uiv_ptr = (PFNGLMULTITEXCOORDP2UIVPROC)a2e_null_func<374>;
	if(_glMultiTexCoordP3ui_ptr == nullptr) _glMultiTexCoordP3ui_ptr = (PFNGLMULTITEXCOORDP3UIPROC)a2e_null_func<375>;
	if(_glMultiTexCoordP3uiv_ptr == nullptr) _glMultiTexCoordP3uiv_ptr = (PFNGLMULTITEXCOORDP3UIVPROC)a2e_null_func<376>;
	if(_glMultiTexCoordP4ui_ptr == nullptr) _glMultiTexCoordP4ui_ptr = (PFNGLMULTITEXCOORDP4UIPROC)a2e_null_func<377>;
	if(_glMultiTexCoordP4uiv_ptr == nullptr) _glMultiTexCoordP4uiv_ptr = (PFNGLMULTITEXCOORDP4UIVPROC)a2e_null_func<378>;
	if(_glNormalP3ui_ptr == nullptr) _glNormalP3ui_ptr = (PFNGLNORMALP3UIPROC)a2e_null_func<379>;
	if(_glNormalP3uiv_ptr == nullptr) _glNormalP3uiv_ptr = (PFNGLNORMALP3UIVPROC)a2e_null_func<380>;
	if(_glColorP3ui_ptr == nullptr) _glColorP3ui_ptr = (PFNGLCOLORP3UIPROC)a2e_null_func<381>;
	if(_glColorP3uiv_ptr == nullptr) _glColorP3uiv_ptr = (PFNGLCOLORP3UIVPROC)a2e_null_func<382>;
	if(_glColorP4ui_ptr == nullptr) _glColorP4ui_ptr = (PFNGLCOLORP4UIPROC)a2e_null_func<383>;
	if(_glColorP4uiv_ptr == nullptr) _glColorP4uiv_ptr = (PFNGLCOLORP4UIVPROC)a2e_null_func<384>;
	if(_glSecondaryColorP3ui_ptr == nullptr) _glSecondaryColorP3ui_ptr = (PFNGLSECONDARYCOLORP3UIPROC)a2e_null_func<385>;
	if(_glSecondaryColorP3uiv_ptr == nullptr) _glSecondaryColorP3uiv_ptr = (PFNGLSECONDARYCOLORP3UIVPROC)a2e_null_func<386>;
	if(_glVertexAttribP1ui_ptr == nullptr) _glVertexAttribP1ui_ptr = (PFNGLVERTEXATTRIBP1UIPROC)a2e_null_func<387>;
	if(_glVertexAttribP1uiv_ptr == nullptr) _glVertexAttribP1uiv_ptr = (PFNGLVERTEXATTRIBP1UIVPROC)a2e_null_func<388>;
	if(_glVertexAttribP2ui_ptr == nullptr) _glVertexAttribP2ui_ptr = (PFNGLVERTEXATTRIBP2UIPROC)a2e_null_func<389>;
	if(_glVertexAttribP2uiv_ptr == nullptr) _glVertexAttribP2uiv_ptr = (PFNGLVERTEXATTRIBP2UIVPROC)a2e_null_func<390>;
	if(_glVertexAttribP3ui_ptr == nullptr) _glVertexAttribP3ui_ptr = (PFNGLVERTEXATTRIBP3UIPROC)a2e_null_func<391>;
	if(_glVertexAttribP3uiv_ptr == nullptr) _glVertexAttribP3uiv_ptr = (PFNGLVERTEXATTRIBP3UIVPROC)a2e_null_func<392>;
	if(_glVertexAttribP4ui_ptr == nullptr) _glVertexAttribP4ui_ptr = (PFNGLVERTEXATTRIBP4UIPROC)a2e_null_func<393>;
	if(_glVertexAttribP4uiv_ptr == nullptr) _glVertexAttribP4uiv_ptr = (PFNGLVERTEXATTRIBP4UIVPROC)a2e_null_func<394>;
	if(_glMinSampleShading_ptr == nullptr) _glMinSampleShading_ptr = (PFNGLMINSAMPLESHADINGPROC)a2e_null_func<395>;
	if(_glBlendEquationi_ptr == nullptr) _glBlendEquationi_ptr = (PFNGLBLENDEQUATIONIPROC)a2e_null_func<396>;
	if(_glBlendEquationSeparatei_ptr == nullptr) _glBlendEquationSeparatei_ptr = (PFNGLBLENDEQUATIONSEPARATEIPROC)a2e_null_func<397>;
	if(_glBlendFunci_ptr == nullptr) _glBlendFunci_ptr = (PFNGLBLENDFUNCIPROC)a2e_null_func<398>;
	if(_glBlendFuncSeparatei_ptr == nullptr) _glBlendFuncSeparatei_ptr = (PFNGLBLENDFUNCSEPARATEIPROC)a2e_null_func<399>;
	if(_glDrawArraysIndirect_ptr == nullptr) _glDrawArraysIndirect_ptr = (PFNGLDRAWARRAYSINDIRECTPROC)a2e_null_func<400>;
	if(_glDrawElementsIndirect_ptr == nullptr) _glDrawElementsIndirect_ptr = (PFNGLDRAWELEMENTSINDIRECTPROC)a2e_null_func<401>;
	if(_glUniform1d_ptr == nullptr) _glUniform1d_ptr = (PFNGLUNIFORM1DPROC)a2e_null_func<402>;
	if(_glUniform2d_ptr == nullptr) _glUniform2d_ptr = (PFNGLUNIFORM2DPROC)a2e_null_func<403>;
	if(_glUniform3d_ptr == nullptr) _glUniform3d_ptr = (PFNGLUNIFORM3DPROC)a2e_null_func<404>;
	if(_glUniform4d_ptr == nullptr) _glUniform4d_ptr = (PFNGLUNIFORM4DPROC)a2e_null_func<405>;
	if(_glUniform1dv_ptr == nullptr) _glUniform1dv_ptr = (PFNGLUNIFORM1DVPROC)a2e_null_func<406>;
	if(_glUniform2dv_ptr == nullptr) _glUniform2dv_ptr = (PFNGLUNIFORM2DVPROC)a2e_null_func<407>;
	if(_glUniform3dv_ptr == nullptr) _glUniform3dv_ptr = (PFNGLUNIFORM3DVPROC)a2e_null_func<408>;
	if(_glUniform4dv_ptr == nullptr) _glUniform4dv_ptr = (PFNGLUNIFORM4DVPROC)a2e_null_func<409>;
	if(_glUniformMatrix2dv_ptr == nullptr) _glUniformMatrix2dv_ptr = (PFNGLUNIFORMMATRIX2DVPROC)a2e_null_func<410>;
	if(_glUniformMatrix3dv_ptr == nullptr) _glUniformMatrix3dv_ptr = (PFNGLUNIFORMMATRIX3DVPROC)a2e_null_func<411>;
	if(_glUniformMatrix4dv_ptr == nullptr) _glUniformMatrix4dv_ptr = (PFNGLUNIFORMMATRIX4DVPROC)a2e_null_func<412>;
	if(_glUniformMatrix2x3dv_ptr == nullptr) _glUniformMatrix2x3dv_ptr = (PFNGLUNIFORMMATRIX2X3DVPROC)a2e_null_func<413>;
	if(_glUniformMatrix2x4dv_ptr == nullptr) _glUniformMatrix2x4dv_ptr = (PFNGLUNIFORMMATRIX2X4DVPROC)a2e_null_func<414>;
	if(_glUniformMatrix3x2dv_ptr == nullptr) _glUniformMatrix3x2dv_ptr = (PFNGLUNIFORMMATRIX3X2DVPROC)a2e_null_func<415>;
	if(_glUniformMatrix3x4dv_ptr == nullptr) _glUniformMatrix3x4dv_ptr = (PFNGLUNIFORMMATRIX3X4DVPROC)a2e_null_func<416>;
	if(_glUniformMatrix4x2dv_ptr == nullptr) _glUniformMatrix4x2dv_ptr = (PFNGLUNIFORMMATRIX4X2DVPROC)a2e_null_func<417>;
	if(_glUniformMatrix4x3dv_ptr == nullptr) _glUniformMatrix4x3dv_ptr = (PFNGLUNIFORMMATRIX4X3DVPROC)a2e_null_func<418>;
	if(_glGetUniformdv_ptr == nullptr) _glGetUniformdv_ptr = (PFNGLGETUNIFORMDVPROC)a2e_null_func<419>;
	if(_glGetSubroutineUniformLocation_ptr == nullptr) _glGetSubroutineUniformLocation_ptr = (PFNGLGETSUBROUTINEUNIFORMLOCATIONPROC)a2e_null_func<420>;
	if(_glGetSubroutineIndex_ptr == nullptr) _glGetSubroutineIndex_ptr = (PFNGLGETSUBROUTINEINDEXPROC)a2e_null_func<421>;
	if(_glGetActiveSubroutineUniformiv_ptr == nullptr) _glGetActiveSubroutineUniformiv_ptr = (PFNGLGETACTIVESUBROUTINEUNIFORMIVPROC)a2e_null_func<422>;
	if(_glGetActiveSubroutineUniformName_ptr == nullptr) _glGetActiveSubroutineUniformName_ptr = (PFNGLGETACTIVESUBROUTINEUNIFORMNAMEPROC)a2e_null_func<423>;
	if(_glGetActiveSubroutineName_ptr == nullptr) _glGetActiveSubroutineName_ptr = (PFNGLGETACTIVESUBROUTINENAMEPROC)a2e_null_func<424>;
	if(_glUniformSubroutinesuiv_ptr == nullptr) _glUniformSubroutinesuiv_ptr = (PFNGLUNIFORMSUBROUTINESUIVPROC)a2e_null_func<425>;
	if(_glGetUniformSubroutineuiv_ptr == nullptr) _glGetUniformSubroutineuiv_ptr = (PFNGLGETUNIFORMSUBROUTINEUIVPROC)a2e_null_func<426>;
	if(_glGetProgramStageiv_ptr == nullptr) _glGetProgramStageiv_ptr = (PFNGLGETPROGRAMSTAGEIVPROC)a2e_null_func<427>;
	if(_glPatchParameteri_ptr == nullptr) _glPatchParameteri_ptr = (PFNGLPATCHPARAMETERIPROC)a2e_null_func<428>;
	if(_glPatchParameterfv_ptr == nullptr) _glPatchParameterfv_ptr = (PFNGLPATCHPARAMETERFVPROC)a2e_null_func<429>;
	if(_glBindTransformFeedback_ptr == nullptr) _glBindTransformFeedback_ptr = (PFNGLBINDTRANSFORMFEEDBACKPROC)a2e_null_func<430>;
	if(_glDeleteTransformFeedbacks_ptr == nullptr) _glDeleteTransformFeedbacks_ptr = (PFNGLDELETETRANSFORMFEEDBACKSPROC)a2e_null_func<431>;
	if(_glGenTransformFeedbacks_ptr == nullptr) _glGenTransformFeedbacks_ptr = (PFNGLGENTRANSFORMFEEDBACKSPROC)a2e_null_func<432>;
	if(_glIsTransformFeedback_ptr == nullptr) _glIsTransformFeedback_ptr = (PFNGLISTRANSFORMFEEDBACKPROC)a2e_null_func<433>;
	if(_glPauseTransformFeedback_ptr == nullptr) _glPauseTransformFeedback_ptr = (PFNGLPAUSETRANSFORMFEEDBACKPROC)a2e_null_func<434>;
	if(_glResumeTransformFeedback_ptr == nullptr) _glResumeTransformFeedback_ptr = (PFNGLRESUMETRANSFORMFEEDBACKPROC)a2e_null_func<435>;
	if(_glDrawTransformFeedback_ptr == nullptr) _glDrawTransformFeedback_ptr = (PFNGLDRAWTRANSFORMFEEDBACKPROC)a2e_null_func<436>;
	if(_glDrawTransformFeedbackStream_ptr == nullptr) _glDrawTransformFeedbackStream_ptr = (PFNGLDRAWTRANSFORMFEEDBACKSTREAMPROC)a2e_null_func<437>;
	if(_glBeginQueryIndexed_ptr == nullptr) _glBeginQueryIndexed_ptr = (PFNGLBEGINQUERYINDEXEDPROC)a2e_null_func<438>;
	if(_glEndQueryIndexed_ptr == nullptr) _glEndQueryIndexed_ptr = (PFNGLENDQUERYINDEXEDPROC)a2e_null_func<439>;
	if(_glGetQueryIndexediv_ptr == nullptr) _glGetQueryIndexediv_ptr = (PFNGLGETQUERYINDEXEDIVPROC)a2e_null_func<440>;
	if(_glReleaseShaderCompiler_ptr == nullptr) _glReleaseShaderCompiler_ptr = (PFNGLRELEASESHADERCOMPILERPROC)a2e_null_func<441>;
	if(_glShaderBinary_ptr == nullptr) _glShaderBinary_ptr = (PFNGLSHADERBINARYPROC)a2e_null_func<442>;
	if(_glGetShaderPrecisionFormat_ptr == nullptr) _glGetShaderPrecisionFormat_ptr = (PFNGLGETSHADERPRECISIONFORMATPROC)a2e_null_func<443>;
	if(_glDepthRangef_ptr == nullptr) _glDepthRangef_ptr = (PFNGLDEPTHRANGEFPROC)a2e_null_func<444>;
	if(_glClearDepthf_ptr == nullptr) _glClearDepthf_ptr = (PFNGLCLEARDEPTHFPROC)a2e_null_func<445>;
	if(_glGetProgramBinary_ptr == nullptr) _glGetProgramBinary_ptr = (PFNGLGETPROGRAMBINARYPROC)a2e_null_func<446>;
	if(_glProgramBinary_ptr == nullptr) _glProgramBinary_ptr = (PFNGLPROGRAMBINARYPROC)a2e_null_func<447>;
	if(_glProgramParameteri_ptr == nullptr) {
		if(_glProgramParameteriEXT_ptr == nullptr) _glProgramParameteri_ptr = (PFNGLPROGRAMPARAMETERIPROC)a2e_null_func<448>;
		else _glProgramParameteri_ptr = (PFNGLPROGRAMPARAMETERIPROC)_glProgramParameteriEXT_ptr;
	}
	if(_glUseProgramStages_ptr == nullptr) _glUseProgramStages_ptr = (PFNGLUSEPROGRAMSTAGESPROC)a2e_null_func<449>;
	if(_glActiveShaderProgram_ptr == nullptr) _glActiveShaderProgram_ptr = (PFNGLACTIVESHADERPROGRAMPROC)a2e_null_func<450>;
	if(_glCreateShaderProgramv_ptr == nullptr) _glCreateShaderProgramv_ptr = (PFNGLCREATESHADERPROGRAMVPROC)a2e_null_func<451>;
	if(_glBindProgramPipeline_ptr == nullptr) _glBindProgramPipeline_ptr = (PFNGLBINDPROGRAMPIPELINEPROC)a2e_null_func<452>;
	if(_glDeleteProgramPipelines_ptr == nullptr) _glDeleteProgramPipelines_ptr = (PFNGLDELETEPROGRAMPIPELINESPROC)a2e_null_func<453>;
	if(_glGenProgramPipelines_ptr == nullptr) _glGenProgramPipelines_ptr = (PFNGLGENPROGRAMPIPELINESPROC)a2e_null_func<454>;
	if(_glIsProgramPipeline_ptr == nullptr) _glIsProgramPipeline_ptr = (PFNGLISPROGRAMPIPELINEPROC)a2e_null_func<455>;
	if(_glGetProgramPipelineiv_ptr == nullptr) _glGetProgramPipelineiv_ptr = (PFNGLGETPROGRAMPIPELINEIVPROC)a2e_null_func<456>;
	if(_glProgramUniform1i_ptr == nullptr) _glProgramUniform1i_ptr = (PFNGLPROGRAMUNIFORM1IPROC)a2e_null_func<457>;
	if(_glProgramUniform1iv_ptr == nullptr) _glProgramUniform1iv_ptr = (PFNGLPROGRAMUNIFORM1IVPROC)a2e_null_func<458>;
	if(_glProgramUniform1f_ptr == nullptr) _glProgramUniform1f_ptr = (PFNGLPROGRAMUNIFORM1FPROC)a2e_null_func<459>;
	if(_glProgramUniform1fv_ptr == nullptr) _glProgramUniform1fv_ptr = (PFNGLPROGRAMUNIFORM1FVPROC)a2e_null_func<460>;
	if(_glProgramUniform1d_ptr == nullptr) _glProgramUniform1d_ptr = (PFNGLPROGRAMUNIFORM1DPROC)a2e_null_func<461>;
	if(_glProgramUniform1dv_ptr == nullptr) _glProgramUniform1dv_ptr = (PFNGLPROGRAMUNIFORM1DVPROC)a2e_null_func<462>;
	if(_glProgramUniform1ui_ptr == nullptr) _glProgramUniform1ui_ptr = (PFNGLPROGRAMUNIFORM1UIPROC)a2e_null_func<463>;
	if(_glProgramUniform1uiv_ptr == nullptr) _glProgramUniform1uiv_ptr = (PFNGLPROGRAMUNIFORM1UIVPROC)a2e_null_func<464>;
	if(_glProgramUniform2i_ptr == nullptr) _glProgramUniform2i_ptr = (PFNGLPROGRAMUNIFORM2IPROC)a2e_null_func<465>;
	if(_glProgramUniform2iv_ptr == nullptr) _glProgramUniform2iv_ptr = (PFNGLPROGRAMUNIFORM2IVPROC)a2e_null_func<466>;
	if(_glProgramUniform2f_ptr == nullptr) _glProgramUniform2f_ptr = (PFNGLPROGRAMUNIFORM2FPROC)a2e_null_func<467>;
	if(_glProgramUniform2fv_ptr == nullptr) _glProgramUniform2fv_ptr = (PFNGLPROGRAMUNIFORM2FVPROC)a2e_null_func<468>;
	if(_glProgramUniform2d_ptr == nullptr) _glProgramUniform2d_ptr = (PFNGLPROGRAMUNIFORM2DPROC)a2e_null_func<469>;
	if(_glProgramUniform2dv_ptr == nullptr) _glProgramUniform2dv_ptr = (PFNGLPROGRAMUNIFORM2DVPROC)a2e_null_func<470>;
	if(_glProgramUniform2ui_ptr == nullptr) _glProgramUniform2ui_ptr = (PFNGLPROGRAMUNIFORM2UIPROC)a2e_null_func<471>;
	if(_glProgramUniform2uiv_ptr == nullptr) _glProgramUniform2uiv_ptr = (PFNGLPROGRAMUNIFORM2UIVPROC)a2e_null_func<472>;
	if(_glProgramUniform3i_ptr == nullptr) _glProgramUniform3i_ptr = (PFNGLPROGRAMUNIFORM3IPROC)a2e_null_func<473>;
	if(_glProgramUniform3iv_ptr == nullptr) _glProgramUniform3iv_ptr = (PFNGLPROGRAMUNIFORM3IVPROC)a2e_null_func<474>;
	if(_glProgramUniform3f_ptr == nullptr) _glProgramUniform3f_ptr = (PFNGLPROGRAMUNIFORM3FPROC)a2e_null_func<475>;
	if(_glProgramUniform3fv_ptr == nullptr) _glProgramUniform3fv_ptr = (PFNGLPROGRAMUNIFORM3FVPROC)a2e_null_func<476>;
	if(_glProgramUniform3d_ptr == nullptr) _glProgramUniform3d_ptr = (PFNGLPROGRAMUNIFORM3DPROC)a2e_null_func<477>;
	if(_glProgramUniform3dv_ptr == nullptr) _glProgramUniform3dv_ptr = (PFNGLPROGRAMUNIFORM3DVPROC)a2e_null_func<478>;
	if(_glProgramUniform3ui_ptr == nullptr) _glProgramUniform3ui_ptr = (PFNGLPROGRAMUNIFORM3UIPROC)a2e_null_func<479>;
	if(_glProgramUniform3uiv_ptr == nullptr) _glProgramUniform3uiv_ptr = (PFNGLPROGRAMUNIFORM3UIVPROC)a2e_null_func<480>;
	if(_glProgramUniform4i_ptr == nullptr) _glProgramUniform4i_ptr = (PFNGLPROGRAMUNIFORM4IPROC)a2e_null_func<481>;
	if(_glProgramUniform4iv_ptr == nullptr) _glProgramUniform4iv_ptr = (PFNGLPROGRAMUNIFORM4IVPROC)a2e_null_func<482>;
	if(_glProgramUniform4f_ptr == nullptr) _glProgramUniform4f_ptr = (PFNGLPROGRAMUNIFORM4FPROC)a2e_null_func<483>;
	if(_glProgramUniform4fv_ptr == nullptr) _glProgramUniform4fv_ptr = (PFNGLPROGRAMUNIFORM4FVPROC)a2e_null_func<484>;
	if(_glProgramUniform4d_ptr == nullptr) _glProgramUniform4d_ptr = (PFNGLPROGRAMUNIFORM4DPROC)a2e_null_func<485>;
	if(_glProgramUniform4dv_ptr == nullptr) _glProgramUniform4dv_ptr = (PFNGLPROGRAMUNIFORM4DVPROC)a2e_null_func<486>;
	if(_glProgramUniform4ui_ptr == nullptr) _glProgramUniform4ui_ptr = (PFNGLPROGRAMUNIFORM4UIPROC)a2e_null_func<487>;
	if(_glProgramUniform4uiv_ptr == nullptr) _glProgramUniform4uiv_ptr = (PFNGLPROGRAMUNIFORM4UIVPROC)a2e_null_func<488>;
	if(_glProgramUniformMatrix2fv_ptr == nullptr) _glProgramUniformMatrix2fv_ptr = (PFNGLPROGRAMUNIFORMMATRIX2FVPROC)a2e_null_func<489>;
	if(_glProgramUniformMatrix3fv_ptr == nullptr) _glProgramUniformMatrix3fv_ptr = (PFNGLPROGRAMUNIFORMMATRIX3FVPROC)a2e_null_func<490>;
	if(_glProgramUniformMatrix4fv_ptr == nullptr) _glProgramUniformMatrix4fv_ptr = (PFNGLPROGRAMUNIFORMMATRIX4FVPROC)a2e_null_func<491>;
	if(_glProgramUniformMatrix2dv_ptr == nullptr) _glProgramUniformMatrix2dv_ptr = (PFNGLPROGRAMUNIFORMMATRIX2DVPROC)a2e_null_func<492>;
	if(_glProgramUniformMatrix3dv_ptr == nullptr) _glProgramUniformMatrix3dv_ptr = (PFNGLPROGRAMUNIFORMMATRIX3DVPROC)a2e_null_func<493>;
	if(_glProgramUniformMatrix4dv_ptr == nullptr) _glProgramUniformMatrix4dv_ptr = (PFNGLPROGRAMUNIFORMMATRIX4DVPROC)a2e_null_func<494>;
	if(_glProgramUniformMatrix2x3fv_ptr == nullptr) _glProgramUniformMatrix2x3fv_ptr = (PFNGLPROGRAMUNIFORMMATRIX2X3FVPROC)a2e_null_func<495>;
	if(_glProgramUniformMatrix3x2fv_ptr == nullptr) _glProgramUniformMatrix3x2fv_ptr = (PFNGLPROGRAMUNIFORMMATRIX3X2FVPROC)a2e_null_func<496>;
	if(_glProgramUniformMatrix2x4fv_ptr == nullptr) _glProgramUniformMatrix2x4fv_ptr = (PFNGLPROGRAMUNIFORMMATRIX2X4FVPROC)a2e_null_func<497>;
	if(_glProgramUniformMatrix4x2fv_ptr == nullptr) _glProgramUniformMatrix4x2fv_ptr = (PFNGLPROGRAMUNIFORMMATRIX4X2FVPROC)a2e_null_func<498>;
	if(_glProgramUniformMatrix3x4fv_ptr == nullptr) _glProgramUniformMatrix3x4fv_ptr = (PFNGLPROGRAMUNIFORMMATRIX3X4FVPROC)a2e_null_func<499>;
	if(_glProgramUniformMatrix4x3fv_ptr == nullptr) _glProgramUniformMatrix4x3fv_ptr = (PFNGLPROGRAMUNIFORMMATRIX4X3FVPROC)a2e_null_func<500>;
	if(_glProgramUniformMatrix2x3dv_ptr == nullptr) _glProgramUniformMatrix2x3dv_ptr = (PFNGLPROGRAMUNIFORMMATRIX2X3DVPROC)a2e_null_func<501>;
	if(_glProgramUniformMatrix3x2dv_ptr == nullptr) _glProgramUniformMatrix3x2dv_ptr = (PFNGLPROGRAMUNIFORMMATRIX3X2DVPROC)a2e_null_func<502>;
	if(_glProgramUniformMatrix2x4dv_ptr == nullptr) _glProgramUniformMatrix2x4dv_ptr = (PFNGLPROGRAMUNIFORMMATRIX2X4DVPROC)a2e_null_func<503>;
	if(_glProgramUniformMatrix4x2dv_ptr == nullptr) _glProgramUniformMatrix4x2dv_ptr = (PFNGLPROGRAMUNIFORMMATRIX4X2DVPROC)a2e_null_func<504>;
	if(_glProgramUniformMatrix3x4dv_ptr == nullptr) _glProgramUniformMatrix3x4dv_ptr = (PFNGLPROGRAMUNIFORMMATRIX3X4DVPROC)a2e_null_func<505>;
	if(_glProgramUniformMatrix4x3dv_ptr == nullptr) _glProgramUniformMatrix4x3dv_ptr = (PFNGLPROGRAMUNIFORMMATRIX4X3DVPROC)a2e_null_func<506>;
	if(_glValidateProgramPipeline_ptr == nullptr) _glValidateProgramPipeline_ptr = (PFNGLVALIDATEPROGRAMPIPELINEPROC)a2e_null_func<507>;
	if(_glGetProgramPipelineInfoLog_ptr == nullptr) _glGetProgramPipelineInfoLog_ptr = (PFNGLGETPROGRAMPIPELINEINFOLOGPROC)a2e_null_func<508>;
	if(_glVertexAttribL1d_ptr == nullptr) _glVertexAttribL1d_ptr = (PFNGLVERTEXATTRIBL1DPROC)a2e_null_func<509>;
	if(_glVertexAttribL2d_ptr == nullptr) _glVertexAttribL2d_ptr = (PFNGLVERTEXATTRIBL2DPROC)a2e_null_func<510>;
	if(_glVertexAttribL3d_ptr == nullptr) _glVertexAttribL3d_ptr = (PFNGLVERTEXATTRIBL3DPROC)a2e_null_func<511>;
	if(_glVertexAttribL4d_ptr == nullptr) _glVertexAttribL4d_ptr = (PFNGLVERTEXATTRIBL4DPROC)a2e_null_func<512>;
	if(_glVertexAttribL1dv_ptr == nullptr) _glVertexAttribL1dv_ptr = (PFNGLVERTEXATTRIBL1DVPROC)a2e_null_func<513>;
	if(_glVertexAttribL2dv_ptr == nullptr) _glVertexAttribL2dv_ptr = (PFNGLVERTEXATTRIBL2DVPROC)a2e_null_func<514>;
	if(_glVertexAttribL3dv_ptr == nullptr) _glVertexAttribL3dv_ptr = (PFNGLVERTEXATTRIBL3DVPROC)a2e_null_func<515>;
	if(_glVertexAttribL4dv_ptr == nullptr) _glVertexAttribL4dv_ptr = (PFNGLVERTEXATTRIBL4DVPROC)a2e_null_func<516>;
	if(_glVertexAttribLPointer_ptr == nullptr) _glVertexAttribLPointer_ptr = (PFNGLVERTEXATTRIBLPOINTERPROC)a2e_null_func<517>;
	if(_glGetVertexAttribLdv_ptr == nullptr) _glGetVertexAttribLdv_ptr = (PFNGLGETVERTEXATTRIBLDVPROC)a2e_null_func<518>;
	if(_glViewportArrayv_ptr == nullptr) _glViewportArrayv_ptr = (PFNGLVIEWPORTARRAYVPROC)a2e_null_func<519>;
	if(_glViewportIndexedf_ptr == nullptr) _glViewportIndexedf_ptr = (PFNGLVIEWPORTINDEXEDFPROC)a2e_null_func<520>;
	if(_glViewportIndexedfv_ptr == nullptr) _glViewportIndexedfv_ptr = (PFNGLVIEWPORTINDEXEDFVPROC)a2e_null_func<521>;
	if(_glScissorArrayv_ptr == nullptr) _glScissorArrayv_ptr = (PFNGLSCISSORARRAYVPROC)a2e_null_func<522>;
	if(_glScissorIndexed_ptr == nullptr) _glScissorIndexed_ptr = (PFNGLSCISSORINDEXEDPROC)a2e_null_func<523>;
	if(_glScissorIndexedv_ptr == nullptr) _glScissorIndexedv_ptr = (PFNGLSCISSORINDEXEDVPROC)a2e_null_func<524>;
	if(_glDepthRangeArrayv_ptr == nullptr) _glDepthRangeArrayv_ptr = (PFNGLDEPTHRANGEARRAYVPROC)a2e_null_func<525>;
	if(_glDepthRangeIndexed_ptr == nullptr) _glDepthRangeIndexed_ptr = (PFNGLDEPTHRANGEINDEXEDPROC)a2e_null_func<526>;
	if(_glGetFloati_v_ptr == nullptr) _glGetFloati_v_ptr = (PFNGLGETFLOATI_VPROC)a2e_null_func<527>;
	if(_glGetDoublei_v_ptr == nullptr) _glGetDoublei_v_ptr = (PFNGLGETDOUBLEI_VPROC)a2e_null_func<528>;
#endif // !__LINUX__ || !WIN_UNIXENV
#endif // !__APPLE__
}

// gl funcs init, get proc address
void ext::init_gl_funcs() {
#ifndef __APPLE__
	_glRenderbufferStorageMultisampleCoverageNV_ptr = (PFNGLRENDERBUFFERSTORAGEMULTISAMPLECOVERAGENVPROC)glGetProcAddress((ProcType)"glRenderbufferStorageMultisampleCoverageNV"); // NV_framebuffer_multisample_coverage
	
#if !defined(__LINUX__)
	// OpenGL 1.3
	_glActiveTexture_ptr = (PFNGLACTIVETEXTUREPROC)glGetProcAddress((ProcType)"glActiveTexture");
	_glSampleCoverage_ptr = (PFNGLSAMPLECOVERAGEPROC)glGetProcAddress((ProcType)"glSampleCoverage");
	_glCompressedTexImage3D_ptr = (PFNGLCOMPRESSEDTEXIMAGE3DPROC)glGetProcAddress((ProcType)"glCompressedTexImage3D");
	_glCompressedTexImage2D_ptr = (PFNGLCOMPRESSEDTEXIMAGE2DPROC)glGetProcAddress((ProcType)"glCompressedTexImage2D");
	_glCompressedTexImage1D_ptr = (PFNGLCOMPRESSEDTEXIMAGE1DPROC)glGetProcAddress((ProcType)"glCompressedTexImage1D");
	_glCompressedTexSubImage3D_ptr = (PFNGLCOMPRESSEDTEXSUBIMAGE3DPROC)glGetProcAddress((ProcType)"glCompressedTexSubImage3D");
	_glCompressedTexSubImage2D_ptr = (PFNGLCOMPRESSEDTEXSUBIMAGE2DPROC)glGetProcAddress((ProcType)"glCompressedTexSubImage2D");
	_glCompressedTexSubImage1D_ptr = (PFNGLCOMPRESSEDTEXSUBIMAGE1DPROC)glGetProcAddress((ProcType)"glCompressedTexSubImage1D");
	_glGetCompressedTexImage_ptr = (PFNGLGETCOMPRESSEDTEXIMAGEPROC)glGetProcAddress((ProcType)"glGetCompressedTexImage");
#endif

	// OpenGL 1.4
	_glBlendFuncSeparate_ptr = (PFNGLBLENDFUNCSEPARATEPROC)glGetProcAddress((ProcType)"glBlendFuncSeparate");
	_glMultiDrawArrays_ptr = (PFNGLMULTIDRAWARRAYSPROC)glGetProcAddress((ProcType)"glMultiDrawArrays");
	_glMultiDrawElements_ptr = (PFNGLMULTIDRAWELEMENTSPROC)glGetProcAddress((ProcType)"glMultiDrawElements");
	_glPointParameterf_ptr = (PFNGLPOINTPARAMETERFPROC)glGetProcAddress((ProcType)"glPointParameterf");
	_glPointParameterfv_ptr = (PFNGLPOINTPARAMETERFVPROC)glGetProcAddress((ProcType)"glPointParameterfv");
	_glPointParameteri_ptr = (PFNGLPOINTPARAMETERIPROC)glGetProcAddress((ProcType)"glPointParameteri");
	_glPointParameteriv_ptr = (PFNGLPOINTPARAMETERIVPROC)glGetProcAddress((ProcType)"glPointParameteriv");

	// OpenGL 1.5
	_glGenQueries_ptr = (PFNGLGENQUERIESPROC)glGetProcAddress((ProcType)"glGenQueries");
	_glDeleteQueries_ptr = (PFNGLDELETEQUERIESPROC)glGetProcAddress((ProcType)"glDeleteQueries");
	_glIsQuery_ptr = (PFNGLISQUERYPROC)glGetProcAddress((ProcType)"glIsQuery");
	_glBeginQuery_ptr = (PFNGLBEGINQUERYPROC)glGetProcAddress((ProcType)"glBeginQuery");
	_glEndQuery_ptr = (PFNGLENDQUERYPROC)glGetProcAddress((ProcType)"glEndQuery");
	_glGetQueryiv_ptr = (PFNGLGETQUERYIVPROC)glGetProcAddress((ProcType)"glGetQueryiv");
	_glGetQueryObjectiv_ptr = (PFNGLGETQUERYOBJECTIVPROC)glGetProcAddress((ProcType)"glGetQueryObjectiv");
	_glGetQueryObjectuiv_ptr = (PFNGLGETQUERYOBJECTUIVPROC)glGetProcAddress((ProcType)"glGetQueryObjectuiv");
	_glBindBuffer_ptr = (PFNGLBINDBUFFERPROC)glGetProcAddress((ProcType)"glBindBuffer");
	_glDeleteBuffers_ptr = (PFNGLDELETEBUFFERSPROC)glGetProcAddress((ProcType)"glDeleteBuffers");
	_glGenBuffers_ptr = (PFNGLGENBUFFERSPROC)glGetProcAddress((ProcType)"glGenBuffers");
	_glIsBuffer_ptr = (PFNGLISBUFFERPROC)glGetProcAddress((ProcType)"glIsBuffer");
	_glBufferData_ptr = (PFNGLBUFFERDATAPROC)glGetProcAddress((ProcType)"glBufferData");
	_glBufferSubData_ptr = (PFNGLBUFFERSUBDATAPROC)glGetProcAddress((ProcType)"glBufferSubData");
	_glGetBufferSubData_ptr = (PFNGLGETBUFFERSUBDATAPROC)glGetProcAddress((ProcType)"glGetBufferSubData");
	_glMapBuffer_ptr = (PFNGLMAPBUFFERPROC)glGetProcAddress((ProcType)"glMapBuffer");
	_glUnmapBuffer_ptr = (PFNGLUNMAPBUFFERPROC)glGetProcAddress((ProcType)"glUnmapBuffer");
	_glGetBufferParameteriv_ptr = (PFNGLGETBUFFERPARAMETERIVPROC)glGetProcAddress((ProcType)"glGetBufferParameteriv");
	_glGetBufferPointerv_ptr = (PFNGLGETBUFFERPOINTERVPROC)glGetProcAddress((ProcType)"glGetBufferPointerv");

	// OpenGL 2.0
	_glBlendEquationSeparate_ptr = (PFNGLBLENDEQUATIONSEPARATEPROC)glGetProcAddress((ProcType)"glBlendEquationSeparate");
	_glDrawBuffers_ptr = (PFNGLDRAWBUFFERSPROC)glGetProcAddress((ProcType)"glDrawBuffers");
	_glStencilOpSeparate_ptr = (PFNGLSTENCILOPSEPARATEPROC)glGetProcAddress((ProcType)"glStencilOpSeparate");
	_glStencilFuncSeparate_ptr = (PFNGLSTENCILFUNCSEPARATEPROC)glGetProcAddress((ProcType)"glStencilFuncSeparate");
	_glStencilMaskSeparate_ptr = (PFNGLSTENCILMASKSEPARATEPROC)glGetProcAddress((ProcType)"glStencilMaskSeparate");
	_glAttachShader_ptr = (PFNGLATTACHSHADERPROC)glGetProcAddress((ProcType)"glAttachShader");
	_glBindAttribLocation_ptr = (PFNGLBINDATTRIBLOCATIONPROC)glGetProcAddress((ProcType)"glBindAttribLocation");
	_glCompileShader_ptr = (PFNGLCOMPILESHADERPROC)glGetProcAddress((ProcType)"glCompileShader");
	_glCreateProgram_ptr = (PFNGLCREATEPROGRAMPROC)glGetProcAddress((ProcType)"glCreateProgram");
	_glCreateShader_ptr = (PFNGLCREATESHADERPROC)glGetProcAddress((ProcType)"glCreateShader");
	_glDeleteProgram_ptr = (PFNGLDELETEPROGRAMPROC)glGetProcAddress((ProcType)"glDeleteProgram");
	_glDeleteShader_ptr = (PFNGLDELETESHADERPROC)glGetProcAddress((ProcType)"glDeleteShader");
	_glDetachShader_ptr = (PFNGLDETACHSHADERPROC)glGetProcAddress((ProcType)"glDetachShader");
	_glDisableVertexAttribArray_ptr = (PFNGLDISABLEVERTEXATTRIBARRAYPROC)glGetProcAddress((ProcType)"glDisableVertexAttribArray");
	_glEnableVertexAttribArray_ptr = (PFNGLENABLEVERTEXATTRIBARRAYPROC)glGetProcAddress((ProcType)"glEnableVertexAttribArray");
	_glGetActiveAttrib_ptr = (PFNGLGETACTIVEATTRIBPROC)glGetProcAddress((ProcType)"glGetActiveAttrib");
	_glGetActiveUniform_ptr = (PFNGLGETACTIVEUNIFORMPROC)glGetProcAddress((ProcType)"glGetActiveUniform");
	_glGetAttachedShaders_ptr = (PFNGLGETATTACHEDSHADERSPROC)glGetProcAddress((ProcType)"glGetAttachedShaders");
	_glGetAttribLocation_ptr = (PFNGLGETATTRIBLOCATIONPROC)glGetProcAddress((ProcType)"glGetAttribLocation");
	_glGetProgramiv_ptr = (PFNGLGETPROGRAMIVPROC)glGetProcAddress((ProcType)"glGetProgramiv");
	_glGetProgramInfoLog_ptr = (PFNGLGETPROGRAMINFOLOGPROC)glGetProcAddress((ProcType)"glGetProgramInfoLog");
	_glGetShaderiv_ptr = (PFNGLGETSHADERIVPROC)glGetProcAddress((ProcType)"glGetShaderiv");
	_glGetShaderInfoLog_ptr = (PFNGLGETSHADERINFOLOGPROC)glGetProcAddress((ProcType)"glGetShaderInfoLog");
	_glGetShaderSource_ptr = (PFNGLGETSHADERSOURCEPROC)glGetProcAddress((ProcType)"glGetShaderSource");
	_glGetUniformLocation_ptr = (PFNGLGETUNIFORMLOCATIONPROC)glGetProcAddress((ProcType)"glGetUniformLocation");
	_glGetUniformfv_ptr = (PFNGLGETUNIFORMFVPROC)glGetProcAddress((ProcType)"glGetUniformfv");
	_glGetUniformiv_ptr = (PFNGLGETUNIFORMIVPROC)glGetProcAddress((ProcType)"glGetUniformiv");
	_glGetVertexAttribdv_ptr = (PFNGLGETVERTEXATTRIBDVPROC)glGetProcAddress((ProcType)"glGetVertexAttribdv");
	_glGetVertexAttribfv_ptr = (PFNGLGETVERTEXATTRIBFVPROC)glGetProcAddress((ProcType)"glGetVertexAttribfv");
	_glGetVertexAttribiv_ptr = (PFNGLGETVERTEXATTRIBIVPROC)glGetProcAddress((ProcType)"glGetVertexAttribiv");
	_glGetVertexAttribPointerv_ptr = (PFNGLGETVERTEXATTRIBPOINTERVPROC)glGetProcAddress((ProcType)"glGetVertexAttribPointerv");
	_glIsProgram_ptr = (PFNGLISPROGRAMPROC)glGetProcAddress((ProcType)"glIsProgram");
	_glIsShader_ptr = (PFNGLISSHADERPROC)glGetProcAddress((ProcType)"glIsShader");
	_glLinkProgram_ptr = (PFNGLLINKPROGRAMPROC)glGetProcAddress((ProcType)"glLinkProgram");
	_glShaderSource_ptr = (PFNGLSHADERSOURCEPROC)glGetProcAddress((ProcType)"glShaderSource");
	_glUseProgram_ptr = (PFNGLUSEPROGRAMPROC)glGetProcAddress((ProcType)"glUseProgram");
	_glUniform1f_ptr = (PFNGLUNIFORM1FPROC)glGetProcAddress((ProcType)"glUniform1f");
	_glUniform2f_ptr = (PFNGLUNIFORM2FPROC)glGetProcAddress((ProcType)"glUniform2f");
	_glUniform3f_ptr = (PFNGLUNIFORM3FPROC)glGetProcAddress((ProcType)"glUniform3f");
	_glUniform4f_ptr = (PFNGLUNIFORM4FPROC)glGetProcAddress((ProcType)"glUniform4f");
	_glUniform1i_ptr = (PFNGLUNIFORM1IPROC)glGetProcAddress((ProcType)"glUniform1i");
	_glUniform2i_ptr = (PFNGLUNIFORM2IPROC)glGetProcAddress((ProcType)"glUniform2i");
	_glUniform3i_ptr = (PFNGLUNIFORM3IPROC)glGetProcAddress((ProcType)"glUniform3i");
	_glUniform4i_ptr = (PFNGLUNIFORM4IPROC)glGetProcAddress((ProcType)"glUniform4i");
	_glUniform1fv_ptr = (PFNGLUNIFORM1FVPROC)glGetProcAddress((ProcType)"glUniform1fv");
	_glUniform2fv_ptr = (PFNGLUNIFORM2FVPROC)glGetProcAddress((ProcType)"glUniform2fv");
	_glUniform3fv_ptr = (PFNGLUNIFORM3FVPROC)glGetProcAddress((ProcType)"glUniform3fv");
	_glUniform4fv_ptr = (PFNGLUNIFORM4FVPROC)glGetProcAddress((ProcType)"glUniform4fv");
	_glUniform1iv_ptr = (PFNGLUNIFORM1IVPROC)glGetProcAddress((ProcType)"glUniform1iv");
	_glUniform2iv_ptr = (PFNGLUNIFORM2IVPROC)glGetProcAddress((ProcType)"glUniform2iv");
	_glUniform3iv_ptr = (PFNGLUNIFORM3IVPROC)glGetProcAddress((ProcType)"glUniform3iv");
	_glUniform4iv_ptr = (PFNGLUNIFORM4IVPROC)glGetProcAddress((ProcType)"glUniform4iv");
	_glUniformMatrix2fv_ptr = (PFNGLUNIFORMMATRIX2FVPROC)glGetProcAddress((ProcType)"glUniformMatrix2fv");
	_glUniformMatrix3fv_ptr = (PFNGLUNIFORMMATRIX3FVPROC)glGetProcAddress((ProcType)"glUniformMatrix3fv");
	_glUniformMatrix4fv_ptr = (PFNGLUNIFORMMATRIX4FVPROC)glGetProcAddress((ProcType)"glUniformMatrix4fv");
	_glValidateProgram_ptr = (PFNGLVALIDATEPROGRAMPROC)glGetProcAddress((ProcType)"glValidateProgram");
	_glVertexAttrib1d_ptr = (PFNGLVERTEXATTRIB1DPROC)glGetProcAddress((ProcType)"glVertexAttrib1d");
	_glVertexAttrib1dv_ptr = (PFNGLVERTEXATTRIB1DVPROC)glGetProcAddress((ProcType)"glVertexAttrib1dv");
	_glVertexAttrib1f_ptr = (PFNGLVERTEXATTRIB1FPROC)glGetProcAddress((ProcType)"glVertexAttrib1f");
	_glVertexAttrib1fv_ptr = (PFNGLVERTEXATTRIB1FVPROC)glGetProcAddress((ProcType)"glVertexAttrib1fv");
	_glVertexAttrib1s_ptr = (PFNGLVERTEXATTRIB1SPROC)glGetProcAddress((ProcType)"glVertexAttrib1s");
	_glVertexAttrib1sv_ptr = (PFNGLVERTEXATTRIB1SVPROC)glGetProcAddress((ProcType)"glVertexAttrib1sv");
	_glVertexAttrib2d_ptr = (PFNGLVERTEXATTRIB2DPROC)glGetProcAddress((ProcType)"glVertexAttrib2d");
	_glVertexAttrib2dv_ptr = (PFNGLVERTEXATTRIB2DVPROC)glGetProcAddress((ProcType)"glVertexAttrib2dv");
	_glVertexAttrib2f_ptr = (PFNGLVERTEXATTRIB2FPROC)glGetProcAddress((ProcType)"glVertexAttrib2f");
	_glVertexAttrib2fv_ptr = (PFNGLVERTEXATTRIB2FVPROC)glGetProcAddress((ProcType)"glVertexAttrib2fv");
	_glVertexAttrib2s_ptr = (PFNGLVERTEXATTRIB2SPROC)glGetProcAddress((ProcType)"glVertexAttrib2s");
	_glVertexAttrib2sv_ptr = (PFNGLVERTEXATTRIB2SVPROC)glGetProcAddress((ProcType)"glVertexAttrib2sv");
	_glVertexAttrib3d_ptr = (PFNGLVERTEXATTRIB3DPROC)glGetProcAddress((ProcType)"glVertexAttrib3d");
	_glVertexAttrib3dv_ptr = (PFNGLVERTEXATTRIB3DVPROC)glGetProcAddress((ProcType)"glVertexAttrib3dv");
	_glVertexAttrib3f_ptr = (PFNGLVERTEXATTRIB3FPROC)glGetProcAddress((ProcType)"glVertexAttrib3f");
	_glVertexAttrib3fv_ptr = (PFNGLVERTEXATTRIB3FVPROC)glGetProcAddress((ProcType)"glVertexAttrib3fv");
	_glVertexAttrib3s_ptr = (PFNGLVERTEXATTRIB3SPROC)glGetProcAddress((ProcType)"glVertexAttrib3s");
	_glVertexAttrib3sv_ptr = (PFNGLVERTEXATTRIB3SVPROC)glGetProcAddress((ProcType)"glVertexAttrib3sv");
	_glVertexAttrib4Nbv_ptr = (PFNGLVERTEXATTRIB4NBVPROC)glGetProcAddress((ProcType)"glVertexAttrib4Nbv");
	_glVertexAttrib4Niv_ptr = (PFNGLVERTEXATTRIB4NIVPROC)glGetProcAddress((ProcType)"glVertexAttrib4Niv");
	_glVertexAttrib4Nsv_ptr = (PFNGLVERTEXATTRIB4NSVPROC)glGetProcAddress((ProcType)"glVertexAttrib4Nsv");
	_glVertexAttrib4Nub_ptr = (PFNGLVERTEXATTRIB4NUBPROC)glGetProcAddress((ProcType)"glVertexAttrib4Nub");
	_glVertexAttrib4Nubv_ptr = (PFNGLVERTEXATTRIB4NUBVPROC)glGetProcAddress((ProcType)"glVertexAttrib4Nubv");
	_glVertexAttrib4Nuiv_ptr = (PFNGLVERTEXATTRIB4NUIVPROC)glGetProcAddress((ProcType)"glVertexAttrib4Nuiv");
	_glVertexAttrib4Nusv_ptr = (PFNGLVERTEXATTRIB4NUSVPROC)glGetProcAddress((ProcType)"glVertexAttrib4Nusv");
	_glVertexAttrib4bv_ptr = (PFNGLVERTEXATTRIB4BVPROC)glGetProcAddress((ProcType)"glVertexAttrib4bv");
	_glVertexAttrib4d_ptr = (PFNGLVERTEXATTRIB4DPROC)glGetProcAddress((ProcType)"glVertexAttrib4d");
	_glVertexAttrib4dv_ptr = (PFNGLVERTEXATTRIB4DVPROC)glGetProcAddress((ProcType)"glVertexAttrib4dv");
	_glVertexAttrib4f_ptr = (PFNGLVERTEXATTRIB4FPROC)glGetProcAddress((ProcType)"glVertexAttrib4f");
	_glVertexAttrib4fv_ptr = (PFNGLVERTEXATTRIB4FVPROC)glGetProcAddress((ProcType)"glVertexAttrib4fv");
	_glVertexAttrib4iv_ptr = (PFNGLVERTEXATTRIB4IVPROC)glGetProcAddress((ProcType)"glVertexAttrib4iv");
	_glVertexAttrib4s_ptr = (PFNGLVERTEXATTRIB4SPROC)glGetProcAddress((ProcType)"glVertexAttrib4s");
	_glVertexAttrib4sv_ptr = (PFNGLVERTEXATTRIB4SVPROC)glGetProcAddress((ProcType)"glVertexAttrib4sv");
	_glVertexAttrib4ubv_ptr = (PFNGLVERTEXATTRIB4UBVPROC)glGetProcAddress((ProcType)"glVertexAttrib4ubv");
	_glVertexAttrib4uiv_ptr = (PFNGLVERTEXATTRIB4UIVPROC)glGetProcAddress((ProcType)"glVertexAttrib4uiv");
	_glVertexAttrib4usv_ptr = (PFNGLVERTEXATTRIB4USVPROC)glGetProcAddress((ProcType)"glVertexAttrib4usv");
	_glVertexAttribPointer_ptr = (PFNGLVERTEXATTRIBPOINTERPROC)glGetProcAddress((ProcType)"glVertexAttribPointer");

	// OpenGL 2.1
	_glUniformMatrix2x3fv_ptr = (PFNGLUNIFORMMATRIX2X3FVPROC)glGetProcAddress((ProcType)"glUniformMatrix2x3fv");
	_glUniformMatrix3x2fv_ptr = (PFNGLUNIFORMMATRIX3X2FVPROC)glGetProcAddress((ProcType)"glUniformMatrix3x2fv");
	_glUniformMatrix2x4fv_ptr = (PFNGLUNIFORMMATRIX2X4FVPROC)glGetProcAddress((ProcType)"glUniformMatrix2x4fv");
	_glUniformMatrix4x2fv_ptr = (PFNGLUNIFORMMATRIX4X2FVPROC)glGetProcAddress((ProcType)"glUniformMatrix4x2fv");
	_glUniformMatrix3x4fv_ptr = (PFNGLUNIFORMMATRIX3X4FVPROC)glGetProcAddress((ProcType)"glUniformMatrix3x4fv");
	_glUniformMatrix4x3fv_ptr = (PFNGLUNIFORMMATRIX4X3FVPROC)glGetProcAddress((ProcType)"glUniformMatrix4x3fv");

	// OpenGL 3.0
	_glColorMaski_ptr = (PFNGLCOLORMASKIPROC)glGetProcAddress((ProcType)"glColorMaski");
	_glGetBooleani_v_ptr = (PFNGLGETBOOLEANI_VPROC)glGetProcAddress((ProcType)"glGetBooleani_v");
	_glGetIntegeri_v_ptr = (PFNGLGETINTEGERI_VPROC)glGetProcAddress((ProcType)"glGetIntegeri_v");
	_glEnablei_ptr = (PFNGLENABLEIPROC)glGetProcAddress((ProcType)"glEnablei");
	_glDisablei_ptr = (PFNGLDISABLEIPROC)glGetProcAddress((ProcType)"glDisablei");
	_glIsEnabledi_ptr = (PFNGLISENABLEDIPROC)glGetProcAddress((ProcType)"glIsEnabledi");
	_glBeginTransformFeedback_ptr = (PFNGLBEGINTRANSFORMFEEDBACKPROC)glGetProcAddress((ProcType)"glBeginTransformFeedback");
	_glEndTransformFeedback_ptr = (PFNGLENDTRANSFORMFEEDBACKPROC)glGetProcAddress((ProcType)"glEndTransformFeedback");
	_glBindBufferRange_ptr = (PFNGLBINDBUFFERRANGEPROC)glGetProcAddress((ProcType)"glBindBufferRange");
	_glBindBufferBase_ptr = (PFNGLBINDBUFFERBASEPROC)glGetProcAddress((ProcType)"glBindBufferBase");
	_glTransformFeedbackVaryings_ptr = (PFNGLTRANSFORMFEEDBACKVARYINGSPROC)glGetProcAddress((ProcType)"glTransformFeedbackVaryings");
	_glGetTransformFeedbackVarying_ptr = (PFNGLGETTRANSFORMFEEDBACKVARYINGPROC)glGetProcAddress((ProcType)"glGetTransformFeedbackVarying");
	_glClampColor_ptr = (PFNGLCLAMPCOLORPROC)glGetProcAddress((ProcType)"glClampColor");
	_glBeginConditionalRender_ptr = (PFNGLBEGINCONDITIONALRENDERPROC)glGetProcAddress((ProcType)"glBeginConditionalRender");
	_glEndConditionalRender_ptr = (PFNGLENDCONDITIONALRENDERPROC)glGetProcAddress((ProcType)"glEndConditionalRender");
	_glVertexAttribIPointer_ptr = (PFNGLVERTEXATTRIBIPOINTERPROC)glGetProcAddress((ProcType)"glVertexAttribIPointer");
	_glGetVertexAttribIiv_ptr = (PFNGLGETVERTEXATTRIBIIVPROC)glGetProcAddress((ProcType)"glGetVertexAttribIiv");
	_glGetVertexAttribIuiv_ptr = (PFNGLGETVERTEXATTRIBIUIVPROC)glGetProcAddress((ProcType)"glGetVertexAttribIuiv");
	_glVertexAttribI1i_ptr = (PFNGLVERTEXATTRIBI1IPROC)glGetProcAddress((ProcType)"glVertexAttribI1i");
	_glVertexAttribI2i_ptr = (PFNGLVERTEXATTRIBI2IPROC)glGetProcAddress((ProcType)"glVertexAttribI2i");
	_glVertexAttribI3i_ptr = (PFNGLVERTEXATTRIBI3IPROC)glGetProcAddress((ProcType)"glVertexAttribI3i");
	_glVertexAttribI4i_ptr = (PFNGLVERTEXATTRIBI4IPROC)glGetProcAddress((ProcType)"glVertexAttribI4i");
	_glVertexAttribI1ui_ptr = (PFNGLVERTEXATTRIBI1UIPROC)glGetProcAddress((ProcType)"glVertexAttribI1ui");
	_glVertexAttribI2ui_ptr = (PFNGLVERTEXATTRIBI2UIPROC)glGetProcAddress((ProcType)"glVertexAttribI2ui");
	_glVertexAttribI3ui_ptr = (PFNGLVERTEXATTRIBI3UIPROC)glGetProcAddress((ProcType)"glVertexAttribI3ui");
	_glVertexAttribI4ui_ptr = (PFNGLVERTEXATTRIBI4UIPROC)glGetProcAddress((ProcType)"glVertexAttribI4ui");
	_glVertexAttribI1iv_ptr = (PFNGLVERTEXATTRIBI1IVPROC)glGetProcAddress((ProcType)"glVertexAttribI1iv");
	_glVertexAttribI2iv_ptr = (PFNGLVERTEXATTRIBI2IVPROC)glGetProcAddress((ProcType)"glVertexAttribI2iv");
	_glVertexAttribI3iv_ptr = (PFNGLVERTEXATTRIBI3IVPROC)glGetProcAddress((ProcType)"glVertexAttribI3iv");
	_glVertexAttribI4iv_ptr = (PFNGLVERTEXATTRIBI4IVPROC)glGetProcAddress((ProcType)"glVertexAttribI4iv");
	_glVertexAttribI1uiv_ptr = (PFNGLVERTEXATTRIBI1UIVPROC)glGetProcAddress((ProcType)"glVertexAttribI1uiv");
	_glVertexAttribI2uiv_ptr = (PFNGLVERTEXATTRIBI2UIVPROC)glGetProcAddress((ProcType)"glVertexAttribI2uiv");
	_glVertexAttribI3uiv_ptr = (PFNGLVERTEXATTRIBI3UIVPROC)glGetProcAddress((ProcType)"glVertexAttribI3uiv");
	_glVertexAttribI4uiv_ptr = (PFNGLVERTEXATTRIBI4UIVPROC)glGetProcAddress((ProcType)"glVertexAttribI4uiv");
	_glVertexAttribI4bv_ptr = (PFNGLVERTEXATTRIBI4BVPROC)glGetProcAddress((ProcType)"glVertexAttribI4bv");
	_glVertexAttribI4sv_ptr = (PFNGLVERTEXATTRIBI4SVPROC)glGetProcAddress((ProcType)"glVertexAttribI4sv");
	_glVertexAttribI4ubv_ptr = (PFNGLVERTEXATTRIBI4UBVPROC)glGetProcAddress((ProcType)"glVertexAttribI4ubv");
	_glVertexAttribI4usv_ptr = (PFNGLVERTEXATTRIBI4USVPROC)glGetProcAddress((ProcType)"glVertexAttribI4usv");
	_glGetUniformuiv_ptr = (PFNGLGETUNIFORMUIVPROC)glGetProcAddress((ProcType)"glGetUniformuiv");
	_glBindFragDataLocation_ptr = (PFNGLBINDFRAGDATALOCATIONPROC)glGetProcAddress((ProcType)"glBindFragDataLocation");
	_glGetFragDataLocation_ptr = (PFNGLGETFRAGDATALOCATIONPROC)glGetProcAddress((ProcType)"glGetFragDataLocation");
	_glUniform1ui_ptr = (PFNGLUNIFORM1UIPROC)glGetProcAddress((ProcType)"glUniform1ui");
	_glUniform2ui_ptr = (PFNGLUNIFORM2UIPROC)glGetProcAddress((ProcType)"glUniform2ui");
	_glUniform3ui_ptr = (PFNGLUNIFORM3UIPROC)glGetProcAddress((ProcType)"glUniform3ui");
	_glUniform4ui_ptr = (PFNGLUNIFORM4UIPROC)glGetProcAddress((ProcType)"glUniform4ui");
	_glUniform1uiv_ptr = (PFNGLUNIFORM1UIVPROC)glGetProcAddress((ProcType)"glUniform1uiv");
	_glUniform2uiv_ptr = (PFNGLUNIFORM2UIVPROC)glGetProcAddress((ProcType)"glUniform2uiv");
	_glUniform3uiv_ptr = (PFNGLUNIFORM3UIVPROC)glGetProcAddress((ProcType)"glUniform3uiv");
	_glUniform4uiv_ptr = (PFNGLUNIFORM4UIVPROC)glGetProcAddress((ProcType)"glUniform4uiv");
	_glTexParameterIiv_ptr = (PFNGLTEXPARAMETERIIVPROC)glGetProcAddress((ProcType)"glTexParameterIiv");
	_glTexParameterIuiv_ptr = (PFNGLTEXPARAMETERIUIVPROC)glGetProcAddress((ProcType)"glTexParameterIuiv");
	_glGetTexParameterIiv_ptr = (PFNGLGETTEXPARAMETERIIVPROC)glGetProcAddress((ProcType)"glGetTexParameterIiv");
	_glGetTexParameterIuiv_ptr = (PFNGLGETTEXPARAMETERIUIVPROC)glGetProcAddress((ProcType)"glGetTexParameterIuiv");
	_glClearBufferiv_ptr = (PFNGLCLEARBUFFERIVPROC)glGetProcAddress((ProcType)"glClearBufferiv");
	_glClearBufferuiv_ptr = (PFNGLCLEARBUFFERUIVPROC)glGetProcAddress((ProcType)"glClearBufferuiv");
	_glClearBufferfv_ptr = (PFNGLCLEARBUFFERFVPROC)glGetProcAddress((ProcType)"glClearBufferfv");
	_glClearBufferfi_ptr = (PFNGLCLEARBUFFERFIPROC)glGetProcAddress((ProcType)"glClearBufferfi");
	_glGetStringi_ptr = (PFNGLGETSTRINGIPROC)glGetProcAddress((ProcType)"glGetStringi");
	_glIsRenderbuffer_ptr = (PFNGLISRENDERBUFFERPROC)glGetProcAddress((ProcType)"glIsRenderbuffer"); // ARB_framebuffer_object
	_glBindRenderbuffer_ptr = (PFNGLBINDRENDERBUFFERPROC)glGetProcAddress((ProcType)"glBindRenderbuffer"); // ARB_framebuffer_object
	_glDeleteRenderbuffers_ptr = (PFNGLDELETERENDERBUFFERSPROC)glGetProcAddress((ProcType)"glDeleteRenderbuffers"); // ARB_framebuffer_object
	_glGenRenderbuffers_ptr = (PFNGLGENRENDERBUFFERSPROC)glGetProcAddress((ProcType)"glGenRenderbuffers"); // ARB_framebuffer_object
	_glRenderbufferStorage_ptr = (PFNGLRENDERBUFFERSTORAGEPROC)glGetProcAddress((ProcType)"glRenderbufferStorage"); // ARB_framebuffer_object
	_glGetRenderbufferParameteriv_ptr = (PFNGLGETRENDERBUFFERPARAMETERIVPROC)glGetProcAddress((ProcType)"glGetRenderbufferParameteriv"); // ARB_framebuffer_object
	_glIsFramebuffer_ptr = (PFNGLISFRAMEBUFFERPROC)glGetProcAddress((ProcType)"glIsFramebuffer"); // ARB_framebuffer_object
	_glBindFramebuffer_ptr = (PFNGLBINDFRAMEBUFFERPROC)glGetProcAddress((ProcType)"glBindFramebuffer"); // ARB_framebuffer_object
	_glDeleteFramebuffers_ptr = (PFNGLDELETEFRAMEBUFFERSPROC)glGetProcAddress((ProcType)"glDeleteFramebuffers"); // ARB_framebuffer_object
	_glGenFramebuffers_ptr = (PFNGLGENFRAMEBUFFERSPROC)glGetProcAddress((ProcType)"glGenFramebuffers"); // ARB_framebuffer_object
	_glCheckFramebufferStatus_ptr = (PFNGLCHECKFRAMEBUFFERSTATUSPROC)glGetProcAddress((ProcType)"glCheckFramebufferStatus"); // ARB_framebuffer_object
	_glFramebufferTexture1D_ptr = (PFNGLFRAMEBUFFERTEXTURE1DPROC)glGetProcAddress((ProcType)"glFramebufferTexture1D"); // ARB_framebuffer_object
	_glFramebufferTexture2D_ptr = (PFNGLFRAMEBUFFERTEXTURE2DPROC)glGetProcAddress((ProcType)"glFramebufferTexture2D"); // ARB_framebuffer_object
	_glFramebufferTexture3D_ptr = (PFNGLFRAMEBUFFERTEXTURE3DPROC)glGetProcAddress((ProcType)"glFramebufferTexture3D"); // ARB_framebuffer_object
	_glFramebufferRenderbuffer_ptr = (PFNGLFRAMEBUFFERRENDERBUFFERPROC)glGetProcAddress((ProcType)"glFramebufferRenderbuffer"); // ARB_framebuffer_object
	_glGetFramebufferAttachmentParameteriv_ptr = (PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVPROC)glGetProcAddress((ProcType)"glGetFramebufferAttachmentParameteriv"); // ARB_framebuffer_object
	_glGenerateMipmap_ptr = (PFNGLGENERATEMIPMAPPROC)glGetProcAddress((ProcType)"glGenerateMipmap"); // ARB_framebuffer_object
	_glBlitFramebuffer_ptr = (PFNGLBLITFRAMEBUFFERPROC)glGetProcAddress((ProcType)"glBlitFramebuffer"); // ARB_framebuffer_object
	_glRenderbufferStorageMultisample_ptr = (PFNGLRENDERBUFFERSTORAGEMULTISAMPLEPROC)glGetProcAddress((ProcType)"glRenderbufferStorageMultisample"); // ARB_framebuffer_object
	_glFramebufferTextureLayer_ptr = (PFNGLFRAMEBUFFERTEXTURELAYERPROC)glGetProcAddress((ProcType)"glFramebufferTextureLayer"); // ARB_framebuffer_object
	_glMapBufferRange_ptr = (PFNGLMAPBUFFERRANGEPROC)glGetProcAddress((ProcType)"glMapBufferRange"); // ARB_map_buffer_range
	_glFlushMappedBufferRange_ptr = (PFNGLFLUSHMAPPEDBUFFERRANGEPROC)glGetProcAddress((ProcType)"glFlushMappedBufferRange"); // ARB_map_buffer_range
	_glBindVertexArray_ptr = (PFNGLBINDVERTEXARRAYPROC)glGetProcAddress((ProcType)"glBindVertexArray"); // ARB_vertex_array_object
	_glDeleteVertexArrays_ptr = (PFNGLDELETEVERTEXARRAYSPROC)glGetProcAddress((ProcType)"glDeleteVertexArrays"); // ARB_vertex_array_object
	_glGenVertexArrays_ptr = (PFNGLGENVERTEXARRAYSPROC)glGetProcAddress((ProcType)"glGenVertexArrays"); // ARB_vertex_array_object
	_glIsVertexArray_ptr = (PFNGLISVERTEXARRAYPROC)glGetProcAddress((ProcType)"glIsVertexArray"); // ARB_vertex_array_object

	// OpenGL 3.1
	_glDrawArraysInstanced_ptr = (PFNGLDRAWARRAYSINSTANCEDPROC)glGetProcAddress((ProcType)"glDrawArraysInstanced");
	_glDrawElementsInstanced_ptr = (PFNGLDRAWELEMENTSINSTANCEDPROC)glGetProcAddress((ProcType)"glDrawElementsInstanced");
	_glTexBuffer_ptr = (PFNGLTEXBUFFERPROC)glGetProcAddress((ProcType)"glTexBuffer");
	_glPrimitiveRestartIndex_ptr = (PFNGLPRIMITIVERESTARTINDEXPROC)glGetProcAddress((ProcType)"glPrimitiveRestartIndex");
	_glCopyBufferSubData_ptr = (PFNGLCOPYBUFFERSUBDATAPROC)glGetProcAddress((ProcType)"glCopyBufferSubData"); // ARB_copy_buffer
	_glGetUniformIndices_ptr = (PFNGLGETUNIFORMINDICESPROC)glGetProcAddress((ProcType)"glGetUniformIndices"); // ARB_uniform_buffer_object
	_glGetActiveUniformsiv_ptr = (PFNGLGETACTIVEUNIFORMSIVPROC)glGetProcAddress((ProcType)"glGetActiveUniformsiv"); // ARB_uniform_buffer_object
	_glGetActiveUniformName_ptr = (PFNGLGETACTIVEUNIFORMNAMEPROC)glGetProcAddress((ProcType)"glGetActiveUniformName"); // ARB_uniform_buffer_object
	_glGetUniformBlockIndex_ptr = (PFNGLGETUNIFORMBLOCKINDEXPROC)glGetProcAddress((ProcType)"glGetUniformBlockIndex"); // ARB_uniform_buffer_object
	_glGetActiveUniformBlockiv_ptr = (PFNGLGETACTIVEUNIFORMBLOCKIVPROC)glGetProcAddress((ProcType)"glGetActiveUniformBlockiv"); // ARB_uniform_buffer_object
	_glGetActiveUniformBlockName_ptr = (PFNGLGETACTIVEUNIFORMBLOCKNAMEPROC)glGetProcAddress((ProcType)"glGetActiveUniformBlockName"); // ARB_uniform_buffer_object
	_glUniformBlockBinding_ptr = (PFNGLUNIFORMBLOCKBINDINGPROC)glGetProcAddress((ProcType)"glUniformBlockBinding"); // ARB_uniform_buffer_object

	// OpenGL 3.2
	_glGetInteger64i_v_ptr = (PFNGLGETINTEGER64I_VPROC)glGetProcAddress((ProcType)"glGetInteger64i_v");
	_glGetBufferParameteri64v_ptr = (PFNGLGETBUFFERPARAMETERI64VPROC)glGetProcAddress((ProcType)"glGetBufferParameteri64v");
	_glFramebufferTexture_ptr = (PFNGLFRAMEBUFFERTEXTUREPROC)glGetProcAddress((ProcType)"glFramebufferTexture");
	_glDrawElementsBaseVertex_ptr = (PFNGLDRAWELEMENTSBASEVERTEXPROC)glGetProcAddress((ProcType)"glDrawElementsBaseVertex"); // ARB_draw_elements_base_vertex
	_glDrawRangeElementsBaseVertex_ptr = (PFNGLDRAWRANGEELEMENTSBASEVERTEXPROC)glGetProcAddress((ProcType)"glDrawRangeElementsBaseVertex"); // ARB_draw_elements_base_vertex
	_glDrawElementsInstancedBaseVertex_ptr = (PFNGLDRAWELEMENTSINSTANCEDBASEVERTEXPROC)glGetProcAddress((ProcType)"glDrawElementsInstancedBaseVertex"); // ARB_draw_elements_base_vertex
	_glMultiDrawElementsBaseVertex_ptr = (PFNGLMULTIDRAWELEMENTSBASEVERTEXPROC)glGetProcAddress((ProcType)"glMultiDrawElementsBaseVertex"); // ARB_draw_elements_base_vertex
	_glProvokingVertex_ptr = (PFNGLPROVOKINGVERTEXPROC)glGetProcAddress((ProcType)"glProvokingVertex"); // ARB_provoking_vertex
	_glFenceSync_ptr = (PFNGLFENCESYNCPROC)glGetProcAddress((ProcType)"glFenceSync"); // ARB_sync
	_glIsSync_ptr = (PFNGLISSYNCPROC)glGetProcAddress((ProcType)"glIsSync"); // ARB_sync
	_glDeleteSync_ptr = (PFNGLDELETESYNCPROC)glGetProcAddress((ProcType)"glDeleteSync"); // ARB_sync
	_glClientWaitSync_ptr = (PFNGLCLIENTWAITSYNCPROC)glGetProcAddress((ProcType)"glClientWaitSync"); // ARB_sync
	_glWaitSync_ptr = (PFNGLWAITSYNCPROC)glGetProcAddress((ProcType)"glWaitSync"); // ARB_sync
	_glGetInteger64v_ptr = (PFNGLGETINTEGER64VPROC)glGetProcAddress((ProcType)"glGetInteger64v"); // ARB_sync
	_glGetSynciv_ptr = (PFNGLGETSYNCIVPROC)glGetProcAddress((ProcType)"glGetSynciv"); // ARB_sync
	_glTexImage2DMultisample_ptr = (PFNGLTEXIMAGE2DMULTISAMPLEPROC)glGetProcAddress((ProcType)"glTexImage2DMultisample"); // ARB_texture_multisample
	_glTexImage3DMultisample_ptr = (PFNGLTEXIMAGE3DMULTISAMPLEPROC)glGetProcAddress((ProcType)"glTexImage3DMultisample"); // ARB_texture_multisample
	_glGetMultisamplefv_ptr = (PFNGLGETMULTISAMPLEFVPROC)glGetProcAddress((ProcType)"glGetMultisamplefv"); // ARB_texture_multisample
	_glSampleMaski_ptr = (PFNGLSAMPLEMASKIPROC)glGetProcAddress((ProcType)"glSampleMaski"); // ARB_texture_multisample

	// OpenGL 3.3
	_glVertexAttribDivisor_ptr = (PFNGLVERTEXATTRIBDIVISORPROC)glGetProcAddress((ProcType)"glVertexAttribDivisor");
	_glBindFragDataLocationIndexed_ptr = (PFNGLBINDFRAGDATALOCATIONINDEXEDPROC)glGetProcAddress((ProcType)"glBindFragDataLocationIndexed"); // ARB_blend_func_extended
	_glGetFragDataIndex_ptr = (PFNGLGETFRAGDATAINDEXPROC)glGetProcAddress((ProcType)"glGetFragDataIndex"); // ARB_blend_func_extended
	_glGenSamplers_ptr = (PFNGLGENSAMPLERSPROC)glGetProcAddress((ProcType)"glGenSamplers"); // ARB_sampler_objects
	_glDeleteSamplers_ptr = (PFNGLDELETESAMPLERSPROC)glGetProcAddress((ProcType)"glDeleteSamplers"); // ARB_sampler_objects
	_glIsSampler_ptr = (PFNGLISSAMPLERPROC)glGetProcAddress((ProcType)"glIsSampler"); // ARB_sampler_objects
	_glBindSampler_ptr = (PFNGLBINDSAMPLERPROC)glGetProcAddress((ProcType)"glBindSampler"); // ARB_sampler_objects
	_glSamplerParameteri_ptr = (PFNGLSAMPLERPARAMETERIPROC)glGetProcAddress((ProcType)"glSamplerParameteri"); // ARB_sampler_objects
	_glSamplerParameteriv_ptr = (PFNGLSAMPLERPARAMETERIVPROC)glGetProcAddress((ProcType)"glSamplerParameteriv"); // ARB_sampler_objects
	_glSamplerParameterf_ptr = (PFNGLSAMPLERPARAMETERFPROC)glGetProcAddress((ProcType)"glSamplerParameterf"); // ARB_sampler_objects
	_glSamplerParameterfv_ptr = (PFNGLSAMPLERPARAMETERFVPROC)glGetProcAddress((ProcType)"glSamplerParameterfv"); // ARB_sampler_objects
	_glSamplerParameterIiv_ptr = (PFNGLSAMPLERPARAMETERIIVPROC)glGetProcAddress((ProcType)"glSamplerParameterIiv"); // ARB_sampler_objects
	_glSamplerParameterIuiv_ptr = (PFNGLSAMPLERPARAMETERIUIVPROC)glGetProcAddress((ProcType)"glSamplerParameterIuiv"); // ARB_sampler_objects
	_glGetSamplerParameteriv_ptr = (PFNGLGETSAMPLERPARAMETERIVPROC)glGetProcAddress((ProcType)"glGetSamplerParameteriv"); // ARB_sampler_objects
	_glGetSamplerParameterIiv_ptr = (PFNGLGETSAMPLERPARAMETERIIVPROC)glGetProcAddress((ProcType)"glGetSamplerParameterIiv"); // ARB_sampler_objects
	_glGetSamplerParameterfv_ptr = (PFNGLGETSAMPLERPARAMETERFVPROC)glGetProcAddress((ProcType)"glGetSamplerParameterfv"); // ARB_sampler_objects
	_glGetSamplerParameterIuiv_ptr = (PFNGLGETSAMPLERPARAMETERIUIVPROC)glGetProcAddress((ProcType)"glGetSamplerParameterIuiv"); // ARB_sampler_objects
	_glQueryCounter_ptr = (PFNGLQUERYCOUNTERPROC)glGetProcAddress((ProcType)"glQueryCounter"); // ARB_timer_query
	_glGetQueryObjecti64v_ptr = (PFNGLGETQUERYOBJECTI64VPROC)glGetProcAddress((ProcType)"glGetQueryObjecti64v"); // ARB_timer_query
	_glGetQueryObjectui64v_ptr = (PFNGLGETQUERYOBJECTUI64VPROC)glGetProcAddress((ProcType)"glGetQueryObjectui64v"); // ARB_timer_query
	_glVertexP2ui_ptr = (PFNGLVERTEXP2UIPROC)glGetProcAddress((ProcType)"glVertexP2ui"); // ARB_vertex_type_2_10_10_10_rev
	_glVertexP2uiv_ptr = (PFNGLVERTEXP2UIVPROC)glGetProcAddress((ProcType)"glVertexP2uiv"); // ARB_vertex_type_2_10_10_10_rev
	_glVertexP3ui_ptr = (PFNGLVERTEXP3UIPROC)glGetProcAddress((ProcType)"glVertexP3ui"); // ARB_vertex_type_2_10_10_10_rev
	_glVertexP3uiv_ptr = (PFNGLVERTEXP3UIVPROC)glGetProcAddress((ProcType)"glVertexP3uiv"); // ARB_vertex_type_2_10_10_10_rev
	_glVertexP4ui_ptr = (PFNGLVERTEXP4UIPROC)glGetProcAddress((ProcType)"glVertexP4ui"); // ARB_vertex_type_2_10_10_10_rev
	_glVertexP4uiv_ptr = (PFNGLVERTEXP4UIVPROC)glGetProcAddress((ProcType)"glVertexP4uiv"); // ARB_vertex_type_2_10_10_10_rev
	_glTexCoordP1ui_ptr = (PFNGLTEXCOORDP1UIPROC)glGetProcAddress((ProcType)"glTexCoordP1ui"); // ARB_vertex_type_2_10_10_10_rev
	_glTexCoordP1uiv_ptr = (PFNGLTEXCOORDP1UIVPROC)glGetProcAddress((ProcType)"glTexCoordP1uiv"); // ARB_vertex_type_2_10_10_10_rev
	_glTexCoordP2ui_ptr = (PFNGLTEXCOORDP2UIPROC)glGetProcAddress((ProcType)"glTexCoordP2ui"); // ARB_vertex_type_2_10_10_10_rev
	_glTexCoordP2uiv_ptr = (PFNGLTEXCOORDP2UIVPROC)glGetProcAddress((ProcType)"glTexCoordP2uiv"); // ARB_vertex_type_2_10_10_10_rev
	_glTexCoordP3ui_ptr = (PFNGLTEXCOORDP3UIPROC)glGetProcAddress((ProcType)"glTexCoordP3ui"); // ARB_vertex_type_2_10_10_10_rev
	_glTexCoordP3uiv_ptr = (PFNGLTEXCOORDP3UIVPROC)glGetProcAddress((ProcType)"glTexCoordP3uiv"); // ARB_vertex_type_2_10_10_10_rev
	_glTexCoordP4ui_ptr = (PFNGLTEXCOORDP4UIPROC)glGetProcAddress((ProcType)"glTexCoordP4ui"); // ARB_vertex_type_2_10_10_10_rev
	_glTexCoordP4uiv_ptr = (PFNGLTEXCOORDP4UIVPROC)glGetProcAddress((ProcType)"glTexCoordP4uiv"); // ARB_vertex_type_2_10_10_10_rev
	_glMultiTexCoordP1ui_ptr = (PFNGLMULTITEXCOORDP1UIPROC)glGetProcAddress((ProcType)"glMultiTexCoordP1ui"); // ARB_vertex_type_2_10_10_10_rev
	_glMultiTexCoordP1uiv_ptr = (PFNGLMULTITEXCOORDP1UIVPROC)glGetProcAddress((ProcType)"glMultiTexCoordP1uiv"); // ARB_vertex_type_2_10_10_10_rev
	_glMultiTexCoordP2ui_ptr = (PFNGLMULTITEXCOORDP2UIPROC)glGetProcAddress((ProcType)"glMultiTexCoordP2ui"); // ARB_vertex_type_2_10_10_10_rev
	_glMultiTexCoordP2uiv_ptr = (PFNGLMULTITEXCOORDP2UIVPROC)glGetProcAddress((ProcType)"glMultiTexCoordP2uiv"); // ARB_vertex_type_2_10_10_10_rev
	_glMultiTexCoordP3ui_ptr = (PFNGLMULTITEXCOORDP3UIPROC)glGetProcAddress((ProcType)"glMultiTexCoordP3ui"); // ARB_vertex_type_2_10_10_10_rev
	_glMultiTexCoordP3uiv_ptr = (PFNGLMULTITEXCOORDP3UIVPROC)glGetProcAddress((ProcType)"glMultiTexCoordP3uiv"); // ARB_vertex_type_2_10_10_10_rev
	_glMultiTexCoordP4ui_ptr = (PFNGLMULTITEXCOORDP4UIPROC)glGetProcAddress((ProcType)"glMultiTexCoordP4ui"); // ARB_vertex_type_2_10_10_10_rev
	_glMultiTexCoordP4uiv_ptr = (PFNGLMULTITEXCOORDP4UIVPROC)glGetProcAddress((ProcType)"glMultiTexCoordP4uiv"); // ARB_vertex_type_2_10_10_10_rev
	_glNormalP3ui_ptr = (PFNGLNORMALP3UIPROC)glGetProcAddress((ProcType)"glNormalP3ui"); // ARB_vertex_type_2_10_10_10_rev
	_glNormalP3uiv_ptr = (PFNGLNORMALP3UIVPROC)glGetProcAddress((ProcType)"glNormalP3uiv"); // ARB_vertex_type_2_10_10_10_rev
	_glColorP3ui_ptr = (PFNGLCOLORP3UIPROC)glGetProcAddress((ProcType)"glColorP3ui"); // ARB_vertex_type_2_10_10_10_rev
	_glColorP3uiv_ptr = (PFNGLCOLORP3UIVPROC)glGetProcAddress((ProcType)"glColorP3uiv"); // ARB_vertex_type_2_10_10_10_rev
	_glColorP4ui_ptr = (PFNGLCOLORP4UIPROC)glGetProcAddress((ProcType)"glColorP4ui"); // ARB_vertex_type_2_10_10_10_rev
	_glColorP4uiv_ptr = (PFNGLCOLORP4UIVPROC)glGetProcAddress((ProcType)"glColorP4uiv"); // ARB_vertex_type_2_10_10_10_rev
	_glSecondaryColorP3ui_ptr = (PFNGLSECONDARYCOLORP3UIPROC)glGetProcAddress((ProcType)"glSecondaryColorP3ui"); // ARB_vertex_type_2_10_10_10_rev
	_glSecondaryColorP3uiv_ptr = (PFNGLSECONDARYCOLORP3UIVPROC)glGetProcAddress((ProcType)"glSecondaryColorP3uiv"); // ARB_vertex_type_2_10_10_10_rev
	_glVertexAttribP1ui_ptr = (PFNGLVERTEXATTRIBP1UIPROC)glGetProcAddress((ProcType)"glVertexAttribP1ui"); // ARB_vertex_type_2_10_10_10_rev
	_glVertexAttribP1uiv_ptr = (PFNGLVERTEXATTRIBP1UIVPROC)glGetProcAddress((ProcType)"glVertexAttribP1uiv"); // ARB_vertex_type_2_10_10_10_rev
	_glVertexAttribP2ui_ptr = (PFNGLVERTEXATTRIBP2UIPROC)glGetProcAddress((ProcType)"glVertexAttribP2ui"); // ARB_vertex_type_2_10_10_10_rev
	_glVertexAttribP2uiv_ptr = (PFNGLVERTEXATTRIBP2UIVPROC)glGetProcAddress((ProcType)"glVertexAttribP2uiv"); // ARB_vertex_type_2_10_10_10_rev
	_glVertexAttribP3ui_ptr = (PFNGLVERTEXATTRIBP3UIPROC)glGetProcAddress((ProcType)"glVertexAttribP3ui"); // ARB_vertex_type_2_10_10_10_rev
	_glVertexAttribP3uiv_ptr = (PFNGLVERTEXATTRIBP3UIVPROC)glGetProcAddress((ProcType)"glVertexAttribP3uiv"); // ARB_vertex_type_2_10_10_10_rev
	_glVertexAttribP4ui_ptr = (PFNGLVERTEXATTRIBP4UIPROC)glGetProcAddress((ProcType)"glVertexAttribP4ui"); // ARB_vertex_type_2_10_10_10_rev
	_glVertexAttribP4uiv_ptr = (PFNGLVERTEXATTRIBP4UIVPROC)glGetProcAddress((ProcType)"glVertexAttribP4uiv"); // ARB_vertex_type_2_10_10_10_rev

	// OpenGL 4.0
	_glMinSampleShading_ptr = (PFNGLMINSAMPLESHADINGPROC)glGetProcAddress((ProcType)"glMinSampleShading");
	_glBlendEquationi_ptr = (PFNGLBLENDEQUATIONIPROC)glGetProcAddress((ProcType)"glBlendEquationi");
	_glBlendEquationSeparatei_ptr = (PFNGLBLENDEQUATIONSEPARATEIPROC)glGetProcAddress((ProcType)"glBlendEquationSeparatei");
	_glBlendFunci_ptr = (PFNGLBLENDFUNCIPROC)glGetProcAddress((ProcType)"glBlendFunci");
	_glBlendFuncSeparatei_ptr = (PFNGLBLENDFUNCSEPARATEIPROC)glGetProcAddress((ProcType)"glBlendFuncSeparatei");
	_glDrawArraysIndirect_ptr = (PFNGLDRAWARRAYSINDIRECTPROC)glGetProcAddress((ProcType)"glDrawArraysIndirect"); // ARB_draw_indirect
	_glDrawElementsIndirect_ptr = (PFNGLDRAWELEMENTSINDIRECTPROC)glGetProcAddress((ProcType)"glDrawElementsIndirect"); // ARB_draw_indirect
	_glUniform1d_ptr = (PFNGLUNIFORM1DPROC)glGetProcAddress((ProcType)"glUniform1d"); // ARB_gpu_shader_fp64
	_glUniform2d_ptr = (PFNGLUNIFORM2DPROC)glGetProcAddress((ProcType)"glUniform2d"); // ARB_gpu_shader_fp64
	_glUniform3d_ptr = (PFNGLUNIFORM3DPROC)glGetProcAddress((ProcType)"glUniform3d"); // ARB_gpu_shader_fp64
	_glUniform4d_ptr = (PFNGLUNIFORM4DPROC)glGetProcAddress((ProcType)"glUniform4d"); // ARB_gpu_shader_fp64
	_glUniform1dv_ptr = (PFNGLUNIFORM1DVPROC)glGetProcAddress((ProcType)"glUniform1dv"); // ARB_gpu_shader_fp64
	_glUniform2dv_ptr = (PFNGLUNIFORM2DVPROC)glGetProcAddress((ProcType)"glUniform2dv"); // ARB_gpu_shader_fp64
	_glUniform3dv_ptr = (PFNGLUNIFORM3DVPROC)glGetProcAddress((ProcType)"glUniform3dv"); // ARB_gpu_shader_fp64
	_glUniform4dv_ptr = (PFNGLUNIFORM4DVPROC)glGetProcAddress((ProcType)"glUniform4dv"); // ARB_gpu_shader_fp64
	_glUniformMatrix2dv_ptr = (PFNGLUNIFORMMATRIX2DVPROC)glGetProcAddress((ProcType)"glUniformMatrix2dv"); // ARB_gpu_shader_fp64
	_glUniformMatrix3dv_ptr = (PFNGLUNIFORMMATRIX3DVPROC)glGetProcAddress((ProcType)"glUniformMatrix3dv"); // ARB_gpu_shader_fp64
	_glUniformMatrix4dv_ptr = (PFNGLUNIFORMMATRIX4DVPROC)glGetProcAddress((ProcType)"glUniformMatrix4dv"); // ARB_gpu_shader_fp64
	_glUniformMatrix2x3dv_ptr = (PFNGLUNIFORMMATRIX2X3DVPROC)glGetProcAddress((ProcType)"glUniformMatrix2x3dv"); // ARB_gpu_shader_fp64
	_glUniformMatrix2x4dv_ptr = (PFNGLUNIFORMMATRIX2X4DVPROC)glGetProcAddress((ProcType)"glUniformMatrix2x4dv"); // ARB_gpu_shader_fp64
	_glUniformMatrix3x2dv_ptr = (PFNGLUNIFORMMATRIX3X2DVPROC)glGetProcAddress((ProcType)"glUniformMatrix3x2dv"); // ARB_gpu_shader_fp64
	_glUniformMatrix3x4dv_ptr = (PFNGLUNIFORMMATRIX3X4DVPROC)glGetProcAddress((ProcType)"glUniformMatrix3x4dv"); // ARB_gpu_shader_fp64
	_glUniformMatrix4x2dv_ptr = (PFNGLUNIFORMMATRIX4X2DVPROC)glGetProcAddress((ProcType)"glUniformMatrix4x2dv"); // ARB_gpu_shader_fp64
	_glUniformMatrix4x3dv_ptr = (PFNGLUNIFORMMATRIX4X3DVPROC)glGetProcAddress((ProcType)"glUniformMatrix4x3dv"); // ARB_gpu_shader_fp64
	_glGetUniformdv_ptr = (PFNGLGETUNIFORMDVPROC)glGetProcAddress((ProcType)"glGetUniformdv"); // ARB_gpu_shader_fp64
	_glGetSubroutineUniformLocation_ptr = (PFNGLGETSUBROUTINEUNIFORMLOCATIONPROC)glGetProcAddress((ProcType)"glGetSubroutineUniformLocation"); // ARB_shader_subroutine
	_glGetSubroutineIndex_ptr = (PFNGLGETSUBROUTINEINDEXPROC)glGetProcAddress((ProcType)"glGetSubroutineIndex"); // ARB_shader_subroutine
	_glGetActiveSubroutineUniformiv_ptr = (PFNGLGETACTIVESUBROUTINEUNIFORMIVPROC)glGetProcAddress((ProcType)"glGetActiveSubroutineUniformiv"); // ARB_shader_subroutine
	_glGetActiveSubroutineUniformName_ptr = (PFNGLGETACTIVESUBROUTINEUNIFORMNAMEPROC)glGetProcAddress((ProcType)"glGetActiveSubroutineUniformName"); // ARB_shader_subroutine
	_glGetActiveSubroutineName_ptr = (PFNGLGETACTIVESUBROUTINENAMEPROC)glGetProcAddress((ProcType)"glGetActiveSubroutineName"); // ARB_shader_subroutine
	_glUniformSubroutinesuiv_ptr = (PFNGLUNIFORMSUBROUTINESUIVPROC)glGetProcAddress((ProcType)"glUniformSubroutinesuiv"); // ARB_shader_subroutine
	_glGetUniformSubroutineuiv_ptr = (PFNGLGETUNIFORMSUBROUTINEUIVPROC)glGetProcAddress((ProcType)"glGetUniformSubroutineuiv"); // ARB_shader_subroutine
	_glGetProgramStageiv_ptr = (PFNGLGETPROGRAMSTAGEIVPROC)glGetProcAddress((ProcType)"glGetProgramStageiv"); // ARB_shader_subroutine
	_glPatchParameteri_ptr = (PFNGLPATCHPARAMETERIPROC)glGetProcAddress((ProcType)"glPatchParameteri"); // ARB_tessellation_shader
	_glPatchParameterfv_ptr = (PFNGLPATCHPARAMETERFVPROC)glGetProcAddress((ProcType)"glPatchParameterfv"); // ARB_tessellation_shader
	_glBindTransformFeedback_ptr = (PFNGLBINDTRANSFORMFEEDBACKPROC)glGetProcAddress((ProcType)"glBindTransformFeedback"); // ARB_transform_feedback2
	_glDeleteTransformFeedbacks_ptr = (PFNGLDELETETRANSFORMFEEDBACKSPROC)glGetProcAddress((ProcType)"glDeleteTransformFeedbacks"); // ARB_transform_feedback2
	_glGenTransformFeedbacks_ptr = (PFNGLGENTRANSFORMFEEDBACKSPROC)glGetProcAddress((ProcType)"glGenTransformFeedbacks"); // ARB_transform_feedback2
	_glIsTransformFeedback_ptr = (PFNGLISTRANSFORMFEEDBACKPROC)glGetProcAddress((ProcType)"glIsTransformFeedback"); // ARB_transform_feedback2
	_glPauseTransformFeedback_ptr = (PFNGLPAUSETRANSFORMFEEDBACKPROC)glGetProcAddress((ProcType)"glPauseTransformFeedback"); // ARB_transform_feedback2
	_glResumeTransformFeedback_ptr = (PFNGLRESUMETRANSFORMFEEDBACKPROC)glGetProcAddress((ProcType)"glResumeTransformFeedback"); // ARB_transform_feedback2
	_glDrawTransformFeedback_ptr = (PFNGLDRAWTRANSFORMFEEDBACKPROC)glGetProcAddress((ProcType)"glDrawTransformFeedback"); // ARB_transform_feedback2
	_glDrawTransformFeedbackStream_ptr = (PFNGLDRAWTRANSFORMFEEDBACKSTREAMPROC)glGetProcAddress((ProcType)"glDrawTransformFeedbackStream"); // ARB_transform_feedback3
	_glBeginQueryIndexed_ptr = (PFNGLBEGINQUERYINDEXEDPROC)glGetProcAddress((ProcType)"glBeginQueryIndexed"); // ARB_transform_feedback3
	_glEndQueryIndexed_ptr = (PFNGLENDQUERYINDEXEDPROC)glGetProcAddress((ProcType)"glEndQueryIndexed"); // ARB_transform_feedback3
	_glGetQueryIndexediv_ptr = (PFNGLGETQUERYINDEXEDIVPROC)glGetProcAddress((ProcType)"glGetQueryIndexediv"); // ARB_transform_feedback3

	// OpenGL 4.1
	_glReleaseShaderCompiler_ptr = (PFNGLRELEASESHADERCOMPILERPROC)glGetProcAddress((ProcType)"glReleaseShaderCompiler"); // ARB_ES2_compatibility
	_glShaderBinary_ptr = (PFNGLSHADERBINARYPROC)glGetProcAddress((ProcType)"glShaderBinary"); // ARB_ES2_compatibility
	_glGetShaderPrecisionFormat_ptr = (PFNGLGETSHADERPRECISIONFORMATPROC)glGetProcAddress((ProcType)"glGetShaderPrecisionFormat"); // ARB_ES2_compatibility
	_glDepthRangef_ptr = (PFNGLDEPTHRANGEFPROC)glGetProcAddress((ProcType)"glDepthRangef"); // ARB_ES2_compatibility
	_glClearDepthf_ptr = (PFNGLCLEARDEPTHFPROC)glGetProcAddress((ProcType)"glClearDepthf"); // ARB_ES2_compatibility
	_glGetProgramBinary_ptr = (PFNGLGETPROGRAMBINARYPROC)glGetProcAddress((ProcType)"glGetProgramBinary"); // ARB_get_program_binary
	_glProgramBinary_ptr = (PFNGLPROGRAMBINARYPROC)glGetProcAddress((ProcType)"glProgramBinary"); // ARB_get_program_binary
	_glProgramParameteri_ptr = (PFNGLPROGRAMPARAMETERIPROC)glGetProcAddress((ProcType)"glProgramParameteri"); // ARB_get_program_binary
	_glUseProgramStages_ptr = (PFNGLUSEPROGRAMSTAGESPROC)glGetProcAddress((ProcType)"glUseProgramStages"); // ARB_separate_shader_objects
	_glActiveShaderProgram_ptr = (PFNGLACTIVESHADERPROGRAMPROC)glGetProcAddress((ProcType)"glActiveShaderProgram"); // ARB_separate_shader_objects
	_glCreateShaderProgramv_ptr = (PFNGLCREATESHADERPROGRAMVPROC)glGetProcAddress((ProcType)"glCreateShaderProgramv"); // ARB_separate_shader_objects
	_glBindProgramPipeline_ptr = (PFNGLBINDPROGRAMPIPELINEPROC)glGetProcAddress((ProcType)"glBindProgramPipeline"); // ARB_separate_shader_objects
	_glDeleteProgramPipelines_ptr = (PFNGLDELETEPROGRAMPIPELINESPROC)glGetProcAddress((ProcType)"glDeleteProgramPipelines"); // ARB_separate_shader_objects
	_glGenProgramPipelines_ptr = (PFNGLGENPROGRAMPIPELINESPROC)glGetProcAddress((ProcType)"glGenProgramPipelines"); // ARB_separate_shader_objects
	_glIsProgramPipeline_ptr = (PFNGLISPROGRAMPIPELINEPROC)glGetProcAddress((ProcType)"glIsProgramPipeline"); // ARB_separate_shader_objects
	_glGetProgramPipelineiv_ptr = (PFNGLGETPROGRAMPIPELINEIVPROC)glGetProcAddress((ProcType)"glGetProgramPipelineiv"); // ARB_separate_shader_objects
	_glProgramUniform1i_ptr = (PFNGLPROGRAMUNIFORM1IPROC)glGetProcAddress((ProcType)"glProgramUniform1i"); // ARB_separate_shader_objects
	_glProgramUniform1iv_ptr = (PFNGLPROGRAMUNIFORM1IVPROC)glGetProcAddress((ProcType)"glProgramUniform1iv"); // ARB_separate_shader_objects
	_glProgramUniform1f_ptr = (PFNGLPROGRAMUNIFORM1FPROC)glGetProcAddress((ProcType)"glProgramUniform1f"); // ARB_separate_shader_objects
	_glProgramUniform1fv_ptr = (PFNGLPROGRAMUNIFORM1FVPROC)glGetProcAddress((ProcType)"glProgramUniform1fv"); // ARB_separate_shader_objects
	_glProgramUniform1d_ptr = (PFNGLPROGRAMUNIFORM1DPROC)glGetProcAddress((ProcType)"glProgramUniform1d"); // ARB_separate_shader_objects
	_glProgramUniform1dv_ptr = (PFNGLPROGRAMUNIFORM1DVPROC)glGetProcAddress((ProcType)"glProgramUniform1dv"); // ARB_separate_shader_objects
	_glProgramUniform1ui_ptr = (PFNGLPROGRAMUNIFORM1UIPROC)glGetProcAddress((ProcType)"glProgramUniform1ui"); // ARB_separate_shader_objects
	_glProgramUniform1uiv_ptr = (PFNGLPROGRAMUNIFORM1UIVPROC)glGetProcAddress((ProcType)"glProgramUniform1uiv"); // ARB_separate_shader_objects
	_glProgramUniform2i_ptr = (PFNGLPROGRAMUNIFORM2IPROC)glGetProcAddress((ProcType)"glProgramUniform2i"); // ARB_separate_shader_objects
	_glProgramUniform2iv_ptr = (PFNGLPROGRAMUNIFORM2IVPROC)glGetProcAddress((ProcType)"glProgramUniform2iv"); // ARB_separate_shader_objects
	_glProgramUniform2f_ptr = (PFNGLPROGRAMUNIFORM2FPROC)glGetProcAddress((ProcType)"glProgramUniform2f"); // ARB_separate_shader_objects
	_glProgramUniform2fv_ptr = (PFNGLPROGRAMUNIFORM2FVPROC)glGetProcAddress((ProcType)"glProgramUniform2fv"); // ARB_separate_shader_objects
	_glProgramUniform2d_ptr = (PFNGLPROGRAMUNIFORM2DPROC)glGetProcAddress((ProcType)"glProgramUniform2d"); // ARB_separate_shader_objects
	_glProgramUniform2dv_ptr = (PFNGLPROGRAMUNIFORM2DVPROC)glGetProcAddress((ProcType)"glProgramUniform2dv"); // ARB_separate_shader_objects
	_glProgramUniform2ui_ptr = (PFNGLPROGRAMUNIFORM2UIPROC)glGetProcAddress((ProcType)"glProgramUniform2ui"); // ARB_separate_shader_objects
	_glProgramUniform2uiv_ptr = (PFNGLPROGRAMUNIFORM2UIVPROC)glGetProcAddress((ProcType)"glProgramUniform2uiv"); // ARB_separate_shader_objects
	_glProgramUniform3i_ptr = (PFNGLPROGRAMUNIFORM3IPROC)glGetProcAddress((ProcType)"glProgramUniform3i"); // ARB_separate_shader_objects
	_glProgramUniform3iv_ptr = (PFNGLPROGRAMUNIFORM3IVPROC)glGetProcAddress((ProcType)"glProgramUniform3iv"); // ARB_separate_shader_objects
	_glProgramUniform3f_ptr = (PFNGLPROGRAMUNIFORM3FPROC)glGetProcAddress((ProcType)"glProgramUniform3f"); // ARB_separate_shader_objects
	_glProgramUniform3fv_ptr = (PFNGLPROGRAMUNIFORM3FVPROC)glGetProcAddress((ProcType)"glProgramUniform3fv"); // ARB_separate_shader_objects
	_glProgramUniform3d_ptr = (PFNGLPROGRAMUNIFORM3DPROC)glGetProcAddress((ProcType)"glProgramUniform3d"); // ARB_separate_shader_objects
	_glProgramUniform3dv_ptr = (PFNGLPROGRAMUNIFORM3DVPROC)glGetProcAddress((ProcType)"glProgramUniform3dv"); // ARB_separate_shader_objects
	_glProgramUniform3ui_ptr = (PFNGLPROGRAMUNIFORM3UIPROC)glGetProcAddress((ProcType)"glProgramUniform3ui"); // ARB_separate_shader_objects
	_glProgramUniform3uiv_ptr = (PFNGLPROGRAMUNIFORM3UIVPROC)glGetProcAddress((ProcType)"glProgramUniform3uiv"); // ARB_separate_shader_objects
	_glProgramUniform4i_ptr = (PFNGLPROGRAMUNIFORM4IPROC)glGetProcAddress((ProcType)"glProgramUniform4i"); // ARB_separate_shader_objects
	_glProgramUniform4iv_ptr = (PFNGLPROGRAMUNIFORM4IVPROC)glGetProcAddress((ProcType)"glProgramUniform4iv"); // ARB_separate_shader_objects
	_glProgramUniform4f_ptr = (PFNGLPROGRAMUNIFORM4FPROC)glGetProcAddress((ProcType)"glProgramUniform4f"); // ARB_separate_shader_objects
	_glProgramUniform4fv_ptr = (PFNGLPROGRAMUNIFORM4FVPROC)glGetProcAddress((ProcType)"glProgramUniform4fv"); // ARB_separate_shader_objects
	_glProgramUniform4d_ptr = (PFNGLPROGRAMUNIFORM4DPROC)glGetProcAddress((ProcType)"glProgramUniform4d"); // ARB_separate_shader_objects
	_glProgramUniform4dv_ptr = (PFNGLPROGRAMUNIFORM4DVPROC)glGetProcAddress((ProcType)"glProgramUniform4dv"); // ARB_separate_shader_objects
	_glProgramUniform4ui_ptr = (PFNGLPROGRAMUNIFORM4UIPROC)glGetProcAddress((ProcType)"glProgramUniform4ui"); // ARB_separate_shader_objects
	_glProgramUniform4uiv_ptr = (PFNGLPROGRAMUNIFORM4UIVPROC)glGetProcAddress((ProcType)"glProgramUniform4uiv"); // ARB_separate_shader_objects
	_glProgramUniformMatrix2fv_ptr = (PFNGLPROGRAMUNIFORMMATRIX2FVPROC)glGetProcAddress((ProcType)"glProgramUniformMatrix2fv"); // ARB_separate_shader_objects
	_glProgramUniformMatrix3fv_ptr = (PFNGLPROGRAMUNIFORMMATRIX3FVPROC)glGetProcAddress((ProcType)"glProgramUniformMatrix3fv"); // ARB_separate_shader_objects
	_glProgramUniformMatrix4fv_ptr = (PFNGLPROGRAMUNIFORMMATRIX4FVPROC)glGetProcAddress((ProcType)"glProgramUniformMatrix4fv"); // ARB_separate_shader_objects
	_glProgramUniformMatrix2dv_ptr = (PFNGLPROGRAMUNIFORMMATRIX2DVPROC)glGetProcAddress((ProcType)"glProgramUniformMatrix2dv"); // ARB_separate_shader_objects
	_glProgramUniformMatrix3dv_ptr = (PFNGLPROGRAMUNIFORMMATRIX3DVPROC)glGetProcAddress((ProcType)"glProgramUniformMatrix3dv"); // ARB_separate_shader_objects
	_glProgramUniformMatrix4dv_ptr = (PFNGLPROGRAMUNIFORMMATRIX4DVPROC)glGetProcAddress((ProcType)"glProgramUniformMatrix4dv"); // ARB_separate_shader_objects
	_glProgramUniformMatrix2x3fv_ptr = (PFNGLPROGRAMUNIFORMMATRIX2X3FVPROC)glGetProcAddress((ProcType)"glProgramUniformMatrix2x3fv"); // ARB_separate_shader_objects
	_glProgramUniformMatrix3x2fv_ptr = (PFNGLPROGRAMUNIFORMMATRIX3X2FVPROC)glGetProcAddress((ProcType)"glProgramUniformMatrix3x2fv"); // ARB_separate_shader_objects
	_glProgramUniformMatrix2x4fv_ptr = (PFNGLPROGRAMUNIFORMMATRIX2X4FVPROC)glGetProcAddress((ProcType)"glProgramUniformMatrix2x4fv"); // ARB_separate_shader_objects
	_glProgramUniformMatrix4x2fv_ptr = (PFNGLPROGRAMUNIFORMMATRIX4X2FVPROC)glGetProcAddress((ProcType)"glProgramUniformMatrix4x2fv"); // ARB_separate_shader_objects
	_glProgramUniformMatrix3x4fv_ptr = (PFNGLPROGRAMUNIFORMMATRIX3X4FVPROC)glGetProcAddress((ProcType)"glProgramUniformMatrix3x4fv"); // ARB_separate_shader_objects
	_glProgramUniformMatrix4x3fv_ptr = (PFNGLPROGRAMUNIFORMMATRIX4X3FVPROC)glGetProcAddress((ProcType)"glProgramUniformMatrix4x3fv"); // ARB_separate_shader_objects
	_glProgramUniformMatrix2x3dv_ptr = (PFNGLPROGRAMUNIFORMMATRIX2X3DVPROC)glGetProcAddress((ProcType)"glProgramUniformMatrix2x3dv"); // ARB_separate_shader_objects
	_glProgramUniformMatrix3x2dv_ptr = (PFNGLPROGRAMUNIFORMMATRIX3X2DVPROC)glGetProcAddress((ProcType)"glProgramUniformMatrix3x2dv"); // ARB_separate_shader_objects
	_glProgramUniformMatrix2x4dv_ptr = (PFNGLPROGRAMUNIFORMMATRIX2X4DVPROC)glGetProcAddress((ProcType)"glProgramUniformMatrix2x4dv"); // ARB_separate_shader_objects
	_glProgramUniformMatrix4x2dv_ptr = (PFNGLPROGRAMUNIFORMMATRIX4X2DVPROC)glGetProcAddress((ProcType)"glProgramUniformMatrix4x2dv"); // ARB_separate_shader_objects
	_glProgramUniformMatrix3x4dv_ptr = (PFNGLPROGRAMUNIFORMMATRIX3X4DVPROC)glGetProcAddress((ProcType)"glProgramUniformMatrix3x4dv"); // ARB_separate_shader_objects
	_glProgramUniformMatrix4x3dv_ptr = (PFNGLPROGRAMUNIFORMMATRIX4X3DVPROC)glGetProcAddress((ProcType)"glProgramUniformMatrix4x3dv"); // ARB_separate_shader_objects
	_glValidateProgramPipeline_ptr = (PFNGLVALIDATEPROGRAMPIPELINEPROC)glGetProcAddress((ProcType)"glValidateProgramPipeline"); // ARB_separate_shader_objects
	_glGetProgramPipelineInfoLog_ptr = (PFNGLGETPROGRAMPIPELINEINFOLOGPROC)glGetProcAddress((ProcType)"glGetProgramPipelineInfoLog"); // ARB_separate_shader_objects
	_glVertexAttribL1d_ptr = (PFNGLVERTEXATTRIBL1DPROC)glGetProcAddress((ProcType)"glVertexAttribL1d"); // ARB_vertex_attrib_64bit
	_glVertexAttribL2d_ptr = (PFNGLVERTEXATTRIBL2DPROC)glGetProcAddress((ProcType)"glVertexAttribL2d"); // ARB_vertex_attrib_64bit
	_glVertexAttribL3d_ptr = (PFNGLVERTEXATTRIBL3DPROC)glGetProcAddress((ProcType)"glVertexAttribL3d"); // ARB_vertex_attrib_64bit
	_glVertexAttribL4d_ptr = (PFNGLVERTEXATTRIBL4DPROC)glGetProcAddress((ProcType)"glVertexAttribL4d"); // ARB_vertex_attrib_64bit
	_glVertexAttribL1dv_ptr = (PFNGLVERTEXATTRIBL1DVPROC)glGetProcAddress((ProcType)"glVertexAttribL1dv"); // ARB_vertex_attrib_64bit
	_glVertexAttribL2dv_ptr = (PFNGLVERTEXATTRIBL2DVPROC)glGetProcAddress((ProcType)"glVertexAttribL2dv"); // ARB_vertex_attrib_64bit
	_glVertexAttribL3dv_ptr = (PFNGLVERTEXATTRIBL3DVPROC)glGetProcAddress((ProcType)"glVertexAttribL3dv"); // ARB_vertex_attrib_64bit
	_glVertexAttribL4dv_ptr = (PFNGLVERTEXATTRIBL4DVPROC)glGetProcAddress((ProcType)"glVertexAttribL4dv"); // ARB_vertex_attrib_64bit
	_glVertexAttribLPointer_ptr = (PFNGLVERTEXATTRIBLPOINTERPROC)glGetProcAddress((ProcType)"glVertexAttribLPointer"); // ARB_vertex_attrib_64bit
	_glGetVertexAttribLdv_ptr = (PFNGLGETVERTEXATTRIBLDVPROC)glGetProcAddress((ProcType)"glGetVertexAttribLdv"); // ARB_vertex_attrib_64bit
	_glViewportArrayv_ptr = (PFNGLVIEWPORTARRAYVPROC)glGetProcAddress((ProcType)"glViewportArrayv"); // ARB_viewport_array
	_glViewportIndexedf_ptr = (PFNGLVIEWPORTINDEXEDFPROC)glGetProcAddress((ProcType)"glViewportIndexedf"); // ARB_viewport_array
	_glViewportIndexedfv_ptr = (PFNGLVIEWPORTINDEXEDFVPROC)glGetProcAddress((ProcType)"glViewportIndexedfv"); // ARB_viewport_array
	_glScissorArrayv_ptr = (PFNGLSCISSORARRAYVPROC)glGetProcAddress((ProcType)"glScissorArrayv"); // ARB_viewport_array
	_glScissorIndexed_ptr = (PFNGLSCISSORINDEXEDPROC)glGetProcAddress((ProcType)"glScissorIndexed"); // ARB_viewport_array
	_glScissorIndexedv_ptr = (PFNGLSCISSORINDEXEDVPROC)glGetProcAddress((ProcType)"glScissorIndexedv"); // ARB_viewport_array
	_glDepthRangeArrayv_ptr = (PFNGLDEPTHRANGEARRAYVPROC)glGetProcAddress((ProcType)"glDepthRangeArrayv"); // ARB_viewport_array
	_glDepthRangeIndexed_ptr = (PFNGLDEPTHRANGEINDEXEDPROC)glGetProcAddress((ProcType)"glDepthRangeIndexed"); // ARB_viewport_array
	_glGetFloati_v_ptr = (PFNGLGETFLOATI_VPROC)glGetProcAddress((ProcType)"glGetFloati_v"); // ARB_viewport_array
	_glGetDoublei_v_ptr = (PFNGLGETDOUBLEI_VPROC)glGetProcAddress((ProcType)"glGetDoublei_v"); // ARB_viewport_array
#endif // !__APPLE__

	check_gl_funcs();
}
