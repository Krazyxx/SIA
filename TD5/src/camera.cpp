#include "camera.h"

using namespace Eigen;

Matrix4f Camera::perspective(float fovy, float aspect, float zNear, float zFar)
{
    float tanHalfFovy = tan(fovy / 2.f);

    Matrix4f m;
    m << 1.f / (aspect * tanHalfFovy), 0,  0, 0,
             0,      1.f / (tanHalfFovy),  0, 0,
             0, 0, - (zFar + zNear) / (zFar - zNear), - (2.f * zFar * zNear) / (zFar - zNear),
             0,     0,       -1,          0;
    return m;
}

Matrix4f Camera::lookAt(const Vector3f& position, const Vector3f& target, const Vector3f& up)
{
    Matrix3f R;
    R.col(2) = (position - target).normalized();
    R.col(0) = up.cross(R.col(2)).normalized();
    R.col(1) = R.col(2).cross(R.col(0));

    Matrix4f m(Matrix4f::Identity());
    m.topLeftCorner<3,3>() = R.transpose();
    m.topRightCorner<3,1>() = -m.topLeftCorner<3,3>() * position;
    return m;
}
