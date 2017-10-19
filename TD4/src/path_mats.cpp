#include "integrator.h"
#include "scene.h"
#include "material.h"
#include "areaLight.h"

class PathMats : public Integrator
{
public:
    PathMats(const PropertyList &props) {
        m_maxRecursion = props.getInteger("maxRecursion",4);
        m_IS = props.getBoolean("IS", false);
    }

    Color3f Li(const Scene *scene, const Ray &ray) const {

        Color3f radiance = Color3f::Zero();

        // stopping criteria:
        if(ray.recursionLevel>=m_maxRecursion) {
            return radiance;
        }

        /* Find the surface that is visible in the requested direction */
        Hit hit;
        scene->intersect(ray, hit);
        if (hit.foundIntersection())
        {
            Normal3f normal = hit.normal();
            Point3f pos = ray.at(hit.t());

            const Material* material = hit.shape()->material();

            // For shapes emitting light (an area light is placed aside them).
            if (hit.shape()->isEmissive()) {
                const AreaLight *light = static_cast<const AreaLight*>(hit.shape()->light());
                return std::max(0.f, normal.dot(-ray.direction)) * light->intensity(pos + light->direction(), pos);
            }

            const LightList &lights = scene->lightList();
            for(LightList::const_iterator it=lights.begin(); it!=lights.end(); ++it)
            {
                Vector3f lightDir;
                float dist;
                const AreaLight* aLight = dynamic_cast<const AreaLight*>(*it);
                Point3f y;
                if(aLight) {
                    y = aLight->position()
                              + aLight->size()[0]*aLight->uVec() * Eigen::internal::random<float>(-0.5,0.5)
                              + aLight->size()[1]*aLight->vVec() * Eigen::internal::random<float>(-0.5,0.5);

                    dist = (pos-y).norm();
                    lightDir = (y-pos).normalized();
                }else{
                    lightDir = (*it)->direction(pos, &dist);
                }
                Ray shadow_ray(pos + normal*Epsilon, lightDir, true);
                Hit shadow_hit;
                scene->intersect(shadow_ray,shadow_hit);
                Color3f attenuation = Color3f(1.f);
                if(shadow_hit.t()<dist){
                    if(!shadow_hit.shape()->isEmissive())
                        attenuation = 0.5f * shadow_hit.shape()->material()->transmissivness();
                    if((attenuation <= Epsilon).all())
                        continue;
                }

                float cos_term = std::max(0.f,lightDir.dot(normal));
                Color3f brdf = material->brdf(-ray.direction, lightDir, normal, hit.texcoord());
                if(aLight)
                    radiance += aLight->intensity(pos,y) * cos_term * brdf * attenuation;
                else
                    radiance += (*it)->intensity(pos) * cos_term * brdf * attenuation;
            }

            Color3f color = material->ambientColor();
            float albedo = (color.r() + color.g() + color.b()) / 3.f;
            if (albedo  > Eigen::internal::random<float>(0, 1) || ray.recursionLevel <= 3) {
                Color3f indirect = Color3f::Zero();
                float pdf = 0.f;
                Vector3f r;
                if(m_IS) {
                    r = material->is(normal, -ray.direction, pdf);
                }else{
                    r = material->us(normal, -ray.direction, pdf);
                }
                if(normal.dot(r)<0)
                    r=-r;
                Ray reflexion_ray(pos + hit.normal()*Epsilon, r);
                reflexion_ray.recursionLevel = ray.recursionLevel + 1;
                Color3f brdf = material->brdf(-ray.direction, r, normal, hit.texcoord());
                float cos_term = std::max(0.f,r.dot(normal));
                indirect = (Li(scene, reflexion_ray) * cos_term * brdf);
                if(pdf>Epsilon)
                    indirect /= pdf;

                if(albedo > Epsilon && ray.recursionLevel > 3)
                        indirect /= albedo;
                radiance += indirect;
            }

            // reflexions
            if((material->reflectivity() > 1e-6).any())
            {
                Vector3f r = (ray.direction - 2.*ray.direction.dot(hit.normal())*hit.normal()).normalized();
                Ray reflexion_ray(pos + hit.normal()*Epsilon, r);
                reflexion_ray.recursionLevel = ray.recursionLevel + 1;
                float cos_term = std::max(0.f,r.dot(normal));
                radiance += material->reflectivity() * Li(scene, reflexion_ray) * cos_term;
            }

            // refraction
            if((material->transmissivness() > 1e-6).any())
            {
                float etaA = material->etaA(), etaB = material->etaB();
                bool entering = -normal.dot(ray.direction) > 0;
                if(!entering) {
                    std::swap(etaA,etaB);
                    normal = -normal;
                }
                Vector3f r;
                if(refract(normal,ray.direction,etaA,etaB,r)) {
                    Ray refraction_ray(pos - normal*Epsilon, r);
                    refraction_ray.recursionLevel = ray.recursionLevel + 1;
                    float cos_term = std::max(0.f,-r.dot(normal));
                    radiance += material->transmissivness() * Li(scene, refraction_ray) * cos_term;
                }
            }

        }else if(ray.recursionLevel == 0)
            return scene->backgroundColor();

        return radiance;
    }

    std::string toString() const {
        return tfm::format("Whitted[\n"
                           "  max recursion = %f\n"
                           " ]\n",
                           m_maxRecursion);
    }
private:
    int m_maxRecursion;
    bool m_IS;
};

REGISTER_CLASS(PathMats, "path_mats")
