#include "PlatformHeaders.h"
#include "hud.h"
#include "cl_util.h"
#include "com_model.h"
#include "SDL2/SDL.h"

#include <string>
#include <functional>
#include <future>
#include "MemUtils.hpp"
#include "Utils.hpp"
#include "patterns.hpp"

#include "funchook/include/funchook.h"

#include "gl_ripples.h"
#include "gl_hooks.h"
#include "gl_patterns.h"

#include "studio.h"
#include "r_studioint.h"

extern engine_studio_api_s IEngineStudio;
extern ref_params_s g_params;

constexpr double r_turbsin[] = {
0.000000, 0.098165, 0.196270, 0.294259, 0.392069, 0.489643, 0.586920, 0.683850,
0.780360, 0.876405, 0.971920, 1.066850, 1.161140, 1.254725, 1.347560, 1.439580,
1.530735, 1.620965, 1.710220, 1.798445, 1.885585, 1.971595, 2.056410, 2.139990,
2.222280, 2.303235, 2.382795, 2.460925, 2.537575, 2.612690, 2.686235, 2.758160,
2.828425, 2.896990, 2.963805, 3.028835, 3.092040, 3.153385, 3.212830, 3.270340,
3.325880, 3.379415, 3.430915, 3.480350, 3.527685, 3.572895, 3.615955, 3.656840,
3.695520, 3.731970, 3.766175, 3.798115, 3.827760, 3.855105, 3.880125, 3.902810,
3.923140, 3.941110, 3.956705, 3.969920, 3.980740, 3.989160, 3.995180, 3.998795,
4.000000, 3.998795, 3.995180, 3.989160, 3.980740, 3.969920, 3.956705, 3.941110,
3.923140, 3.902810, 3.880125, 3.855105, 3.827760, 3.798115, 3.766175, 3.731970,
3.695520, 3.656840, 3.615955, 3.572895, 3.527685, 3.480350, 3.430915, 3.379415,
3.325880, 3.270340, 3.212830, 3.153385, 3.092040, 3.028835, 2.963805, 2.896990,
2.828425, 2.758160, 2.686235, 2.612690, 2.537575, 2.460925, 2.382795, 2.303235,
2.222280, 2.139990, 2.056410, 1.971595, 1.885585, 1.798445, 1.710220, 1.620965,
1.530735, 1.439580, 1.347560, 1.254725, 1.161140, 1.066850, 0.971920, 0.876405,
0.780360, 0.683850, 0.586920, 0.489643, 0.392069, 0.294259, 0.196270, 0.098165,
0.000000, -0.098165, -0.196270, -0.294259, -0.392069, -0.489643, -0.586920, -0.683850,
-0.780360, -0.876405, -0.971920, -1.066850, -1.161140, -1.254725, -1.347560, -1.439580,
-1.530735, -1.620965, -1.710220, -1.798445, -1.885585, -1.971595, -2.056410, -2.139990,
-2.222280, -2.303235, -2.382795, -2.460925, -2.537575, -2.612690, -2.686235, -2.758160,
-2.828425, -2.896990, -2.963805, -3.028835, -3.092040, -3.153385, -3.212830, -3.270340,
-3.325880, -3.379415, -3.430915, -3.480350, -3.527685, -3.572895, -3.615955, -3.656840,
-3.695520, -3.731970, -3.766175, -3.798115, -3.827760, -3.855105, -3.880125, -3.902810,
-3.923140, -3.941110, -3.956705, -3.969920, -3.980740, -3.989160, -3.995180, -3.998795,
-4.000000, -3.998795, -3.995180, -3.989160, -3.980740, -3.969920, -3.956705, -3.941110,
-3.923140, -3.902810, -3.880125, -3.855105, -3.827760, -3.798115, -3.766175, -3.731970,
-3.695520, -3.656840, -3.615955, -3.572895, -3.527685, -3.480350, -3.430915, -3.379415,
-3.325880, -3.270340, -3.212830, -3.153385, -3.092040, -3.028835, -2.963805, -2.896990,
-2.828425, -2.758160, -2.686235, -2.612690, -2.537575, -2.460925, -2.382795, -2.303235,
-2.222280, -2.139990, -2.056410, -1.971595, -1.885585, -1.798445, -1.710220, -1.620965,
-1.530735, -1.439580, -1.347560, -1.254725, -1.161140, -1.066850, -0.971920, -0.876405,
-0.780360, -0.683850, -0.586920, -0.489643, -0.392069, -0.294259, -0.196270, -0.098165,
};

funchook* glHook;

Utils utils = Utils::Utils(0, 0, 0);

float GetTextureScale();

#define SUBDIVIDE_SIZE 64
int EmitWaterPolys(msurface_t* warp, int direction)
{
	if ((int)r_ripple->value <= 0)
		return ORIG_EmitWaterPolys(warp, direction);

	auto currententity = IEngineStudio.GetCurrentEntity();

	auto pt = ORIG_R_TextureAnimation(warp);
	unsigned __int8* fogparams;
	float *v, nv;
	float s, t, os, ot;
	glpoly_t* p;

	float waveHeight = 0;

	int i = 0;

	R_UploadRipples(pt);

	if (!warp->polys)
		return 0;

	fogparams = *(unsigned __int8**)(*(DWORD*)(*(DWORD*)((int)warp + 44) + 36) + 68);

	ORIG_D_SetFadeColor(fogparams[9], fogparams[10], fogparams[11], fogparams[12]);

	if (warp->polys->verts[0][2] >= g_params.vieworg[2])
		waveHeight = -currententity->curstate.scale;
	else
		waveHeight = currententity->curstate.scale;

	for (p = warp->polys; p; p = p->next)
	{
		if (direction > 0)
			v = p->verts[0] + (p->numverts - 1) * VERTEXSIZE;
		else
			v = p->verts[0];

		glBegin(GL_POLYGON);

		for (i = 0; i < p->numverts; i++)
		{
			if (waveHeight != 0.0f && (int)r_ripple_waves->value > 0)
			{
				nv = r_turbsin[(int)(gEngfuncs.GetClientTime() * 160.0f + v[1] + v[0]) & 255] + 8.0f;
				nv = (r_turbsin[(int)(v[0] * 5.0f + gEngfuncs.GetClientTime() * 171.0f - v[1]) & 255] + 8.0f) * 0.8f + nv;
				nv = nv * waveHeight + v[2];
			}
			else
				nv = v[2];

			os = v[3];
			ot = v[4];

			s = os / GetTextureScale();
			t = ot / GetTextureScale();
			
			s *= (1.0f / SUBDIVIDE_SIZE);
			t *= (1.0f / SUBDIVIDE_SIZE);

			glTexCoord2f(s, t);
			glVertex3f(v[0], v[1], nv);

			if (direction > 0)
				v -= VERTEXSIZE;
			else
				v += VERTEXSIZE;
		}

		glEnd();
	}

	return i;
}

void InitHooks()
{
	void* handle, *base;
	size_t size;

	if (!MemUtils::GetModuleInfo(L"hw.dll", &handle, &base, &size))
	{
		return;
	}

	utils = Utils::Utils(handle, base, size);

	pFuncHook = funchook_create();
	glHook = funchook_create();

	FIND(R_TextureAnimation);
	HOOK(EmitWaterPolys);
	FIND(D_SetFadeColor);

	funchook_install(pFuncHook, 0);

	R_InitRipples();
}

