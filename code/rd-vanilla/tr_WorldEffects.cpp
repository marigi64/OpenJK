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
//
////////////////////////////////////////////////////////////////////////////////////////
#include "../server/exe_headers.h"

////////////////////////////////////////////////////////////////////////////////////////
// Externs & Fwd Decl.
////////////////////////////////////////////////////////////////////////////////////////
extern void			SetViewportAndScissor( void );

////////////////////////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////////////////////////
#include "tr_local.h"
#include "tr_WorldEffects.h"
#include "../Ravl/CVec.h"
#include "../Ratl/vector_vs.h"
#include "../Ratl/bits_vs.h"


#ifdef JK2_MODE


#pragma warning( disable : 4512 )

inline void VectorMA(vec3_t vecAdd, const float scale, const vec3_t vecScale)
{
	vecAdd[0] += (scale * vecScale[0]);
	vecAdd[1] += (scale * vecScale[1]);
	vecAdd[2] += (scale * vecScale[2]);
}

#define GLS_ALPHA			(GLS_SRCBLEND_SRC_ALPHA | GLS_DSTBLEND_ONE_MINUS_SRC_ALPHA)

static bool debugShowWind = false;
static int	originContents;

extern qboolean ParseVector(const char** text, int count, float* v);
extern void SetViewportAndScissor(void);



float FloatRand(void)
{
	//	char	temp[1000];
	float	result;
	float	r;

	r = (float)rand();
	result = r / (float)RAND_MAX;
	//	sprintf(temp, "%f - %f\n", r, result);
	//	OutputDebugString(temp);

	return result;
}

void MYgluPerspective(GLdouble fovy, GLdouble aspect, GLdouble zNear, GLdouble zFar)
{
	GLdouble xmin, xmax, ymin, ymax;

	ymax = zNear * tan(fovy * M_PI / 360.0);
	ymin = -ymax;

	xmin = ymin * aspect;
	xmax = ymax * aspect;

	qglFrustum(xmin, xmax, ymin, ymax, zNear, zFar);
}







CWorldEffect::CWorldEffect(CWorldEffect* owner) :
	mNext(0),
	mSlave(0),
	mOwner(owner),
	mIsSlave(owner ? true : false),
	mEnabled(true)
{
}

CWorldEffect::~CWorldEffect(void)
{
	if (mIsSlave && mNext)
	{
		delete mNext;
		mNext = 0;
	}
	if (mSlave)
	{
		delete mSlave;
		mSlave = 0;
	}
}


bool CWorldEffect::Command(const char* command)
{
	if (mSlave)
	{
		if (mSlave->Command(command))
		{
			return true;
		}
	}
	if (mIsSlave && mNext)
	{
		if (mNext->Command(command))
		{
			return true;
		}
	}

	return false;
}

void CWorldEffect::ParmUpdate(CWorldEffectsSystem* system, int which)
{
	if (mSlave)
	{
		mSlave->ParmUpdate(system, which);
	}
	if (mIsSlave && mNext)
	{
		mNext->ParmUpdate(system, which);
	}
}

void CWorldEffect::ParmUpdate(CWorldEffect* effect, int which)
{
	if (mSlave)
	{
		mSlave->ParmUpdate(effect, which);
	}
	if (mIsSlave && mNext)
	{
		mNext->ParmUpdate(effect, which);
	}
}

void CWorldEffect::SetVariable(int which, bool newValue, bool doSlave)
{
	if (doSlave)
	{
		mSlave->SetVariable(which, newValue, doSlave);
	}
	if (doSlave && mIsSlave && mNext)
	{
		mNext->SetVariable(which, newValue, doSlave);
	}

	switch (which)
	{
	case WORLDEFFECT_ENABLED:
		mEnabled = newValue;
		break;
	}
}

void CWorldEffect::SetVariable(int which, float newValue, bool doSlave)
{
	if (doSlave)
	{
		mSlave->SetVariable(which, newValue, doSlave);
	}
	if (doSlave && mIsSlave && mNext)
	{
		mNext->SetVariable(which, newValue, doSlave);
	}
}

void CWorldEffect::SetVariable(int which, int newValue, bool doSlave)
{
	if (doSlave)
	{
		mSlave->SetVariable(which, newValue, doSlave);
	}
	if (doSlave && mIsSlave && mNext)
	{
		mNext->SetVariable(which, newValue, doSlave);
	}
}

void CWorldEffect::SetVariable(int which, vec3_t newValue, bool doSlave)
{
	if (doSlave)
	{
		mSlave->SetVariable(which, newValue, doSlave);
	}
	if (doSlave && mIsSlave && mNext)
	{
		mNext->SetVariable(which, newValue, doSlave);
	}
}

void CWorldEffect::AddSlave(CWorldEffect* slave)
{
	slave->SetNext(mSlave);
	mSlave = slave;

	slave->SetIsSlave(true);
	slave->SetOwner(this);
}

void CWorldEffect::Update(CWorldEffectsSystem* system, float elapseTime)
{
	if (mSlave && mEnabled)
	{
		mSlave->Update(system, elapseTime);
	}
	if (mIsSlave && mNext)
	{
		mNext->Update(system, elapseTime);
	}
}

void CWorldEffect::Render(CWorldEffectsSystem* system)
{
	if (mSlave && mEnabled)
	{
		mSlave->Render(system);
	}
	if (mIsSlave && mNext)
	{
		mNext->Render(system);
	}
}












CWorldEffectsSystem::CWorldEffectsSystem(void) :
	mList(0),
	mLast(0)
{
}

CWorldEffectsSystem::~CWorldEffectsSystem(void)
{
	CWorldEffect* next;

	while (mList)
	{
		next = mList->GetNext();
		delete mList;
		mList = next;
	}
}

void CWorldEffectsSystem::AddWorldEffect(CWorldEffect* effect)
{
	if (!mList)
	{
		mList = mLast = effect;
	}
	else
	{
		mLast->SetNext(effect);
		mLast = effect;
	}
}

bool CWorldEffectsSystem::Command(const char* command)
{
	CWorldEffect* current;

	current = mList;
	while (current)
	{
		if (current->Command(command))
		{
			return true;
		}
		current = current->GetNext();
	}

	return false;
}

void CWorldEffectsSystem::Update(float elapseTime)
{
	CWorldEffect* current;

	current = mList;
	while (current)
	{
		current->Update(this, elapseTime);
		current = current->GetNext();
	}
}

void CWorldEffectsSystem::ParmUpdate(int which)
{
	CWorldEffect* current;

	current = mList;
	while (current)
	{
		current->ParmUpdate(this, which);
		current = current->GetNext();
	}
}

void CWorldEffectsSystem::Render(void)
{
	CWorldEffect* current;

	current = mList;
	while (current)
	{
		current->Render(this);
		current = current->GetNext();
	}
}









class CRainSystem : public CWorldEffectsSystem
{
private:
	// configurable
	int			mMaxRain;
	float		mRainHeight;
	vec3_t		mSpread;
	float		mAlpha;
	float		mWindAngle;

	image_t* mImage;
	vec3_t		mMinVelocity, mMaxVelocity;
	int			mNextWindGust, mWindDuration, mWindLow;
	float		mWindMin, mWindMax;
	vec3_t		mWindDirection, mNewWindDirection, mWindSpeed;
	int			mWindChange;

	SParticle* mRainList;
	float		mFadeAlpha;
	bool		mIsRaining;

public:
	enum
	{
		RAINSYSTEM_WIND_DIRECTION,
		RAINSYSTEM_WIND_SPEED,
	};

public:
	CRainSystem(int maxRain);
	~CRainSystem(void);

	virtual	int			GetIntVariable(int which);
	virtual	SParticle* GetParticleVariable(int which);
	virtual float		GetFloatVariable(int which);
	virtual	float* GetVecVariable(int which);

	virtual	bool	Command(const char* command);

	virtual	void	Update(float elapseTime);
	virtual	void	Render(void);

	void	Init(void);

	bool	IsRaining() { return mIsRaining; }
};




class CMistyFog : public CWorldEffect
{
private:
	//	GLuint		mImage;
	//	image_t		*mImage;
	GLfloat		mTextureCoords[2][2];
	GLfloat		mAlpha;
	bool		mAlphaFade, mRendering, mBuddy;
	float		mSpeed, mAlphaDirection;
	float		mCurrentSize, mMinSize, mMaxSize;
	vec3_t		mWindTransform;

	int				mWidth, mHeight;
	unsigned char* mData;

	const	float	mSize;

public:
	enum
	{
		MISTYFOG_RENDERING = WORLDEFFECT_END
	};

public:
	CMistyFog(int index, CWorldEffect* owner = 0, bool buddy = false);

	//			image_t	*GetImage(void) { return mImage; }

	int				GetWidth(void) { return mWidth; }
	int				GetHeight(void) { return mHeight; }
	unsigned char* GetData(void) { return mData; }
	GLfloat			GetTextureCoord(int s, int y) { return mTextureCoords[s][y]; }
	float			GetAlpha(void) { return mAlpha; }
	bool			GetRendering(void) { return mRendering; }

	virtual	void	Update(CWorldEffectsSystem* system, float elapseTime);
	virtual	void	ParmUpdate(CWorldEffectsSystem* system, int which);
	virtual	void	ParmUpdate(CWorldEffect* effect, int which);
	virtual	void	Render(CWorldEffectsSystem* system);

	void	CreateTextureCoords(void);
};

CMistyFog::CMistyFog(int index, CWorldEffect* owner, bool buddy) :
	CWorldEffect(owner),

	mSize(0.05f * 2.0f),
	mMinSize(0.05f * 3.0f),
	mMaxSize(0.15f * 2.0f),
	mAlpha(1.0f),
	mAlphaFade(false),
	mBuddy(buddy)
{
	char			name[1024];
	unsigned char* pos;
	int				x, y;

	if (mBuddy)
	{
		mRendering = false;

		//		mImage = ((CMistyFog *)owner)->GetImage();
		mData = ((CMistyFog*)owner)->GetData();
		mWidth = ((CMistyFog*)owner)->GetWidth();
		mHeight = ((CMistyFog*)owner)->GetHeight();
	}
	else
	{
		sprintf(name, "gfx/world/fog%d.tga", index);

		R_LoadImage(name, &mData, &mWidth, &mHeight);
		if (!mData)
		{
			ri.Error(ERR_DROP, "Could not load %s", name);
		}

		pos = mData;
		for (y = 0; y < mHeight; y++)
		{
			for (x = 0; x < mWidth; x++)
			{
				pos[3] = pos[0];
				pos += 4;
			}
		}

		//mImage = R_CreateImage(name, mData, mWidth, mHeight, false, true, false, GL_REPEAT);

		mRendering = true;
		AddSlave(new CMistyFog(index, this, true));
	}

	mSpeed = 90.0 + FloatRand() * 20.0;

	CreateTextureCoords();
}

void CMistyFog::Update(CWorldEffectsSystem* system, float elapseTime)
{
	bool	removeImage = false;
	float	forwardWind, rightWind;

	CWorldEffect::Update(system, elapseTime);

	if (!mRendering)
	{
		return;
	}

	// translate

	forwardWind = DotProduct(mWindTransform, backEnd.viewParms.ori.axis[0]);
	rightWind = DotProduct(mWindTransform, backEnd.viewParms.ori.axis[1]);

	mTextureCoords[0][0] += rightWind / mSpeed;
	mTextureCoords[1][0] += rightWind / mSpeed;

	mTextureCoords[0][0] -= forwardWind / mSpeed / 4.0;
	mTextureCoords[0][1] -= forwardWind / mSpeed / 4.0;
	mTextureCoords[1][0] += forwardWind / mSpeed / 4.0;
	mTextureCoords[1][1] += forwardWind / mSpeed / 4.0;

	/*	if (mTextureCoords[0][0] > mTextureCoords[1][0] ||
			mTextureCoords[0][1] > mTextureCoords[1][1])
		{

			mAlphaFade = true;
			mAlphaDirection = -1.0;
			mAlpha = -1.0;
		}
	*/
	if ((fabs(mTextureCoords[0][0] - mTextureCoords[1][0]) < mMinSize ||
		fabs(mTextureCoords[0][1] - mTextureCoords[1][1]) < mMinSize))// && forwardWind > 0.0)
	{
		removeImage = true;
	}

	if ((fabs(mTextureCoords[0][0] - mTextureCoords[1][0]) > mMaxSize ||
		fabs(mTextureCoords[0][1] - mTextureCoords[1][1]) > mMaxSize))// && forwardWind < 0.0)
	{
		removeImage = true;
	}

	if (mTextureCoords[0][0] < mCurrentSize || mTextureCoords[0][1] < mCurrentSize ||
		mTextureCoords[0][0] > 1.0 - mCurrentSize || mTextureCoords[0][1] > 1.0 - mCurrentSize)
	{
		//		mAlphaFade = true;
	}
	if (mTextureCoords[1][0] < mCurrentSize || mTextureCoords[1][1] < mCurrentSize ||
		mTextureCoords[1][0] > 1.0 - mCurrentSize || mTextureCoords[1][1] > 1.0 - mCurrentSize)
	{
		//		mAlphaFade = true;
	}

	if (removeImage && !mAlphaFade)
	{
		mAlphaFade = true;
		mAlphaDirection = -0.025f;
		if (mBuddy)
		{
			mOwner->ParmUpdate(this, MISTYFOG_RENDERING);
		}
		else if (mSlave)
		{
			mSlave->ParmUpdate(this, MISTYFOG_RENDERING);
		}
	}

	if (mAlphaFade)
	{
		mAlpha += mAlphaDirection * 0.4;
		if (mAlpha < 0.0)
		{
			mRendering = false;
			mAlpha = 0.0;
		}
		else if (mAlpha >= 1.0)
		{
			mAlphaFade = false;
			mAlpha = 1.0;
		}
	}
}

void CMistyFog::ParmUpdate(CWorldEffectsSystem* system, int which)
{
	CWorldEffect::ParmUpdate(system, which);

	switch (which)
	{
	case CRainSystem::RAINSYSTEM_WIND_DIRECTION:
		VectorCopy(system->GetVecVariable(which), mWindTransform);
		break;
	}
}

void CMistyFog::ParmUpdate(CWorldEffect* effect, int which)
{
	CWorldEffect::ParmUpdate(effect, which);

	switch (which)
	{
	case MISTYFOG_RENDERING:
		if (effect == mOwner || effect == mSlave)
		{
			mAlpha = 0.0;
			mAlphaDirection = 0.025f;
			mAlphaFade = true;
			CreateTextureCoords();
			mRendering = true;
		}
		break;
	}
}

void CMistyFog::Render(CWorldEffectsSystem* system)
{
	CWorldEffect::Render(system);

	/*	if (!mRendering)
		{
			return;
		}

		GL_Bind(mImage);
		GL_State(GLS_SRCBLEND_SRC_ALPHA|GLS_DSTBLEND_ONE);

	//	qglColor4f(1.0, 1.0, 1.0, mAlpha*0.4);

		if (mSlave)
		{
			qglColor4f(1.0, 0.0, 0.0, mAlpha);
		}
		else
		{
			qglColor4f(0.0, 1.0, 0.0, mAlpha);
		}

		qglBegin(GL_QUADS);
		qglTexCoord2f(mTextureCoords[0][0], mTextureCoords[0][1]);
		qglVertex3f(-10, 10, -10);

		qglTexCoord2f(mTextureCoords[1][0], mTextureCoords[0][1]);
		qglVertex3f(10, 10, -10);

		qglTexCoord2f(mTextureCoords[1][0], mTextureCoords[1][1]);
		qglVertex3f(10, -10, -10);

		qglTexCoord2f(mTextureCoords[0][0], mTextureCoords[1][1]);
		qglVertex3f(-10, -10, -10);

		qglEnd();*/
}

void CMistyFog::CreateTextureCoords(void)
{
	float	xStart, yStart;
	float	forwardWind, rightWind;

	mSpeed = 800.0 + FloatRand() * 2000.0;
	mSpeed /= 4.0;

	forwardWind = DotProduct(mWindTransform, backEnd.viewParms.ori.axis[0]);
	rightWind = DotProduct(mWindTransform, backEnd.viewParms.ori.axis[1]);

	if (forwardWind > 0.5)
	{	// moving away, so make the size smaller
		mCurrentSize = mMinSize + (FloatRand() * mMinSize * 0.01);
		//		mCurrentSize = mMinSize / 3.0;
	}
	else if (forwardWind < -0.5)
	{	// moving towards, so make bigger
//		mCurrentSize = (mSize * 0.8) + (FloatRand() * mSize * 0.8);
		mCurrentSize = mMaxSize - (FloatRand() * mMinSize);
	}
	else
	{	// normal range
		mCurrentSize = mMinSize * 1.5 + (FloatRand() * mSize);
	}

	mCurrentSize /= 2.0;

	xStart = (1.0 - mCurrentSize - 0.40) * FloatRand() + 0.20;
	yStart = (1.0 - mCurrentSize - 0.40) * FloatRand() + 0.20;

	mTextureCoords[0][0] = xStart - mCurrentSize;
	mTextureCoords[0][1] = yStart - mCurrentSize;
	mTextureCoords[1][0] = xStart + mCurrentSize;
	mTextureCoords[1][1] = yStart + mCurrentSize;
}









