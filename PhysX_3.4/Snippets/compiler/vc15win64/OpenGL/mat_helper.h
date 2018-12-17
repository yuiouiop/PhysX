#pragma once
#ifndef _MAT_HELPER_H_
#define _MAT_HELPER_H_

#define M_PI 3.14159265358979

struct Vector3{
	float x;
	float y;
	float z;

	Vector3() {
		x = 0;
		y = 0;
		z = 0;
	}
	Vector3(float f) {
		x = f;
		y = f;
		z = f;
	}

	Vector3(float _x, float _y, float _z) {
		x = _x;
		y = _y;
		z = _z;
	}

	void setXYZ(float _x, float _y, float _z)
	{
		x = _x;
		y = _y;
		z = _z;
	}

	Vector3 operator*(float f) {
		return Vector3(f*x, f*y, f*z);
	}

	Vector3 operator-(Vector3 f) {
		return Vector3(x - f.x, y - f.y, z - f.z);
	}

	Vector3	operator-() const { return Vector3(-x, -y, -z); }

	Vector3 operator+(Vector3 f) {
		return Vector3(x + f.x, y + f.y, z + f.z);
	}

	Vector3 operator+=(Vector3 &v) {
		x += v.x;
		y += v.y;
		z += v.z;
		return *this;
	}

	Vector3 operator-=(Vector3 &v) {
		x -= v.x;
		y -= v.y;
		z -= v.z;
		return *this;
	}

	Vector3 operator*=(float f) {
		x *= f;
		y *= f;
		z *= f;
		return *this;
	}
};

struct Vector2
{
	float x;
	float y;

	Vector2() {
		x = 0;
		y = 0;
	}

	Vector2(float _x, float _y) {
		x = _x;
		y = _y;
	}
};

struct Vector4 {
	float x;
	float y;
	float z;
	float w;

	Vector4() {
		x = 0;
		y = 0;
		z = 0;
		w = 0;
	}
	Vector4(float f) {
		x = f;
		y = f;
		z = f;
		w = f;
	}

	Vector4(float _x, float _y, float _z, float _w) {
		x = _x;
		y = _y;
		z = _z;
		w = _w;
	}

	void setXYZW(float _x, float _y, float _z, float _w)
	{
		x = _x;
		y = _y;
		z = _z;
		w = _w;
	}
};

struct Mat4 {
	float m[4][4];
	
	Mat4() {
		m[0][0] = 1.0; m[0][1] = 0.0; m[0][2] = 0.0; m[0][3] = 0.0;
		m[1][0] = 0.0; m[1][1] = 1.0; m[1][2] = 0.0; m[1][3] = 0.0;
		m[2][0] = 0.0; m[2][1] = 0.0; m[2][2] = 1.0; m[2][3] = 0.0;
		m[3][0] = 0.0; m[3][1] = 0.0; m[3][2] = 0.0; m[3][3] = 1.0;
	}

	Mat4(float f){
		m[0][0] = f; m[0][1] = 0.0; m[0][2] = 0.0; m[0][3] = 0.0;
		m[1][0] = 0.0; m[1][1] = f; m[1][2] = 0.0; m[1][3] = 0.0;
		m[2][0] = 0.0; m[2][1] = 0.0; m[2][2] = f; m[2][3] = 0.0;
		m[3][0] = 0.0; m[3][1] = 0.0; m[3][2] = 0.0; m[3][3] = f;
	}

	Mat4(float f00, float f01, float f02, float f03,
		float f10, float f11, float f12, float f13,
		float f20, float f21, float f22, float f23,
		float f30, float f31, float f32, float f33) 
	{
		m[0][0] = f00; m[0][1] = f01; m[0][2] = f02; m[0][3] = f03;
		m[1][0] = f10; m[1][1] = f11; m[1][2] = f12; m[1][3] = f13;
		m[2][0] = f20; m[2][1] = f21; m[2][2] = f22; m[2][3] = f23;
		m[3][0] = f30; m[3][1] = f31; m[3][2] = f32; m[3][3] = f33;
	}

