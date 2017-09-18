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
#include <math.h>

Point2f Warp::squareToUniformSquare(const Point2f &sample) {
    return sample;
}

float Warp::squareToUniformSquarePdf(const Point2f &sample) {
    return ((sample.array() >= 0).all() && (sample.array() <= 1).all()) ? 1.0f : 0.0f;
}

Point2f Warp::squareToUniformDisk(const Point2f &sample) {

    float x = (sample.x() - 0.5) * 2;
    float y = (sample.y() - 0.5) * 2;

    /*
    float phi, pi = 3.1415, r;

    if (x > -y) {
        if(x > y) {
            r = x;
            phi = pi/4.0 * (y/x);
        } else {
            r = y;
            phi = pi/4.0 * (2 - (x/y));
        }
    } else {
        if(x < y) {
            r = -x;
            phi = (pi/4.0) * (4 + (y/x));
        } else {
            r = -y;
            phi = pi/4.0 * (6 - (x/y));
        }
    }
    */

    float r = sqrt(x);
    float phi = 2 * M_PI * y;
    float u = r * cos(phi);
    float v = r * sin(phi);
    return Point2f(u,v);
    throw RTException("Warp::squareToUniformDisk() is not yet implemented!");
}

float Warp::squareToUniformDiskPdf(const Point2f &p) {
    throw RTException("Warp::squareToUniformDiskPdf() is not yet implemented!");
}

Vector3f Warp::squareToUniformHemisphere(const Point2f &sample) {
    throw RTException("Warp::squareToUniformHemisphere() is not yet implemented!");
}

float Warp::squareToUniformHemispherePdf(const Vector3f &v) {
    throw RTException("Warp::squareToUniformHemispherePdf() is not yet implemented!");
}

Vector3f Warp::squareToCosineHemisphere(const Point2f &sample) {
    throw RTException("Warp::squareToCosineHemisphere() is not yet implemented!");
}

float Warp::squareToCosineHemispherePdf(const Vector3f &v) {
    throw RTException("Warp::squareToCosineHemispherePdf() is not yet implemented!");
}