#define	MISTYFOG_WIDTH	30
#define MISTYFOG_HEIGHT	30


class CMistyFog2 : public CWorldEffect
{
protected:
	vec4_t			mColors[MISTYFOG_HEIGHT][MISTYFOG_WIDTH];
	vec3_t			mVerts[MISTYFOG_HEIGHT][MISTYFOG_WIDTH];
	uint32_t		mIndexes[MISTYFOG_HEIGHT - 1][MISTYFOG_WIDTH - 1][4];
	float			mAlpha;

	float			mFadeAlpha;

public:
	CMistyFog2(void);

	virtual	bool	Command(const char* command);

	void	UpdateTexture(CMistyFog* fog);

	virtual	void	Update(CWorldEffectsSystem* system, float elapseTime);
	virtual	void	Render(CWorldEffectsSystem* system);
};


CMistyFog2::CMistyFog2(void) :
	CWorldEffect(),
	mAlpha(0.3f),

	mFadeAlpha(0.0f)
{
	int			x, y;
	float		xStep, yStep;

	AddSlave(new CMistyFog(2));
	AddSlave(new CMistyFog(2));

	xStep = 20.0f / (MISTYFOG_WIDTH - 1);
	yStep = 20.0f / (MISTYFOG_HEIGHT - 1);

	for (y = 0; y < MISTYFOG_HEIGHT; y++)
	{
		for (x = 0; x < MISTYFOG_WIDTH; x++)
		{
			mVerts[y][x][0] = -10 + (x * xStep) + Q_flrand(-xStep / 16.0, xStep / 16.0);
			mVerts[y][x][1] = 10 - (y * yStep) + Q_flrand(-xStep / 16.0, xStep / 16.0);
			mVerts[y][x][2] = -10;

			mColors[y][x][0] = 1.0;
			mColors[y][x][1] = 1.0;
			mColors[y][x][2] = 1.0;

			if (y < MISTYFOG_HEIGHT - 1 && x < MISTYFOG_WIDTH - 1)
			{
				mIndexes[y][x][0] = (y * MISTYFOG_WIDTH) + x;
				mIndexes[y][x][1] = (y * MISTYFOG_WIDTH) + x + 1;
				mIndexes[y][x][2] = ((y + 1) * MISTYFOG_WIDTH) + x + 1;
				mIndexes[y][x][3] = ((y + 1) * MISTYFOG_WIDTH) + x;
			}
		}
	}
}



bool CMistyFog2::Command(const char* command)
{
	char* token;

	if (CWorldEffect::Command(command))
	{
		return true;
	}
	COM_BeginParseSession();
	token = COM_ParseExt(&command, qfalse);
	if (strcmpi(token, "fog") != 0)
	{
		return false;
	}
	COM_BeginParseSession();
	token = COM_ParseExt(&command, qfalse);
	if (strcmpi(token, "density") == 0)
	{
		token = COM_ParseExt(&command, qfalse);
		mAlpha = atof(token);

		return true;
	}

	return false;
}

void CMistyFog2::Update(CWorldEffectsSystem* system, float elapseTime)
{
	CMistyFog* current;
	int			x, y;

	if (originContents & CONTENTS_OUTSIDE && !(originContents & CONTENTS_WATER))
	{
		if (mFadeAlpha < 1.0)
		{
			mFadeAlpha += elapseTime / 2.0;
		}
		if (mFadeAlpha > 1.0)
		{
			mFadeAlpha = 1.0;
		}
	}
	else
	{
		if (mFadeAlpha > 0.0)
		{
			mFadeAlpha -= elapseTime / 2.0;
		}

		if (mFadeAlpha <= 0.0)
		{
			return;
		}
	}

	for (y = 0; y < MISTYFOG_HEIGHT; y++)
	{
		for (x = 0; x < MISTYFOG_WIDTH; x++)
		{
			mColors[y][x][3] = 0.0;
		}
	}

	CWorldEffect::Update(system, elapseTime);

	current = (CMistyFog*)mSlave;
	while (current)
	{
		UpdateTexture(current);
		UpdateTexture((CMistyFog*)current->GetSlave());
		current = (CMistyFog*)current->GetNext();
	}
}

void CMistyFog2::UpdateTexture(CMistyFog* fog)
{
	int				x, y, tx, ty;
	float			xSize, ySize;
	float			xStep, yStep;
	float			xPos, yPos;
	unsigned char* data = fog->GetData();
	int				width = fog->GetWidth();
	int				height = fog->GetHeight();
	int				andWidth, andHeight;
	float			alpha = fog->GetAlpha() * mAlpha * (1.0 / 255.0) * mFadeAlpha;
	float* color;

	if (!fog->GetRendering())
	{
		return;
	}

	andWidth = width - 1;		// width must be power of 2
	andHeight = height - 1;	// height must be power of 2
	xSize = fog->GetTextureCoord(1, 0) - fog->GetTextureCoord(0, 0);
	ySize = fog->GetTextureCoord(1, 1) - fog->GetTextureCoord(0, 1);
	xStep = xSize / (float)MISTYFOG_WIDTH;
	yStep = ySize / (float)MISTYFOG_HEIGHT;

	color = &mColors[0][0][3];
	for (y = 0, yPos = fog->GetTextureCoord(0, 1); y < MISTYFOG_HEIGHT; y++, yPos += yStep)
	{
		for (x = 0, xPos = fog->GetTextureCoord(0, 0); x < MISTYFOG_WIDTH; x++, xPos += xStep)
		{
			tx = xPos * width;
			tx &= andWidth;
			ty = yPos * height;
			ty &= andHeight;

			(*color) += data[(ty * width + tx) * 4] * alpha;
			color += 4;
		}
	}
}

void CMistyFog2::Render(CWorldEffectsSystem* system)
{
	if (mFadeAlpha <= 0.0)
	{
		return;
	}

	qglMatrixMode(GL_PROJECTION);
	qglPushMatrix();
	qglLoadIdentity();
	MYgluPerspective(80.0, 1.0, 4, 2048.0);

	qglMatrixMode(GL_MODELVIEW);
	qglPushMatrix();
	qglLoadIdentity();
	qglRotatef(-90, 1, 0, 0);	    // put Z going up
	qglRotatef(90, 0, 0, 1);	    // put Z going up
	qglRotatef(0, 1, 0, 0);
	qglRotatef(-90, 0, 1, 0);
	qglRotatef(-90, 0, 0, 1);

	qglDisable(GL_TEXTURE_2D);
	GL_State(GLS_SRCBLEND_SRC_ALPHA | GLS_DSTBLEND_ONE);
	qglShadeModel(GL_SMOOTH);

	qglColorPointer(4, GL_FLOAT, 0, mColors);
	qglEnableClientState(GL_COLOR_ARRAY);

	qglVertexPointer(3, GL_FLOAT, 0, mVerts);
	qglEnableClientState(GL_VERTEX_ARRAY);

	if (qglLockArraysEXT)
	{
		qglLockArraysEXT(0, MISTYFOG_HEIGHT * MISTYFOG_WIDTH);
	}
	qglDrawElements(GL_QUADS, (MISTYFOG_HEIGHT - 1) * (MISTYFOG_WIDTH - 1) * 4, GL_UNSIGNED_INT, mIndexes);
	if (qglUnlockArraysEXT)
	{
		qglUnlockArraysEXT();
	}

	qglDisableClientState(GL_COLOR_ARRAY);
	//	qglDisableClientState(GL_VERTEX_ARRAY);	 backend doesn't ever re=enable this properly

	qglPopMatrix();
	qglMatrixMode(GL_PROJECTION);
	qglPopMatrix();
	qglMatrixMode(GL_MODELVIEW);	// bug somewhere in the backend which requires this
}



class CWind : public CWorldEffect
{
private:
	vec4_t	mPlanes[3];		// x y z normal, distance
	float	mMaxDistance[3];
	vec3_t	mVelocity;
	int		mNumPlanes;
	int		mAffectedDuration;
	int* mAffectedCount;
	vec3_t	mPoint, mSize;
	bool	mGlobal;

public:
	CWind(bool global = false);
	CWind(vec3_t point, vec3_t velocity, vec3_t size, int duration, bool global = false);
	~CWind(void);

	virtual	void	Update(CWorldEffectsSystem* system, float elapseTime);
	virtual	void	ParmUpdate(CWorldEffectsSystem* system, int which);
	virtual	void	Render(CWorldEffectsSystem* system);

	void	UpdateParms(vec3_t point, vec3_t velocity, vec3_t size, int duration);
};




CWind::CWind(bool global) :
	CWorldEffect(),
	mNumPlanes(0),
	mAffectedCount(0),
	mGlobal(global)
{
	mEnabled = false;
}

CWind::CWind(vec3_t point, vec3_t velocity, vec3_t size, int duration, bool global) :
	CWorldEffect(),
	mNumPlanes(0),
	mAffectedCount(0),
	mGlobal(global)
{
	UpdateParms(point, velocity, size, duration);
}

CWind::~CWind(void)
{
	if (mAffectedCount)
	{
		delete[] mAffectedCount;
		mAffectedCount = 0;
	}
}

void CWind::UpdateParms(vec3_t point, vec3_t velocity, vec3_t size, int duration)
{
	vec3_t	normalDistance;

	mNumPlanes = 0;

	VectorCopy(point, mPoint);
	VectorCopy(size, mSize);
	mSize[0] /= 2.0;
	VectorScale(mSize, 2, mSize);
	VectorCopy(velocity, mVelocity);

	VectorCopy(velocity, mPlanes[mNumPlanes]);
	VectorNormalize(mPlanes[mNumPlanes]);
	mPlanes[mNumPlanes][3] = DotProduct(mPoint, mPlanes[mNumPlanes]);
	mMaxDistance[mNumPlanes] = mSize[0];
	mNumPlanes++;

	VectorScale(mPlanes[0], mPlanes[0][3], normalDistance);
	VectorSubtract(mPoint, normalDistance, mPlanes[mNumPlanes]);
	VectorNormalize(mPlanes[mNumPlanes]);
	mPlanes[mNumPlanes][3] = DotProduct(mPoint, mPlanes[mNumPlanes]);
	mMaxDistance[mNumPlanes] = mSize[1];
	mNumPlanes++;

	CrossProduct(mPlanes[0], mPlanes[1], mPlanes[mNumPlanes]);
	VectorNormalize(mPlanes[mNumPlanes]);
	mPlanes[mNumPlanes][3] = DotProduct(mPoint, mPlanes[mNumPlanes]);
	mMaxDistance[mNumPlanes] = mSize[2];
	mNumPlanes++;

	mPlanes[0][3] -= (mSize[0] / 2.0);
	mPlanes[1][3] -= (mSize[1] / 2.0);
	mPlanes[2][3] -= (mSize[2] / 2.0);

	mAffectedDuration = duration;
}

void CWind::Update(CWorldEffectsSystem* system, float elapseTime)
{
	SParticle* item;
	int						i, j, * affected;
	float					dist, calcDist[3];
	vec3_t					difference;

	if (!mEnabled)
	{
		return;
	}

	VectorSubtract(backEnd.viewParms.ori.origin, mPoint, difference);
	if (VectorLength(difference) > 300.0)
	{
		return;
	}

	calcDist[0] = 0.0;
	item = system->GetParticleVariable(WORLDEFFECT_PARTICLES);
	affected = mAffectedCount;
	for (i = system->GetIntVariable(WORLDEFFECT_PARTICLE_COUNT); i; i--)
	{
		if ((*affected))
		{
			(*affected)--;
		}
		else
		{
			if (!mGlobal)
			{
				for (j = 0; j < mNumPlanes; j++)
				{
					dist = DotProduct(item->pos, mPlanes[j]) - mPlanes[j][3];

					if (dist < 0.01 || dist > mMaxDistance[j])
					{
						break;
					}
					else
					{
						calcDist[j] = dist;
					}
				}
				if (j != mNumPlanes)
				{
					continue;
				}
			}

			float	scaleLength = 1.0 - (calcDist[0] / mMaxDistance[0]);

			(*affected) = mAffectedDuration * scaleLength;

			VectorMA(item->velocity, elapseTime, mVelocity);
			//			VectorMA(item->velocity, scaleLength, mVelocity, item->velocity);
		}
		affected++;
		item++;
	}
}

void CWind::ParmUpdate(CWorldEffectsSystem* system, int which)
{
	CWorldEffect::ParmUpdate(system, which);

	switch (which)
	{
	case WORLDEFFECT_PARTICLE_COUNT:
		if (mAffectedCount)
		{
			delete[] mAffectedCount;
		}
		mAffectedCount = new int[system->GetIntVariable(WORLDEFFECT_PARTICLE_COUNT)];
		memset(mAffectedCount, 0, system->GetIntVariable(WORLDEFFECT_PARTICLE_COUNT) * sizeof(int));
		break;
	}
}

void CWind::Render(CWorldEffectsSystem* system)
{
	vec3_t	output;

	if (!mEnabled || !debugShowWind)
	{
		return;
	}

	qglDisable(GL_TEXTURE_2D);
	qglDisable(GL_CULL_FACE);
	GL_State(GLS_ALPHA);

	qglColor4f(1.0, 0.0, 0.0, 0.5);
	qglBegin(GL_QUADS);

	VectorMA(mPoint, -(mSize[0] / 2.0), mPlanes[0], output);
	VectorMA(output, -(mSize[1] / 2.0), mPlanes[1], output);
	VectorMA(output, -(mSize[2] / 2.0), mPlanes[2], output);
	qglVertex3fv(output);

	VectorMA(mPoint, -(mSize[0] / 2.0), mPlanes[0], output);
	VectorMA(output, (mSize[1] / 2.0), mPlanes[1], output);
	VectorMA(output, -(mSize[2] / 2.0), mPlanes[2], output);
	qglVertex3fv(output);

	VectorMA(mPoint, -(mSize[0] / 2.0), mPlanes[0], output);
	VectorMA(output, (mSize[1] / 2.0), mPlanes[1], output);
	VectorMA(output, (mSize[2] / 2.0), mPlanes[2], output);
	qglVertex3fv(output);

	VectorMA(mPoint, -(mSize[0] / 2.0), mPlanes[0], output);
	VectorMA(output, -(mSize[1] / 2.0), mPlanes[1], output);
	VectorMA(output, (mSize[2] / 2.0), mPlanes[2], output);
	qglVertex3fv(output);



	qglColor4f(0.0, 1.0, 0.0, 0.5);
	VectorMA(mPoint, -(mSize[0] / 2.0), mPlanes[0], output);
	VectorMA(output, -(mSize[1] / 2.0), mPlanes[1], output);
	VectorMA(output, -(mSize[2] / 2.0), mPlanes[2], output);
	qglVertex3fv(output);

	VectorMA(mPoint, (mSize[0] / 2.0), mPlanes[0], output);
	VectorMA(output, -(mSize[1] / 2.0), mPlanes[1], output);
	VectorMA(output, -(mSize[2] / 2.0), mPlanes[2], output);
	qglVertex3fv(output);

	VectorMA(mPoint, (mSize[0] / 2.0), mPlanes[0], output);
	VectorMA(output, -(mSize[1] / 2.0), mPlanes[1], output);
	VectorMA(output, (mSize[2] / 2.0), mPlanes[2], output);
	qglVertex3fv(output);

	VectorMA(mPoint, -(mSize[0] / 2.0), mPlanes[0], output);
	VectorMA(output, -(mSize[1] / 2.0), mPlanes[1], output);
	VectorMA(output, (mSize[2] / 2.0), mPlanes[2], output);
	qglVertex3fv(output);


	qglColor4f(0.0, 0.0, 1.0, 0.5);
	VectorMA(mPoint, -(mSize[0] / 2.0), mPlanes[0], output);
	VectorMA(output, -(mSize[2] / 2.0), mPlanes[2], output);
	VectorMA(output, -(mSize[1] / 2.0), mPlanes[1], output);
	qglVertex3fv(output);

	VectorMA(mPoint, (mSize[0] / 2.0), mPlanes[0], output);
	VectorMA(output, -(mSize[2] / 2.0), mPlanes[2], output);
	VectorMA(output, -(mSize[1] / 2.0), mPlanes[1], output);
	qglVertex3fv(output);

	VectorMA(mPoint, (mSize[0] / 2.0), mPlanes[0], output);
	VectorMA(output, -(mSize[2] / 2.0), mPlanes[2], output);
	VectorMA(output, (mSize[1] / 2.0), mPlanes[1], output);
	qglVertex3fv(output);

	VectorMA(mPoint, -(mSize[0] / 2.0), mPlanes[0], output);
	VectorMA(output, -(mSize[2] / 2.0), mPlanes[2], output);
	VectorMA(output, (mSize[1] / 2.0), mPlanes[1], output);
	qglVertex3fv(output);


	qglEnd();

	qglEnable(GL_CULL_FACE);
	qglEnable(GL_TEXTURE_2D);
}












