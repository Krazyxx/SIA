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

Vector3f Material::is(Normal3f normal, Vector3f direction) const
{
    float pdf;
    Vector3f dir = us(normal, pdf);
    return dir / pdf;
}

Color3f Material::premultBrdf(const Vector3f& viewDir, const Vector3f& lightDir, const Normal3f& normal, const Vector2f& uv) const
{
    return brdf(viewDir, lightDir, normal, uv);
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


REGISTER_CLASS(Diffuse, "diffuse")
