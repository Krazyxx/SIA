#include "material.h"

#include <Eigen/Geometry>
#include <iostream>
#include "warp.h"

void Material::loadTextureFromFile(const std::string& fileName)
{
    if (fileName.size()==0)
        std::cerr << "Material error : no texture file name provided" << std::endl;
    else
        m_texture = new Bitmap(fileName);
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

Vector3f Material::us(const Normal3f& n, const Vector3f &i, float& pdf) const
{
    Vector3f u = n.unitOrthogonal();
    Vector3f v = n.cross(u);
    Vector3f d = Warp::squareToCosineHemisphere(Point2f(Eigen::internal::random<float>(0,1),Eigen::internal::random<float>(0,1)));
    pdf = Warp::squareToCosineHemispherePdf(d);
    return d.x() * u + d.y() * v + d.z() * n;
}


Vector3f Diffuse::is(const Normal3f& n, const Vector3f &i, float& pdf) const
{
    return Material::us(n,i,pdf);
}

REGISTER_CLASS(Diffuse, "diffuse")