#define CONTENTS_X_SIZE		16
#define CONTENTS_Y_SIZE		16
#define CONTENTS_Z_SIZE		8


class CSnowSystem : public CWorldEffectsSystem
{
private:
	// configurable
	float		mAlpha;
	vec3_t		mMinSpread, mMaxSpread;
	vec3_t		mMinVelocity, mMaxVelocity;
	int			mMaxSnowflakes;
	float		mWindDuration, mWindLow;
	float		mWindMin, mWindMax;
	vec3_t		mWindSize;

	image_t* mImage;
	vec3_t		mMins, mMaxs;
	float		mNextWindGust, mWindLowSize;
	CWind* mWindGust;

	vec3_t		mWindDirection, mWindSpeed;
	int			mWindChange;

	SParticle* mSnowList;
	int			mContents[CONTENTS_Z_SIZE][CONTENTS_Y_SIZE][CONTENTS_X_SIZE];
	vec3_t		mContentsSize;
	vec3_t		mContentsStart;

	int			mUpdateCount;
	int			mOverallContents;
	bool		mIsSnowing;

	const		float	mVelocityStabilize;
	const		int		mUpdateMax;

public:
	CSnowSystem(int maxSnowflakes);
	~CSnowSystem(void);

	virtual	int			GetIntVariable(int which);
	virtual	SParticle* GetParticleVariable(int which);
	virtual	float* GetVecVariable(int which);

	virtual bool	Command(const char* command);

	virtual	void	Update(float elapseTime);
	virtual	void	Render(void);

	void	Init(void);

	bool	IsSnowing() { return mIsSnowing; }
};

CSnowSystem::CSnowSystem(int maxSnowflakes) :
	mMaxSnowflakes(maxSnowflakes),
	mNextWindGust(0.0),
	mWindLowSize(0.0),
	mWindGust(0),
	mWindChange(0),

	mAlpha(0.4f),
	mWindDuration(2.0f),
	mWindLow(3.0f),
	mWindMin(30.0f), // .6 3
	mWindMax(70.0f),
	mUpdateCount(0),
	mOverallContents(0),

	mVelocityStabilize(18),
	mUpdateMax(10),
	mIsSnowing(false)
{
	mMinSpread[0] = -600;
	mMinSpread[1] = -600;
	mMinSpread[2] = -200;
	mMaxSpread[0] = 600;
	mMaxSpread[1] = 600;
	mMaxSpread[2] = 250;

	mMinVelocity[0] = -15.0;
	mMaxVelocity[0] = 15.0;
	mMinVelocity[1] = -15.0;
	mMaxVelocity[1] = 15.0;
	mMinVelocity[2] = -20.0;
	mMaxVelocity[2] = -70.0;

	mWindSize[0] = 1000.0;
	mWindSize[1] = 300.0;
	mWindSize[2] = 300.0;

	mSnowList = new SParticle[mMaxSnowflakes];

	mContentsSize[0] = (mMaxSpread[0] - mMinSpread[0]) / CONTENTS_X_SIZE;
	mContentsSize[1] = (mMaxSpread[1] - mMinSpread[1]) / CONTENTS_Y_SIZE;
	mContentsSize[2] = (mMaxSpread[2] - mMinSpread[2]) / CONTENTS_Z_SIZE;

	Init();

	AddWorldEffect(mWindGust = new CWind(true));
	ParmUpdate(CWorldEffect::WORLDEFFECT_PARTICLE_COUNT);
}

CSnowSystem::~CSnowSystem(void)
{
	delete[] mSnowList;
}

void CSnowSystem::Init(void)
{
	int			i;
	SParticle* item;

	mMins[0] = mMaxs[0] = mMins[1] = mMaxs[1] = mMins[2] = mMaxs[2] = 99999;
	item = mSnowList;
	for (i = mMaxSnowflakes; i; i--)
	{
		item->pos[0] = item->pos[1] = item->pos[2] = 99999;
		item->velocity[0] = item->velocity[1] = item->velocity[2] = 0.0;
		item->flags = 0;
		item++;
	}
}

int CSnowSystem::GetIntVariable(int which)
{
	switch (which)
	{
	case CWorldEffect::WORLDEFFECT_PARTICLE_COUNT:
		return mMaxSnowflakes;
	}

	return CWorldEffectsSystem::GetIntVariable(which);
}

SParticle* CSnowSystem::GetParticleVariable(int which)
{
	switch (which)
	{
	case CWorldEffect::WORLDEFFECT_PARTICLES:
		return mSnowList;
	}

	return CWorldEffectsSystem::GetParticleVariable(which);
}

float* CSnowSystem::GetVecVariable(int which)
{
	switch (which)
	{
	case CRainSystem::RAINSYSTEM_WIND_DIRECTION:
		return mWindDirection;
	}
	return 0;
}

bool CSnowSystem::Command(const char* command)
{
	char* token;

	if (CWorldEffectsSystem::Command(command))
	{
		return true;
	}
	COM_BeginParseSession();
	token = COM_ParseExt(&command, qfalse);

	if (strcmpi(token, "wind") == 0)
	{	// snow wind ( windOriginX windOriginY windOriginZ ) ( windVelocityX windVelocityY windVelocityZ ) ( sizeX sizeY sizeZ )
		vec3_t	origin, velocity, size;

		ParseVector(&command, 3, origin);
		ParseVector(&command, 3, velocity);
		ParseVector(&command, 3, size);

		AddWorldEffect(new CWind(origin, velocity, size, 0));

		return true;
	}
	else if (strcmpi(token, "fog") == 0)
	{	// snow fog
		AddWorldEffect(new CMistyFog2);
		mWindChange = 0;
		return true;
	}
	else if (strcmpi(token, "alpha") == 0)
	{	// snow alpha <float>											default: 0.09
		COM_BeginParseSession();
		token = COM_ParseExt(&command, qfalse);
		mAlpha = atof(token);
		return true;
	}
	else if (strcmpi(token, "spread") == 0)
	{	// snow spread ( minX minY minZ ) ( maxX maxY maxZ )			default: ( -600 -600 -200 ) ( 600 600 250 )
		ParseVector(&command, 3, mMinSpread);
		ParseVector(&command, 3, mMaxSpread);
		return true;
	}
	else if (strcmpi(token, "velocity") == 0)
	{	// snow velocity ( minX minY minZ ) ( maxX maxY maxZ )			default: ( -15 -15 -20 ) ( 15 15 -70 )
		ParseVector(&command, 3, mMinSpread);
		ParseVector(&command, 3, mMaxSpread);
		return true;
	}
	else if (strcmpi(token, "blowing") == 0)
	{
		COM_BeginParseSession();
		token = COM_ParseExt(&command, qfalse);
		if (strcmpi(token, "duration") == 0)
		{	// snow blowing duration <int>									default: 2
			token = COM_ParseExt(&command, qfalse);
			mWindDuration = atol(token);
			return true;
		}
		else if (strcmpi(token, "low") == 0)
		{	// snow blowing low <int>										default: 3
			token = COM_ParseExt(&command, qfalse);
			mWindLow = atol(token);
			return true;
		}
		else if (strcmpi(token, "velocity") == 0)
		{	// snow blowing velocity ( min max )							default: ( 30 70 )
			float	data[2];

			ParseVector(&command, 2, data);
			mWindMin = data[0];
			mWindMax = data[1];
			return true;
		}
		else if (strcmpi(token, "size") == 0)
		{	// snow blowing size ( minX minY minZ )							default: ( 1000 300 300 )
			ParseVector(&command, 3, mWindSize);
			return true;
		}
	}

	return false;
}

void CSnowSystem::Update(float elapseTime)
{
	int			i;
	SParticle* item;
	vec3_t		origin, newMins, newMaxs;
	vec3_t		difference, start;
	bool		resetFlake;
	int			x, y, z;
	int			contents;

	mWindChange--;
	if (mWindChange < 0)
	{
		mWindDirection[0] = 1.0 - (FloatRand() * 2.0);
		mWindDirection[1] = 1.0 - (FloatRand() * 2.0);
		mWindDirection[2] = 0.0;
		VectorNormalize(mWindDirection);
		VectorScale(mWindDirection, 0.025f, mWindSpeed);

		mWindChange = 200 + rand() % 250;
		//		mWindChange = 10;

		ParmUpdate(CRainSystem::RAINSYSTEM_WIND_DIRECTION);
	}

	if ((mOverallContents & CONTENTS_OUTSIDE))
	{
		CWorldEffectsSystem::Update(elapseTime);
	}

	VectorCopy(backEnd.viewParms.ori.origin, origin);

	mNextWindGust -= elapseTime;
	if (mNextWindGust < 0.0)
	{
		mWindGust->SetVariable(CWorldEffect::WORLDEFFECT_ENABLED, false);
	}

	if (mNextWindGust < mWindLowSize)
	{
		vec3_t		windPos;
		vec3_t		windDirection;

		windDirection[0] = Q_flrand(-1.0, 1.0);
		windDirection[1] = Q_flrand(-1.0, 1.0);
		windDirection[2] = 0.0;//ri.flrand(-0.1, 0.1);
		VectorNormalize(windDirection);
		VectorScale(windDirection, Q_flrand(mWindMin, mWindMax), windDirection);

		VectorCopy(origin, windPos);

		mWindGust->SetVariable(CWorldEffect::WORLDEFFECT_ENABLED, true);
		mWindGust->UpdateParms(windPos, windDirection, mWindSize, 0);

		mNextWindGust = Q_flrand(mWindDuration, mWindDuration * 2.0);
		mWindLowSize = -Q_flrand(mWindLow, mWindLow * 3.0);
	}

	newMins[0] = mMinSpread[0] + origin[0];
	newMaxs[0] = mMaxSpread[0] + origin[0];

	newMins[1] = mMinSpread[1] + origin[1];
	newMaxs[1] = mMaxSpread[1] + origin[1];

	newMins[2] = mMinSpread[2] + origin[2];
	newMaxs[2] = mMaxSpread[2] + origin[2];

	for (i = 0; i < 3; i++)
	{
		difference[i] = newMaxs[i] - mMaxs[i];
		if (difference[i] >= 0.0)
		{
			if (difference[i] > newMaxs[i] - newMins[i])
			{
				difference[i] = newMaxs[i] - newMins[i];
			}
			start[i] = newMaxs[i] - difference[i];
		}
		else
		{
			if (difference[i] < newMins[i] - newMaxs[i])
			{
				difference[i] = newMins[i] - newMaxs[i];
			}
			start[i] = newMins[i] - difference[i];
		}
	}

	//	contentsStart[0] = (((origin[0] + mMinSpread[0]) / mContentsSize[0])) * mContentsSize[0];
	//	contentsStart[1] = (((origin[1] + mMinSpread[1]) / mContentsSize[1])) * mContentsSize[1];
	//	contentsStart[2] = (((origin[2] + mMinSpread[2]) / mContentsSize[2])) * mContentsSize[2];

	if (fabs(difference[0]) > 25.0 || fabs(difference[1]) > 25.0 || fabs(difference[2]) > 25.0)
	{
		vec3_t		pos;
		int* store;

		mContentsStart[0] = ((int)((origin[0] + mMinSpread[0]) / mContentsSize[0])) * mContentsSize[0];
		mContentsStart[1] = ((int)((origin[1] + mMinSpread[1]) / mContentsSize[1])) * mContentsSize[1];
		mContentsStart[2] = ((int)((origin[2] + mMinSpread[2]) / mContentsSize[2])) * mContentsSize[2];

		mOverallContents = 0;
		store = (int*)mContents;
		for (z = 0, pos[2] = mContentsStart[2]; z < CONTENTS_Z_SIZE; z++, pos[2] += mContentsSize[2])
		{
			for (y = 0, pos[1] = mContentsStart[1]; y < CONTENTS_Y_SIZE; y++, pos[1] += mContentsSize[1])
			{
				for (x = 0, pos[0] = mContentsStart[0]; x < CONTENTS_X_SIZE; x++, pos[0] += mContentsSize[0])
				{
					contents = ri.CM_PointContents(pos, 0);
					mOverallContents |= contents;
					*store++ = contents;
				}
			}
		}

		item = mSnowList;
		for (i = mMaxSnowflakes; i; i--)
		{
			resetFlake = false;

			if (item->pos[0] < newMins[0] || item->pos[0] > newMaxs[0])
			{
				item->pos[0] = Q_flrand(0.0, difference[0]) + start[0];
				resetFlake = true;
			}
			if (item->pos[1] < newMins[1] || item->pos[1] > newMaxs[1])
			{
				item->pos[1] = Q_flrand(0.0, difference[1]) + start[1];
				resetFlake = true;
			}
			if (item->pos[2] < newMins[2] || item->pos[2] > newMaxs[2])
			{
				item->pos[2] = Q_flrand(0.0, difference[2]) + start[2];
				resetFlake = true;
			}

			if (resetFlake)
			{
				item->velocity[0] = 0.0;
				item->velocity[1] = 0.0;
				item->velocity[2] = Q_flrand(mMaxVelocity[2], mMinVelocity[2]);
			}
			item++;
		}

		VectorCopy(newMins, mMins);
		VectorCopy(newMaxs, mMaxs);
	}

	if (!(mOverallContents & CONTENTS_OUTSIDE))
	{
		mIsSnowing = false;
		return;
	}

	mIsSnowing = true;

	mUpdateCount = (mUpdateCount + 1) % mUpdateMax;

	x = y = z = 0;
	item = mSnowList;
	for (i = mMaxSnowflakes; i; i--)
	{
		resetFlake = false;

		//		if ((i & mUpdateCount) == 0)   wrong check
		{
			if (item->velocity[0] < mMinVelocity[0])
			{
				item->velocity[0] += mVelocityStabilize * elapseTime;
			}
			else if (item->velocity[0] > mMaxVelocity[0])
			{
				item->velocity[0] -= mVelocityStabilize * elapseTime;
			}
			else
			{
				item->velocity[0] += Q_flrand(-1.4f, 1.4f);
			}
			if (item->velocity[1] < mMinVelocity[1])
			{
				item->velocity[1] += mVelocityStabilize * elapseTime;
			}
			else if (item->velocity[1] > mMaxVelocity[1])
			{
				item->velocity[1] -= mVelocityStabilize * elapseTime;
			}
			else
			{
				item->velocity[1] += Q_flrand(-1.4f, 1.4f);
			}
			if (item->velocity[2] > mMinVelocity[2])
			{
				item->velocity[2] -= mVelocityStabilize * 2.0;
			}
		}
		VectorMA(item->pos, elapseTime, item->velocity);

		if (item->pos[2] < newMins[2])
		{
			resetFlake = true;
		}
		else
		{
			//			if ((i & mUpdateCount) == 0)
			{
				x = (item->pos[0] - mContentsStart[0]) / mContentsSize[0];
				y = (item->pos[1] - mContentsStart[1]) / mContentsSize[1];
				z = (item->pos[2] - mContentsStart[2]) / mContentsSize[2];
				if (x < 0 || x >= CONTENTS_X_SIZE ||
					y < 0 || y >= CONTENTS_Y_SIZE ||
					z < 0 || z >= CONTENTS_Z_SIZE)
				{
					resetFlake = true;
				}
			}
		}

		if (resetFlake)
		{
			item->pos[2] = newMaxs[2] - (newMins[2] - item->pos[2]);
			if (item->pos[2] < newMins[2] || item->pos[2] > newMaxs[2])
			{	// way out of range
				item->pos[2] = Q_flrand(newMins[2], newMaxs[2]);
			}

			item->pos[0] = Q_flrand(newMins[0], newMaxs[0]);
			item->pos[1] = Q_flrand(newMins[1], newMaxs[1]);

			item->velocity[0] = 0.0;
			item->velocity[1] = 0.0;
			item->velocity[2] = Q_flrand(mMaxVelocity[2], mMinVelocity[2]);
			item->flags &= ~PARTICLE_FLAG_RENDER;
		}
		else if (mContents[z][y][x] & CONTENTS_OUTSIDE)
		{
			item->flags |= PARTICLE_FLAG_RENDER;
		}
		else
		{
			item->flags &= ~PARTICLE_FLAG_RENDER;
		}

		item++;
	}
}

const float	attenuation[3] =
{
	1, 0.0f, 0.0004f
};

