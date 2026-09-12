/*
Part of Newcastle University's Game Engineering source code.

Use as you see fit!

Comments and queries to: richard-gordon.davison AT ncl.ac.uk
https://research.ncl.ac.uk/game/
*/
#include "Quaternion.h"
#include "Maths.h"

using namespace NCL;
using namespace NCL::Maths;

Quaternion::Quaternion(void)
{
	x = y = z = 0.0f;
	w = 1.0f;
}

Quaternion::Quaternion(float x, float y, float z, float w)
{
	this->x = x;
	this->y = y;
	this->z = z;
	this->w = w;
}

Quaternion::Quaternion(double x, double y, double z, double w)
{
	this->x = (float)x;
	this->y = (float)y;
	this->z = (float)z;
	this->w = (float)w;
}

Quaternion::Quaternion(const Vector3& vector, float w) {
	this->x = vector.x;
	this->y = vector.y;
	this->z = vector.z;
	this->w = w;
}

Quaternion::Quaternion(const Matrix4& m) {
	w = sqrt(std::max(0.0f, (1.0f + m.array[0][0] + m.array[1][1] + m.array[2][2]))) * 0.5f;

	if (abs(w) < 0.0001f) {
		x = sqrt(std::max(0.0f, (1.0f + m.array[0][0] - m.array[1][1] - m.array[2][2]))) / 2.0f;
		y = sqrt(std::max(0.0f, (1.0f - m.array[0][0] + m.array[1][1] - m.array[2][2]))) / 2.0f;
		z = sqrt(std::max(0.0f, (1.0f - m.array[0][0] - m.array[1][1] + m.array[2][2]))) / 2.0f;

		x = (float)copysign(x, m.array[2][1] - m.array[1][2]);
		y = (float)copysign(y, m.array[0][2] - m.array[2][0]);
		z = (float)copysign(z, m.array[1][0] - m.array[0][1]);
	}
	else {
		float qrFour = 4.0f * w;
		float qrFourRecip = 1.0f / qrFour;

		x = (m.array[1][2] - m.array[2][1]) * qrFourRecip;
		y = (m.array[2][0] - m.array[0][2]) * qrFourRecip;
		z = (m.array[0][1] - m.array[1][0]) * qrFourRecip;
	}
}

Quaternion::Quaternion(const Matrix3& m) {
	w = sqrt(std::max(0.0f, (1.0f + m.array[0][0] + m.array[1][1] + m.array[2][2]))) * 0.5f;

	if (abs(w) < 0.0001f) {
		x = sqrt(std::max(0.0f, (1.0f + m.array[0][0] - m.array[1][1] - m.array[2][2]))) / 2.0f;
		y = sqrt(std::max(0.0f, (1.0f - m.array[0][0] + m.array[1][1] - m.array[2][2]))) / 2.0f;
		z = sqrt(std::max(0.0f, (1.0f - m.array[0][0] - m.array[1][1] + m.array[2][2]))) / 2.0f;

		x = (float)copysign(x, m.array[2][1] - m.array[1][2]);
		y = (float)copysign(y, m.array[0][2] - m.array[2][0]);
		z = (float)copysign(z, m.array[1][0] - m.array[0][1]);
	}
	else {
		float qrFour = 4.0f * w;
		float qrFourRecip = 1.0f / qrFour;

		x = (m.array[1][2] - m.array[2][1]) * qrFourRecip;
		y = (m.array[2][0] - m.array[0][2]) * qrFourRecip;
		z = (m.array[0][1] - m.array[1][0]) * qrFourRecip;
	}
}

float Quaternion::Dot(const Quaternion& a, const Quaternion& b) {
	return (a.x * b.x) + (a.y * b.y) + (a.z * b.z) + (a.w * b.w);
}

void Quaternion::Normalise() {
	float magnitude = sqrt(x * x + y * y + z * z + w * w);

	if (magnitude > 0.0f) {
		float t = 1.0f / magnitude;

		x *= t;
		y *= t;
		z *= t;
		w *= t;
	}
}

Quaternion Quaternion::Normalised() const {
	Quaternion temp(*this);
	temp.Normalise();
	return temp;
}

void Quaternion::CalculateW() {
	w = 1.0f - (x * x) - (y * y) - (z * z);
	if (w < 0.0f) {
		w = 0.0f;
	}
	else {
		w = -sqrt(w);
	}
}

Quaternion Quaternion::Conjugate() const
{
	return Quaternion(-x, -y, -z, w);
}

Quaternion Quaternion::Lerp(const Quaternion& from, const Quaternion& to, float by) {
	Quaternion temp = to;

	float dot = Quaternion::Dot(from, to);

	if (dot < 0.0f) {
		temp = -to;
	}

	return (from * (1.0f - by)) + (temp * by);
}
//SIGGRAPH Shoemake
Quaternion Quaternion::Slerp(const Quaternion& from, const Quaternion& to, float by) {
	float t = by;

	float dot = std::clamp(Quaternion::Dot(from, to), -1.0f, 1.0f);

	if (dot == 1.0f) {
		return from;
	}

	float theta = std::abs(acos(dot));

	float aScale = sin((1 - t) * theta);
	float bScale = sin(t * theta);

	Quaternion q = (from * aScale) + (to * bScale);

	q *= 1.0f / sin(theta);

	q.Normalise();
	return q;
}

