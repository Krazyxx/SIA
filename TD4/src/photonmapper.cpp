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

#include "integrator.h"
#include "material.h"
#include "scene.h"
#include "photon.h"
#include "areaLight.h"
#include <memory>

class PhotonMapper : public Integrator
{
public:
    /// Photon map data structure
    typedef PointKDTree<Photon> PhotonMap;

    PhotonMapper(const PropertyList &props)
    {
        /* Lookup parameters */
        m_photonCount  = props.getInteger("photonCount", 1000000);
        m_photonRadius = props.getFloat("photonRadius", 0.0f /* Default: automatic */);
    }

    void preprocess(const Scene *scene)
    {
        std::cout << "preprocess" << std::endl;

        /* Allocate memory for the photon map */
        m_photonMap = std::unique_ptr<PhotonMap>(new PhotonMap());
        m_photonMap->reserve(m_photonCount);

        /* Estimate a default photon radius */
        if (m_photonRadius == 0) {
            Eigen::AlignedBox3f aabb;
            for(uint i=0; i<scene->shapeList().size(); ++i) {
                aabb.extend(scene->shapeList()[i]->AABB());
            }
            m_photonRadius = (aabb.max()-aabb.min()).norm() / 500.0f;
        }

        /// TODO

        /* Trace photons */
        m_nPaths = 0;

        /* Build the photon map */
        m_photonMap->build();

        /* Now let's do a lookup to see if it worked */
        std::vector<uint32_t> results;
        m_photonMap->search(Point3f(0, 0, 0) /* lookup position */, m_photonRadius, results);

        for (uint32_t i : results) {
            const Photon &photon = (*m_photonMap)[i];
            cout << "Found photon!" << endl;
            cout << " Position  : " << photon.getPosition().toString() << endl;
            cout << " Power     : " << photon.getPower().toString() << endl;
            cout << " Direction : " << photon.getDirection().toString() << endl;
        }
    }

    Color3f Li(const Scene *scene, const Ray & ray) const
    {
        /// TODO
        return Color3f(0);
    }

    std::string toString() const
    {
        return tfm::format(
            "PhotonMapper[\n"
            "  photonCount = %i,\n"
            "  photonRadius = %f\n"
            "]",
            m_photonCount,
            m_photonRadius
        );
    }
private:
    int m_photonCount, m_nPaths;
    float m_photonRadius;
    std::unique_ptr<PhotonMap> m_photonMap;
};

REGISTER_CLASS(PhotonMapper, "photonmapper")