void CSnowSystem::Render(void)
{
	int			i;
	SParticle* item;
	vec3_t		origin;

	if (!(mOverallContents & CONTENTS_OUTSIDE))
	{
		return;
	}

	CWorldEffectsSystem::Render();

	VectorAdd(backEnd.viewParms.ori.origin, mMinSpread, origin);

	qglColor4f(0.8f, 0.8f, 0.8f, mAlpha);

	//	GL_State(GLS_SRCBLEND_SRC_ALPHA|GLS_DSTBLEND_ONE);
	GL_State(GLS_ALPHA);
	qglDisable(GL_TEXTURE_2D);

	if (qglPointParameterfEXT)
	{
		qglPointSize(10.0);
		qglPointParameterfEXT(GL_POINT_SIZE_MIN_EXT, 1.0);
		qglPointParameterfEXT(GL_POINT_SIZE_MAX_EXT, 4.0);
		//	qglPointParameterfEXT(GL_POINT_FADE_THRESHOLD_SIZE_EXT, 3.0);
		qglPointParameterfvEXT(GL_DISTANCE_ATTENUATION_EXT, (float*)attenuation);
	}
	else
	{
		qglPointSize(2.0);
	}


	item = mSnowList;
	qglBegin(GL_POINTS);
	for (i = mMaxSnowflakes; i; i--)
	{
		if (item->flags & PARTICLE_FLAG_RENDER)
		{
			qglVertex3fv(item->pos);
		}
		item++;
	}
	qglEnd();
	qglEnable(GL_TEXTURE_2D);
}

CSnowSystem* snowSystem = 0;






CRainSystem::CRainSystem(int maxRain) :
	mMaxRain(maxRain),
	mNextWindGust(0),
	mRainHeight(5),
	mAlpha(0.1f),
	mWindAngle(1.0f),

	mFadeAlpha(0.0f),
	mIsRaining(false)

{
	char			name[256];
	unsigned char* data, * pos;
	int				width, height;
	int				x, y;

	mSpread[0] = (float)(M_PI * 2.0);		// angle spread
	mSpread[1] = 20.0f;			// radius spread
	mSpread[2] = 20.0f;			// z spread

	mMinVelocity[0] = 0.1f;
	mMaxVelocity[0] = -0.1f;
	mMinVelocity[1] = 0.1f;
	mMaxVelocity[1] = -0.1f;
	mMinVelocity[2] = -60.0;
	mMaxVelocity[2] = -50.0;

	mWindDuration = 15;
	mWindLow = 50;
	mWindMin = 0.01f;
	mWindMax = 0.05f;

	mWindChange = 0;
	mWindDirection[0] = mWindDirection[1] = mWindDirection[2] = 0.0;

	mRainList = new SParticle[mMaxRain];

	sprintf(name, "gfx/world/rain.tga");
	R_LoadImage(name, &data, &width, &height);
	if (!data)
	{
		ri.Error(ERR_DROP, "Could not load %s", name);
	}

	pos = data;
	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{
			pos[3] = pos[0];
			pos += 4;
		}
	}

	mImage = R_FindImageFile(name, qfalse, qfalse, qfalse, qfalse);
	GL_Bind(mImage);


	qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	Init();
}

CRainSystem::~CRainSystem(void)
{
	delete[] mRainList;
}

void CRainSystem::Init(void)
{
	int			i;
	SParticle* item;

	item = mRainList;
	for (i = mMaxRain; i; i--)
	{
		item->pos[0] = Q_flrand(0.0, mSpread[0]);
		item->pos[1] = Q_flrand(0.0, mSpread[1]);
		item->pos[2] = Q_flrand(-mSpread[2], 40);
		item->velocity[0] = Q_flrand(mMinVelocity[0], mMaxVelocity[0]);
		item->velocity[1] = Q_flrand(mMinVelocity[1], mMaxVelocity[1]);
		item->velocity[2] = Q_flrand(mMinVelocity[2], mMaxVelocity[2]);
		item++;
	}
}

int CRainSystem::GetIntVariable(int which)
{
	switch (which)
	{
	case CWorldEffect::WORLDEFFECT_PARTICLE_COUNT:
		return mMaxRain;
	}

	return CWorldEffectsSystem::GetIntVariable(which);
}

SParticle* CRainSystem::GetParticleVariable(int which)
{
	switch (which)
	{
	case CWorldEffect::WORLDEFFECT_PARTICLES:
		return mRainList;
	}

	return CWorldEffectsSystem::GetParticleVariable(which);
}

float CRainSystem::GetFloatVariable(int which)
{
	switch (which)
	{
	case CRainSystem::RAINSYSTEM_WIND_SPEED:
		return mWindAngle * 75.0;		// pat scaled
	}

	return 0.0;
}

float* CRainSystem::GetVecVariable(int which)
{
	switch (which)
	{
	case CRainSystem::RAINSYSTEM_WIND_DIRECTION:
		return mWindDirection;
	}
	return 0;
}

bool CRainSystem::Command(const char* command)
{
	char* token;

	if (CWorldEffectsSystem::Command(command))
	{
		return true;
	}
	COM_BeginParseSession();
	token = COM_ParseExt(&command, qfalse);

	if (strcmpi(token, "fog") == 0)
	{	// rain fog
		AddWorldEffect(new CMistyFog2);
		mWindChange = 0;
		return true;
	}
	else if (strcmpi(token, "fall") == 0)
	{	// rain fall ( minVelocity maxVelocity )			default: ( -60 -50 )
		float	data[2];

		if (ParseVector(&command, 2, data))
		{
			mMinVelocity[2] = data[0];
			mMaxVelocity[2] = data[1];
		}
		return true;
	}
	else if (strcmpi(token, "spread") == 0)
	{	// rain spread ( radius height )					default: ( 20 20 )
		ParseVector(&command, 2, &mSpread[1]);
		return true;
	}
	else if (strcmpi(token, "alpha") == 0)
	{	// rain alpha <float>								default: 0.15
		token = COM_ParseExt(&command, qfalse);
		mAlpha = atof(token);
		return true;
	}
	else if (strcmpi(token, "height") == 0)
	{	// rain height <float>								default: 1.5
		token = COM_ParseExt(&command, qfalse);
		mRainHeight = atof(token);
		return true;
	}
	else if (strcmpi(token, "angle") == 0)
	{	// rain angle <float>								default: 1.0
		token = COM_ParseExt(&command, qfalse);
		mWindAngle = atof(token);
		return true;
	}

	return false;
}

void CRainSystem::Update(float elapseTime)
{
	int			i;
	SParticle* item;
	vec3_t		windDifference;

	if (elapseTime < 0.0f)
	{
		// sanity check
		elapseTime = 0.0f;
	}

	mWindChange--;

	if (mWindChange < 0)
	{
		mNewWindDirection[0] = 1.0 - (FloatRand() * 2.0);
		mNewWindDirection[1] = 1.0 - (FloatRand() * 2.0);
		mNewWindDirection[2] = 0.0;
		VectorNormalize(mNewWindDirection);
		VectorScale(mNewWindDirection, 0.025f, mWindSpeed);

		mWindChange = 200 + rand() % 250;
		//		mWindChange = 10;

		ParmUpdate(CRainSystem::RAINSYSTEM_WIND_DIRECTION);
	}

	VectorSubtract(mNewWindDirection, mWindDirection, windDifference);
	VectorMA(mWindDirection, elapseTime, windDifference);

	CWorldEffectsSystem::Update(elapseTime);

	if (originContents & CONTENTS_OUTSIDE && !(originContents & CONTENTS_WATER))
	{
		mIsRaining = true;
		if (mFadeAlpha < 1.0)
		{
			mFadeAlpha += elapseTime / 2.0;
		}
		if (mFadeAlpha > 1.0)
		{
			mFadeAlpha = 1.0;
		}
	}
	else
	{
		mIsRaining = false;
		if (mFadeAlpha > 0.0)
		{
			mFadeAlpha -= elapseTime / 2.0;
		}

		if (mFadeAlpha <= 0.0)
		{
			return;
		}
	}

	item = mRainList;
	for (i = mMaxRain; i; i--)
	{
		VectorMA(item->pos, elapseTime, item->velocity);

		if (item->pos[2] < -mSpread[2])
		{
			item->pos[0] = Q_flrand(0.0, mSpread[0]);
			item->pos[1] = Q_flrand(0.0, mSpread[1]);
			item->pos[2] = 40;

			item->velocity[0] = Q_flrand(mMinVelocity[0], mMaxVelocity[0]);
			item->velocity[1] = Q_flrand(mMinVelocity[1], mMaxVelocity[1]);
			item->velocity[2] = Q_flrand(mMinVelocity[2], mMaxVelocity[2]);
		}

		item++;
	}
}

extern vec3_t	mViewAngles, mOrigin;

void CRainSystem::Render(void)
{
	int			i;
	SParticle* item;
	vec4_t		forward, down, left;
	vec3_t		pos;
	//	float		percent;
	float		radius;

	CWorldEffectsSystem::Render();

	if (mFadeAlpha <= 0.0)
	{
		return;
	}

	VectorScale(backEnd.viewParms.ori.axis[0], 1, forward);		// forward
	VectorScale(backEnd.viewParms.ori.axis[1], 0.2f, left);		// left
	down[0] = 0 - mWindDirection[0] * mRainHeight * mWindAngle;
	down[1] = 0 - mWindDirection[1] * mRainHeight * mWindAngle;
	down[2] = -mRainHeight;

	GL_Bind(mImage);

	GL_State(GLS_SRCBLEND_SRC_ALPHA | GLS_DSTBLEND_ONE);
	qglEnable(GL_TEXTURE_2D);
	qglDisable(GL_CULL_FACE);

	qglMatrixMode(GL_MODELVIEW);
	qglPushMatrix();
	qglTranslatef(backEnd.viewParms.ori.origin[0], backEnd.viewParms.ori.origin[1], backEnd.viewParms.ori.origin[2]);

	item = mRainList;
	qglBegin(GL_TRIANGLES);
	for (i = mMaxRain; i; i--)
	{
		/*		percent = (item->pos[1] -(-20.0)) / (20.0 - (-20.0));
				percent *= forward[2];
				if (percent < 0.0)
				{
					radius = 10 * (percent + 1.0);
				}
				else
				{
					radius = 10 * (1.0 - percent);
				}*/
		radius = item->pos[1];
		if (item->pos[2] < 0.0)
		{
			//			radius *= 1.0 - (item->pos[2] / 40.0);
			float alpha = mAlpha * (item->pos[1] / -item->pos[2]);

			if (alpha > mAlpha)
			{
				alpha = mAlpha;
			}
			qglColor4f(1.0, 1.0, 1.0, alpha * mFadeAlpha);
		}
		else
		{
			qglColor4f(1.0, 1.0, 1.0, mAlpha * mFadeAlpha);
			//			radius *= 1.0 + (item->pos[2] / 20.0);
		}

		pos[0] = sin(item->pos[0]) * radius + (item->pos[2] * mWindDirection[0] * mWindAngle);
		pos[1] = cos(item->pos[0]) * radius + (item->pos[2] * mWindDirection[1] * mWindAngle);
		pos[2] = item->pos[2];

		qglTexCoord2f(1.0, 0.0);
		qglVertex3f(pos[0],
			pos[1],
			pos[2]);

		qglTexCoord2f(0.0, 0.0);
		qglVertex3f(pos[0] + left[0],
			pos[1] + left[1],
			pos[2] + left[2]);

		qglTexCoord2f(0.0, 1.0);
		qglVertex3f(pos[0] + down[0] + left[0],
			pos[1] + down[1] + left[1],
			pos[2] + down[2] + left[2]);
		item++;
	}
	qglEnd();

	qglEnable(GL_CULL_FACE);

	qglPopMatrix();
}






CRainSystem* rainSystem = 0;


void R_InitWorldEffects(void)
{
	if (rainSystem)
	{
		delete rainSystem;
	}

	if (snowSystem)
	{
		delete snowSystem;
	}
}

void R_ShutdownWorldEffects(void)
{
	if (rainSystem)
	{
		delete rainSystem;
		rainSystem = 0;
	}
	if (snowSystem)
	{
		delete snowSystem;
		snowSystem = 0;
	}
}

void SetViewportAndScissor(void);

void RB_RenderWorldEffects(void)
{
	float					elapseTime = backEnd.refdef.frametime / 1000.0;

	if (tr.refdef.rdflags & RDF_NOWORLDMODEL || !tr.world || ri.CL_IsRunningInGameCinematic())
	{	//  no world rendering or no world
		return;
	}

	SetViewportAndScissor();
	qglMatrixMode(GL_MODELVIEW);
	//	qglPushMatrix();
	qglLoadMatrixf(backEnd.viewParms.world.modelMatrix);

	originContents = ri.CM_PointContents(backEnd.viewParms.ori.origin, 0);

	if (rainSystem)
	{
		rainSystem->Update(elapseTime);
		rainSystem->Render();
	}

	if (snowSystem)
	{
		snowSystem->Update(elapseTime);
		snowSystem->Render();
	}

	//	qglMatrixMode(GL_MODELVIEW);
	//	qglPopMatrix();
}

//	console commands for r_we
//
//	SNOW
//		snow init <particles>
//		snow remove
//		snow alpha <float>											default: 0.09
//		snow spread ( minX minY minZ ) ( maxX maxY maxZ )			default: ( -600 -600 -200 ) ( 600 600 250 )
//		snow velocity ( minX minY minZ ) ( maxX maxY maxZ )			default: ( -15 -15 -20 ) ( 15 15 -70 )
//		snow blowing duration <int>									default: 2
//		snow blowing low <int>										default: 3
//		snow blowing velocity ( min max )							default: ( 30 70 )
//		snow blowing size ( minX minY minZ )						default: ( 1000 300 300 )
//		snow wind ( windOriginX windOriginY windOriginZ ) ( windVelocityX windVelocityY windVelocityZ ) ( sizeX sizeY sizeZ )
//		snow fog
//		snow fog density <alpha>									default: 0.3
//
//	RAIN
//		rain init <particles>
//		rain remove
//		rain fog
//		rain fog density <alpha>									default: 0.3
//		rain fall ( minVelocity maxVelocity )						default: ( -60 -50 )
//		rain spread ( radius height )								default: ( 20 20 )
//		rain alpha <float>											default: 0.1
//		rain height <float>											default: 5
//		rain angle <float>											default: 1.0
//
//	DEBUG
//		debug wind

void R_WorldEffectCommand(const char* command)
{
	const char* token, * origCommand;
	COM_BeginParseSession();
	token = COM_ParseExt(&command, qfalse);

	if (strcmpi(token, "snow") == 0)
	{
		origCommand = command;
		COM_BeginParseSession();
		token = COM_ParseExt(&command, qfalse);
		if (strcmpi(token, "init") == 0)
		{	//	snow init <particles>
			token = COM_ParseExt(&command, qfalse);
			if (snowSystem)
			{
				delete snowSystem;
			}
			snowSystem = new CSnowSystem(atoi(token));
		}
		else if (strcmpi(token, "remove") == 0)
		{	//	snow remove
			if (snowSystem)
			{
				delete snowSystem;
				snowSystem = 0;
			}
		}
		else if (snowSystem)
		{
			snowSystem->Command(origCommand);
		}
	}
	else if (strcmpi(token, "rain") == 0)
	{
		origCommand = command;
		COM_BeginParseSession();
		token = COM_ParseExt(&command, qfalse);
		if (strcmpi(token, "init") == 0)
		{	//	rain init <particles>
			token = COM_ParseExt(&command, qfalse);
			if (rainSystem)
			{
				delete rainSystem;
			}
			rainSystem = new CRainSystem(atoi(token));
		}
		else if (strcmpi(token, "remove") == 0)
		{	//	rain remove
			if (rainSystem)
			{
				delete rainSystem;
				rainSystem = 0;
			}
		}
		else if (rainSystem)
		{
			rainSystem->Command(origCommand);
		}
	}
	else if (strcmpi(token, "debug") == 0)
	{
		COM_BeginParseSession();
		token = COM_ParseExt(&command, qfalse);
		if (strcmpi(token, "wind") == 0)
		{
			debugShowWind = !debugShowWind;
		}
		else if (strcmpi(token, "blah") == 0)
		{
			R_WorldEffectCommand("snow init 1000");
			R_WorldEffectCommand("snow alpha 1");
			R_WorldEffectCommand("snow fog");
		}
	}
}

void R_WorldEffect_f(void)
{
	char		temp[2048];

	ri.Cmd_ArgsBuffer(temp, sizeof(temp));
	R_WorldEffectCommand(temp);
}

