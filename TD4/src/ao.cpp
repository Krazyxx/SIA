#include "integrator.h"
#include "scene.h"
#include "material.h"
#include "warp.h"

class AO : public Integrator
{
public:
    AO(const PropertyList &props) {
        m_sampleCount = props.getInteger("samples", 10);
        m_cosineWeighted = props.getBoolean("cosine", true);
    }

    Color3f Li(const Scene *scene, const Ray &ray) const {
        Hit hit;
        scene->intersect(ray, hit);
        if (!hit.foundIntersection())
            return scene->backgroundColor(ray.direction);

        Point3f pos = ray.at(hit.t());
        const Material* material = hit.shape()->material();

        Color3f radiance = Color3f::Zero();

        Normal3f normal = hit.normal();

        for(int i=0; i<m_sampleCount; ++i){

                Vector3f u = normal.unitOrthogonal();
                Vector3f v = normal.cross(u);

            Vector3f d;
            float pdf;

            if (m_cosineWeighted) {
                d = Warp::squareToCosineHemisphere(Point2f(Eigen::internal::random<float>(0,1),Eigen::internal::random<float>(0,1)));
                pdf = Warp::squareToCosineHemispherePdf(d);
            }else{
                d = Warp::squareToUniformHemisphere(Point2f(Eigen::internal::random<float>(0,1),Eigen::internal::random<float>(0,1)));
                pdf = Warp::squareToUniformHemispherePdf(d);
            }
            Vector3f r = d.x() * u + d.y() * v + d.z() * normal;

            Ray shadow_ray(pos + normal*Epsilon, r, true);
            Hit shadow_hit;
            scene->intersect(shadow_ray,shadow_hit);

            if(!shadow_hit.foundIntersection()){
                radiance += INV_PI * material->diffuseColor(Vector2f::Zero()) * normal.dot(r) / pdf;
            }

        }

        return radiance / m_sampleCount;
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
