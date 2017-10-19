#include "material.h"
#include "warp.h"

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

    float dAvg = (m_diffuseColor[0]+m_diffuseColor[1]+m_diffuseColor[2])/3.f,
          sAvg = (m_specularColor[0]+m_specularColor[1]+m_specularColor[2])/3.f;
    m_specularSamplingWeight = sAvg / (dAvg + sAvg);
}

Color3f Ward::brdf(const Vector3f& viewDir, const Vector3f& dir, const Normal3f& normal, const Vector2f& uv) const
{
    Vector3f x = Vector3f(0,1,0) - Vector3f(0,1,0).dot(normal)*normal;
    x.normalize();
    Vector3f y = normal.cross(x);

    Vector3f h = (viewDir+dir).normalized();
    float denom = h.dot(normal);
    float hx = h.dot(x)/m_alphaX;
    float hy = h.dot(y)/m_alphaY;
    float coef = (4.f * M_PI * m_alphaX * m_alphaY * sqrtf(viewDir.dot(normal)*dir.dot(normal))) ;
    float exponent = -(hx*hx+hy*hy)/(denom*denom);
    Color3f specular = m_specularColor *  expf(exponent) / coef;
    Color3f diffuse = m_diffuseColor * M_1_PI;
    if(coef > 1e-8f)
        return diffuse + specular;
    return diffuse;
}

Color3f Ward::premultBrdf(const Vector3f& viewDir, const Vector3f& dir, const Normal3f& normal, const Vector2f& uv) const
{
    Color3f specular(0.f,0.f,0.f);

    Vector3f h = (viewDir+dir).normalized();
    float hn = h.dot(normal);
    float on = dir.dot(normal);
    float in = viewDir.dot(normal);
    if(in > Epsilon){
        float quotient = on/in;
        if(quotient>0)
            specular = m_specularColor *  h.dot(viewDir) * hn*hn*hn * sqrtf(quotient);
    }
    Color3f diffuse = m_diffuseColor;
    return diffuse +specular;
}

//float Ward::pdf(const Normal3f &n, const Vector3f &i, const Vector3f &o) const
//{
//    Vector3f h = (i+o).normalized();
//    Vector3f X = n.unitOrthogonal();
//    Vector3f Y = n.cross(X);
//    float cos_thetaH = h.dot(n);
//    float factor1 = h.dot(X) / m_alphaX;
//    float factor2 = h.dot(Y) / m_alphaY;

//    float exponent = -(factor1*factor1+factor2*factor2)/(cos_thetaH*cos_thetaH);
//    float coef = (4.f * M_PI * m_alphaX * m_alphaY * h.dot(i) * cos_thetaH*cos_thetaH*cos_thetaH);
//    float specProb = 0.f;
//    if(coef > Epsilon)
//        specProb = 1.f/coef * expf(exponent);

//    float diffuseProb = M_1_PI * o.dot(n);

//    return m_specularSamplingWeight * specProb + (1-m_specularSamplingWeight) * diffuseProb;
//}

Vector3f Ward::is(const Normal3f& n, const Vector3f& i, float& pdf) const
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

    Vector3f x = Vector3f(0,1,0) - Vector3f(0,1,0).dot(n)*n;
    x.normalize();
    Vector3f y = n.cross(x);
    Vector3f dir;

    if (chooseSpecular) {
        float phiH = atanf(m_alphaY/m_alphaX * tanf(2.f * M_PI * v));
        // if(v > 0.5f)
        //     phiH += M_PI;
        // shift phiH from [-PI/2, PI/2] to [-PI, PI]
        if((v > 0.25) && (v <= 0.5)) // II
            phiH += M_PI;
        else if((v > 0.5) && (v < 0.75)) // III
            phiH -= M_PI;

        float cos_phiH = cosf(phiH);
        float cos_phiH_exp =  cos_phiH / m_alphaX ;
        float sin_phiH = sinf(phiH);
        float sin_phiH_exp = sin_phiH / m_alphaY;
        float thetaH = atanf( sqrtf(std::max(0.0f,-logf(u) / ((cos_phiH_exp*cos_phiH_exp) + (sin_phiH_exp*sin_phiH_exp)))));

        float cos_thetaH = cosf(thetaH);
        float sin_thetaH = sinf(thetaH);
        Vector3f h = sin_thetaH*cos_phiH*x + sin_thetaH*sin_phiH*y + cos_thetaH*n;
        h.normalize();

        dir = h * (2.f * i.dot(h)) - i;

        // PDF
        float factor1 = sin_thetaH * cos_phiH / m_alphaX;
        float factor2 = sin_thetaH * sin_phiH / m_alphaY;
        float exponent = -(factor1*factor1 + factor2*factor2)/(cos_thetaH*cos_thetaH);
        float coef = (4.f * M_PI * m_alphaX * m_alphaY * h.dot(i) * cos_thetaH*cos_thetaH*cos_thetaH);
        float pdf = 0.f;
        if(coef > Epsilon)
            pdf = 1.f/coef * expf(exponent);

        return dir;
    }

    Vector3f d = Warp::squareToCosineHemisphere(Point2f(u,v));
    pdf = Warp::squareToCosineHemispherePdf(d);
    return d.x() * x + d.y() * y + d.z() * n;

}

std::string Ward::toString() const {
    return tfm::format(
        "Ward[\n"
        "  diffuse color = %s\n"
        "  specular color = %s\n"
        "  alphaX = %f  alphaY = %f\n"
        "]", m_diffuseColor.toString(),
             m_specularColor.toString(),
             m_alphaX, m_alphaY);
}

REGISTER_CLASS(Ward, "ward")