bool R_GetWindVector(vec3_t windVector)
{
	if (rainSystem)
	{
		VectorCopy(rainSystem->GetVecVariable(CRainSystem::RAINSYSTEM_WIND_DIRECTION), windVector);
		return true;
	}

	if (snowSystem)
	{
		VectorCopy(snowSystem->GetVecVariable(CRainSystem::RAINSYSTEM_WIND_DIRECTION), windVector);
		return true;
	}


	return false;
}

bool R_GetWindSpeed(float& windSpeed)
{
	if (rainSystem)
	{
		windSpeed = rainSystem->GetFloatVariable(CRainSystem::RAINSYSTEM_WIND_SPEED);
		return true;
	}

	return false;
}

bool R_IsRaining()
{
	if (rainSystem)
	{
		return rainSystem->IsRaining();
	}
	return false;
}

bool R_IsSnowing()
{
	if (snowSystem)
	{
		return snowSystem->IsSnowing();
	}
	return false;
}









#else

////////////////////////////////////////////////////////////////////////////////////////
// Defines
////////////////////////////////////////////////////////////////////////////////////////
#define GLS_ALPHA				(GLS_SRCBLEND_SRC_ALPHA | GLS_DSTBLEND_ONE_MINUS_SRC_ALPHA)
#define	MAX_WIND_ZONES			12
#define MAX_WEATHER_ZONES		50	// so we can more zones that are smaller
#define	MAX_PUFF_SYSTEMS		2
#define	MAX_PARTICLE_CLOUDS		5
#define POINTCACHE_CELL_SIZE	32.0f

////////////////////////////////////////////////////////////////////////////////////////
// Globals
////////////////////////////////////////////////////////////////////////////////////////
float		mMillisecondsElapsed = 0;
float		mSecondsElapsed = 0;
bool		mFrozen = false;

CVec3		mGlobalWindVelocity;
CVec3		mGlobalWindDirection;
float		mGlobalWindSpeed;
int			mParticlesRendered;



////////////////////////////////////////////////////////////////////////////////////////
// Handy Functions
////////////////////////////////////////////////////////////////////////////////////////
inline void		VectorMA( vec3_t vecAdd, const float scale, const vec3_t vecScale)
{
	vecAdd[0] += (scale * vecScale[0]);
	vecAdd[1] += (scale * vecScale[1]);
	vecAdd[2] += (scale * vecScale[2]);
}

inline void VectorFloor(vec3_t in)
{
	in[0] = floorf(in[0]);
	in[1] = floorf(in[1]);
	in[2] = floorf(in[2]);
}

inline void VectorCeil(vec3_t in)
{
	in[0] = ceilf(in[0]);
	in[1] = ceilf(in[1]);
	in[2] = ceilf(in[2]);
}

inline float	FloatRand(void)
{
	return ((float)rand() / (float)RAND_MAX);
}

inline float	fast_flrand(float min, float max)
{
	//return min + (max - min) * flrand;
	return Q_flrand(min, max); //fixme?
}

inline	void	SnapFloatToGrid(float& f, int GridSize)
{
	f = (int)(f);

	bool	fNeg		= (f<0);
	if (fNeg)
	{
		f *= -1;		// Temporarly make it positive
	}

	int		Offset		= ((int)(f) % (int)(GridSize));
	int		OffsetAbs	= abs(Offset);
	if (OffsetAbs>(GridSize/2))
	{
		Offset = (GridSize - OffsetAbs) * -1;
	}

	f -= Offset;

	if (fNeg)
	{
		f *= -1;		// Put It Back To Negative
	}

	f = (int)(f);

	assert(((int)(f)%(int)(GridSize)) == 0);
}

inline	void	SnapVectorToGrid(CVec3& Vec, int GridSize)
{
	SnapFloatToGrid(Vec[0], GridSize);
	SnapFloatToGrid(Vec[1], GridSize);
	SnapFloatToGrid(Vec[2], GridSize);
}





////////////////////////////////////////////////////////////////////////////////////////
// Range Structures
////////////////////////////////////////////////////////////////////////////////////////
struct	SVecRange
{
	CVec3	mMins;
	CVec3	mMaxs;

	inline void	Clear()
	{
		mMins.Clear();
		mMaxs.Clear();
	}

	inline void Pick(CVec3& V)
	{
		V[0] = Q_flrand(mMins[0], mMaxs[0]);
		V[1] = Q_flrand(mMins[1], mMaxs[1]);
		V[2] = Q_flrand(mMins[2], mMaxs[2]);
	}
	inline void Wrap(CVec3& V)
	{
		if (V[0]<=mMins[0])
		{
			if ((mMins[0]-V[0])>500)
			{
				Pick(V);
				return;
			}
			V[0] = mMaxs[0] - 10.0f;
		}
		if (V[0]>=mMaxs[0])
		{
			if ((V[0]-mMaxs[0])>500)
			{
				Pick(V);
				return;
			}
			V[0] = mMins[0] + 10.0f;
		}

		if (V[1]<=mMins[1])
		{
			if ((mMins[1]-V[1])>500)
			{
				Pick(V);
				return;
			}
			V[1] = mMaxs[1] - 10.0f;
		}
		if (V[1]>=mMaxs[1])
		{
			if ((V[1]-mMaxs[1])>500)
			{
				Pick(V);
				return;
			}
			V[1] = mMins[1] + 10.0f;
		}

		if (V[2]<=mMins[2])
		{
			if ((mMins[2]-V[2])>500)
			{
				Pick(V);
				return;
			}
			V[2] = mMaxs[2] - 10.0f;
		}
		if (V[2]>=mMaxs[2])
		{
			if ((V[2]-mMaxs[2])>500)
			{
				Pick(V);
				return;
			}
			V[2] = mMins[2] + 10.0f;
		}
	}

	inline bool In(const CVec3& V)
	{
		return (V>mMins && V<mMaxs);
	}
};

struct	SFloatRange
{
	float	mMin;
	float	mMax;

	inline void	Clear()
	{
		mMin = 0;
		mMin = 0;
	}
	inline void Pick(float& V)
	{
		V = Q_flrand(mMin, mMax);
	}
	inline bool In(const float& V)
	{
		return (V>mMin && V<mMax);
	}
};

struct	SIntRange
{
	int	mMin;
	int	mMax;

	inline void	Clear()
	{
		mMin = 0;
		mMax = 0;
	}
	inline void Pick(int& V)
	{
		V = Q_irand(mMin, mMax);
	}
	inline bool In(const int& V)
	{
		return (V>mMin && V<mMax);
	}
};





////////////////////////////////////////////////////////////////////////////////////////
// The Particle Class
////////////////////////////////////////////////////////////////////////////////////////
class	WFXParticle
{
public:
	enum
	{
		FLAG_RENDER = 0,

		FLAG_FADEIN,
		FLAG_FADEOUT,
		FLAG_RESPAWN,

		FLAG_MAX
	};
	typedef		ratl::bits_vs<FLAG_MAX>		TFlags;

	float	mAlpha;
	TFlags	mFlags;
	CVec3	mPosition;
	CVec3	mVelocity;
	float	mMass;			// A higher number will more greatly resist force and result in greater gravity
};





////////////////////////////////////////////////////////////////////////////////////////
// The Wind
////////////////////////////////////////////////////////////////////////////////////////
class	CWindZone
{
public:
	bool		mGlobal;
	SVecRange	mRBounds;
	SVecRange	mRVelocity;
	SIntRange	mRDuration;
	SIntRange	mRDeadTime;
	float		mMaxDeltaVelocityPerUpdate;
	float		mChanceOfDeadTime;

	CVec3		mCurrentVelocity;
	CVec3		mTargetVelocity;
	int			mTargetVelocityTimeRemaining;


public:
	////////////////////////////////////////////////////////////////////////////////////
	// Initialize - Will setup default values for all data
	////////////////////////////////////////////////////////////////////////////////////
	void		Initialize()
	{
		mRBounds.Clear();
		mGlobal						= true;

		mRVelocity.mMins			= -1500.0f;
		mRVelocity.mMins[2]			= -10.0f;
		mRVelocity.mMaxs			= 1500.0f;
		mRVelocity.mMaxs[2]			= 10.0f;

		mMaxDeltaVelocityPerUpdate	= 10.0f;

		mRDuration.mMin				= 1000;
		mRDuration.mMax				= 2000;

		mChanceOfDeadTime			= 0.3f;
		mRDeadTime.mMin				= 1000;
		mRDeadTime.mMax				= 3000;

		mCurrentVelocity.Clear();
		mTargetVelocity.Clear();
		mTargetVelocityTimeRemaining = 0;
	}

	////////////////////////////////////////////////////////////////////////////////////
	// Update - Changes wind when current target velocity expires
	////////////////////////////////////////////////////////////////////////////////////
	void		Update()
	{
		if (mTargetVelocityTimeRemaining==0)
		{
			if (FloatRand()<mChanceOfDeadTime)
			{
				mRDeadTime.Pick(mTargetVelocityTimeRemaining);
				mTargetVelocity.Clear();
			}
			else
			{
				mRDuration.Pick(mTargetVelocityTimeRemaining);
				mRVelocity.Pick(mTargetVelocity);
			}
		}
		else if (mTargetVelocityTimeRemaining!=-1)
		{
			mTargetVelocityTimeRemaining--;

			CVec3	DeltaVelocity(mTargetVelocity - mCurrentVelocity);
			float	DeltaVelocityLen = VectorNormalize(DeltaVelocity.v);
			if (DeltaVelocityLen > mMaxDeltaVelocityPerUpdate)
			{
				DeltaVelocityLen = mMaxDeltaVelocityPerUpdate;
			}
			DeltaVelocity *= (DeltaVelocityLen);
			mCurrentVelocity += DeltaVelocity;
		}
	}
};
ratl::vector_vs<CWindZone, MAX_WIND_ZONES>		mWindZones;
ratl::vector_vs<CWindZone*, MAX_WIND_ZONES>		mLocalWindZones;

bool R_GetWindVector(vec3_t windVector, vec3_t atpoint)
{
	VectorCopy(mGlobalWindDirection.v, windVector);
	if (atpoint && mLocalWindZones.size())
	{
		for (int curLocalWindZone=0; curLocalWindZone<mLocalWindZones.size(); curLocalWindZone++)
		{
			if (mLocalWindZones[curLocalWindZone]->mRBounds.In(atpoint))
			{
				VectorAdd(windVector, mLocalWindZones[curLocalWindZone]->mCurrentVelocity.v, windVector);
			}
		}
		VectorNormalize(windVector);
	}
	return true;
}

bool R_GetWindSpeed(float &windSpeed, vec3_t atpoint)
{
	windSpeed = mGlobalWindSpeed;
	if (atpoint && mLocalWindZones.size())
	{
		for (int curLocalWindZone=0; curLocalWindZone<mLocalWindZones.size(); curLocalWindZone++)
		{
			if (mLocalWindZones[curLocalWindZone]->mRBounds.In(atpoint))
			{
				windSpeed += VectorLength(mLocalWindZones[curLocalWindZone]->mCurrentVelocity.v);
			}
		}
	}
	return true;
}

bool R_GetWindGusting(vec3_t atpoint)
{
	float windSpeed;
	R_GetWindSpeed(windSpeed, atpoint);
	return (windSpeed>1000.0f);
}

////////////////////////////////////////////////////////////////////////////////////////
// Outside Point Cache
////////////////////////////////////////////////////////////////////////////////////////
class COutside
{
#define COUTSIDE_STRUCT_VERSION 1	// you MUST increase this any time you change any binary (fields) inside this class, or cahced files will fuck up
public:
	////////////////////////////////////////////////////////////////////////////////////
	//Global Public Outside Variables
	////////////////////////////////////////////////////////////////////////////////////

	bool			mOutsideShake;
	float			mOutsidePain;

	CVec3			mFogColor;
	int				mFogColorInt;
	bool			mFogColorTempActive;

private:
	////////////////////////////////////////////////////////////////////////////////////
	// The Outside Cache
	////////////////////////////////////////////////////////////////////////////////////
	bool			mCacheInit;			// Has It Been Cached?

	struct SWeatherZone
	{
		static bool	mMarkedOutside;
		uint32_t	*mPointCache;			// malloc block ptr

		int			miPointCacheByteSize;	// size of block
		SVecRange	mExtents;
		SVecRange	mSize;
		int			mWidth;
		int			mHeight;
		int			mDepth;

		void WriteToDisk( fileHandle_t f )
		{
			ri.FS_Write(&mMarkedOutside,sizeof(mMarkedOutside),f);
			ri.FS_Write( mPointCache, miPointCacheByteSize, f );
		}

		void ReadFromDisk( fileHandle_t f )
		{
			ri.FS_Read(&mMarkedOutside,sizeof(mMarkedOutside),f);
			ri.FS_Read( mPointCache, miPointCacheByteSize, f);
		}

		////////////////////////////////////////////////////////////////////////////////////
		// Convert To Cell
		////////////////////////////////////////////////////////////////////////////////////
		inline	void	ConvertToCell(const CVec3& pos, int& x, int& y, int& z, int& bit)
		{
			x = (int)((pos[0] / POINTCACHE_CELL_SIZE) - mSize.mMins[0]);
			y = (int)((pos[1] / POINTCACHE_CELL_SIZE) - mSize.mMins[1]);
			z = (int)((pos[2] / POINTCACHE_CELL_SIZE) - mSize.mMins[2]);

			bit = (z & 31);
			z >>= 5;
		}

		////////////////////////////////////////////////////////////////////////////////////
		// CellOutside - Test to see if a given cell is outside
		////////////////////////////////////////////////////////////////////////////////////
		inline	bool	CellOutside(int x, int y, int z, int bit)
		{
			if ((x < 0 || x >= mWidth) || (y < 0 || y >= mHeight) || (z < 0 || z >= mDepth) || (bit < 0 || bit >= 32))
			{
				return !(mMarkedOutside);
			}
			return (mMarkedOutside==(!!(mPointCache[((z * mWidth * mHeight) + (y * mWidth) + x)]&(1 << bit))));
		}
	};
	ratl::vector_vs<SWeatherZone, MAX_WEATHER_ZONES>	mWeatherZones;


private:
	////////////////////////////////////////////////////////////////////////////////////
	// Iteration Variables
	////////////////////////////////////////////////////////////////////////////////////
	int				mWCells;
	int				mHCells;

	int				mXCell;
	int				mYCell;
	int				mZBit;

	int				mXMax;
	int				mYMax;
	int				mZMax;


private:


	////////////////////////////////////////////////////////////////////////////////////
	// Contents Outside
	////////////////////////////////////////////////////////////////////////////////////
	inline	bool	ContentsOutside(int contents)
	{
		if (contents&CONTENTS_WATER || contents&CONTENTS_SOLID)
		{
			return false;
		}
		if (mCacheInit)
		{
			if (SWeatherZone::mMarkedOutside)
			{
				return (!!(contents&CONTENTS_OUTSIDE));
			}
			return (!(contents&CONTENTS_INSIDE));
		}
		return !!(contents&CONTENTS_OUTSIDE);
	}




public:
	////////////////////////////////////////////////////////////////////////////////////
	// Constructor - Will setup default values for all data
	////////////////////////////////////////////////////////////////////////////////////
	void Reset()
	{
		mOutsideShake = false;
		mOutsidePain = 0.0;
		mCacheInit = false;
		SWeatherZone::mMarkedOutside = false;

		mFogColor.Clear();
		mFogColorInt = 0;
		mFogColorTempActive = false;

		for (int wz=0; wz<mWeatherZones.size(); wz++)
		{
			R_Free(mWeatherZones[wz].mPointCache);
			mWeatherZones[wz].mPointCache = 0;
			mWeatherZones[wz].miPointCacheByteSize = 0;	// not really necessary because of .clear() below, but keeps things together in case stuff changes
		}
		mWeatherZones.clear();
	}

	COutside()
	{
		Reset();
	}
	~COutside()
	{
		Reset();
	}

	bool			Initialized()
	{
		return mCacheInit;
	}

	////////////////////////////////////////////////////////////////////////////////////
	// AddWeatherZone - Will add a zone of mins and maxes
	////////////////////////////////////////////////////////////////////////////////////
	void			AddWeatherZone(vec3_t mins, vec3_t maxs)
	{
		if (mCacheInit)
		{
			return;
		}
		if (!mWeatherZones.full())
		{
			SWeatherZone&	Wz = mWeatherZones.push_back();
			Wz.mExtents.mMins = mins;
			Wz.mExtents.mMaxs = maxs;

			SnapVectorToGrid(Wz.mExtents.mMins, POINTCACHE_CELL_SIZE);
			SnapVectorToGrid(Wz.mExtents.mMaxs, POINTCACHE_CELL_SIZE);

			Wz.mSize.mMins = Wz.mExtents.mMins;
			Wz.mSize.mMaxs = Wz.mExtents.mMaxs;

			Wz.mSize.mMins /= POINTCACHE_CELL_SIZE;
			Wz.mSize.mMaxs /= POINTCACHE_CELL_SIZE;
			Wz.mWidth		=  (int)(Wz.mSize.mMaxs[0] - Wz.mSize.mMins[0]);
			Wz.mHeight		=  (int)(Wz.mSize.mMaxs[1] - Wz.mSize.mMins[1]);
			Wz.mDepth		= ((int)(Wz.mSize.mMaxs[2] - Wz.mSize.mMins[2]) + 31) >> 5;

			Wz.miPointCacheByteSize = (Wz.mWidth * Wz.mHeight * Wz.mDepth) * sizeof(uint32_t);
			Wz.mPointCache  = (uint32_t *)R_Malloc( Wz.miPointCacheByteSize, TAG_POINTCACHE, qtrue );
		}
		else
		{
			assert("MaxWeatherZones Hit!"==0);
		}
	}

