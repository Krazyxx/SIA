#include "integrator.h"
#include "scene.h"
#include "material.h"
#include "warp.h"
#include <stdlib.h>

class AO : public Integrator
{
public:
    AO(const PropertyList &props) {
        m_sampleCount = props.getInteger("samples", 10);
        m_cosineWeighted = props.getBoolean("cosine", true);
        srand(time(NULL));
    }

    Color3f Li(const Scene *scene, const Ray &ray) const {
        Hit hit;
        scene->intersect(ray, hit);

        if (!hit.foundIntersection())
            return scene->backgroundColor();

        float L = 0.0; // Ambiant Occlusion
        Point3f pos = ray.at(hit.t());
        Normal3f normal = hit.normal();
        Vector3f u = normal.unitOrthogonal(); /*cross(Vector3f(0,0,1));*/
        Vector3f v = normal.cross(u);

        for (int i = 0; i < m_sampleCount; i++) {
            float x = Eigen::internal::random<float>(0,1);
            float y = Eigen::internal::random<float>(0,1);

            Vector3f d = m_cosineWeighted ? Warp::squareToCosineHemisphere(Point2f(x,y)) :
                                            Warp::squareToUniformHemisphere(Point2f(x,y));
            Vector3f p = d.x() * u + d.y() * v + d.z() * normal;

            Ray shadowRay(pos + normal * Epsilon, p, true);
            Hit shadowHit;
            scene->intersect(shadowRay, shadowHit);
            if(!shadowHit.foundIntersection()) {
                float theta = p.dot(normal) / (p.norm() * normal.norm());
                float pdf = m_cosineWeighted ? Warp::squareToCosineHemispherePdf(d) :
                                               Warp::squareToUniformHemispherePdf(d);
                L += (theta / M_PI) / pdf;
            }
        }

        L /= (float) m_sampleCount;

        return Color3f(L,L,L);
    }

    std::string toString() const {
      return tfm::format("AO[\n"
                         "  samples = %f\n"
                         "  cosine-weighted = %s]\n",
                         m_sampleCount,
                         m_cosineWeighted ? "true" : "false");
  }

private:
    int m_sampleCount;
    bool m_cosineWeighted;
};

REGISTER_CLASS(AO, "ao")
