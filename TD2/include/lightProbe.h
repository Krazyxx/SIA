#ifndef LIGHTPROBE_H
#define LIGHTPROBE_H

#include "bitmap.h"
#include "vector.h"

class LightProbe
{
public:
    bool load(const std::string& filename);

    Color3f intensity(const Vector3f& dir) const;

private:
    Bitmap* m_image;
};

#endif // LIGHTPROBE_H
