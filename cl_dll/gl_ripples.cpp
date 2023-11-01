// Software Water RIPPLE CODE
// Credits : Xash3D FWGS Team
//			 Original pull request : (https://github.com/FWGS/xash3d-fwgs/pull/1475)
//			 by a1batross (https://github.com/a1batross)

#include "PlatformHeaders.h"
#include "SDL2/SDL_opengl.h"
#include <vector>
#include <algorithm>

#include "hud.h"
#include "cl_util.h"
#include "com_model.h"
#include <string>

#include "gl_ripples.h"

#define RIPPLES_CACHEWIDTH_BITS 7
#define RIPPLES_CACHEWIDTH (1 << RIPPLES_CACHEWIDTH_BITS)
#define RIPPLES_CACHEWIDTH_MASK ((RIPPLES_CACHEWIDTH)-1)
#define RIPPLES_TEXSIZE (RIPPLES_CACHEWIDTH * RIPPLES_CACHEWIDTH)
#define RIPPLES_TEXSIZE_MASK (RIPPLES_TEXSIZE - 1)

static struct
{
	double time;
	double oldtime;

	short *curbuf, *oldbuf;
	short buf[2][RIPPLES_TEXSIZE];
	bool update;

	uint texture[RIPPLES_TEXSIZE]; // might be a gl_texture_t
	int gl_texturenum;
	GLuint rippletexturenum;
	float texturescale; // not all textures are 128x128, scale the texcoords down

	unsigned char* buffer[RIPPLES_TEXSIZE];
} g_ripple;

/*
============================================================
	HALF-LIFE SOFTWARE WATER
============================================================
*/
void R_ResetRipples(void)
{
	g_ripple.curbuf = g_ripple.buf[0];
	g_ripple.oldbuf = g_ripple.buf[1];
	g_ripple.time = g_ripple.oldtime = gEngfuncs.GetClientTime() - 0.1;
	memset(g_ripple.buf, 0, sizeof(g_ripple.buf));
}

void R_InitRipples(void)
{
	glGenTextures(1, &g_ripple.rippletexturenum);
	glBindTexture(GL_TEXTURE_2D, g_ripple.rippletexturenum);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, RIPPLES_CACHEWIDTH, RIPPLES_CACHEWIDTH, 0, GL_RGBA, GL_UNSIGNED_BYTE, g_ripple.buffer);

	r_ripple = gEngfuncs.pfnRegisterVariable("r_ripple", "1", FCVAR_ARCHIVE);
	r_ripple_updatetime = gEngfuncs.pfnRegisterVariable("r_ripple_updatetime", "0.05", FCVAR_ARCHIVE);
	r_ripple_spawntime = gEngfuncs.pfnRegisterVariable("r_ripple_spawntime", "0.1", FCVAR_ARCHIVE);
	r_ripple_waves = gEngfuncs.pfnRegisterVariable("r_ripple_waves", "1", FCVAR_ARCHIVE);
	r_ripple_tex_nearest = gEngfuncs.pfnRegisterVariable("r_ripple_tex_nearest", "0", FCVAR_ARCHIVE);
}

static void R_SwapBufs(void)
{
	short* tempbufp = g_ripple.curbuf;
	g_ripple.curbuf = g_ripple.oldbuf;
	g_ripple.oldbuf = tempbufp;
}

static void R_SpawnNewRipple(int x, int y, short val)
{
#define PIXEL(x, y) (((x)&RIPPLES_CACHEWIDTH_MASK) + (((y)&RIPPLES_CACHEWIDTH_MASK) << 7))
	g_ripple.oldbuf[PIXEL(x, y)] += val;

	val >>= 2;
	g_ripple.oldbuf[PIXEL(x + 1, y)] += val;
	g_ripple.oldbuf[PIXEL(x - 1, y)] += val;
	g_ripple.oldbuf[PIXEL(x, y + 1)] += val;
	g_ripple.oldbuf[PIXEL(x, y - 1)] += val;
#undef PIXEL
}

