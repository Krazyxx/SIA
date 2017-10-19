#include "integrator.h"
#include "scene.h"
#include "material.h"
#include "warp.h"

class DirectMats : public Integrator
{
public:
    DirectMats(const PropertyList &props) {
        m_sampleCount = props.getInteger("samples",4);
        m_IS = props.getBoolean("IS", false);
    }

    Color3f Li(const Scene *scene, const Ray &ray) const {
        Hit hit;
        scene->intersect(ray, hit);
        if (!hit.foundIntersection())
            return scene->backgroundColor(ray.direction);

        Color3f radiance = Color3f::Zero();
        const Material* material = hit.shape()->material();

        Normal3f normal = hit.normal();

        for(int i=0; i<m_sampleCount; ++i){
            float pdf;
            Vector3f r;
            if(m_IS) {
                r = material->is(normal, -ray.direction, pdf);
            }else{
                r = material->us(normal, -ray.direction, pdf);
            }
            if(normal.dot(r)<0)
                r=-r;
            Color3f envmap = scene->backgroundColor(r);
            float cos_term = std::max(0.f,r.dot(normal));
            Color3f BRDF = material->brdf(-ray.direction, r, normal, hit.texcoord());
            if(pdf>0)
                radiance += (envmap * BRDF * cos_term) / pdf;
        }

        return radiance / m_sampleCount;
    }

    std::string toString() const {
        return tfm::format("DirectMats[\n"
                           "  samples = %f\n"
                           "  IS = %s\n"
                           " ]\n",
                           m_sampleCount,
                           m_IS ? "true" : "false");
    }

private:
    int m_sampleCount;
    bool m_IS;
};

REGISTER_CLASS(DirectMats, "direct_mats")
