#ifndef MATERIAL_H
#define MATERIAL_H

#include "bitmap.h"
#include "object.h"

class Material : public Object
{
public:
    Material() : m_texture(nullptr) {}

    virtual Color3f ambientColor() const = 0;
    virtual Color3f diffuseColor(const Vector2f& uv) const = 0;
    virtual Color3f reflectivity() const { return m_reflectivity; }
    virtual Color3f transmissivness() const { return m_transmissivness; }
    virtual float etaA() const { return m_etaA; }
    virtual float etaB() const { return m_etaB; }

    /// evaluate the BRDF
    virtual Color3f brdf(const Vector3f& viewDir, const Vector3f& lightDir, const Normal3f& normal, const Vector2f& uv) const = 0;
    virtual Color3f premultBrdf(const Vector3f& viewDir, const Vector3f& lightDir, const Normal3f& normal, const Vector2f& uv) const = 0;

    Vector3f us(const Normal3f& n, const Vector3f &i, float& pdf) const;
    virtual Vector3f is(const Normal3f& n, const Vector3f &i, float& pdf) const = 0;
    virtual float specularSamplingWeight() const = 0;

    /// texture
    enum TextureMode { MODULATE, REPLACE };

    float textureScaleU() const { return m_textureScaleU; }
    float textureScaleV() const { return m_textureScaleV; }
    TextureMode textureMode() const { return m_textureMode; }
    const Bitmap* texture() const { return m_texture; }

    void setTexture(Bitmap* texture) { m_texture = texture; }
    void loadTextureFromFile(const std::string& fileName);
    void setTextureScale(float textureScale) { setTextureScaleU(textureScale); setTextureScaleV(textureScale); }
    void setTextureScaleU(float textureScaleU) { if (fabs(textureScaleU) > 1e-3) m_textureScaleU = textureScaleU; }
    void setTextureScaleV(float textureScaleV) { if (fabs(textureScaleV) > 1e-3) m_textureScaleV = textureScaleV; }
    void setTextureMode(TextureMode textureMode) { m_textureMode = textureMode; }

    EClassType getClassType() const { return EMaterial; }

protected:
    TextureMode m_textureMode;
    Bitmap* m_texture;
    float m_textureScaleU, m_textureScaleV;
    Color3f m_reflectivity;
    Color3f m_transmissivness;
    float m_etaA, m_etaB;
};

class Diffuse : public Material
{
public:
   Diffuse(const Color3f& diffuseColor) : m_diffuseColor(diffuseColor) {}
   Diffuse(const PropertyList &propList);

   Color3f ambientColor() const { return m_diffuseColor; }
   Color3f diffuseColor(const Vector2f& uv) const;

   Color3f brdf(const Vector3f& viewDir, const Vector3f& lightDir, const Normal3f& normal, const Vector2f& uv) const { return diffuseColor(uv) * INV_PI; }
   Color3f premultBrdf(const Vector3f& viewDir, const Vector3f& lightDir, const Normal3f& normal, const Vector2f& uv) const { return diffuseColor(uv); }

   Vector3f is(const Normal3f& n, const Vector3f &i, float& pdf) const;
   float specularSamplingWeight() const { return 0.f; }

   std::string toString() const {
       return tfm::format(
           "Diffuse[\n"
           "  color = %s\n"
           "]", m_diffuseColor.toString());
   }

protected:
    Color3f m_diffuseColor;
};


class Phong : public Diffuse
{
public:
    Phong(const PropertyList &propList);

    Color3f brdf(const Vector3f& viewDir, const Vector3f& lightDir, const Normal3f& normal, const Vector2f& uv) const;
    Color3f premultBrdf(const Vector3f& viewDir, const Vector3f& lightDir, const Normal3f& normal, const Vector2f& uv) const;

    Vector3f is(const Normal3f& n, const Vector3f &i, float& pdf) const;
    float specularSamplingWeight() const { return m_specularSamplingWeight; }

    std::string toString() const;

protected:
    Color3f m_specularColor;
    float m_exponent;
    float m_specularSamplingWeight;
};

class Ward : public Diffuse
{
public:
    Ward(const PropertyList &propList);

    Color3f brdf(const Vector3f& viewDir, const Vector3f& lightDir, const Normal3f& normal, const Vector2f& uv) const;
    Color3f premultBrdf(const Vector3f& viewDir, const Vector3f& lightDir, const Normal3f& normal, const Vector2f& uv) const;

    Vector3f is(const Normal3f& n, const Vector3f &i, float& pdf) const;
    float specularSamplingWeight() const { return m_specularSamplingWeight; }

    std::string toString() const;

protected:
    Color3f m_specularColor;
    float m_alphaX;
    float m_alphaY;
    float m_specularSamplingWeight;
};

#endif // MATERIAL_H
