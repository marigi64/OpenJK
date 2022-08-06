/*
===========================================================================
Copyright (C) 2000 - 2013, Raven Software, Inc.
Copyright (C) 2001 - 2013, Activision, Inc.
Copyright (C) 2013 - 2015, OpenJK contributors

This file is part of the OpenJK source code.

OpenJK is free software; you can redistribute it and/or modify it
under the terms of the GNU General Public License version 2 as
published by the Free Software Foundation.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, see <http://www.gnu.org/licenses/>.
===========================================================================
*/

////////////////////////////////////////////////////////////////////////////////////////
// RAVEN SOFTWARE - STAR WARS: JK II
//  (c) 2002 Activision
//
// World Effects
//
////////////////////////////////////////////////////////////////////////////////////////


#ifdef JK2_MODE

#pragma once
#if !defined __TR_WORLDEFFECTS_H
#define __TR_WORLDEFFECTS_H



class CWorldEffectsSystem;
#include "../qcommon/q_shared.h"		// For Vec3_t





#define PARTICLE_FLAG_RENDER			0x00000001




struct	SParticle
{
	vec3_t		pos;
	vec3_t		velocity;
	unsigned	flags;
};














class CWorldEffect
{
protected:
	CWorldEffect* mNext, * mSlave, * mOwner;
	bool			mEnabled, mIsSlave;

public:
	enum
	{
		WORLDEFFECT_ENABLED = 0,
		WORLDEFFECT_PARTICLES,
		WORLDEFFECT_PARTICLE_COUNT,

		WORLDEFFECT_END
	};

public:
	CWorldEffect(CWorldEffect* owner = 0);
	virtual ~CWorldEffect(void);

	void			SetNext(CWorldEffect* next) { mNext = next; }
	CWorldEffect* GetNext(void) { return mNext; }
	void			SetSlave(CWorldEffect* slave) { mSlave = slave; }
	CWorldEffect* GetSlave(void) { return mSlave; }
	void			AddSlave(CWorldEffect* slave);

	void			SetIsSlave(bool isSlave) { mIsSlave = isSlave; }
	void			SetOwner(CWorldEffect* owner) { mOwner = owner; }

	virtual	bool	Command(const char* command);

	virtual	void	ParmUpdate(CWorldEffectsSystem* system, int which);
	virtual	void	ParmUpdate(CWorldEffect* effect, int which);
	virtual	void	SetVariable(int which, bool newValue, bool doSlave = false);
	virtual	void	SetVariable(int which, float newValue, bool doSlave = false);
	virtual	void	SetVariable(int which, int newValue, bool doSlave = false);
	virtual	void	SetVariable(int which, vec3_t newValue, bool doSlave = false);

	virtual	int			GetIntVariable(int which) { return 0; }
	virtual	SParticle* GetParticleVariable(int which) { return 0; }

	virtual	void	Update(CWorldEffectsSystem* system, float elapseTime);
	virtual	void	Render(CWorldEffectsSystem* system);
};



class CWorldEffectsSystem
{
protected:
	CWorldEffect* mList, * mLast;

public:
	CWorldEffectsSystem(void);
	virtual	~CWorldEffectsSystem(void);

	void	AddWorldEffect(CWorldEffect* effect);

	virtual	int			GetIntVariable(int which) { return 0; }
	virtual	SParticle* GetParticleVariable(int which) { return 0; }
	virtual float		GetFloatVariable(int which) { return 0.0; }
	virtual	float* GetVecVariable(int which) { return 0; }

	virtual	bool	Command(const char* command);

	virtual	void	Update(float elapseTime);
	virtual	void	ParmUpdate(int which);
	virtual	void	Render(void);
};


void R_InitWorldEffects(void);
void R_ShutdownWorldEffects(void);
void RB_RenderWorldEffects(void);

void R_WorldEffectCommand(const char* command);
void R_WorldEffect_f(void);




bool R_GetWindVector(vec3_t windVector);
bool R_GetWindSpeed(float& windSpeed);







bool R_IsRaining();
bool R_IsSnowing();


#endif // __TR_WORLDEFFECTS_H

#else

#pragma once
#if !defined __TR_WORLDEFFECTS_H
#define __TR_WORLDEFFECTS_H

#include "../qcommon/q_shared.h"		// For Vec3_t


////////////////////////////////////////////////////////////////////////////////////////
// Supported Commands
////////////////////////////////////////////////////////////////////////////////////////
void R_AddWeatherZone(vec3_t mins, vec3_t maxs);

void R_InitWorldEffects(void);
void R_ShutdownWorldEffects(void);
void RB_RenderWorldEffects(void);

void R_WorldEffectCommand(const char *command);
void R_WorldEffect_f(void);

////////////////////////////////////////////////////////////////////////////////////////
// Exported Functionality
////////////////////////////////////////////////////////////////////////////////////////
bool R_GetWindVector(vec3_t windVector, vec3_t atpoint);
bool R_GetWindSpeed(float &windSpeed, vec3_t atpoint);
bool R_GetWindGusting(vec3_t atpoint);
bool R_IsOutside(vec3_t pos);
float R_IsOutsideCausingPain(vec3_t pos);
float R_GetChanceOfSaberFizz();
bool R_IsShaking(vec3_t pos);
bool R_SetTempGlobalFogColor(vec3_t color);

bool R_IsRaining();
bool R_IsPuffing();


#endif // __TR_WORLDEFFECTS_H
#endif // JK2_MODE