//http://en.wikipedia.org/wiki/Conversion_between_quaternions_and_Euler_angles
//Verified! Different values to above, due to difference between x/z being 'forward'
Vector3 Quaternion::ToEuler() const {
	Vector3 euler;

	float t = x * y + z * w;

	if (t > 0.4999) {
		euler.z = Maths::RadiansToDegrees(Maths::PI / 2.0f);
		euler.y = Maths::RadiansToDegrees(2.0f * atan2(x, w));
		euler.x = 0.0f;

		return euler;
	}

	if (t < -0.4999) {
		euler.z = -Maths::RadiansToDegrees(Maths::PI / 2.0f);
		euler.y = -Maths::RadiansToDegrees(2.0f * atan2(x, w));
		euler.x = 0.0f;
		return euler;
	}

	float sqx = x * x;
	float sqy = y * y;
	float sqz = z * z;

	euler.z = Maths::RadiansToDegrees(asin(2 * t));
	euler.y = Maths::RadiansToDegrees(atan2(2 * y * w - 2 * x * z, 1.0f - 2 * sqy - 2 * sqz));
	euler.x = Maths::RadiansToDegrees(atan2(2 * x * w - 2 * y * z, 1.0f - 2 * sqx - 2.0f * sqz));

	return euler;
}

//http://www.euclideanspace.com/maths/geometry/rotations/conversions/eulerToQuaternion/
//VERIFIED AS CORRECT - Pitch and roll are changed around as the above uses x as 'forward', whereas we use -z
Quaternion Quaternion::EulerAnglesToQuaternion(float roll, float yaw, float pitch) {
	float cos1 = (float)cos(Maths::DegreesToRadians(yaw * 0.5f));
	float cos2 = (float)cos(Maths::DegreesToRadians(pitch * 0.5f));
	float cos3 = (float)cos(Maths::DegreesToRadians(roll * 0.5f));

	float sin1 = (float)sin(Maths::DegreesToRadians(yaw * 0.5f));
	float sin2 = (float)sin(Maths::DegreesToRadians(pitch * 0.5f));
	float sin3 = (float)sin(Maths::DegreesToRadians(roll * 0.5f));

	Quaternion q;

	q.x = (sin1 * sin2 * cos3) + (cos1 * cos2 * sin3);
	q.y = (sin1 * cos2 * cos3) + (cos1 * sin2 * sin3);
	q.z = (cos1 * sin2 * cos3) - (sin1 * cos2 * sin3);
	q.w = (cos1 * cos2 * cos3) - (sin1 * sin2 * sin3);

	return q;
};



Vector3		Quaternion::operator *(const Vector3& a)	const {
	Quaternion newVec = *this * Quaternion(a.x, a.y, a.z, 0.0f) * Conjugate();
	return Vector3(newVec.x, newVec.y, newVec.z);
}

Quaternion Quaternion::FromTwoVectors(const Vector3& from, const Vector3& to) {
	Vector3 f = Vector::Normalise(from); // 归一化输入向量
	Vector3 t = Vector::Normalise(to);

	// 计算点积和旋转轴
	float dot = Vector::Dot(f, t); // 余弦值
	Vector3 cross = Vector::Cross(f, t); // 旋转轴

	// 特殊情况处理
	if (dot > 0.9999f) {
		// from 和 to 几乎相等，无需旋转
		return Quaternion(0.0, 0.0, 0.0, 1.0);
	}
	if (dot < -0.9999f) {
		// from 和 to 完全相反，选择一个正交轴
		Vector3 orthogonal = Vector::Orthogonal(f); // 找一个和 from 不平行的向量
		orthogonal = Vector::Normalise(orthogonal);
		return AxisAngleToQuaternion(orthogonal, 180.0f);
	}

	// 正常情况：根据轴和角构造四元数
	float angle = acos(dot); // 计算夹角（弧度）
	return AxisAngleToQuaternion(cross, Maths::RadiansToDegrees(angle));
}

Quaternion Quaternion::Inverse() const {
	float norm = x * x + y * y + z * z + w * w;
	if (norm > 0.0f) {
		float invNorm = 1.0f / norm;
		return Quaternion(-x * invNorm, -y * invNorm, -z * invNorm, w * invNorm);
	}
	else {
		// 如果四元数的模为0，返回单位四元数
		return Quaternion(0.0f, 0.0f, 0.0f, 1.0f);
	}
}

void Quaternion::ToAxisAngle(Vector3& axis, float& angle) const {
	float norm = x * x + y * y + z * z;
	if (norm > 0.0f) {
		norm = sqrt(norm);
		angle = 2.0f * atan2(norm, w);
		axis.x = x / norm;
		axis.y = y / norm;
		axis.z = z / norm;
	}
	else {
		// 如果四元数的模为0，表示没有旋转，轴可以任意，角度为0
		axis = Vector3(1.0f, 0.0f, 0.0f);
		angle = 0.0f;
	}
}