static void R_RunRipplesAnimation(const short* oldbuf, short* pbuf)
{
	size_t i = 0;
	const int w = RIPPLES_CACHEWIDTH;
	const int m = RIPPLES_TEXSIZE_MASK;

	for (i = w; i < m + w; i++, pbuf++)
	{
		*pbuf = (((int)oldbuf[(i - (w * 2)) & m] + (int)oldbuf[(i - (w + 1)) & m] + (int)oldbuf[(i - (w - 1)) & m] + (int)oldbuf[(i)&m]) >> 1) - (int)*pbuf;

		*pbuf -= (*pbuf >> 6);
	}
}

static int MostSignificantBit(unsigned int v)
{
#if __GNUC__
	return 31 - __builtin_clz(v);
#else
	int i;
	for (i = 0, v >>= 1; v; v >>= 1, i++)
		;
	return i;
#endif
}

void R_AnimateRipples(void)
{
	double frametime = gEngfuncs.GetClientTime() - g_ripple.time;

	g_ripple.update = (r_ripple->value > 0) && frametime >= r_ripple_updatetime->value;

	if (!g_ripple.update)
		return;

	g_ripple.time = gEngfuncs.GetClientTime();

	R_SwapBufs();

	if (g_ripple.time - g_ripple.oldtime > r_ripple_spawntime->value)
	{
		int x, y, val;

		g_ripple.oldtime = g_ripple.time;

		x = rand() & 0x7fff;
		y = rand() & 0x7fff;
		val = rand() & 0x3ff;

		R_SpawnNewRipple(x, y, val);
	}

	R_RunRipplesAnimation(g_ripple.oldbuf, g_ripple.curbuf);
}

cvar_s* gl_texturemode;

void R_UpdateRippleTexParams(void)
{
	glBindTexture(GL_TEXTURE0, g_ripple.rippletexturenum);

	if (!gl_texturemode)
	{
		gl_texturemode = gEngfuncs.pfnGetCvarPointer("gl_texturemode");
		return;
	}

	if ((int)r_ripple_tex_nearest->value > 0 || !(strnicmp(gl_texturemode->string, "GL_NEAREST", 10)))
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	}
	else
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}
}


void R_UploadRipples(struct texture_s* image)
{
	R_UpdateRippleTexParams();

	uint pixels[RIPPLES_TEXSIZE];
	int wbits, wmask, wshft;

	// discard unuseful textures
	if (r_ripple->value <= 0 || image->width > RIPPLES_CACHEWIDTH || image->width != image->height)
	{
		return;
	}

	glBindTexture(GL_TEXTURE_2D, image->gl_texturenum);
	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

	glBindTexture(GL_TEXTURE_2D, g_ripple.rippletexturenum);

	// no updates this frame
	if (!g_ripple.update && image->gl_texturenum == g_ripple.gl_texturenum)
		return;

	g_ripple.gl_texturenum = image->gl_texturenum;

	if (r_ripple->value == 1.0f)
	{
		g_ripple.texturescale = V_max(1.0f, image->width / 64.0f);
	}
	else
	{
		g_ripple.texturescale = 1.0f;
	}

	wbits = MostSignificantBit(image->width);
	wshft = 7 - wbits;
	wmask = image->width - 1;

	for (int y = 0; y < image->height; y++)
	{
		int ry = y << (7 + wshft);

		for (int x = 0; x < image->width; x++)
		{
			int rx = x << wshft;
			int val = g_ripple.curbuf[ry + rx] >> 4;

			int py = (y - val) & wmask;
			int px = (x + val) & wmask;
			int p = (py << wbits) + px;

			g_ripple.texture[(y << wbits) + x] = pixels[p];
		}
	}

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, image->width, image->height, 0,
		GL_RGBA, GL_UNSIGNED_BYTE, g_ripple.texture);
}

float GetTextureScale()
{
	return g_ripple.texturescale;
}