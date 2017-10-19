#include "integrator.h"
#include "scene.h"
#include "material.h"
#include "areaLight.h"

class DirectLMS : public Integrator
{
public:
    DirectLMS(const PropertyList &props) {
        m_sampleCount = props.getInteger("samples",4);
        m_stratified = props.getBoolean("stratified", false);
    }

    Color3f Li(const Scene *scene, const Ray &ray) const {
        /* Find the surface that is visible in the requested direction */
        Hit hit;
        scene->intersect(ray, hit);
        if (!hit.foundIntersection())
            return scene->backgroundColor();

        Color3f radiance = Color3f::Zero();
        const Material* material = hit.shape()->material();

        Point3f pos = ray.at(hit.t());
        Normal3f normal = hit.normal();

        if(hit.shape()->isEmissive()){
            const AreaLight* l = static_cast<const AreaLight*>(hit.shape()->light());
            Color3f radiance = std::max(0.f,normal.dot(-ray.direction)) * l->intensity(pos+l->direction(),pos);
            return radiance;
        }

        const LightList &lights = scene->lightList();
        for(LightList::const_iterator it=lights.begin(); it!=lights.end(); ++it)
        {
            const AreaLight* light = dynamic_cast<const AreaLight*>(*it);
            if(light)
            {
                for(int u=0; u<m_sampleCount; ++u)
                {
                    for(int v=0; v<m_sampleCount; ++v)
                    {
                        Point3f y = light->position() - 0.5f*light->size()[0]*light->uVec() - 0.5f*light->size()[1]*light->vVec();
                        if(!m_stratified)
                                y = y + light->size()[0]*light->uVec()*((float(u+0.5f)/float(m_sampleCount)))
                                      + light->size()[1]*light->vVec()*((float(v+0.5f)/float(m_sampleCount)));
                        else
                               y = y + light->size()[0]*light->uVec()*((u + Eigen::internal::random<float>(0,1))/float(m_sampleCount))
                                     + light->size()[1]*light->vVec()*((v + Eigen::internal::random<float>(0,1))/float(m_sampleCount));

                        float dist = (pos-y).norm();
                        Vector3f lightDir = (y-pos).normalized();
                        Ray shadowRay(pos + normal*Epsilon, lightDir);
                        Hit shadow_hit;
                        scene->intersect(shadowRay,shadow_hit);
                        Color3f attenuation = Color3f(1.f);
                        if(shadow_hit.t()<dist){
                            if(!shadow_hit.shape()->isEmissive())
                                attenuation = 0.5f * shadow_hit.shape()->material()->transmissivness();
                            if((attenuation <= 1e-6).all())
                                continue;
                        }

                        float cos_term = std::max(0.f,lightDir.dot(hit.normal()));
                        Color3f brdf = material->brdf(-ray.direction, lightDir, normal ,hit.texcoord());
                        radiance += (1.f / float(m_sampleCount*m_sampleCount)) * light->intensity(pos,y) * cos_term * brdf;
                    }
                }
            }else{
                float dist;
                Vector3f lightDir = (*it)->direction(pos, &dist);
                Ray shadowRay(pos + normal*Epsilon, lightDir);
                Hit shadowHit;
                scene->intersect(shadowRay,shadowHit);
                if(shadowHit.t()<dist)
                    continue;

                float cos_term = std::max(0.f,lightDir.dot(normal));
                Color3f brdf = material->brdf(-ray.direction, lightDir, normal, hit.texcoord());
                radiance += (*it)->intensity(pos) * cos_term * brdf;
            }
        }

        return radiance;
    }

    std::string toString() const {
        return tfm::format("DirectLMS[\n"
                           "  samples = %f\n"
                           "  startified = %s\n"
                           " ]\n",
                           m_sampleCount,
                           m_stratified ? "true" : "false");
    }

private:
    int m_sampleCount;
    bool m_stratified;
};

REGISTER_CLASS(DirectLMS, "direct_lms");
