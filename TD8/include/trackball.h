/*
 Copyright 2015 Simon Boyé.

 This Source Code Form is subject to the terms of the Mozilla Public
 License, v. 2.0. If a copy of the MPL was not distributed with this
 file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#ifndef _TRACKBALL_H
#define _TRACKBALL_H


#include <Eigen/Geometry>


/// \brief Compute an orthographic projection matrix
Eigen::Matrix4f orthographicProjection(
        float l, float r, float b, float t, float n, float f);

/// \brief Compute an orthographic projection matrix
Eigen::Matrix4f orthographicProjection(
        const Eigen::AlignedBox3f& viewBox);

/// \brief Compute a projection matrix. l, r, b and t are defined on the near plane.
Eigen::Matrix4f perspectiveProjection(
        float l, float r, float b, float t, float n, float f);


/**
 * \brief A (relatively) simple trackball.
 *
 * \verbatim
 *        -  -
 *        |  |
 * radius |  |
 *        |  |
 *        -  o----------------> camera
 *    center |    distance
 *           |
 *           |
 *           -
  *     object plane
 * \endverbatim
 *
 * FOV is set such that a circle of radius sceneRadius() drawn around the
 * center fit the screen (the circle touch the borders). When distance grow
 * beyond some threshold, the perspective projection is replaced by an
 * orthographic projection.
 *
 * The near and far plane are computed such that everything in the range
 * [center - nearOffset, center + farOffset] is visible. It should be set
 * according to the radius of the scene. If the camera is inside the depth
 * range, near is set to minNear().
 *
 * There is a typical trackball initialization code:
 * ```
 * Trackball trackball;
 * Eigen::AlignedBox3f bb = [ subject bounding box ];
 *
 * trackball.setSceneCenter(bb.center());
 * // The camera will see the whole object by default.
 * trackball.setSceneRadius(bb.sizes().maxCoeff());
 * // This place the camera at the distance required for a 60 degree FOV.
 * trackball.setSceneDistance(trackball.sceneRadius() * 3.f);
 * // A big margin for near/far planes allow to move the object far from the
 * // center. A smaller margin is acceptable most of the time.
 * trackball.setNearFarOffsets(-trackball.sceneRadius() * 100.f,
 *                                trackball.sceneRadius() * 100.f);
 * // Setup the size of the viewport, in pixels. Used to convert screen
 * // coordinates into scene coordinates during interaction and for
 * // aspect-ratio (assume square pixels).
 * trackball.setScreenViewport(Eigen::AlignedBox2f(
 *         Eigen::Vector2f(0, 0),
 *         Eigen::Vector2f(m_info.windowWidth, m_info.windowHeight)));
 * ```
 *
 * Mouse interaction is stateful. You can start view pan/rotate with the
 * start*() functions. While the mouse move, you can update transform using
 * drag*() functions. To return to Idle state (no interaction), you must use
 * either an end*() function to keep the transformation or a cancel*() function
 * to get back to the original state.
 *
 * Rotation interaction is made such that placing the mouse back to the
 * mouse press position result in no rotation.
 *
 * Translation interaction is designed such that the point on the object plane
 * when the mouse is pressed stay under the cursor. This ensure a correct
 * sensitivity in most cases (when the subject is near to the view center).
 *
 * The methods zoom, grow and dollyZoom allow to manipulate the zoom level.
 * zoom change the camera distance and the object radius, thus leaving the
 * fov unchanged. grow change the object radius, changing the fov without
 * moving the camera. Finaly, dollyZoom move the camera without changing the
 * radius, changing the perspective.
 *
 * View and projection matrices are obtained using the methods
 * computeViewMatrix() and computeProjectionMatrix(). The matrices are
 * recomputed each time, so it might be a good idea to cache the result if you
 * often need them.
 *
 * Note: the prefix "scene" (scn for attributes) mean that the coordinates are
 * in scene space. "screen" (src for attributes) is used for screen coordinates
 * (in pixels).
 *
 * Potential enhancements: add methods and state(s) for interactive zoom,
 * better rotation trackball, methods to convert points to/from different
 * spaces.
 */
class Trackball {
public:
    enum State {
        Idle, Rotating, Translating
    };


public:
    Trackball();

    /// \brief The center of the view. Trackball rotate around this point.
    const Eigen::Vector3f& sceneCenter() const;

    /// \brief The distance between `center` and the camera.
    float sceneDistance() const;

    /// \brief The radius of a circle centered on a object plane that define the fov.
    float sceneRadius() const;

    /// \brief The orientation of the camera.
    const Eigen::Quaternionf& sceneOrientation() const;

    /// \brief The viewport box in screen-space.
    const Eigen::AlignedBox2f& screenViewport() const;

    /// \brief The smallest possible near distance.
    float minNear() const;

    float nearOffset() const;
    float farOffset() const;

    float minScreenViewportSize() const;
    bool isOrthographic() const;
    bool isPerspective() const;

    Eigen::Matrix4f computeViewMatrix() const;
    Eigen::Matrix4f computeProjectionMatrix() const;

    void setSceneCenter(const Eigen::Vector3f& scnCenter);
    void setSceneDistance(float scnDistance);
    void setSceneRadius(float scnRadius);
    void setSceneOrientation(const Eigen::Quaternionf& scnOrientation);
    void setScreenViewport(const Eigen::AlignedBox2f& scnViewport);
    void setMinNear(float minNear);
    void setNearFarOffsets(float nearOffset, float farOffset);

    State state() const;
    bool isIdle() const;

    /// \brief Rotate around `center`.
    void rotate(const Eigen::Quaternionf& rot);
    bool isRotating() const;
    void startRotation(const Eigen::Vector2f& scrPos);
    void dragRotate(const Eigen::Vector2f& scrPos);
    void cancelRotation();
    void endRotation();

    /// \brief Translate in the view plane.
    void translate(const Eigen::Vector2f& scnVec);
    void translate(const Eigen::Vector3f& scnVec);
    bool isTranslating() const;
    void startTranslation(const Eigen::Vector2f& scrPos);
    void dragTranslate(const Eigen::Vector2f& scrPos);
    void cancelTranslation();
    void endTranslation();

    /// \brief Move camera forward or backward without changing fov. (So it
    /// changes radius.)
    void zoom(float factor);

    /// \brief Grow or shrink radius, changing the fov.
    void grow(float factor);

    /// \brief Do a dolly zoom: move the camera and change the fov so that the
    /// subject stay in the same frame.
    void dollyZoom(float factor);

    EIGEN_MAKE_ALIGNED_OPERATOR_NEW

private:
    Eigen::Vector2f normFromScr(const Eigen::Vector2f& scrPos) const;

    Eigen::Quaternionf computeRotation(const Eigen::Vector2f& scrPos) const;
    Eigen::Vector3f computeTranslation(const Eigen::Vector2f& scrPos) const;

private:
    Eigen::Vector3f _scnCenter;
    float _scnDistance;
    float _scnRadius;
    Eigen::Quaternionf _scnOrientation;
    Eigen::AlignedBox2f _scrViewport;
    float _minNear;
    float _nearOffset;
    float _farOffset;

    State _state;
    Eigen::Vector2f _scrMouseInit;
    Eigen::Quaternionf _scnOrientInit;
    Eigen::Vector3f _scnCenterInit;

};


#endif