	Mat4 operator*(Mat4 b) {
		Mat4 result;
		result.m[0][0] = m[0][0] * b.m[0][0] + m[0][1] * b.m[1][0] + m[0][2] * b.m[2][0] + m[0][3] * b.m[3][0];
		result.m[0][1] = m[0][0] * b.m[0][1] + m[0][1] * b.m[1][1] + m[0][2] * b.m[2][1] + m[0][3] * b.m[3][1];
		result.m[0][2] = m[0][0] * b.m[0][2] + m[0][1] * b.m[1][2] + m[0][2] * b.m[2][2] + m[0][3] * b.m[3][2];
		result.m[0][3] = m[0][0] * b.m[0][3] + m[0][1] * b.m[1][3] + m[0][2] * b.m[2][3] + m[0][3] * b.m[3][3];

		result.m[1][0] = m[1][0] * b.m[0][0] + m[1][1] * b.m[1][0] + m[1][2] * b.m[2][0] + m[1][3] * b.m[3][0];
		result.m[1][1] = m[1][0] * b.m[0][1] + m[1][1] * b.m[1][1] + m[1][2] * b.m[2][1] + m[1][3] * b.m[3][1];
		result.m[1][2] = m[1][0] * b.m[0][2] + m[1][1] * b.m[1][2] + m[1][2] * b.m[2][2] + m[1][3] * b.m[3][2];
		result.m[1][3] = m[1][0] * b.m[0][3] + m[1][1] * b.m[1][3] + m[1][2] * b.m[2][3] + m[1][3] * b.m[3][3];

		result.m[2][0] = m[2][0] * b.m[0][0] + m[2][1] * b.m[1][0] + m[2][2] * b.m[2][0] + m[2][3] * b.m[3][0];
		result.m[2][1] = m[2][0] * b.m[0][1] + m[2][1] * b.m[1][1] + m[2][2] * b.m[2][1] + m[2][3] * b.m[3][1];
		result.m[2][2] = m[2][0] * b.m[0][2] + m[2][1] * b.m[1][2] + m[2][2] * b.m[2][2] + m[2][3] * b.m[3][2];
		result.m[2][3] = m[2][0] * b.m[0][3] + m[2][1] * b.m[1][3] + m[2][2] * b.m[2][3] + m[2][3] * b.m[3][3];

		result.m[3][0] = m[3][0] * b.m[0][0] + m[3][1] * b.m[1][0] + m[3][2] * b.m[2][0] + m[3][3] * b.m[3][0];
		result.m[3][1] = m[3][0] * b.m[0][1] + m[3][1] * b.m[1][1] + m[3][2] * b.m[2][1] + m[3][3] * b.m[3][1];
		result.m[3][2] = m[3][0] * b.m[0][2] + m[3][1] * b.m[1][2] + m[3][2] * b.m[2][2] + m[3][3] * b.m[3][2];
		result.m[3][3] = m[3][0] * b.m[0][3] + m[3][1] * b.m[1][3] + m[3][2] * b.m[2][3] + m[3][3] * b.m[3][3];

		return result;
	}

	Vector3 operator*(Vector3 v)
	{
		Vector3 result;
		result.x = m[0][0] * v.x + m[0][1] * v.y + m[0][2] * v.z + m[0][3] * 1.0;
		result.y = m[1][0] * v.x + m[1][1] * v.y + m[1][2] * v.z + m[1][3] * 1.0;
		result.z = m[2][0] * v.x + m[2][1] * v.y + m[2][2] * v.z + m[2][3] * 1.0;
		return result;
	}
};

float radians(float angle)
{
	return (angle / 180.0 *M_PI);
}

float length_v3(Vector3 v)
{
	float result = v.x*v.x + v.y*v.y + v.z*v.z;
	return sqrtf(result);
}

Vector3 normalize(Vector3 v) 
{
	float len = sqrt(v.x*v.x + v.y*v.y + v.z*v.z);
	return Vector3(v.x / len, v.y / len, v.z / len);
}