	const char *GenCachedWeatherFilename(void)
	{
		return va("maps/%s.weather", sv_mapname->string);
	}

	// weather file format...
	//
	struct WeatherFileHeader_t
	{
		int m_iVersion;
		int m_iChecksum;

		WeatherFileHeader_t()
		{
			m_iVersion			= COUTSIDE_STRUCT_VERSION;
			m_iChecksum			= sv_mapChecksum->integer;
		}
	};

	fileHandle_t WriteCachedWeatherFile( void )
	{
		fileHandle_t f = ri.FS_FOpenFileWrite( GenCachedWeatherFilename(), qtrue );
		if (f)
		{
			WeatherFileHeader_t WeatherFileHeader;

            ri.FS_Write(&WeatherFileHeader, sizeof(WeatherFileHeader), f);
			return f;
		}
		else
		{
			ri.Printf( PRINT_WARNING, "(Unable to open weather file \"%s\" for writing!)\n",GenCachedWeatherFilename());
		}

		return 0;
	}

	// returns 0 for not-found or invalid file, else open handle to continue read from (which you then close yourself)...
	//
	fileHandle_t ReadCachedWeatherFile( void )
	{
		fileHandle_t f = 0;
		ri.FS_FOpenFileRead( GenCachedWeatherFilename(), &f, qfalse );
		if ( f )
		{
			// ok, it exists, but is it valid for this map?...
			//
			WeatherFileHeader_t WeatherFileHeaderForCompare;
			WeatherFileHeader_t WeatherFileHeaderFromDisk;

			ri.FS_Read(&WeatherFileHeaderFromDisk, sizeof(WeatherFileHeaderFromDisk), f);

			if (!memcmp(&WeatherFileHeaderForCompare, &WeatherFileHeaderFromDisk, sizeof(WeatherFileHeaderFromDisk)))
			{
				// go for it...
				//
				return f;
			}

            ri.Printf( PRINT_WARNING, "( Cached weather file \"%s\" out of date, regenerating... )\n",GenCachedWeatherFilename());
			ri.FS_FCloseFile( f );
		}
		else
		{
			ri.Printf( PRINT_WARNING, "( No cached weather file found, generating... )\n");
		}

		return 0;
	}

	////////////////////////////////////////////////////////////////////////////////////
	// Cache - Will Scan the World, Creating The Cache
	////////////////////////////////////////////////////////////////////////////////////
	void			Cache()
	{
		if (!tr.world || mCacheInit)
		{
			return;
		}

		// all this piece of code does really is fill in the bool "SWeatherZone::mMarkedOutside", plus the mPointCache[] for each zone,
		//	so we can diskload those. Maybe.
		fileHandle_t f = ReadCachedWeatherFile();
		if ( f )
		{
			for (int iZone=0; iZone<mWeatherZones.size(); iZone++)
			{
				SWeatherZone wz = mWeatherZones[iZone];
				wz.ReadFromDisk( f );
			}

			mCacheInit = true;
		}
		else
		{
			CVec3		CurPos;
			CVec3		Size;
			CVec3		Mins;
			int			x, y, z, q, zbase;
			bool		curPosOutside;
			uint32_t		contents;
			uint32_t		bit;


			// Record The Extents Of The World Incase No Other Weather Zones Exist
			//---------------------------------------------------------------------
			if (!mWeatherZones.size())
			{
				Com_Printf("WARNING: No Weather Zones Encountered\n");
				AddWeatherZone(tr.world->bmodels[0].bounds[0], tr.world->bmodels[0].bounds[1]);
			}

			f = WriteCachedWeatherFile();

			// Iterate Over All Weather Zones
			//--------------------------------
			for (int zone=0; zone<mWeatherZones.size(); zone++)
			{
				SWeatherZone	wz = mWeatherZones[zone];

				// Make Sure Point Contents Checks Occur At The CENTER Of The Cell
				//-----------------------------------------------------------------
				Mins = wz.mExtents.mMins;
				for (x=0; x<3; x++)
				{
					Mins[x] += (POINTCACHE_CELL_SIZE/2);
				}


				// Start Scanning
				//----------------
				for(z = 0; z < wz.mDepth; z++)
				{
					for(q = 0; q < 32; q++)
					{
						bit = (1 << q);
						zbase = (z << 5);

						for(x = 0; x < wz.mWidth; x++)
						{
							for(y = 0; y < wz.mHeight; y++)
							{
								CurPos[0] = x			* POINTCACHE_CELL_SIZE;
								CurPos[1] = y			* POINTCACHE_CELL_SIZE;
								CurPos[2] = (zbase + q)	* POINTCACHE_CELL_SIZE;
								CurPos	  += Mins;

								contents = ri.CM_PointContents(CurPos.v, 0);
								if (contents&CONTENTS_INSIDE || contents&CONTENTS_OUTSIDE)
								{
									curPosOutside = ((contents&CONTENTS_OUTSIDE)!=0);
									if (!mCacheInit)
									{
										mCacheInit = true;
										SWeatherZone::mMarkedOutside = curPosOutside;
									}
									else if (SWeatherZone::mMarkedOutside!=curPosOutside)
									{
										assert(0);
										Com_Error (ERR_DROP, "Weather Effect: Both Indoor and Outdoor brushs encountered in map.\n" );
										return;
									}

									// Mark The Point
									//----------------
									wz.mPointCache[((z * wz.mWidth * wz.mHeight) + (y * wz.mWidth) + x)] |= bit;
								}
							}// for (y)
						}// for (x)
					}// for (q)
				}// for (z)

				if (f)
				{
					mWeatherZones[ zone ].WriteToDisk( f );
				}
			}
		}

		if (f)
		{
			ri.FS_FCloseFile(f);
			f=0;	// not really necessary, but wtf.
		}

		// If no indoor or outdoor brushes were found
		//--------------------------------------------
		if (!mCacheInit)
		{
			mCacheInit = true;
			SWeatherZone::mMarkedOutside = false;		// Assume All Is Outside, Except Solid
		}
	}

public:
	////////////////////////////////////////////////////////////////////////////////////
	// PointOutside - Test to see if a given point is outside
	////////////////////////////////////////////////////////////////////////////////////
	inline	bool	PointOutside(const CVec3& pos)
	{
		if (!mCacheInit)
		{
			return ContentsOutside(ri.CM_PointContents(pos.v, 0));
		}
		for (int zone=0; zone<mWeatherZones.size(); zone++)
		{
			SWeatherZone	wz = mWeatherZones[zone];
			if (wz.mExtents.In(pos))
			{
				int		bit, x, y, z;
				wz.ConvertToCell(pos, x, y, z, bit);
				return wz.CellOutside(x, y, z, bit);
			}
		}
		return !(SWeatherZone::mMarkedOutside);

	}


	////////////////////////////////////////////////////////////////////////////////////
	// PointOutside - Test to see if a given bounded plane is outside
	////////////////////////////////////////////////////////////////////////////////////
	inline	bool	PointOutside(const CVec3& pos, float width, float height)
	{
		for (int zone=0; zone<mWeatherZones.size(); zone++)
		{
			SWeatherZone	wz = mWeatherZones[zone];
			if (wz.mExtents.In(pos))
			{
				int		bit, x, y, z;
				wz.ConvertToCell(pos, x, y, z, bit);
				if (width<POINTCACHE_CELL_SIZE || height<POINTCACHE_CELL_SIZE)
				{
 					return (wz.CellOutside(x, y, z, bit));
				}

				mWCells = ((int)width  / POINTCACHE_CELL_SIZE);
				mHCells = ((int)height / POINTCACHE_CELL_SIZE);

				mXMax = x + mWCells;
				mYMax = y + mWCells;
				mZMax = bit + mHCells;

				for (mXCell=x-mWCells; mXCell<=mXMax; mXCell++)
				{
					for (mYCell=y-mWCells; mYCell<=mYMax; mYCell++)
					{
						for (mZBit=bit-mHCells; mZBit<=mZMax; mZBit++)
						{
							if (!wz.CellOutside(mXCell, mYCell, z, mZBit))
							{
								return false;
							}
						}
					}
				}
				return true;
			}
		}
		return !(SWeatherZone::mMarkedOutside);
	}
};
COutside			mOutside;
bool				COutside::SWeatherZone::mMarkedOutside = false;


void R_AddWeatherZone(vec3_t mins, vec3_t maxs)
{
	mOutside.AddWeatherZone(mins, maxs);
}

bool R_IsOutside(vec3_t pos)
{
	return mOutside.PointOutside(pos);
}

bool R_IsShaking(vec3_t pos)
{
	return (mOutside.mOutsideShake && mOutside.PointOutside(pos));
}

float R_IsOutsideCausingPain(vec3_t pos)
{
	return (mOutside.mOutsidePain && mOutside.PointOutside(pos));
}

bool R_SetTempGlobalFogColor(vec3_t color)
{
	if (tr.world && tr.world->globalFog != -1)
	{
		// If Non Zero, Try To Set The Color
		//-----------------------------------
		if (color[0] || color[1] || color[2])
		{
			// Remember The Normal Fog Color
			//-------------------------------
			if (!mOutside.mFogColorTempActive)
			{
				mOutside.mFogColor				= tr.world->fogs[tr.world->globalFog].parms.color;
				mOutside.mFogColorInt			= tr.world->fogs[tr.world->globalFog].colorInt;
				mOutside.mFogColorTempActive	= true;
			}

			// Set The New One
			//-----------------
			tr.world->fogs[tr.world->globalFog].parms.color[0] = color[0];
			tr.world->fogs[tr.world->globalFog].parms.color[1] = color[1];
			tr.world->fogs[tr.world->globalFog].parms.color[2] = color[2];
			tr.world->fogs[tr.world->globalFog].colorInt = ColorBytes4 (
												color[0] * tr.identityLight,
												color[1] * tr.identityLight,
												color[2] * tr.identityLight,
												1.0 );
		}

		// If Unable TO Parse The Command Color Vector, Restore The Previous Fog Color
		//-----------------------------------------------------------------------------
		else if (mOutside.mFogColorTempActive)
		{
			mOutside.mFogColorTempActive = false;

			tr.world->fogs[tr.world->globalFog].parms.color[0] = mOutside.mFogColor[0];
			tr.world->fogs[tr.world->globalFog].parms.color[1] = mOutside.mFogColor[1];
			tr.world->fogs[tr.world->globalFog].parms.color[2] = mOutside.mFogColor[2];
			tr.world->fogs[tr.world->globalFog].colorInt	   = mOutside.mFogColorInt;
		}
	}
	return true;
}

////////////////////////////////////////////////////////////////////////////////////////
// Particle Cloud
////////////////////////////////////////////////////////////////////////////////////////
class	CParticleCloud
{
private:
	////////////////////////////////////////////////////////////////////////////////////
	// DYNAMIC MEMORY
	////////////////////////////////////////////////////////////////////////////////////
	image_t*	mImage;
	WFXParticle*	mParticles;

private:
	////////////////////////////////////////////////////////////////////////////////////
	// RUN TIME VARIANTS
	////////////////////////////////////////////////////////////////////////////////////
	float		mSpawnSpeed;
	CVec3		mSpawnPlaneNorm;
	CVec3		mSpawnPlaneRight;
	CVec3		mSpawnPlaneUp;
	SVecRange	mRange;

	CVec3		mCameraPosition;
	CVec3		mCameraForward;
	CVec3		mCameraLeft;
	CVec3		mCameraDown;
	CVec3		mCameraLeftPlusUp;
	CVec3		mCameraLeftMinusUp;


	int			mParticleCountRender;
	int			mGLModeEnum;

	bool		mPopulated;


public:
	////////////////////////////////////////////////////////////////////////////////////
	// CONSTANTS
	////////////////////////////////////////////////////////////////////////////////////
	bool		mOrientWithVelocity;
	float		mSpawnPlaneSize;
	float		mSpawnPlaneDistance;
	SVecRange	mSpawnRange;

	float		mGravity;			// How much gravity affects the velocity of a particle
	CVec4		mColor;				// RGBA color
	int			mVertexCount;		// 3 for triangle, 4 for quad, other numbers not supported

	float		mWidth;
	float		mHeight;

	int			mBlendMode;			// 0 = ALPHA, 1 = SRC->SRC
	int			mFilterMode;		// 0 = LINEAR, 1 = NEAREST

	float		mFade;				// How much to fade in and out 1.0 = instant, 0.01 = very slow

	SFloatRange	mRotation;
	float		mRotationDelta;
	float		mRotationDeltaTarget;
	float		mRotationCurrent;
	SIntRange	mRotationChangeTimer;
	int			mRotationChangeNext;

	SFloatRange	mMass;				// Determines how slowness to accelerate, higher number = slower
	float		mFrictionInverse;	// How much air friction does this particle have 1.0=none, 0.0=nomove

	int			mParticleCount;

	bool		mWaterParticles;




public:
	////////////////////////////////////////////////////////////////////////////////////
	// Initialize - Create Image, Particles, And Setup All Values
	////////////////////////////////////////////////////////////////////////////////////
	void	Initialize(int count, const char* texturePath, int VertexCount=4)
	{
		Reset();
		assert(mParticleCount==0 && mParticles==0);
		assert(mImage==0);

		// Create The Image
		//------------------
		mImage = R_FindImageFile(texturePath, qfalse, qfalse, qfalse, GL_CLAMP);
		if (!mImage)
		{
			Com_Error(ERR_DROP, "CParticleCloud: Could not texture %s", texturePath);
		}

		GL_Bind(mImage);



		// Create The Particles
		//----------------------
		mParticleCount	= count;
		mParticles		= new WFXParticle[mParticleCount];



		WFXParticle*	part=0;
		for (int particleNum=0; particleNum<mParticleCount; particleNum++)
		{
			part = &(mParticles[particleNum]);
			part->mPosition.Clear();
			part->mVelocity.Clear();
			part->mAlpha	= 0.0f;
			mMass.Pick(part->mMass);
		}

		mVertexCount = VertexCount;
		mGLModeEnum = (mVertexCount==3)?(GL_TRIANGLES):(GL_QUADS);
	}


	////////////////////////////////////////////////////////////////////////////////////
	// Reset - Initializes all data to default values
	////////////////////////////////////////////////////////////////////////////////////
	void		Reset()
	{
		if (mImage)
		{
			// TODO: Free Image?
		}
		mImage				= 0;
		if (mParticleCount)
		{
			delete [] mParticles;
		}
		mParticleCount		= 0;
		mParticles			= 0;

		mPopulated			= 0;



		// These Are The Default Startup Values For Constant Data
		//========================================================
		mOrientWithVelocity = false;
		mWaterParticles		= false;

		mSpawnPlaneDistance	= 500;
		mSpawnPlaneSize		= 500;
		mSpawnRange.mMins	= -(mSpawnPlaneDistance*1.25f);
		mSpawnRange.mMaxs	=  (mSpawnPlaneDistance*1.25f);

		mGravity			= 300.0f;	// Units Per Second

		mColor				= 1.0f;

		mVertexCount		= 4;
		mWidth				= 1.0f;
		mHeight				= 1.0f;

		mBlendMode			= 0;
		mFilterMode			= 0;

		mFade				= 10.0f;

		mRotation.Clear();
		mRotationDelta		= 0.0f;
		mRotationDeltaTarget= 0.0f;
		mRotationCurrent	= 0.0f;
		mRotationChangeNext	= -1;
		mRotation.mMin		= -0.7f;
		mRotation.mMax		=  0.7f;
		mRotationChangeTimer.mMin = 500;
		mRotationChangeTimer.mMax = 2000;

		mMass.mMin			= 5.0f;
		mMass.mMax			= 10.0f;

		mFrictionInverse	= 0.7f;		// No Friction?
	}

	////////////////////////////////////////////////////////////////////////////////////
	// Constructor - Will setup default values for all data
	////////////////////////////////////////////////////////////////////////////////////
	CParticleCloud()
	{
		mImage = 0;
		mParticleCount = 0;
		Reset();
	}

