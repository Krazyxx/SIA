#include "meshloader.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdio>

using namespace std;
using namespace Eigen;

void MeshLoader::loadOFF(const std::string& filename,
                         vector<Vector3f>& position,
                         vector<Vector3i>& faceId,
                         vector<Vector4f>& colors)
{
    ifstream in(filename.c_str(),std::ios::in);
    if (!in)
        throw MeshLoaderException("loadOFF : cannot load file :" + filename);

    string header;
    in >> header;

    bool hasColor = false;

    // check the header file
    if(header != "OFF"){
        if(header != "COFF")
            throw MeshLoaderException("loadOFF : Wrong header :" + header);
        hasColor = true;
    }

    int nofVertices, nofFaces, inull;
    int nb, id0, id1, id2;
    Vector3f v;
    Vector4f color;

    in >> nofVertices >> nofFaces >> inull;

    for(int i=0 ; i<nofVertices ; ++i)
    {
        in >> v[0] >> v[1] >> v[2];
        if(hasColor){
            in >> color[0] >> color[1] >> color[2] >> color[3];
            colors.push_back(color/255.f);
        }
        position.push_back(v);
    }

    for(int i=0 ; i<nofFaces ; ++i)
    {
        in >> nb >> id0 >> id1 >> id2;
        assert(nb==3);
        faceId.push_back(Vector3i(id0,id1,id2));
    }

    in.close();
}

void MeshLoader::loadObj ( const string&  filename,
                           vector<Vector3f>& position,
                           vector<Vector3i>& faceId,
                           vector<Vector3f>& normal,
                           vector<Vector3i>& normalId,
                           vector<Vector2f>& texcoord,
                           vector<Vector3i>& texId )
{
    ifstream input(filename.c_str());

    if (!input)
        throw MeshLoaderException("loadObj : cannot load file :" + filename);

    char line[1024];
    while(!input.eof()) {

        if (! input.getline(line, 1024, '\n').good() )
            continue;

        istringstream line_input(line);
        string keyword;

        line_input >> keyword;

        if(keyword == "v") { // vertex position
            Vector3f p;
            line_input >> p[0] >> p[1] >> p[2];
            position.push_back(p);
        }
        else if(keyword == "vt") { // vertex texture coordinate
            Vector2f p;
            line_input >> p[0] >> p[1];
            texcoord.push_back(p);
        }
        else if(keyword == "vn") { // vertex normal
            Vector3f n;
            line_input >> n[0] >> n[1] >> n[2];
            normal.push_back(n);
        }
        else if(keyword == "f") { // face
            // vertex_id / vertex texcoord / vertex normal
            Vector3i fId;
            Vector3i tId;
            Vector3i nId;

            if ( sscanf(line,"f %d/%d/%d %d/%d/%d %d/%d/%d",
                        &fId[0],&tId[0],&nId[0],
                        &fId[1],&tId[1],&nId[1],
                        &fId[2],&tId[2],&nId[2]) == 9 ) {
                for ( int i = 0; i < 3; ++i ) {
                    fId[i] -= 1;
                    tId[i] -= 1;
                    nId[i] -= 1;
                }
                faceId.push_back(fId);
                texId.push_back(tId);
                normalId.push_back(nId);
            }
            else if ( sscanf(line,"f %d//%d %d//%d %d//%d",
                             &fId[0],&nId[0],
                             &fId[1],&nId[1],
                             &fId[2],&nId[2]) == 6 ) {
                for ( int i = 0; i < 3; ++i ) {
                    fId[i] -= 1;
                    nId[i] -= 1;
                }
                faceId.push_back(fId);
                normalId.push_back(nId);
            }
            else if ( sscanf(line,"f %d// %d// %d//",
                             &fId[0],
                             &fId[1],
                             &fId[2]) == 3 ) {
                for ( int i = 0; i < 3; ++i ) {
                    fId[i] -= 1;
                }
                faceId.push_back(fId);
            }
            else if ( sscanf(line,"f %d %d %d",
                             &fId[0],
                             &fId[1],
                             &fId[2]) == 3 ) {
                for ( int i = 0; i < 3; ++i ) {
                    fId[i] -= 1;
                }
                faceId.push_back(fId);
            }
            else { cerr << "error reading line -> \"" << line << "\"" << endl; }
        }
    }

    input.close();

}

