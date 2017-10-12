/*
    This file is part of Nori, a simple educational ray tracer

    Copyright (c) 2015 by Wenzel Jakob

    Nori is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License Version 3
    as published by the Free Software Foundation.

    Nori is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include <warp.h>
#include <vector.h>

Point2f Warp::squareToUniformSquare(const Point2f &sample) {
    return sample;
}

float Warp::squareToUniformSquarePdf(const Point2f &sample) {
    return ((sample.array() >= 0).all() && (sample.array() <= 1).all()) ? 1.0f : 0.0f;
}

Point2f Warp::squareToUniformDisk(const Point2f &sample) {
    float r = std::sqrt(sample.x());
    float theta = 2.f * M_PI * sample.y();
    return Point2f(r * std::cos(theta), r * std::sin(theta));
}

float Warp::squareToUniformDiskPdf(const Point2f &p) {
    if(p.squaredNorm() <= 1)
        return INV_PI;
    return 0;
}

Vector3f Warp::squareToUniformHemisphere(const Point2f &sample) {
    float sintheta = std::sqrt(1-sample.x()*sample.x());
    float phi = 2.f * M_PI * sample.y();
    return Vector3f( sintheta * std::cos(phi) , sintheta * std::sin(phi), sample.x());
}

float Warp::squareToUniformHemispherePdf(const Vector3f &v) {
    if(v.z() < 0)
        return 0;
    return INV_TWOPI;
}

Vector3f Warp::squareToCosineHemisphere(const Point2f &sample) {
    float costheta = sqrt(1.f - sample.x());
    float phi = 2.f * M_PI * sample.y();
    return Vector3f(cos(phi) * sqrt(sample.x()), sin(phi) * sqrt(sample.x()), costheta);
}

float Warp::squareToCosineHemispherePdf(const Vector3f &v) {
    if(v.z() < 0)
        return 0;
    return v.z() * INV_PI;
}

