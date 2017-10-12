// This file is part of Eigen, a lightweight C++ template library
// for linear algebra.
//
// Copyright (C) 2008 Gael Guennebaud <gael.guennebaud@inria.fr>
//
// Eigen is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 3 of the License, or (at your option) any later version.
//
// Alternatively, you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 2 of
// the License, or (at your option) any later version.
//
// Eigen is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
// FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License or the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License and a copy of the GNU General Public License along with
// Eigen. If not, see <http://www.gnu.org/licenses/>.

#include "trackball.h"
#include "camera.h"

void Trackball::track(const Vector2i& point2D)
{
  if (m_pCamera==0)
    return;
  Vector3f newPoint3D;
  bool newPointOk = mapToSphere(point2D, newPoint3D);

  if (m_lastPointOk && newPointOk)
  {
    Vector3f axis = m_lastPoint3D.cross(newPoint3D);
    float axis_norm = axis.norm();
    if(axis_norm<=1e-7)
      return;
    axis /= axis_norm;
    float cos_angle = m_lastPoint3D.dot(newPoint3D);
    if ( std::abs(cos_angle) < 1.0 )
    {
      float angle = 2. * acos(cos_angle);
      if (m_mode==Around)
        m_pCamera->rotateAroundTarget(Eigen::Quaternionf(Eigen::AngleAxisf(angle, axis)));
      else
        m_pCamera->localRotate(Eigen::Quaternionf(Eigen::AngleAxisf(-angle, axis)));
    }
  }

  m_lastPoint3D = newPoint3D;
  m_lastPointOk = newPointOk;
}

bool Trackball::mapToSphere(const Vector2i& p2, Vector3f& v3)
{
  if ((p2.x() >= 0) && (p2.x() <= int(m_pCamera->vpWidth())) &&
      (p2.y() >= 0) && (p2.y() <= int(m_pCamera->vpHeight())) )
  {
    double x  = (double)(p2.x() - 0.5*m_pCamera->vpWidth())  / (double)m_pCamera->vpWidth();
    double y  = (double)(0.5*m_pCamera->vpHeight() - p2.y()) / (double)m_pCamera->vpHeight();
    double sinx         = sin(M_PI * x * 0.5);
    double siny         = sin(M_PI * y * 0.5);
    double sinx2siny2   = sinx * sinx + siny * siny;

    v3.x() = sinx;
    v3.y() = siny;
    v3.z() = sinx2siny2 < 1.0 ? sqrt(1.0 - sinx2siny2) : 0.0;

    return true;
  }
  else
    return false;
}