	////////////////////////////////////////////////////////////////////////////////////
	// Initialize - Will setup default values for all data
	////////////////////////////////////////////////////////////////////////////////////
	~CParticleCloud()
	{
		Reset();
	}


	////////////////////////////////////////////////////////////////////////////////////
	// UseSpawnPlane - Check To See If We Should Spawn On A Plane, Or Just Wrap The Box
	////////////////////////////////////////////////////////////////////////////////////
	inline bool	UseSpawnPlane()
	{
		return (mGravity!=0.0f);
	}


	////////////////////////////////////////////////////////////////////////////////////
	// Update - Applies All Physics Forces To All Contained Particles
	////////////////////////////////////////////////////////////////////////////////////
	void		Update()
	{
		WFXParticle*	part=0;
		CVec3		partForce;
		CVec3		partMoved;
		CVec3		partToCamera;
		bool		partRendering;
		bool		partOutside;
		bool		partInRange;
		bool		partInView;
		int			particleNum;
		float		particleFade = (mFade * mSecondsElapsed);
		int			numLocalWindZones = mLocalWindZones.size();
		int			curLocalWindZone;


		// Compute Camera
		//----------------
		{
			mCameraPosition	= backEnd.viewParms.ori.origin;
			mCameraForward	= backEnd.viewParms.ori.axis[0];
			mCameraLeft		= backEnd.viewParms.ori.axis[1];
			mCameraDown		= backEnd.viewParms.ori.axis[2];

			if (mRotationChangeNext!=-1)
			{
				if (mRotationChangeNext==0)
				{
					mRotation.Pick(mRotationDeltaTarget);
					mRotationChangeTimer.Pick(mRotationChangeNext);
					if (mRotationChangeNext<=0)
					{
						mRotationChangeNext = 1;
					}
				}
				mRotationChangeNext--;

				float	RotationDeltaDifference = (mRotationDeltaTarget - mRotationDelta);
				if (fabsf(RotationDeltaDifference)>0.01)
				{
					mRotationDelta += RotationDeltaDifference;		// Blend To New Delta
				}
                mRotationCurrent += (mRotationDelta * mSecondsElapsed);
				float s = sinf(mRotationCurrent);
				float c = cosf(mRotationCurrent);

				CVec3	TempCamLeft(mCameraLeft);

				mCameraLeft *= (c * mWidth);
				mCameraLeft.ScaleAdd(mCameraDown, (s * mWidth * -1.0f));

				mCameraDown *= (c * mHeight);
				mCameraDown.ScaleAdd(TempCamLeft, (s * mHeight));
			}
			else
			{
				mCameraLeft		*= mWidth;
 				mCameraDown		*= mHeight;
			}
		}


		// Compute Global Force
		//----------------------
		CVec3		force;
		{
			force.Clear();

			// Apply Gravity
			//---------------
			force[2] = -1.0f * mGravity;

			// Apply Wind Velocity
			//---------------------
			force    += mGlobalWindVelocity;
		}


		// Update Range
		//--------------
		{
			mRange.mMins = mCameraPosition + mSpawnRange.mMins;
			mRange.mMaxs = mCameraPosition + mSpawnRange.mMaxs;

			// If Using A Spawn Plane, Increase The Range Box A Bit To Account For Rotation On The Spawn Plane
			//-------------------------------------------------------------------------------------------------
			if (UseSpawnPlane())
			{
				for (int dim=0; dim<3; dim++)
				{
					if (force[dim]>0.01)
					{
						mRange.mMins[dim] -= (mSpawnPlaneDistance/2.0f);
					}
					else if (force[dim]<-0.01)
					{
						mRange.mMaxs[dim] += (mSpawnPlaneDistance/2.0f);
					}
				}
				mSpawnPlaneNorm	= force;
				mSpawnSpeed		= VectorNormalize(mSpawnPlaneNorm.v);
				MakeNormalVectors(mSpawnPlaneNorm.v, mSpawnPlaneRight.v, mSpawnPlaneUp.v);
			}

			// Optimization For Quad Position Calculation
			//--------------------------------------------
			if (mVertexCount==4)
			{
		 		mCameraLeftPlusUp  = (mCameraLeft - mCameraDown);
				mCameraLeftMinusUp = (mCameraLeft + mCameraDown);
			}
			else
			{
				mCameraLeftPlusUp  = (mCameraDown + mCameraLeft);		// should really be called mCamera Left + Down
			}
		}

		// Stop All Additional Processing
		//--------------------------------
		if (mFrozen)
		{
			return;
		}



		// Now Update All Particles
		//--------------------------
		mParticleCountRender = 0;
		for (particleNum=0; particleNum<mParticleCount; particleNum++)
		{
			part			= &(mParticles[particleNum]);

			if (!mPopulated)
			{
				mRange.Pick(part->mPosition);		// First Time Spawn Location
			}

			// Grab The Force And Apply Non Global Wind
			//------------------------------------------
			partForce = force;

			if (numLocalWindZones)
			{
				for (curLocalWindZone=0; curLocalWindZone<numLocalWindZones; curLocalWindZone++)
				{
					if (mLocalWindZones[curLocalWindZone]->mRBounds.In(part->mPosition))
					{
						partForce += mLocalWindZones[curLocalWindZone]->mCurrentVelocity;
					}
				}
			}

			partForce /= part->mMass;


			// Apply The Force
			//-----------------
			part->mVelocity		+= partForce;
			part->mVelocity		*= mFrictionInverse;

			part->mPosition.ScaleAdd(part->mVelocity, mSecondsElapsed);

			partToCamera	= (part->mPosition - mCameraPosition);
			partRendering	= part->mFlags.get_bit(WFXParticle::FLAG_RENDER);
			partOutside		= mOutside.PointOutside(part->mPosition, mWidth, mHeight);
			partInRange		= mRange.In(part->mPosition);
			partInView		= (partOutside && partInRange && (partToCamera.Dot(mCameraForward)>0.0f));

			// Process Respawn
			//-----------------
			if (!partInRange && !partRendering)
			{
				part->mVelocity.Clear();

				// Reselect A Position On The Spawn Plane
				//----------------------------------------
				if (UseSpawnPlane())
				{
					part->mPosition		= mCameraPosition;
					part->mPosition		-= (mSpawnPlaneNorm* mSpawnPlaneDistance);
					part->mPosition		+= (mSpawnPlaneRight*Q_flrand(-mSpawnPlaneSize, mSpawnPlaneSize));
					part->mPosition		+= (mSpawnPlaneUp*   Q_flrand(-mSpawnPlaneSize, mSpawnPlaneSize));
				}

				// Otherwise, Just Wrap Around To The Other End Of The Range
				//-----------------------------------------------------------
				else
				{
					mRange.Wrap(part->mPosition);
				}
				partInRange = true;
			}

			// Process Fade
			//--------------
			{
				// Start A Fade Out
				//------------------
				if		(partRendering && !partInView)
				{
					part->mFlags.clear_bit(WFXParticle::FLAG_FADEIN);
					part->mFlags.set_bit(WFXParticle::FLAG_FADEOUT);
				}

				// Switch From Fade Out To Fade In
				//---------------------------------
				else if (partRendering && partInView && part->mFlags.get_bit(WFXParticle::FLAG_FADEOUT))
				{
					part->mFlags.set_bit(WFXParticle::FLAG_FADEIN);
					part->mFlags.clear_bit(WFXParticle::FLAG_FADEOUT);
				}

				// Start A Fade In
				//-----------------
				else if (!partRendering && partInView)
				{
					partRendering = true;
					part->mAlpha = 0.0f;
					part->mFlags.set_bit(WFXParticle::FLAG_RENDER);
					part->mFlags.set_bit(WFXParticle::FLAG_FADEIN);
					part->mFlags.clear_bit(WFXParticle::FLAG_FADEOUT);
				}

				// Update Fade
				//-------------
				if (partRendering)
				{

					// Update Fade Out
					//-----------------
					if (part->mFlags.get_bit(WFXParticle::FLAG_FADEOUT))
					{
						part->mAlpha -= particleFade;
						if (part->mAlpha<=0.0f)
						{
							part->mAlpha = 0.0f;
							part->mFlags.clear_bit(WFXParticle::FLAG_FADEOUT);
							part->mFlags.clear_bit(WFXParticle::FLAG_FADEIN);
							part->mFlags.clear_bit(WFXParticle::FLAG_RENDER);
							partRendering = false;
						}
					}

					// Update Fade In
					//----------------
					else if (part->mFlags.get_bit(WFXParticle::FLAG_FADEIN))
					{
						part->mAlpha += particleFade;
						if (part->mAlpha>=mColor[3])
						{
							part->mFlags.clear_bit(WFXParticle::FLAG_FADEIN);
							part->mAlpha = mColor[3];
						}
					}
				}
			}

			// Keep Track Of The Number Of Particles To Render
			//-------------------------------------------------
			if (part->mFlags.get_bit(WFXParticle::FLAG_RENDER))
			{
				mParticleCountRender ++;
			}
		}
		mPopulated = true;
	}

	////////////////////////////////////////////////////////////////////////////////////
	// Render -
	////////////////////////////////////////////////////////////////////////////////////
	void		Render()
	{
		WFXParticle*	part=0;
		int			particleNum;
		CVec3		partDirection;


		// Set The GL State And Image Binding
		//------------------------------------
		GL_State((mBlendMode==0)?(GLS_ALPHA):(GLS_SRCBLEND_ONE | GLS_DSTBLEND_ONE));
		GL_Bind(mImage);


		// Enable And Disable Things
		//---------------------------
		qglEnable(GL_TEXTURE_2D);
		qglDisable(GL_CULL_FACE);

		qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, (mFilterMode==0)?(GL_LINEAR):(GL_NEAREST));
		qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, (mFilterMode==0)?(GL_LINEAR):(GL_NEAREST));


		// Setup Matrix Mode And Translation
		//-----------------------------------
		qglMatrixMode(GL_MODELVIEW);
		qglPushMatrix();


		// Begin
		//-------
		qglBegin(mGLModeEnum);
		for (particleNum=0; particleNum<mParticleCount; particleNum++)
		{
			part = &(mParticles[particleNum]);
			if (!part->mFlags.get_bit(WFXParticle::FLAG_RENDER))
			{
				continue;
			}

			// If Oriented With Velocity, We Want To Calculate Vertx Offsets Differently For Each Particle
			//---------------------------------------------------------------------------------------------
			if (mOrientWithVelocity)
			{
				partDirection = part->mVelocity;
				VectorNormalize(partDirection.v);
				mCameraDown = partDirection;
				mCameraDown *= (mHeight * -1);
				if (mVertexCount==4)
				{
		 			mCameraLeftPlusUp  = (mCameraLeft - mCameraDown);
					mCameraLeftMinusUp = (mCameraLeft + mCameraDown);
				}
				else
				{
					mCameraLeftPlusUp  = (mCameraDown + mCameraLeft);
				}
			}

			// Blend Mode Zero -> Apply Alpha Just To Alpha Channel
			//------------------------------------------------------
			if (mBlendMode==0)
			{
				qglColor4f(mColor[0], mColor[1], mColor[2], part->mAlpha);
			}

			// Otherwise Apply Alpha To All Channels
			//---------------------------------------
			else
			{
				qglColor4f(mColor[0]*part->mAlpha, mColor[1]*part->mAlpha, mColor[2]*part->mAlpha, mColor[3]*part->mAlpha);
			}


			// Render A Triangle
			//-------------------
			if (mVertexCount==3)
			{
 				qglTexCoord2f(1.0, 0.0);
				qglVertex3f(part->mPosition[0],
							part->mPosition[1],
							part->mPosition[2]);

				qglTexCoord2f(0.0, 1.0);
				qglVertex3f(part->mPosition[0] + mCameraLeft[0],
							part->mPosition[1] + mCameraLeft[1],
							part->mPosition[2] + mCameraLeft[2]);

				qglTexCoord2f(0.0, 0.0);
				qglVertex3f(part->mPosition[0] + mCameraLeftPlusUp[0],
							part->mPosition[1] + mCameraLeftPlusUp[1],
							part->mPosition[2] + mCameraLeftPlusUp[2]);
			}

			// Render A Quad
			//---------------
			else
			{
				// Left bottom.
				qglTexCoord2f( 0.0, 0.0 );
				qglVertex3f(part->mPosition[0] - mCameraLeftMinusUp[0],
							part->mPosition[1] - mCameraLeftMinusUp[1],
							part->mPosition[2] - mCameraLeftMinusUp[2] );

				// Right bottom.
				qglTexCoord2f( 1.0, 0.0 );
				qglVertex3f(part->mPosition[0] - mCameraLeftPlusUp[0],
							part->mPosition[1] - mCameraLeftPlusUp[1],
							part->mPosition[2] - mCameraLeftPlusUp[2] );

				// Right top.
				qglTexCoord2f( 1.0, 1.0 );
				qglVertex3f(part->mPosition[0] + mCameraLeftMinusUp[0],
							part->mPosition[1] + mCameraLeftMinusUp[1],
							part->mPosition[2] + mCameraLeftMinusUp[2] );

				// Left top.
				qglTexCoord2f( 0.0, 1.0 );
				qglVertex3f(part->mPosition[0] + mCameraLeftPlusUp[0],
							part->mPosition[1] + mCameraLeftPlusUp[1],
							part->mPosition[2] + mCameraLeftPlusUp[2] );
			}
		}
		qglEnd();

		qglEnable(GL_CULL_FACE);
		qglPopMatrix();

		mParticlesRendered += mParticleCountRender;
	}
};
ratl::vector_vs<CParticleCloud, MAX_PARTICLE_CLOUDS>	mParticleClouds;



////////////////////////////////////////////////////////////////////////////////////////
// Init World Effects - Will Iterate Over All Particle Clouds, Clear Them Out, And Erase
////////////////////////////////////////////////////////////////////////////////////////
void R_InitWorldEffects(void)
{
	for (int i=0; i<mParticleClouds.size(); i++)
	{
		mParticleClouds[i].Reset();
	}
	mParticleClouds.clear();
	mWindZones.clear();
	mLocalWindZones.clear();
	mOutside.Reset();
	mGlobalWindSpeed = 0.0f;
	mGlobalWindDirection[0]=1.0f;
	mGlobalWindDirection[1]=0.0f;
	mGlobalWindDirection[2]=0.0f;
}

////////////////////////////////////////////////////////////////////////////////////////
// Init World Effects - Will Iterate Over All Particle Clouds, Clear Them Out, And Erase
////////////////////////////////////////////////////////////////////////////////////////
void R_ShutdownWorldEffects(void)
{
	R_InitWorldEffects();
}

////////////////////////////////////////////////////////////////////////////////////////
// RB_RenderWorldEffects - If any particle clouds exist, this will update and render them
////////////////////////////////////////////////////////////////////////////////////////
void RB_RenderWorldEffects(void)
{
	if (!tr.world ||
		(tr.refdef.rdflags & RDF_NOWORLDMODEL) ||
		(backEnd.refdef.rdflags & RDF_SKYBOXPORTAL) ||
		!mParticleClouds.size() ||
		ri.CL_IsRunningInGameCinematic())
	{	//  no world rendering or no world or no particle clouds
		return;
	}

	SetViewportAndScissor();
	qglMatrixMode(GL_MODELVIEW);
	qglLoadMatrixf(backEnd.viewParms.world.modelMatrix);


	// Calculate Elapsed Time For Scale Purposes
	//-------------------------------------------
	mMillisecondsElapsed = backEnd.refdef.frametime;
	if (mMillisecondsElapsed<1)
	{
		mMillisecondsElapsed = 1.0f;
	}
	if (mMillisecondsElapsed>1000.0f)
	{
		mMillisecondsElapsed = 1000.0f;
	}
	mSecondsElapsed = (mMillisecondsElapsed / 1000.0f);


	// Make Sure We Are Always Outside Cached
	//----------------------------------------
	if (!mOutside.Initialized())
	{
		mOutside.Cache();
	}
	else
	{
		// Update All Wind Zones
		//-----------------------
		if (!mFrozen)
		{
			mGlobalWindVelocity.Clear();
			for (int wz=0; wz<mWindZones.size(); wz++)
			{
				mWindZones[wz].Update();
				if (mWindZones[wz].mGlobal)
				{
					mGlobalWindVelocity += mWindZones[wz].mCurrentVelocity;
				}
			}
			mGlobalWindDirection	= mGlobalWindVelocity;
			mGlobalWindSpeed		= VectorNormalize(mGlobalWindDirection.v);
		}

		// Update All Particle Clouds
		//----------------------------
		mParticlesRendered = 0;
		for (int i=0; i<mParticleClouds.size(); i++)
		{
			mParticleClouds[i].Update();
			mParticleClouds[i].Render();
		}
		if (false)
		{
			Com_Printf( "Weather: %d Particles Rendered\n", mParticlesRendered);
		}
	}
}


