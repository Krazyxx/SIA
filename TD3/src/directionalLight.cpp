#include "light.h"
#include "glPrimitives.h"

class DirectionalLight : public Light
{
public:
    DirectionalLight(const PropertyList &propList)
        : Light(propList.getColor("intensity", Color3f(1.f)))
    {
        m_direction = propList.getVector("direction", Vector3f(1.f,0.f,0.f)).normalized();
    }

    Vector3f direction(const Point3f& /*x*/, float* dist = 0) const
    {
        if(dist)
            *dist = std::numeric_limits<float>::max();
        return -m_direction;
    }

    Color3f intensity(const Point3f& x) const
    {
        return m_intensity;
    }

    void draw()
    {
        if(m_shader){
            m_shader->bind();
            Eigen::Matrix4f M = Eigen::Matrix4f::Identity();
            glUniformMatrix4fv(m_shader->uniform("mat_obj"), 1, GL_FALSE, M.data());
            glUniform3fv(m_shader->uniform("color"), 1, (m_intensity/m_intensity.maxCoeff()).eval().data());
            Line::draw(m_shader, Point3f::Constant(1.f), Point3f::Constant(1.f)+m_direction*0.5f);
        }
    }

    std::string toString() const {
        return tfm::format(
            "DirectionalLight[\n"
            "  intensity = %s\n"
            "  direction = %s\n"
            "]", m_intensity.toString(),
                 ::toString(m_direction));
    }

protected:
    Vector3f m_direction;
};

REGISTER_CLASS(DirectionalLight, "directionalLight")
