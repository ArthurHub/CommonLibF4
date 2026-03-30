#pragma once

// hkVector4f — 16-byte aligned 4-component vector (Havok's SIMD vector type)
//
// Used throughout the Havok 2014 physics engine for positions, velocities,
// normals, impulses, and quaternions. Always 16-byte aligned for SSE.
//
// Coordinate space:
//   Havok uses a different scale than Bethesda game units.
//   1 Havok unit ≈ 70 game units.
//   Convert: havokPos = gamePos * (1.0f / 70.0f)
//            gamePos  = havokPos * 70.0f
//
// The w component:
//   - For positions: usually 0 or contains encoded data (ignore)
//   - For quaternions: w is the scalar part {x,y,z,w}
//   - For velocities/impulses: should be 0
//
// Interop with NiPoint3:
//   hkVector4f v(niPoint);     // implicit conversion from NiPoint3
//   NiPoint3 p = (NiPoint3)v;  // explicit cast back (drops w)

typedef float hkReal;
namespace RE
{
	/// hkVector4f — 16-byte aligned SIMD vector.
	///
	/// Memory layout: 16 bytes, 16-byte aligned.
	/// {x, y, z, w} — w is typically 0 for directions/positions, or the
	/// quaternion scalar part for orientations.
	class hkVector4f
	{
	public:
		__declspec(align(16)) hkReal x;
		hkReal y;
		hkReal z;
		hkReal w;

		/// Default constructor — zero vector.
		hkVector4f()
		{
			x = 0;
			y = 0;
			z = 0;
			w = 0;
		};

		/// Construct from NiPoint3 (game-space point, w = 0).
		/// Note: does NOT apply Havok scale — caller must scale if needed.
		hkVector4f(NiPoint3 p)
		{
			x = p.x;
			y = p.y;
			z = p.z;
			w = 0.0f;
		}

		/// Construct from individual components.
		hkVector4f(float _x, float _y, float _z, float _w = 0.0f)
		{
			x = _x;
			y = _y;
			z = _z;
			w = _w;
		}

		hkVector4f& operator=(const hkVector4f& v)
		{
			this->x = v.x;
			this->y = v.y;
			this->z = v.z;
			this->w = v.w;
			return *this;
		}
		hkVector4f operator+(const hkVector4f& v)
		{
			return hkVector4f(x + v.x, y + v.y, z + v.z);
		}
		hkVector4f& operator+=(const hkVector4f& v)
		{
			this->x += v.x;
			this->y += v.y;
			this->z += v.z;
			this->w += v.w;
			return *this;
		}
		hkVector4f operator-(const hkVector4f& v)
		{
			return hkVector4f(x - v.x, y - v.y, z - v.z);
		}
		hkVector4f operator-()
		{
			return hkVector4f(x * -1.0f, y * -1.0f, z * -1.0f);
		}
		hkVector4f& operator-=(const hkVector4f& v)
		{
			this->x -= v.x;
			this->y -= v.y;
			this->z -= v.z;
			this->w -= v.w;
			return *this;
		}
		hkVector4f operator*(float a)
		{
			return hkVector4f(x * a, y * a, z * a, w * a);
		}
		hkVector4f& operator*=(float a)
		{
			this->x *= a;
			this->y *= a;
			this->z *= a;
			this->w *= a;
			return *this;
		}
		hkVector4f operator*(const hkVector4f& v)
		{
			return hkVector4f(x * v.x, y * v.y, z * v.z, w * v.w);
		}
		hkVector4f& operator*=(const hkVector4f& v)
		{
			this->x *= v.x;
			this->y *= v.y;
			this->z *= v.z;
			this->w *= v.w;
			return *this;
		}
		hkVector4f operator/(float a)
		{
			return hkVector4f(x / a, y / a, z / a);
		}
		hkVector4f& operator/=(float a)
		{
			this->x /= a;
			this->y /= a;
			this->z /= a;
			this->w /= a;
			return *this;
		}
		hkVector4f operator/(const hkVector4f& v)
		{
			return hkVector4f(x / v.x, y / v.y, z / v.z, w / v.w);
		}
		hkVector4f& operator/=(const hkVector4f& v)
		{
			this->x /= v.x;
			this->y /= v.y;
			this->z /= v.z;
			this->w /= v.w;
			return *this;
		}

		/// 3D vector length (ignores w).
		float Length()
		{
			return sqrt(x * x + y * y + z * z);
		}

		/// Normalize in-place (3D, ignores w). Returns zero vector if length is 0.
		hkVector4f& Normalize()
		{
			float l = Length();
			if (l == 0) {
				this->x = 0;
				this->y = 0;
				this->z = 0;
				return *this;
			}
			this->x /= l;
			this->y /= l;
			this->z /= l;
			return *this;
		}

		/// Return a normalized copy (3D, ignores w).
		hkVector4f GetNormalized()
		{
			hkVector4f norm = *this;
			norm.Normalize();
			return norm;
		}

		/// 3D dot product (ignores w).
		float Dot(const hkVector4f& v)
		{
			return this->x * v.x + this->y * v.y + this->z * v.z;
		}

		/// 3D cross product (ignores w, result w = 0).
		hkVector4f Cross(const hkVector4f& v)
		{
			return hkVector4f(
				y * v.z - z * v.y,
				z * v.x - x * v.z,
				x * v.y - y * v.x,
				0.0f);
		}

		/// 3D distance to another point.
		float DistanceTo(const hkVector4f& v)
		{
			return (*this - v).Length();
		}

		/// Implicit conversion to NiPoint3 (drops w component).
		operator NiPoint3()
		{
			return NiPoint3(x, y, z);
		}
	};
	static_assert(sizeof(hkVector4f) == 0x10);
};
