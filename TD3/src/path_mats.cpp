#include "integrator.h"
#include "scene.h"
#include "material.h"
#include "areaLight.h"

class PathMats : public Integrator
{
public:
    PathMats(const PropertyList &props) {
        m_maxRecursion = props.getInteger("maxRecursion",4);
    }

    Color3f Li(const Scene *scene, const Ray &ray) const {
        Color3f radiance = Color3f::Zero();

        if (ray.recursionLevel >= m_maxRecursion) {
            return radiance;
        }

        Hit hit;
        scene->intersect(ray, hit);

        if (!hit.foundIntersection()) {
            if (ray.recursionLevel == 0) {
                return scene->backgroundColor();
            }
            return radiance;
        }

        // If there is an intersection.
        Normal3f normal = hit.normal();
        Point3f pos = ray.at(hit.t());
        const Material *material = hit.shape()->material();

        const LightList &lights = scene->lightList();
        for (LightList::const_iterator it = lights.begin(); it != lights.end(); it++) {
            float dist;
            Vector3f lightDir;
            const AreaLight* aLight = dynamic_cast<const AreaLight*>(*it);
            Point3f y;
            if (aLight) {
                float u = Eigen::internal::random<float>(0,1);
                float v = Eigen::internal::random<float>(0,1);
                Point2f p = Point2f(u,v); // Permet de se positionner sur l'areaLight

                y = aLight->position()
                            + p.x() * aLight->size()[0] * aLight->uVec()
                            + p.y() * aLight->size()[1] * aLight->vVec();
                dist = (pos-y).norm();
                lightDir = (y-pos).normalized();
            } else {
                lightDir = (*it)->direction(pos, &dist);
            }

            Ray shadow_ray(pos+normal*Epsilon, lightDir, true);
            Hit shadow_hit;
            scene->intersect(shadow_ray, shadow_hit);
            if (shadow_hit.t() < dist) {
                continue;
            }

            float cos_term = std::max(0.f, lightDir.dot(normal));
            Color3f brdf = material->brdf(-ray.direction, lightDir, normal, hit.texcoord());
            if(aLight) {
                radiance += aLight->intensity(pos, y) * cos_term * brdf;
            } else {
                radiance += (*it)->intensity(pos) * cos_term * brdf;
            }
        }

        // Indirect Ray
        Vector3f dir_indirect = material->is(normal, ray.direction);
        Color3f brdf = material->premultBrdf(-ray.direction, dir_indirect, normal, hit.texcoord());
        Ray indirect_ray(pos + hit.normal() * Epsilon, dir_indirect);
        indirect_ray.recursionLevel = ray.recursionLevel + 1;
        radiance += Li(scene, indirect_ray) * brdf;
    }

    std::string toString() const {
        return tfm::format("Whitted[\n"
                           "  max recursion = %f\n"
                           " ]\n",
                           m_maxRecursion);
    }
private:
    int m_maxRecursion;
};

REGISTER_CLASS(PathMats, "path_mats")
