#include "plane.h"

//--------------------------------------------------------------------------------
#define X 2.0

static float vdata[4][3] = {
    {X, X, 0.0}, {-X, X, 0.0}, {-X, -X, 0.0}, {X, -X, 0.0}
};

static int tindices[2][3] = {
    {0,1,3}, {3,1,2}
};
//--------------------------------------------------------------------------------

Plane::Plane()
{
    m_pMesh = new Mesh;
    Eigen::Matrix<float,3,4> vertices((float*)vdata);
    m_pMesh->loadRawData(vertices.data(), 4, (int*)tindices, 2);
}

Plane::Plane(const PropertyList &propList)
{
    m_transformation = propList.getTransform("toWorld", Transform());
    m_pMesh = new Mesh;
    Eigen::Matrix<float,3,4> vertices((float*)vdata);
    m_pMesh->loadRawData(vertices.data(), 4, (int*)tindices, 2);
}

Plane::~Plane()
{
    delete m_pMesh;
}

void Plane::drawGeometry() const
{
    glUniform1i(m_shader->uniform("twoSided"),1);
    m_pMesh->drawGeometry();
}

const Eigen::AlignedBox3f& Plane::AABB() const
{
    return m_pMesh->AABB();
}

bool Plane::intersect(const Ray& ray, Hit& hit) const
{
    Normal3f Pn(0.0,0.0,1.0);

    float Vd = Pn.dot(ray.direction);

    if(Vd >= -Epsilon && Vd <= Epsilon) // dot product close to zero => ray parallel to the plane
        return false;

    float V0 = Pn.dot(ray.origin);
    float t = -V0/Vd;

    if(t<Epsilon)
        return false;

    hit.setT(t);
    hit.setNormal(Pn);
    hit.setTexcoord(Eigen::Vector2f(ray.at(t)[0],ray.at(t)[1]));

    return true;
}

REGISTER_CLASS(Plane, "plane")
