#pragma once
#include "Define.h"
#include <math.h>

struct POS3_t
{
	float		x;
	float		y;
	float		z;

	POS3_t() : x(0.0f), y(0.0f), z(0.0f) {}
	POS3_t(const float dx, const float dy, const float dz) : x(dx), y(dy), z(dz) {}
	POS3_t(const POS3_t& rPos) { x = rPos.x; y = rPos.y; z = rPos.z; }

	void		Clear() { x = 0.0f; y = 0.0f; z = 0.0f; }
	bool		IsZeroPoint() { return (x == 0.0f && y == 0.0f && z == 0.0f); }

	POS3_t& operator = (const POS3_t& rPos)
	{
		x = rPos.x;
		y = rPos.y;
		z = rPos.z;
		return *this;
	}


	// assignment operators
	POS3_t&	operator += (const POS3_t& v)
	{
		x += v.x;
		y += v.y;
		z += v.z;
		return *this;
	}

	POS3_t&	operator -= (const POS3_t& v)
	{
		x -= v.x;
		y -= v.y;
		z -= v.z;
		return *this;
	}

	POS3_t&	operator *= (float f)
	{
		x *= f;
		y *= f;
		z *= f;
		return *this;
	}

	POS3_t&	operator /= (float f)
	{
		float fInv = 1.0f / f;
		x *= fInv;
		y *= fInv;
		z *= fInv;
		return *this;
	}


	// unary operators
	POS3_t	operator + () const
	{
		return *this;
	}

	POS3_t	operator - () const
	{
		return POS3_t(-x, -y, -z);
	}


	// binary operators
	POS3_t	operator + (const POS3_t& v) const
	{
		return POS3_t(x + v.x, y + v.y, z + v.z);
	}

	POS3_t	operator - (const POS3_t& v) const
	{
		return POS3_t(x - v.x, y - v.y, z - v.z);
	}

	POS3_t	operator * (float f) const
	{
		return POS3_t(x * f, y * f, z * f);
	}

	POS3_t	operator / (float f) const
	{
		float fInv = 1.0f / f;
		return POS3_t(x * fInv, y * fInv, z * fInv);
	}


	friend POS3_t	operator * (float f, const struct POS3_t& v)
	{
		return POS3_t(f * v.x, f * v.y, f * v.z);
	}


	bool	operator == (const POS3_t& v) const
	{
		return x == v.x && y == v.y && z == v.z;
	}

	bool	operator != (const POS3_t& v) const
	{
		return x != v.x || y != v.y || z != v.z;
	}

	void	SetPosZZero() { z = 0; }

	float GetMagnitude()
	{
		return sqrt(x * x + y * y + z * z);
	}

	POS3_t Normalize()
	{
		float temp = GetMagnitude();
		float inverse = 1.0f;

		if (temp != 0.0f)
		{
			inverse = 1.0f / temp;
		}

		POS3_t mPos;
		mPos.x = x * inverse;
		mPos.y = y * inverse;
		mPos.z = z * inverse;

		return mPos;
	}

	void NormalizeVector()
	{
		float fLength = GetMagnitude();
		if (fLength == 0.0f)
		{
			fLength = 1.0f;
		}

		x /= fLength;
		y /= fLength;
		z /= fLength;
	}

	//내적
	float DotProduct(const POS3_t &p1, const POS3_t &p2)
	{
		return p1.x*p2.x + p1.y * p2.y + p1.z*p2.z;
	}

	POS3_t CrossVector(const POS3_t &vec) const
	{
		POS3_t mRetPos;
		mRetPos.x = y * vec.z - z * vec.y;
		mRetPos.y = z * vec.x - x * vec.z;
		mRetPos.z = x * vec.y - y * vec.x;
		return mRetPos;
	}

	bool IsZeroVector()
	{
		if (x == 0 && y == 0 && z == 0)
		{
			return true;
		}
		return false;
	}

	//거리
	//float Distance(MVector v)
	float Distance(POS3_t v) const
	{
		float xfom = (x - v.x) * (x - v.x);
		float yfom = (y - v.y) * (y - v.y);
		float zfom = (z - v.z) * (z - v.z);

		float dist = sqrt(xfom + yfom + zfom);
		return dist;


		//return sqrt( (x - v.x) * (x - v.x) + (y - v.y) * (y - v.y) + (z - v.z) * (z - v.z) );
	}

	static float Distance(const POS3_t& p1, const POS3_t& p2)
	{
		float xfom = (p1.x - p2.x) * (p1.x - p2.x);
		float yfom = (p1.y - p2.y) * (p1.y - p2.y);
		float zfom = (p1.z - p2.z) * (p1.z - p2.z);

		float dist = sqrt(xfom + yfom + zfom);
		return dist;
	}

	float Distance(POS3_t v, float ignoreHeight)
	{
		float xfom = (x - v.x) * (x - v.x);
		float yfom = (y - v.y) * (y - v.y);

		float dist = 0.0f;

		//z축 차이가 ignoreHeight 보다 크면 z축을 무시하고 x, y축만으로 거리를 구함
		if (z - v.z >= ignoreHeight)
		{
			float zfom = (z - v.z) * (z - v.z);

			dist = sqrt(xfom + yfom + zfom);
		}
		else
		{
			dist = sqrt(xfom + yfom);
		}

		return dist;
	}

	//특정 지점 사이의 일정 거리 내 좌표를 구함
	POS3_t AdvancedPositionWithDirection(POS3_t direction, float distance) const
	{
		POS3_t startpos;
		startpos.x = x;
		startpos.y = y;
		startpos.z = z;

		// 		MVector advancedpos(startpos);
		// 		MVector normaldir(direction);

		POS3_t advancedpos(startpos);
		POS3_t normaldir(direction);

		normaldir.NormalizeVector();

		advancedpos = advancedpos + normaldir * distance;

		return advancedpos;
	}


