#pragma once

// software water
void R_InitRipples(void);
void R_ResetRipples(void);
void R_AnimateRipples(void);
void R_UpdateRippleTexParams(void);
void R_UploadRipples(struct texture_s* image);

inline cvar_t* r_ripple;
inline cvar_t* r_ripple_updatetime;
inline cvar_t* r_ripple_spawntime;
inline cvar_t* r_ripple_waves;
inline cvar_t* r_ripple_tex_nearest;