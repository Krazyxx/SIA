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

#ifndef EIGEN_TRACKBALL_H
#define EIGEN_TRACKBALL_H

#include "common.h"
#include "vector.h"

class Camera;

class Trackball
{
  public:

    enum Mode {Around, Local};

    Trackball() : m_pCamera(0) {}

    void start(Mode m = Around) { m_mode = m; m_lastPointOk = false; }

    void setCamera(Camera* pCam) { m_pCamera = pCam; }

    void track(const Vector2i& newPoint2D);

  protected:

    bool mapToSphere( const Vector2i& p2, Vector3f& v3);

    Camera* m_pCamera;
    Point3f m_lastPoint3D;
    Mode m_mode;
    bool m_lastPointOk;

};

#endif // EIGEN_TRACKBALL_H
