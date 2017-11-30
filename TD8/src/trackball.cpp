/*
 Copyright 2015 Simon Boyé.

 This Source Code Form is subject to the terms of the Mozilla Public
 License, v. 2.0. If a copy of the MPL was not distributed with this
 file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "trackball.h"

#define ORTHO_THRESOLD 1.e2f

Eigen::Matrix4f orthographicProjection(
        float l, float r, float b, float t, float n, float f) {
    float rpl = r + l;
    float tpb = t + b;
    float fpn = f + n;

    float rml = r - l;
    float tmb = t - b;
    float fmn = f - n;

    Eigen::Matrix4f m;
    m << 2.f/rml,       0,        0, -rpl/rml,
               0, 2.f/tmb,        0, -tpb/tmb,
               0,       0, -2.f/fmn,  fpn/fmn,
               0,       0,        0,        1;
    return m;
}


Eigen::Matrix4f perspectiveProjection(
        float l, float r, float b, float t, float n, float f) {
    float rpl = r + l;
    float tpb = t + b;
    float fpn = f + n;

    float rml = r - l;
    float tmb = t - b;
    float fmn = f - n;

    Eigen::Matrix4f m;
    m << 2/rml,     0,  rpl/rml,          0,
             0, 2/tmb,  tpb/tmb,          0,
             0,     0, -fpn/fmn, -2*f*n/fmn,
             0,     0,       -1,          0;
    return m;
}


Trackball::Trackball() :
    _scnCenter(Eigen::Vector3f::Zero()),
    _scnDistance(3.),
    _scnRadius(1.),
    _scnOrientation(1., 0., 0., 0.),
    _scrViewport(),
    _minNear(.001),
    _nearOffset(-1),
    _farOffset(1),
    _state(Idle) {
}


const Eigen::Vector3f& Trackball::sceneCenter() const {
    return _scnCenter;
}


float Trackball::sceneDistance() const {
    return _scnDistance;
}


float Trackball::sceneRadius() const {
    return _scnRadius;
}


const Eigen::Quaternionf& Trackball::sceneOrientation() const {
    return _scnOrientation;
}


const Eigen::AlignedBox2f& Trackball::screenViewport() const {
    return _scrViewport;
}


float Trackball::minNear() const {
    return _minNear;
}


float Trackball::nearOffset() const {
    return _nearOffset;
}


float Trackball::farOffset() const {
    return _farOffset;
}


float Trackball::minScreenViewportSize() const {
    return _scrViewport.sizes().minCoeff();
}


bool Trackball::isOrthographic() const {
    return (_scnDistance / _scnRadius) > ORTHO_THRESOLD;
}


bool Trackball::isPerspective() const {
    return !isOrthographic();
}


Eigen::Matrix4f Trackball::computeViewMatrix() const {
    Eigen::Vector3f scnCamera = _scnCenter;
    if(isPerspective())
        scnCamera += _scnOrientation * Eigen::Vector3f::UnitZ() * _scnDistance;
    return Eigen::Affine3f(
               _scnOrientation.inverse() * Eigen::Translation3f(-scnCamera)).matrix();
}


Eigen::Matrix4f Trackball::computeProjectionMatrix() const {
    float scrMinSize = minScreenViewportSize();
    float r = _scnRadius * _scrViewport.sizes().x() / (scrMinSize);
    float t = _scnRadius * _scrViewport.sizes().y() / (scrMinSize);

    if(isPerspective()) {
        r /= _scnDistance;
        t /= _scnDistance;
        return perspectiveProjection(
            -r, r, -t, t,
            std::max(_minNear, _scnDistance + _nearOffset),
            _scnDistance + _farOffset);
    } else {
        return orthographicProjection(-r, r, -t, t, _nearOffset, _farOffset);
    }
}


void Trackball::setSceneCenter(const Eigen::Vector3f& scnCenter) {
    _scnCenter = scnCenter;
}


void Trackball::setSceneDistance(float scnDistance) {
    _scnDistance = scnDistance;
}


void Trackball::setSceneRadius(float scnRadius) {
    _scnRadius = scnRadius;
}


void Trackball::setSceneOrientation(const Eigen::Quaternionf& scnOrientation) {
    _scnOrientation = scnOrientation;
}


void Trackball::setScreenViewport(const Eigen::AlignedBox2f& scrViewport) {
    _scrViewport = scrViewport;
}


void Trackball::setMinNear(float minNear) {
    _minNear = minNear;
}


void Trackball::setNearFarOffsets(float nearOffset, float farOffset) {
    _nearOffset = nearOffset;
    _farOffset = farOffset;
}


bool Trackball::isIdle() const {
    return _state == Idle;
}


void Trackball::rotate(const Eigen::Quaternionf& rot) {
    _scnOrientation *= rot;
}


bool Trackball::isRotating() const {
    return _state == Rotating;
}


void Trackball::startRotation(const Eigen::Vector2f& scrPos) {
    assert(_state == Idle);
    _state = Rotating;
    _scrMouseInit = scrPos;
    _scnOrientInit = _scnOrientation;
}


void Trackball::dragRotate(const Eigen::Vector2f& scrPos) {
    assert(_state == Rotating);
    _scnOrientation = computeRotation(scrPos);
}


void Trackball::cancelRotation() {
    assert(_state == Rotating);
    _state = Idle;
    _scnOrientation = _scnOrientInit;
}


void Trackball::endRotation() {
    assert(_state == Rotating);
    _state = Idle;
}


bool Trackball::isTranslating() const {
    return _state == Translating;
}


void Trackball::startTranslation(const Eigen::Vector2f& scrPos) {
    assert(_state == Idle);
    _state = Translating;
    _scrMouseInit = scrPos;
    _scnCenterInit = _scnCenter;
}


void Trackball::dragTranslate(const Eigen::Vector2f& scrPos) {
    assert(_state == Translating);
    _scnCenter = computeTranslation(scrPos);
}


void Trackball::cancelTranslation() {
    assert(_state == Translating);
    _state = Idle;
    _scnCenter = _scnCenterInit;
}


void Trackball::endTranslation() {
    assert(_state == Translating);
    _state = Idle;
}


void Trackball::zoom(float factor) {
    if(isPerspective())
        _scnDistance /= factor;
    _scnRadius   /= factor;
}


void Trackball::grow(float factor) {
    _scnRadius *= factor;
}


void Trackball::dollyZoom(float factor) {
    if(_scnDistance / _scnRadius > ORTHO_THRESOLD)
        _scnDistance = ORTHO_THRESOLD * _scnRadius;
    _scnDistance /= factor;
}


Eigen::Vector2f Trackball::normFromScr(const Eigen::Vector2f& scrPos) const {
    return (scrPos - _scrViewport.center()) / minScreenViewportSize();
}


Eigen::Quaternionf Trackball::computeRotation(const Eigen::Vector2f& scrPos) const {
    Eigen::Vector2f v = (_scrMouseInit - scrPos)
                      * (float(2. * M_PI) / _scrViewport.sizes().x());
    Eigen::Vector3f x = Eigen::Vector3f::UnitX();
    Eigen::Vector3f y = Eigen::Vector3f::UnitY();
    return _scnOrientInit * Eigen::Quaternionf(Eigen::AngleAxisf(v.x(), y))
                          * Eigen::Quaternionf(Eigen::AngleAxisf(v.y(), x));
}


Eigen::Vector3f Trackball::computeTranslation(const Eigen::Vector2f& scrPos) const {
    Eigen::Matrix<float, 3, 2> m;
    Eigen::Vector2f normOffset = 2.f * (scrPos - _scrMouseInit) / minScreenViewportSize();
    m << _scnOrientation *  Eigen::Vector3f::UnitX(),
         _scnOrientation * -Eigen::Vector3f::UnitY();
    return _scnCenterInit - m * normOffset * _scnRadius;
}