void R_WorldEffect_f(void)
{
	if (ri.Cvar_VariableIntegerValue("helpUsObi"))
	{
		char	temp[2048];
		ri.Cmd_ArgsBuffer(temp, sizeof(temp));
		R_WorldEffectCommand(temp);
	}
}

/*
==================
WE_ParseVector
Imported from MP/Ensiform's fixes --eez
==================
*/

qboolean WE_ParseVector( const char **text, int count, float *v ) {
	char	*token;
	int		i;
	// FIXME: spaces are currently required after parens, should change parseext...
	COM_BeginParseSession();
	token = COM_ParseExt( text, qfalse );
	if ( strcmp( token, "(" ) ) {
		Com_Printf ("^3WARNING: missing parenthesis in weather effect\n" );
		COM_EndParseSession();
		return qfalse;
	}

	for ( i = 0 ; i < count ; i++ ) {
		token = COM_ParseExt( text, qfalse );
		if ( !token[0] ) {
			Com_Printf ("^3WARNING: missing vector element in weather effect\n" );
			COM_EndParseSession();
			return qfalse;
		}
		v[i] = atof( token );
	}

	token = COM_ParseExt( text, qfalse );
	COM_EndParseSession();
	if ( strcmp( token, ")" ) ) {
		Com_Printf ("^3WARNING: missing parenthesis in weather effect\n" );
		return qfalse;
	}
	return qtrue;
}


void R_WorldEffectCommand(const char *command)
{
	if ( !command )
	{
		return;
	}

	const char	*token;//, *origCommand;

	COM_BeginParseSession();

	token = COM_ParseExt(&command, qfalse);

	if ( !token )
	{
		COM_EndParseSession();
		return;
	}


	// Clear - Removes All Particle Clouds And Wind Zones
	//----------------------------------------------------
	if (Q_stricmp(token, "clear") == 0)
	{
		for (int p=0; p<mParticleClouds.size(); p++)
		{
			mParticleClouds[p].Reset();
		}
		mParticleClouds.clear();
		mWindZones.clear();
		mLocalWindZones.clear();
	}

	// Freeze / UnFreeze - Stops All Particle Motion Updates
	//--------------------------------------------------------
	else if (Q_stricmp(token, "freeze") == 0)
	{
		mFrozen = !mFrozen;
	}

	// Add a zone
	//---------------
	else if (Q_stricmp(token, "zone") == 0)
	{
		vec3_t	mins;
		vec3_t	maxs;
		if (WE_ParseVector(&command, 3, mins) && WE_ParseVector(&command, 3, maxs))
		{
			mOutside.AddWeatherZone(mins, maxs);
		}
	}

	// Basic Wind
	//------------
	else if (Q_stricmp(token, "wind") == 0)
	{
		if (mWindZones.full())
		{
			COM_EndParseSession();
			return;
		}
		CWindZone& nWind = mWindZones.push_back();
		nWind.Initialize();
	}

	// Constant Wind
	//---------------
	else if (Q_stricmp(token, "constantwind") == 0)
	{
		if (mWindZones.full())
		{
			COM_EndParseSession();
			return;
		}
		CWindZone& nWind = mWindZones.push_back();
		nWind.Initialize();
		if (!WE_ParseVector(&command, 3, nWind.mCurrentVelocity.v))
		{
			nWind.mCurrentVelocity.Clear();
			nWind.mCurrentVelocity[1] = 800.0f;
		}
		nWind.mTargetVelocityTimeRemaining = -1;
	}

	// Gusting Wind
	//--------------
	else if (Q_stricmp(token, "gustingwind") == 0)
	{
		if (mWindZones.full())
		{
			COM_EndParseSession();
			return;
		}
		CWindZone& nWind = mWindZones.push_back();
		nWind.Initialize();
		nWind.mRVelocity.mMins				= -3000.0f;
		nWind.mRVelocity.mMins[2]			= -100.0f;
		nWind.mRVelocity.mMaxs				=  3000.0f;
		nWind.mRVelocity.mMaxs[2]			=  100.0f;

		nWind.mMaxDeltaVelocityPerUpdate	=  10.0f;

		nWind.mRDuration.mMin				=  1000;
		nWind.mRDuration.mMax				=  3000;

		nWind.mChanceOfDeadTime				=  0.5f;
		nWind.mRDeadTime.mMin				=  2000;
		nWind.mRDeadTime.mMax				=  4000;
	}

	// Local Wind Zone
	//-----------------
	else if (Q_stricmp(token, "windzone") == 0)
	{
		if (mWindZones.full())
		{
			COM_EndParseSession();
			return;
		}
		CWindZone& nWind = mWindZones.push_back();
		nWind.Initialize();

		nWind.mGlobal = false;

		// Read Mins
		if (!WE_ParseVector(&command, 3, nWind.mRBounds.mMins.v))
		{
			assert("Wind Zone: Unable To Parse Mins Vector!"==0);
			mWindZones.pop_back();
			COM_EndParseSession();
			return;
		}

		// Read Maxs
		if (!WE_ParseVector(&command, 3, nWind.mRBounds.mMaxs.v))
		{
			assert("Wind Zone: Unable To Parse Maxs Vector!"==0);
			mWindZones.pop_back();
			COM_EndParseSession();
			return;
		}

		// Read Velocity
		if (!WE_ParseVector(&command, 3, nWind.mCurrentVelocity.v))
		{
			nWind.mCurrentVelocity.Clear();
			nWind.mCurrentVelocity[1] = 800.0f;
		}
		nWind.mTargetVelocityTimeRemaining = -1;

		mLocalWindZones.push_back(&nWind);
	}


	// Create A Rain Storm
	//---------------------
	else if (Q_stricmp(token, "lightrain") == 0)
	{
		if (mParticleClouds.full())
		{
			COM_EndParseSession();
			return;
		}
		CParticleCloud& nCloud = mParticleClouds.push_back();
		nCloud.Initialize(500, "gfx/world/rain.jpg", 3);
		nCloud.mHeight		= 80.0f;
		nCloud.mWidth		= 1.2f;
		nCloud.mGravity		= 2000.0f;
		nCloud.mFilterMode	= 1;
		nCloud.mBlendMode	= 1;
		nCloud.mFade		= 100.0f;
		nCloud.mColor		= 0.5f;
		nCloud.mOrientWithVelocity = true;
		nCloud.mWaterParticles = true;
	}

	// Create A Rain Storm
	//---------------------
	else if (Q_stricmp(token, "rain") == 0)
	{
		if (mParticleClouds.full())
		{
			COM_EndParseSession();
			return;
		}
		CParticleCloud& nCloud = mParticleClouds.push_back();
		nCloud.Initialize(1000, "gfx/world/rain.jpg", 3);
		nCloud.mHeight		= 80.0f;
		nCloud.mWidth		= 1.2f;
		nCloud.mGravity		= 2000.0f;
		nCloud.mFilterMode	= 1;
		nCloud.mBlendMode	= 1;
		nCloud.mFade		= 100.0f;
		nCloud.mColor		= 0.5f;
		nCloud.mOrientWithVelocity = true;
		nCloud.mWaterParticles = true;
	}

	// Create A Rain Storm
	//---------------------
	else if (Q_stricmp(token, "acidrain") == 0)
	{
		if (mParticleClouds.full())
		{
			COM_EndParseSession();
			return;
		}
		CParticleCloud& nCloud = mParticleClouds.push_back();
		nCloud.Initialize(1000, "gfx/world/rain.jpg", 3);
		nCloud.mHeight		= 80.0f;
		nCloud.mWidth		= 2.0f;
		nCloud.mGravity		= 2000.0f;
		nCloud.mFilterMode	= 1;
		nCloud.mBlendMode	= 1;
		nCloud.mFade		= 100.0f;

		nCloud.mColor[0]	= 0.34f;
		nCloud.mColor[1]	= 0.70f;
		nCloud.mColor[2]	= 0.34f;
		nCloud.mColor[3]	= 0.70f;

		nCloud.mOrientWithVelocity = true;
		nCloud.mWaterParticles = true;

		mOutside.mOutsidePain = 0.1f;
	}

	// Create A Rain Storm
	//---------------------
	else if (Q_stricmp(token, "heavyrain") == 0)
	{
		if (mParticleClouds.full())
		{
			COM_EndParseSession();
			return;
		}
		CParticleCloud& nCloud = mParticleClouds.push_back();
		nCloud.Initialize(1000, "gfx/world/rain.jpg", 3);
		nCloud.mHeight		= 80.0f;
		nCloud.mWidth		= 1.2f;
		nCloud.mGravity		= 2800.0f;
		nCloud.mFilterMode	= 1;
		nCloud.mBlendMode	= 1;
		nCloud.mFade		= 15.0f;
		nCloud.mColor		= 0.5f;
		nCloud.mOrientWithVelocity = true;
		nCloud.mWaterParticles = true;
	}

	// Create A Snow Storm
	//---------------------
	else if (Q_stricmp(token, "snow") == 0)
	{
		if (mParticleClouds.full())
		{
			COM_EndParseSession();
			return;
		}
		CParticleCloud& nCloud = mParticleClouds.push_back();
		nCloud.Initialize(1000, "gfx/effects/snowflake1.bmp");
		nCloud.mBlendMode			= 1;
		nCloud.mRotationChangeNext	= 0;
		nCloud.mColor		= 0.75f;
		nCloud.mWaterParticles = true;
	}

	// Create A Some stuff
	//---------------------
	else if (Q_stricmp(token, "spacedust") == 0)
	{
		int count;
		if (mParticleClouds.full())
		{
			COM_EndParseSession();
			return;
		}
		token = COM_ParseExt(&command, qfalse);
		count = atoi(token);

		CParticleCloud& nCloud = mParticleClouds.push_back();
		nCloud.Initialize(count, "gfx/effects/snowpuff1.tga");
		nCloud.mHeight		= 1.2f;
		nCloud.mWidth		= 1.2f;
		nCloud.mGravity		= 0.0f;
		nCloud.mBlendMode			= 1;
		nCloud.mRotationChangeNext	= 0;
		nCloud.mColor		= 0.75f;
		nCloud.mWaterParticles = true;
		nCloud.mMass.mMax	= 30.0f;
		nCloud.mMass.mMin	= 10.0f;
		nCloud.mSpawnRange.mMins[0]	= -1500.0f;
		nCloud.mSpawnRange.mMins[1]	= -1500.0f;
		nCloud.mSpawnRange.mMins[2]	= -1500.0f;
		nCloud.mSpawnRange.mMaxs[0]	= 1500.0f;
		nCloud.mSpawnRange.mMaxs[1]	= 1500.0f;
		nCloud.mSpawnRange.mMaxs[2]	= 1500.0f;
	}

	// Create A Sand Storm
	//---------------------
	else if (Q_stricmp(token, "sand") == 0)
	{
		if (mParticleClouds.full())
		{
			COM_EndParseSession();
			return;
		}
		CParticleCloud& nCloud = mParticleClouds.push_back();
		nCloud.Initialize(400, "gfx/effects/alpha_smoke2b.tga");

		nCloud.mGravity		= 0;
 		nCloud.mWidth		= 70;
		nCloud.mHeight		= 70;
		nCloud.mColor[0]	= 0.9f;
		nCloud.mColor[1]	= 0.6f;
		nCloud.mColor[2]	= 0.0f;
		nCloud.mColor[3]	= 0.5f;
		nCloud.mFade		= 5.0f;
		nCloud.mMass.mMax	= 30.0f;
		nCloud.mMass.mMin	= 10.0f;
		nCloud.mSpawnRange.mMins[2]	= -150;
		nCloud.mSpawnRange.mMaxs[2]	= 150;

		nCloud.mRotationChangeNext	= 0;
	}

	// Create Blowing Clouds Of Fog
	//------------------------------
	else if (Q_stricmp(token, "fog") == 0)
	{
		if (mParticleClouds.full())
		{
			COM_EndParseSession();
			return;
		}
		CParticleCloud& nCloud = mParticleClouds.push_back();
		nCloud.Initialize(60, "gfx/effects/alpha_smoke2b.tga");
		nCloud.mBlendMode	= 1;
		nCloud.mGravity		= 0;
 		nCloud.mWidth		= 70;
		nCloud.mHeight		= 70;
		nCloud.mColor		= 0.2f;
		nCloud.mFade		= 5.0f;
		nCloud.mMass.mMax	= 30.0f;
		nCloud.mMass.mMin	= 10.0f;
		nCloud.mSpawnRange.mMins[2]	= -150;
		nCloud.mSpawnRange.mMaxs[2]	= 150;

		nCloud.mRotationChangeNext	= 0;
	}

	// Create Heavy Rain Particle Cloud
	//-----------------------------------
	else if (Q_stricmp(token, "heavyrainfog") == 0)
	{
		if (mParticleClouds.full())
		{
			COM_EndParseSession();
			return;
		}
		CParticleCloud& nCloud = mParticleClouds.push_back();
 		nCloud.Initialize(70, "gfx/effects/alpha_smoke2b.tga");
		nCloud.mBlendMode	= 1;
		nCloud.mGravity		= 0;
 		nCloud.mWidth		= 100;
		nCloud.mHeight		= 100;
		nCloud.mColor		= 0.3f;
		nCloud.mFade		= 1.0f;
		nCloud.mMass.mMax	= 10.0f;
		nCloud.mMass.mMin	= 5.0f;

		nCloud.mSpawnRange.mMins	= -(nCloud.mSpawnPlaneDistance*1.25f);
		nCloud.mSpawnRange.mMaxs	=  (nCloud.mSpawnPlaneDistance*1.25f);
		nCloud.mSpawnRange.mMins[2]	= -150;
		nCloud.mSpawnRange.mMaxs[2]	=  150;

		nCloud.mRotationChangeNext	= 0;
	}

	// Create Blowing Clouds Of Fog
	//------------------------------
	else if (Q_stricmp(token, "light_fog") == 0)
	{
		if (mParticleClouds.full())
		{
			COM_EndParseSession();
			return;
		}
		CParticleCloud& nCloud = mParticleClouds.push_back();
		nCloud.Initialize(40, "gfx/effects/alpha_smoke2b.tga");
		nCloud.mBlendMode	= 1;
		nCloud.mGravity		= 0;
 		nCloud.mWidth		= 100;
		nCloud.mHeight		= 100;
		nCloud.mColor[0]	= 0.19f;
		nCloud.mColor[1]	= 0.6f;
		nCloud.mColor[2]	= 0.7f;
		nCloud.mColor[3]	= 0.12f;
		nCloud.mFade		= 0.10f;
		nCloud.mMass.mMax	= 30.0f;
		nCloud.mMass.mMin	= 10.0f;
		nCloud.mSpawnRange.mMins[2]	= -150;
		nCloud.mSpawnRange.mMaxs[2]	= 150;

		nCloud.mRotationChangeNext	= 0;
	}
	else if (Q_stricmp(token, "outsideshake") == 0)
	{
		mOutside.mOutsideShake = !mOutside.mOutsideShake;
	}
	else if (Q_stricmp(token, "outsidepain") == 0)
	{
		mOutside.mOutsidePain = !mOutside.mOutsidePain;
	}
	else
	{
		Com_Printf( "Weather Effect: Please enter a valid command.\n" );
		Com_Printf( "	clear\n" );
		Com_Printf( "	freeze\n" );
		Com_Printf( "	zone (mins) (maxs)\n" );
		Com_Printf( "	wind\n" );
		Com_Printf( "	constantwind (velocity)\n" );
		Com_Printf( "	gustingwind\n" );
		Com_Printf( "	windzone (mins) (maxs) (velocity)\n" );
		Com_Printf( "	lightrain\n" );
		Com_Printf( "	rain\n" );
		Com_Printf( "	acidrain\n" );
		Com_Printf( "	heavyrain\n" );
		Com_Printf( "	snow\n" );
		Com_Printf( "	spacedust\n" );
		Com_Printf( "	sand\n" );
		Com_Printf( "	fog\n" );
		Com_Printf( "	heavyrainfog\n" );
		Com_Printf( "	light_fog\n" );
		Com_Printf( "	outsideshake\n" );
		Com_Printf( "	outsidepain\n" );
	}
	COM_EndParseSession();
}




float R_GetChanceOfSaberFizz()
{
 	float	chance = 0.0f;
	int		numWater = 0;
	for (int i=0; i<mParticleClouds.size(); i++)
	{
		if (mParticleClouds[i].mWaterParticles)
		{
			chance += (mParticleClouds[i].mGravity/20000.0f);
			numWater ++;
		}
	}
	if (numWater)
	{
		return (chance / numWater);
	}
	return 0.0f;
}

bool R_IsRaining()
{
	return !mParticleClouds.empty();
}

bool R_IsPuffing()
{
	return false;
}




#endif // JK2_MODE