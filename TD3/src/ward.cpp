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


Color3f Ward::premultBrdf(const Vector3f& viewDir, const Vector3f& lightDir, const Normal3f& normal, const Vector2f& uv) const
{
    if (lightDir.dot(normal) * viewDir.dot(normal) <= 0) { return Color3f(0,0,0); }

    Vector3f h = (viewDir + lightDir).normalized();
    return m_specularColor * h.dot(lightDir) * h.dot(normal) * h.dot(normal) * h.dot(normal) * sqrt(viewDir.dot(normal) / lightDir.dot(normal));
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