Vector3 cross(Vector3 a, Vector3 b)
{
	return Vector3(
		a.y * b.z - b.y * a.z,
		a.z * b.x - b.z * a.x,
		a.x * b.y - b.x * a.y);
}

float dot(Vector3 a, Vector3 b)
{
	return a.x*b.x + a.y*b.y + a.z*b.z;
}

Mat4 translate(Mat4 &m, Vector3 &v) {
	Mat4 ret(m);
	ret.m[3][0] += m.m[0][0] * v.x + m.m[1][0] * v.x + m.m[2][0] * v.x;
	ret.m[3][1] += m.m[0][1] * v.y + m.m[1][1] * v.y + m.m[2][1] * v.y;
	ret.m[3][2] += m.m[0][2] * v.z + m.m[1][2] * v.z + m.m[2][2] * v.z;
	return ret;
}

Mat4 scale(Mat4 &m, Vector3 &v) {
	Mat4 ret(0);
	ret.m[0][0] = m.m[0][0] * v.x;
	ret.m[0][1] = m.m[0][1] * v.x;
	ret.m[0][2] = m.m[0][2] * v.x;
	ret.m[1][0] = m.m[1][0] * v.y;
	ret.m[1][1] = m.m[1][1] * v.y;
	ret.m[1][2] = m.m[1][2] * v.y;
	ret.m[2][0] = m.m[2][0] * v.z;
	ret.m[2][1] = m.m[2][1] * v.z;
	ret.m[2][2] = m.m[2][2] * v.z;
	ret.m[3][0] = m.m[3][0];
	ret.m[3][1] = m.m[3][1];
	ret.m[3][2] = m.m[3][2];
	return ret;
}

Mat4 LookAt(Vector3 eye, Vector3 center, Vector3 up)
{
	Vector3 f(normalize(center - eye));
	Vector3 s(normalize(cross(f, up)));
	Vector3 u(cross(s, f));

	Mat4 Result(0);
	Result.m[0][0] = s.x;
	Result.m[1][0] = s.y;
	Result.m[2][0] = s.z;
	Result.m[0][1] = u.x;
	Result.m[1][1] = u.y;
	Result.m[2][1] = u.z;
	Result.m[0][2] = -f.x;
	Result.m[1][2] = -f.y;
	Result.m[2][2] = -f.z;
	Result.m[3][0] = -dot(s, eye);
	Result.m[3][1] = -dot(u, eye);
	Result.m[3][2] = dot(f, eye);
	Result.m[3][3] = 1.0;
	return Result;
}

Mat4 perspective(float fov, float aspect, float zNear, float zFar)
{
	Mat4 Result(0);
	float tanHalfFovy = tan(fov / 2.0);
	Result.m[0][0] = 1 / (aspect * tanHalfFovy);
	Result.m[1][1] = 1 / (tanHalfFovy);
	Result.m[2][2] = -(zFar + zNear) / (zFar - zNear);
	Result.m[2][3] = -1;
	Result.m[3][2] = -(2 * zFar * zNear) / (zFar - zNear);
	return Result;
}

Mat4 myortho(float left, float right, float bottom, float top/*, float zNear, float zFar*/)
{
	Mat4 Result(1.0);
	//float tanHalfFovy = tan(fov / 2.0);
	Result.m[0][0] = 2.0 / (right - left);
	//Result.m[0][3] = -(right + left) / (right - left);
	Result.m[1][1] = 2.0 / (top - bottom);
	//Result.m[1][3] = -(top + bottom) / (top - bottom);
	//Result.m[2][2] = -2.0 / (zFar - zNear);
	Result.m[2][2] = -1.0;
	//Result.m[2][3] = -(zFar + zNear) / (zFar - zNear);
	Result.m[3][0] = -(right + left) / (right - left);
	Result.m[3][1] = -(top + bottom) / (top - bottom);
	return Result;
}
#endif