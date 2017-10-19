#include "material.h"
#include "warp.h"

Phong::Phong(const PropertyList &propList)
    : Diffuse(propList.getColor("diffuse",Color3f(0.2)))
{
    m_reflectivity = propList.getColor("reflectivity",Color3f(0.0));
    m_transmissivness = propList.getColor("transmissivness",Color3f(0.0));
    m_etaA = propList.getFloat("etaA",1);
    m_etaB = propList.getFloat("etaB",1);
    m_specularColor = propList.getColor("specular",Color3f(0.9));
    m_exponent = propList.getFloat("exponent",0.2);

    std::string texturePath = propList.getString("texture","");
    if(texturePath.size()>0){
        filesystem::path filepath = getFileResolver()->resolve(texturePath);
        loadTextureFromFile(filepath.str());
        setTextureScale(propList.getFloat("scale",1));
        setTextureMode(TextureMode(propList.getInteger("mode",0)));
    }

    float dAvg = (m_diffuseColor[0]+m_diffuseColor[1]+m_diffuseColor[2])/3.f,
          sAvg = (m_specularColor[0]+m_specularColor[1]+m_specularColor[2])/3.f;
    m_specularSamplingWeight = sAvg / (dAvg + sAvg);
}

Color3f Phong::brdf(const Vector3f& viewDir, const Vector3f& lightDir, const Normal3f& normal, const Vector2f& uv) const
{
        Vector3f reflected = -lightDir + 2.f*(lightDir.dot(normal))*normal;
        float alpha = viewDir.dot(reflected);

        Color3f color = diffuseColor(uv) * M_1_PI;

        if (alpha > 0.0f) {
            color +=  m_specularColor * ((m_exponent + 2.f) * std::pow(alpha, m_exponent) / (2.f* M_PI));
        }

        return color;
//    Vector3f h = (viewDir+dir).normalized();
//    return diffuseColor(uv) + m_specularColor * pow(std::max(0.f,normal.dot(h)), m_exponent);
}

Color3f Phong::premultBrdf(const Vector3f& viewDir, const Vector3f& lightDir, const Normal3f& normal, const Vector2f& uv) const
{
    Vector3f h = (viewDir+lightDir).normalized();
    Color3f specular = m_specularColor * (m_exponent+2.f)/(m_exponent+1.f) * normal.dot(h);
    return diffuseColor(uv) + specular;
}

//float Phong::pdf(const Normal3f& n, const Vector3f &i, const Vector3f &o) const
//{
//    Vector3f reflected = -i + 2.f*(i.dot(n))*n;
//    float specProb = 0.0f;
//    float alpha = o.dot(reflected);
//    if (alpha > 0)
//        specProb = powf(alpha, m_exponent) * (m_exponent + 1.0f) / (2.0f * M_PI);

//    float diffuseProb = M_1_PI * o.dot(n);
//    return m_specularSamplingWeight * specProb + (1-m_specularSamplingWeight) * diffuseProb;
//}

Vector3f Phong::is(const Normal3f& n, const Vector3f& i, float &pdf) const
{
    float u = Eigen::internal::random<float>(0,1);
    float v = Eigen::internal::random<float>(0,1);

    bool chooseSpecular = true;
    if(u<=m_specularSamplingWeight){
        u /= m_specularSamplingWeight;
    }else{
        u = (u - m_specularSamplingWeight)
                / (1.f - m_specularSamplingWeight);
        chooseSpecular = false;
    }

    float phi =  2. * M_PI * u;
    Vector3f X = n.unitOrthogonal();
    Vector3f Y = n.cross(X);

    Vector3f dir;

    if (chooseSpecular) {
        float z = powf(v,1/(m_exponent+1));
        float sin_theta = sqrtf(1-z*z);
        float x = std::cos(phi)*sin_theta;
        float y = std::sin(phi)*sin_theta;
        Vector3f o = (x*X+y*Y+z*n).normalized();
        Vector3f reflected = -i + 2.f*(i.dot(n))*n;

        float alpha = o.dot(reflected);
        pdf = powf(alpha, m_exponent) * (m_exponent + 1.0f) / (2.0f * M_PI);

        return o;
    }
    Vector3f d = Warp::squareToCosineHemisphere(Point2f(Eigen::internal::random<float>(0,1),Eigen::internal::random<float>(0,1)));
    pdf = Warp::squareToCosineHemispherePdf(d);
    return d.x() * X + d.y() * Y + d.z() * n;
}

std::string Phong::toString() const {
    return tfm::format(
        "Phong[\n"
        "  diffuse color = %s\n"
        "  specular color = %s\n"
        "  exponent = %f\n"
        "  reflectivity = %s\n"
        "  transmissivness = %s\n"
        "  etA = %f etaB = %f\n"
        "]", m_diffuseColor.toString(),
             m_specularColor.toString(),
             m_exponent,
             m_reflectivity.toString(),
             m_transmissivness.toString(),
             m_etaA, m_etaB);
}

REGISTER_CLASS(Phong, "phong")
