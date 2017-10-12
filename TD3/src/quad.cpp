#include "quad.h"

static int tindices[2][3] = {
    {0,1,3}, {3,1,2}
};

Quad::Quad(const PropertyList &propList)
{
    m_transformation = propList.getTransform("toWorld", Transform());
    Vector3f s = propList.getVector("size",Vector3f(1,1,0));
    m_size = s.head<2>();
    m_pMesh = new Mesh;
    float vdata[4][3] = { {0.5f*m_size[0], 0.5f*m_size[1], 0.0}, {-0.5f*m_size[0], 0.5f*m_size[1], 0.0}, {-0.5f*m_size[0], -0.5f*m_size[1], 0.0}, {0.5f*m_size[0], -0.5f*m_size[1], 0.0} };
    Eigen::Matrix<float,3,4> vertices((float*)vdata);
    m_pMesh->loadRawData(vertices.data(), 4, (int*)tindices, 2);
    m_material = new Diffuse(Color3f(1));
}

Quad::~Quad()
{
    delete m_pMesh;
}

void Quad::drawGeometry() const
{
    glUniform1i(m_shader->uniform("twoSided"),1);
    m_pMesh->drawGeometry();
}

const Eigen::AlignedBox3f& Quad::AABB() const
{
    return m_pMesh->AABB();
}

bool Quad::intersect(const Ray& ray, Hit& hit) const
{
    Normal3f Pn(0.0,0.0,1.0);

    float Vd = Pn.dot(ray.direction);

    if(Vd >= -Epsilon && Vd <= Epsilon) // dot product close to zero => ray parallel to the plane
        return false;

    float V0 = Pn.dot(ray.origin);
    float t = -V0/Vd;

    if(t<Epsilon)
        return false;

    Point3f pos = ray.at(t);
    float u = pos.dot(Vector3f::UnitX());
    float v = pos.dot(Vector3f::UnitY());
    if(u<-m_size[0]*0.5f || v<-m_size[1]*0.5f || u>m_size[0]*0.5f || v>m_size[1]*0.5f)
        return false;

    hit.setT(t);
    hit.setNormal(Pn);
    hit.setTexcoord(Eigen::Vector2f(ray.at(t)[0],ray.at(t)[1]));

    return true;
}

REGISTER_CLASS(Quad, "quad")
