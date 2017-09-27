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

        Color3f radiance = Color3f::Zero();
        Normal3f normal = hit.normal();
        Point3f pos = ray.at(hit.t());

        for (int i = 0; i < m_sampleCount; i++) {
            float x = Eigen::internal::random<float>(0,1);
            float y = Eigen::internal::random<float>(0,1);

            Vector3f d = Warp::squareToUniformHemisphere(Point2f(x,y)) - pos;
            Vector3f u = normal.unitOrthogonal();
            Vector3f v = normal.cross(u);
            Vector3f p = d.x() * u + d.y() * v + d.z() * normal;

            Ray shadowRay(pos + normal * Epsilon, d, true);
            Hit shadowHit;
            scene->intersect(shadowRay, shadowHit);
            if(!shadowHit.foundIntersection()) {
                /*
                float cos_term = std::max(0.f, direction.dot(normal));
                Color3f brdf = material->brdf(-ray.direction, lightDir, normal, hit.texcoord());
                radiance += (*it)->intensity(pos) * cos_term * brdf;
                */
            }
        }

        return radiance;
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
