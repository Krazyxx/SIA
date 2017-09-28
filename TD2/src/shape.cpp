#include "shape.h"

void Shape::draw() const {
    if(m_shader)
    {
        m_shader->bind();
        glUniformMatrix4fv(m_shader->uniform("mat_obj"),  1, GL_FALSE, m_transformation.getMatrix().data());
        glUniformMatrix3fv(m_shader->uniform("mat_normal"),  1, GL_FALSE, Eigen::Matrix3f(m_transformation.getMatrix().topLeftCorner<3,3>().inverse().transpose()).data());
        glUniform3fv(m_shader->uniform("ambient_color"), 1, m_material->ambientColor().data());
        glUniform1i(m_shader->uniform("twoSided"),0);

        drawGeometry();
    }
}

void Shape::setParent(Object *parent)
{
    if(parent->getClassType() == ELight)
        m_light = static_cast<Light *>(parent);
}

void Shape::addChild(Object *obj) {
    switch (obj->getClassType()) {
        case EMaterial:
            if (m_material)
                throw RTException(
                    "Shape: tried to register multiple material instances!");
            m_material = static_cast<Material *>(obj);
            break;
        default:
            throw RTException("Shape::addChild(<%s>) is not supported!",
                                classTypeName(obj->getClassType()));
    }
}

REGISTER_CLASS(Shape, "shape")
