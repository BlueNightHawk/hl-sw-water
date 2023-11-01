#pragma once

#include "PlatformHeaders.h"
#include "SDL2/SDL_opengl.h"
#include "com_model.h"


#include <array>
#include <functional>
#include "Utils.hpp"

typedef struct lump_s
{
	int fileofs;
	int filelen;
} lump_t;

typedef texture_t* (*_pfn_R_TextureAnimation)(msurface_t* s);
typedef int (*_pfn_EmitWaterPolys)(msurface_t *fa, int direction);
typedef int (*_pfn_D_SetFadeColor)(int r, int g, int b, int fog);

inline _pfn_R_TextureAnimation ORIG_R_TextureAnimation;
inline _pfn_EmitWaterPolys ORIG_EmitWaterPolys;
inline _pfn_D_SetFadeColor ORIG_D_SetFadeColor;


inline funchook_t* pFuncHook;
extern Utils utils;

#define FIND(x)                                                               \
	{                                                                           \
		utils.FindAsync(ORIG_##x, patterns::engine::##x);                           \
		if (!ORIG_##x)                                                              \
		{                                                                       \
			gEngfuncs.Con_DPrintf("Could not hook %s!\n", #x);                  \
		}                                                                       \
		else                                                                    \
		{                                                                       \
			gEngfuncs.Con_DPrintf("Succesfully found %s at 0x%p!\n", #x, ORIG_##x); \
		}                                                                       \
	}

#define HOOK(x)                                                                  \
	{                                                                            \
		utils.FindAsync(ORIG_##x, patterns::engine::##x);                            \
		if (!ORIG_##x)                                                               \
		{                                                                        \
			gEngfuncs.Con_DPrintf("Could not hook %s!\n", #x);                   \
		}                                                                        \
		else                                                                     \
		{                                                                        \
			funchook_prepare(pFuncHook, (void**)&ORIG_##x, x);              \
			gEngfuncs.Con_DPrintf("Succesfully hooked %s at 0x%p!\n", #x, ORIG_##x); \
		}                                                                        \
	}