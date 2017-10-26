#ifndef MESHLOADER_H
#define MESHLOADER_H

#include <Eigen/Core>

#include <string>
#include <vector>

// define an Exception for MeshLoader
class MeshLoaderException
{
public :
    MeshLoaderException (const std::string & msg) : message (msg) {}
    virtual  ~MeshLoaderException () {}
    const std::string getMessage () const { return "[MeshLoaderException] " + message; }

private:
    std::string message;
};


class MeshLoader
{
public:

    static void loadOFF (const std::string& filename,
                         std::vector<Eigen::Vector3f>& position,
                         std::vector<Eigen::Vector3i>& faceId,
                         std::vector<Eigen::Vector4f>& colors);

    static void loadObj ( const std::string& filename,
                          std::vector<Eigen::Vector3f>& position,
                          std::vector<Eigen::Vector3i>& faceId,
                          std::vector<Eigen::Vector3f>& normal,
                          std::vector<Eigen::Vector3i>& normalId,
                          std::vector<Eigen::Vector2f>& texcoord,
                          std::vector<Eigen::Vector3i>& texId );
};


#endif // MESHLOADER_H
