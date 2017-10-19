#include "lightProbe.h"

bool LightProbe::load(const std::string& filename)
{
    try {
        m_image = new Bitmap(filename);
    } catch (std::exception &e) {
        return false;
    }
    return true;
}

Color3f LightProbe::intensity(const Vector3f& dir) const
{
    return (*m_image)((acosf(dir[1]) / M_PI) * (m_image->rows()-1), (1.f + atan2f(dir[0],-dir[2]) / M_PI)/2.f * (m_image->cols()-1));
}

