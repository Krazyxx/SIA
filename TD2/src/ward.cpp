#include "material.h"

Ward::Ward(const PropertyList &propList)
    : Diffuse(propList.getColor("diffuse",Color3f(0.2)))
{
    m_reflectivity = propList.getColor("reflectivity",Color3f(0.0));
    m_transmissivness = propList.getColor("transmissivness",Color3f(0.0));
    m_etaA = propList.getFloat("etaA",1);
    m_etaB = propList.getFloat("etaB",1);
    m_specularColor = propList.getColor("specular",Color3f(0.9));
    m_alphaX = propList.getFloat("alphaX",0.2);
    m_alphaY = propList.getFloat("alphaY",0.2);

    std::string texturePath = propList.getString("texture","");
    if(texturePath.size()>0){
        filesystem::path filepath = getFileResolver()->resolve(texturePath);
        loadTextureFromFile(filepath.str());
        setTextureScale(propList.getFloat("scale",1));
        setTextureMode(TextureMode(propList.getInteger("mode",0)));
    }
}

Color3f Ward::brdf(const Vector3f& viewDir, const Vector3f& lightDir, const Normal3f& normal, const Vector2f& uv) const
{
    // brdf(i,o) = rho_d / M_PI + Fr(i,o)

    float sqrtX = lightDir.dot(normal) * viewDir.dot(normal);
    if (sqrtX <= 0) { return Color3f(0,0,0); }

    Vector3f d(0,1,0);
    Vector3f x = (d - (d.dot(normal) * normal)).normalized();
    Vector3f y = x.cross(normal);

    Vector3f h = (viewDir + lightDir) / (viewDir + lightDir).norm();
    float squareHX = (h.dot(x) / m_alphaX) * (h.dot(x) / m_alphaX);
    float squareHY = (h.dot(y) / m_alphaY) * (h.dot(y) / m_alphaY);
    float squareHN = h.dot(normal) * h.dot(normal);

    float expo = - ((squareHX + squareHY) / squareHN);
    float deno = 4 * M_PI * m_alphaX * m_alphaY * sqrt(sqrtX);
    Color3f Fr = (m_specularColor / deno) * exp(expo);

    return diffuseColor(uv) / M_PI + Fr;
}

std::string Ward::toString() const {
    return tfm::format(
        "Ward [\n"
        "  diffuse color = %s\n"
        "  specular color = %s\n"
        "  alphaX = %f  alphaY = %f\n"
        "]", m_diffuseColor.toString(),
             m_specularColor.toString(),
             m_alphaX, m_alphaY);
}

REGISTER_CLASS(Ward, "ward")
