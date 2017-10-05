#include "material.h"
#include "warp.h"
#include <Eigen/Geometry>
#include <iostream>
#include <math.h>

void Material::loadTextureFromFile(const std::string& fileName)
{
    if (fileName.size()==0)
        std::cerr << "Material error : no texture file name provided" << std::endl;
    else
        m_texture = new Bitmap(fileName);
}

// Uniform Sampling
Vector3f Material::us(Normal3f normal, float& pdf) const
{
    Point2f p = Point2f(Eigen::internal::random<float>(0,1),
                        Eigen::internal::random<float>(0,1));
    Vector3f d = Warp::squareToCosineHemisphere(p);
    pdf = Warp::squareToCosineHemispherePdf(d);

    Vector3f u = normal.unitOrthogonal();
    Vector3f v = normal.cross(u);
    Vector3f r = d.x() * u + d.y() * v + d.z() * normal;

    return r;
}

Diffuse::Diffuse(const PropertyList &propList)
{
    m_diffuseColor = propList.getColor("diffuse",Color3f(0.2));

    std::string texturePath = propList.getString("texture","");
    if(texturePath.size()>0){
        filesystem::path filepath = getFileResolver()->resolve(texturePath);
        loadTextureFromFile(filepath.str());
        setTextureScale(propList.getFloat("scale",1));
        setTextureMode(TextureMode(propList.getInteger("mode",0)));
    }
}

Color3f Diffuse::diffuseColor(const Vector2f& uv) const
{
    if(texture() == nullptr)
        return m_diffuseColor;

    float u = uv[0];
    float v = uv[1];

    // Take texture scaling into account
    u /= textureScaleU();
    v /= textureScaleV();

    // Compute pixel coordinates
    const int i = int(fabs(u - floor(u)) * texture()->cols());
    const int j = int(fabs(v - floor(v)) * texture()->rows());

    Color3f fColor = (*texture())(j,i);

    // Compute color
    switch(textureMode())
    {
    case MODULATE:
        return  fColor * m_diffuseColor;
    case REPLACE:
        return fColor;
    }
    return fColor;
}

// Importance Sampling
Vector3f Ward::is(Normal3f normal, Vector3f direction) const
{
    float u = Eigen::internal::random<float>(0,1);
    float v = Eigen::internal::random<float>(0,1);

    float angle = 2 * M_PI * v;
    float phi_h = atan((this->m_alphaY/this->m_alphaX) * tan(angle));

    //int quadrant_phi = floor(phi_h / (M_PI/2));
    //int quadrant_dir = floor(angle / (M_PI/2));

    float squared_cos = (cos(phi_h) * cos(phi_h)) / (this->m_alphaX * this->m_alphaX);
    float squared_sin = (sin(phi_h) * sin(phi_h)) / (this->m_alphaY * this->m_alphaY);
    float theta_h = atan(sqrt(-log(u) / (squared_cos + squared_sin)));

    Vector3f h(sin(theta_h) * cos(phi_h), sin(theta_h) * sin(phi_h), cos(theta_h));

    Vector3f d(0,1,0);
    Vector3f x = (d - (d.dot(normal) * normal)).normalized();
    Vector3f y = x.cross(normal);
    h = h.x() * x + h.y() * y + h.z() * normal;

    Vector3f o = 2.0 * direction.dot(h) * h - direction;


    return o;
}


REGISTER_CLASS(Diffuse, "diffuse")