	//두개의 위치 사이의 각도 구하기
	float AngleBetweenPositions(POS3_t dest) const
	{
		POS3_t startpos;
		startpos.x = x;
		startpos.y = y;
		startpos.z = z;

		// 		MVector Posvec(startpos);
		// 		MVector destvec(dest);

		POS3_t Posvec(startpos);
		POS3_t destvec(dest);

		// 		MVector Dirvec = Posvec - destvec;

		POS3_t Dirvec = Posvec - destvec;
		if (Dirvec.IsZeroVector())
		{
			return 0.0f;
		}


		//Dirvec.SetPosZZero();
		Dirvec.SetPosZZero();

		//normalize
		POS3_t NorPos = Dirvec.Normalize();

		//원점(0.0.0)에과 dirpos와의 각도 계산
		//MVector mBaseVec(1, 0, 0);
		POS3_t mBaseVec(1, 0, 0);
		POS3_t BasePos = mBaseVec.Normalize();

		//내적된 값
		float mDot = mBaseVec.DotProduct(BasePos, NorPos);

		float angle = acos(mDot);

		float newangle = (float)RadianToDegree(angle);


		if (NorPos.y < 0)
		{
			newangle = 360 - newangle;
		}

		return newangle;
	}

	bool IsAlmostSamePosition(POS3_t pos)
	{
		//MVector posvec(pos);

		//if (Distance(posvec) < 1.0f)
		if (Distance(pos) < 1.0f)
		{
			return true;
		}
		else
		{
			return false;
		}
	}

	static double RadianToDegree(double radian)
	{
		return radian * 180.0f / g_fPI;
	}

	static double DegreeToRadian(double degree)
	{
		return g_fPI * degree / 180.0f;
	}


	POS3_t AdvancedPosWithAngle(float degree, float dist) const
	{
		POS3_t direction;
		POS3_t AdvanPos;

		double pi = DegreeToRadian(degree);
		direction.x = (float)cos(pi);
		direction.y = (float)sin(pi);

		AdvanPos = AdvancedPositionWithDirection(direction, dist);
		return AdvanPos;
	}

	static POS3_t RotationPos(const POS3_t& InPos, const float InRadian)
	{
		POS3_t Result;
		Result.x = InPos.x * cos(InRadian) - InPos.y * sin(InRadian);
		Result.y = InPos.x * sin(InRadian) + InPos.y * cos(InRadian);

		return Result;
	};

	float Size()
	{
		float size_v = sqrt(x*x + y*y + z*z);
		return size_v;
	}


	//FORCEINLINE float FRotator::ClampAxis(float Angle)
	//{
	//	// returns Angle in the range (-360,360)
	//	Angle = FMath::Fmod(Angle, 360.f);
	//
	//	if ( Angle < 0.f )
	//	{
	//		// shift to [0,360) range
	//		Angle += 360.f;
	//	}
	//
	//	return Angle;
	//}


	static FORCEINLINE float NormalizeAxis(float Angle)
	{
		// returns Angle in the range [0,360)
		//Angle = ClampAxis(Angle);

		if (Angle > 180.f)
		{
			// shift to (-180,180]
			Angle -= 360.f;
		}

		return Angle;
	}
	static FORCEINLINE float UnnormalizeAxis(float degree)
	{
		if (degree < 0)
		{
			degree += 360.f;
		}

		return degree;
	}

	// 0~359 사이 값으로 각도를 변경해 줌
	static float ValidateDegree(float fDegree)
	{
		float fResult = 0;
		if (fDegree < 360 && fDegree >= 0)
			return fDegree;

		// 소수점 이하는 버린다.
		int nDegree = (int)fDegree;
		nDegree = nDegree % 360;
		if (nDegree < 0)
			nDegree = nDegree + 360;

		fResult = (float)nDegree;

		//float under_point = fDegree - (float)nDegree;
		//if ( nDegree >= 0 )
		//{
		//   nDegree = nDegree % 360;
		//}
		//else
		//{
		//   int cycle = abs(nDegree / 360);
		//   int nPositive = 360 * ( cycle + 1 );
		//   nDegree = nPositive + nDegree;
		//}

		//fResult = (float)nDegree + under_point;

		return fResult;
	}

	static bool IsLineCollision(const POS3_t& A1, const POS3_t& A2, const POS3_t& B1, const POS3_t& B2)
	{
		float t = 0.0f;
		float s = 0.0f;
		float under = (B2.y - B1.y)*(A2.x - A1.x) - (B2.x - B1.x)*(A2.y - A1.y);
		if (under == 0)
			return false;

		float _t = (B2.x - B1.x)*(A1.y - B1.y) - (B2.y - B1.y)*(A1.x - B1.x);
		float _s = (A2.x - A1.x)*(A1.y - B1.y) - (A2.y - A1.y)*(A1.x - B1.x);

		t = _t / under;
		s = _s / under;

		if (t <= 0.0 || t>1.0 || s <= 0.0 || s>1.0)
			return false;

		return true;
	}
};

class PathData
{
public:
	PathData();
	~PathData();

	tid					GetMapTID()			{	return m_mapTID;				}
	const POS3_t&		GetMapSize()		{	return m_sizePos;				}
	const POS3_t&		GetMinPos()			{	return m_minPos;				}
	const POS3_t&		GetMaxPos()			{	return m_maxPos;				}
	float				GetPaddingY()		{	return m_maxPos.y + m_minPos.y;	}

private:
	tid					m_mapTID;
	POS3_t				m_sizePos;
	POS3_t				m_minPos;
	POS3_t				m_maxPos;
};

