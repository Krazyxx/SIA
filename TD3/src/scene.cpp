#include "scene.h"
#include "mesh.h"
#include "sphere.h"
#include "plane.h"

#include <Eigen/Geometry>
#include <filesystem/resolver.h>

Scene::Scene(const PropertyList& props) {
    m_backgroundColor = props.getColor("background", Color3f(0.6));
    std::string env_filename = props.getString("envmap","");
    if(env_filename.size()>0) {
        filesystem::path filepath = getFileResolver()->resolve(env_filename);
        m_env = new LightProbe();
        if(!m_env->load(filepath.str())) {
            delete m_env;
            m_env = NULL;
        }
    }
}

void Scene::draw() const
{
    for(uint i=0; i<m_shapeList.size(); ++i)
    {
        m_shapeList[i]->draw();
    }

    for(uint i=0; i<m_lightList.size(); ++i)
    {
        m_lightList[i]->draw();
    }
}

void Scene::clear()
{
    m_shapeList.clear();
    m_lightList.clear();
    if(m_camera)
        delete m_camera;
    m_camera = nullptr;
    if(m_integrator)
        delete m_integrator;
    m_integrator = nullptr;
    if(m_env)
        delete m_env;
    m_env = nullptr;
}

void Scene::attachShaderToShapes(nanogui::GLShader* prg)
{
    for(uint i=0; i<m_shapeList.size(); ++i)
    {
        m_shapeList[i]->attachShader(prg);
    }
}

void Scene::attachShaderToLights(nanogui::GLShader* prg)
{
    for(uint i=0; i<m_lightList.size(); ++i)
    {
        m_lightList[i]->attachShader(prg);
    }
}


/** Search for the nearest intersection between the ray and the object list */
void Scene::intersect(const Ray& ray, Hit& hit) const
{
    for(uint i=0; i<m_shapeList.size(); ++i)
    {
        // apply transformation
        Transform invTransform = m_shapeList[i]->transformation().inverse();
        Ray local_ray = invTransform * ray;
        float old_t = hit.t();
        if(hit.foundIntersection())
        {
            // If previous intersection found, transform intersection point
            Point3f x = ray.at(hit.t());
            hit.setT( (invTransform * x - local_ray.origin).norm() );
//            if(ray.shadowRay)
//                return;
        }
        Hit h;
        h.setT(hit.t());
        m_shapeList[i]->intersect(local_ray, h);

        if(h.t()<hit.t())
        {
            // we found a new closest intersection point for this object, record it:
            hit.setShape(m_shapeList[i]);
            Point3f x = local_ray.at(h.t());
            hit.setNormal( (m_shapeList[i]->transformation() * h.normal()).normalized() );
            hit.setT( (m_shapeList[i]->transformation() * x - ray.origin).norm() );
            hit.setTexcoord(h.texcoord());
        }else{
            hit.setT(old_t);
        }
    }
}


void Scene::addChild(Object *obj) {
    switch (obj->getClassType()) {
        case EShape: {
                Shape *shape = static_cast<Shape *>(obj);
                m_shapeList.push_back(shape);
            }
            break;

        case ELight: {
                Light *light = static_cast<Light *>(obj);
                m_lightList.push_back(light);
            }
            break;

        case ECamera:
            if (m_camera)
                throw RTException("There can only be one camera per scene!");
            m_camera = static_cast<Camera *>(obj);
            break;

        case EIntegrator:
            if (m_integrator)
                throw RTException("There can only be one integrator per scene!");
            m_integrator = static_cast<Integrator *>(obj);
            break;

        default:
            throw RTException("Scene::addChild(<%s>) is not supported!",
                classTypeName(obj->getClassType()));
    }
}

std::string Scene::toString() const {
    std::string shapes;
    for (size_t i=0; i<m_shapeList.size(); ++i) {
        shapes += std::string("  ") + indent(m_shapeList[i]->toString(), 2);
        if (i + 1 < m_shapeList.size())
            shapes += ",";
        shapes += "\n";
    }
    std::string lights;
    for (size_t i=0; i<m_lightList.size(); ++i) {
        lights += std::string("  ") + indent(m_lightList[i]->toString(), 2);
        if (i + 1 < m_lightList.size())
            lights += ",";
        lights += "\n";
    }

    return tfm::format(
        "Scene[\n"
        "  background = %s,\n"
        "  integrator = %s,\n"
        "  camera = %s,\n"
        "  shapes = {\n"
        "  %s  }\n"
        "  lights = {\n"
        "  %s  }\n"
        "]",
        m_env ? "envmap" : m_backgroundColor.toString(),
        indent(m_integrator->toString()),
        indent(m_camera->toString()),
        indent(shapes, 2),
        indent(lights, 2)
    );
}

REGISTER_CLASS(Scene, "scene")
