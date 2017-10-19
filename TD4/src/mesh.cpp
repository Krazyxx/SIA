// Copyright (C) 2008-2011 Gael Guennebaud <gael.guennebaud@inria.fr>

#include "mesh.h"
#include "bvh.h"

#include <iostream>
#include <fstream>
#include <limits>
#include <filesystem/resolver.h>
#include <tiny_obj_loader.h>
#include <lib3ds.h>

Mesh::Mesh(const PropertyList &propList)
    : m_isInitialized(false), m_BVH(nullptr)
{
    m_transformation = propList.getTransform("toWorld", ::Transform());
    std::string filename = propList.getString("filename");
    loadFromFile(filename);
    buildBVH();
}

void Mesh::loadFromFile(const std::string& filename)
{
    filesystem::path filepath = getFileResolver()->resolve(filename);
    std::ifstream is(filepath.str());
    if (is.fail())
        throw RTException("Unable to open mesh file \"%s\"!", filepath.str());

    const std::string ext = filepath.extension();
    if(ext=="off" || ext=="OFF")
        loadOFF(filepath.str());
    else if(ext=="obj" || ext=="OBJ")
        loadOBJ(filepath.str());
    else if(ext=="3ds" || ext=="3DS")
        load3DS(filepath.str());
    else
        std::cerr << "Mesh: extension \'" << ext << "\' not supported." << std::endl;
}

void Mesh::loadOFF(const std::string& filename)
{
    std::ifstream in(filename.c_str(),std::ios::in);
    if(!in)
    {
        std::cerr << "File not found " << filename << std::endl;
        return;
    }

    std::string header;
    in >> header;

    // check the header file
    if(header != "OFF")
    {
        std::cerr << "Wrong header = " << header << std::endl;
        return;
    }

    int nofVertices, nofFaces, inull;
    int nb, id0, id1, id2;
    Vector3f v;

    in >> nofVertices >> nofFaces >> inull;

    for(int i=0 ; i<nofVertices ; ++i)
    {
        in >> v.x() >> v.y() >> v.z();
        m_vertices.push_back(Vertex(v));
    }

    for(int i=0 ; i<nofFaces ; ++i)
    {
        in >> nb >> id0 >> id1 >> id2;
        assert(nb==3);
        m_faces.push_back(FaceIndex(id0, id1, id2));
    }

    in.close();

    computeNormals();
    computeAABB();
}

void Mesh::loadOBJ(const std::string& filename)
{
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string err;
    bool success = tinyobj::LoadObj(&attrib, &shapes, &materials, &err, filename.c_str());

    if (!success) {
        throw RTException("Mesh::loadObj: error loading file %s: %s", filename, err);
    }

    bool needNormals = false;

    int face_idx = 0;

    // Loop over shapes
    for (size_t s = 0; s < shapes.size(); s++) {
        // Loop over faces
        size_t index_offset = 0;
        for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {
            size_t fv = shapes[s].mesh.num_face_vertices[f];
            assert(fv == 3);

            // Loop over vertices in the face.
            for (size_t v = 0; v < fv; v++) {
                // access to vertex
                tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];

                Point3f pos;
                pos[0] = attrib.vertices[3*idx.vertex_index+0];
                pos[1] = attrib.vertices[3*idx.vertex_index+1];
                pos[2] = attrib.vertices[3*idx.vertex_index+2];

                Vector3f n;
                if (attrib.normals.size() > 0) {
                    n[0] = attrib.normals[3*idx.normal_index+0];
                    n[1] = attrib.normals[3*idx.normal_index+1];
                    n[2] = attrib.normals[3*idx.normal_index+2];
                } else {
                    needNormals = true;
                }

                Vector2f tc = Vector2f::Zero();
                if (attrib.texcoords.size() > 0) {
                    tc[0] = attrib.texcoords[2*idx.texcoord_index+0];
                    tc[1] = attrib.texcoords[2*idx.texcoord_index+1];
                }
                m_vertices.push_back(Vertex(pos, n, tc));
            }

            m_faces.push_back(FaceIndex(face_idx, face_idx+1, face_idx+2));
            face_idx += 3;
            index_offset += fv;
        }
    }

    if(needNormals)
    {
        std::cout << "compute normals\n";
        computeNormals();
    }
    computeAABB();
}

void Mesh::load3DS(const std::string& filename)
{
    Lib3dsFile* pFile = lib3ds_file_open(filename.c_str());

    if(pFile==0)
    {
        std::cerr << "file not find !" << std::endl;
        return;
    }

    lib3ds_file_eval(pFile,0);

    /*
      1 objet 3DS = ensemble de sous-objets
      1 sous-objet = ensemble de sommets et de faces (triangles)
      1 triangle = liste de 3 indices
  */

    // Parcours de tous les sous-objets

    /* pFile->meshes == pointeur sur le premier sous-objet */
    for(int m=0; m<pFile->nmeshes; m++)
    {
        /* pointeur sur le sous-objet courrant */
        Lib3dsMesh* pMesh = pFile->meshes[m];

        uint i;
        uint offset_id = m_vertices.size();

        // Parcours de tous les points du sous-objet
        for (i = 0; i < pMesh->nvertices; i++)
        {
            /* position du sommet i */
            m_vertices.push_back(Vertex(Vector3f(pMesh->vertices[i][0],
                                                pMesh->vertices[i][1],
                                                pMesh->vertices[i][2])));

            // ... alors les coordonnées de texture sont disponibles :
            m_vertices.back().texcoord[0] = pMesh->texcos[i][0]; // i = numéro du sommet
            m_vertices.back().texcoord[1] = pMesh->texcos[i][1];
        }

        // Parcours de toutes les faces du sous-objet
        for (i = 0; i < pMesh->nfaces; i++)
        {
            m_faces.push_back(FaceIndex(
                                 offset_id + pMesh->faces[i].index[0],
                             offset_id + pMesh->faces[i].index[1],
                    offset_id + pMesh->faces[i].index[2]));
        }
    }

    computeNormals();
    computeAABB();
}

void Mesh::loadRawData(float* positions, int nbVertices, int* indices, int nbTriangles)
{
    m_vertices.resize(nbVertices);
    for(int i=0; i<nbVertices; ++i)
        m_vertices[i].position = Point3f::Map(positions+3*i);
    m_faces.resize(nbTriangles);
    for(int i=0; i<nbTriangles; ++i)
        m_faces[i] = Eigen::Vector3i::Map(indices+3*i);

    computeNormals();
    computeAABB();
}

Mesh::~Mesh()
{
    if(m_isInitialized)
    {
        glDeleteBuffers(1,&m_vertexBufferId);
        glDeleteBuffers(1,&m_indexBufferId);
    }
    delete m_BVH;
}

void Mesh::makeUnitary()
{
    // computes the lowest and highest coordinates of the axis aligned bounding box,
    // which are equal to the lowest and highest coordinates of the vertex positions.
    Eigen::Vector3f lowest, highest;
    lowest.fill(std::numeric_limits<float>::max());   // "fill" sets all the coefficients of the vector to the same value
    highest.fill(-std::numeric_limits<float>::max());

    for(VertexArray::iterator v_iter = m_vertices.begin() ; v_iter!=m_vertices.end() ; ++v_iter)
    {
        // - v_iter is an iterator over the elements of mVertices,
        //   an iterator behaves likes a pointer, it has to be dereferenced (*v_iter, or v_iter->) to access the referenced element.
        // - Here the .aray().min(_) and .array().max(_) operators work per component.
        //
        lowest  = lowest.array().min(v_iter->position.array());
        highest = highest.array().max(v_iter->position.array());
    }

    Point3f center = (lowest+highest)/2.0;
    float m = (highest-lowest).maxCoeff();
    for(VertexArray::iterator v_iter = m_vertices.begin() ; v_iter!=m_vertices.end() ; ++v_iter)
        v_iter->position = (v_iter->position - center) / m;

    computeAABB();
}

void Mesh::computeNormals()
{
    // pass 1: set the normal to 0
    for(VertexArray::iterator v_iter = m_vertices.begin() ; v_iter!=m_vertices.end() ; ++v_iter)
        v_iter->normal.setZero();

    // pass 2: compute face normals and accumulate
    for(FaceIndexArray::iterator f_iter = m_faces.begin() ; f_iter!=m_faces.end() ; ++f_iter)
    {
        Vector3f v0 = m_vertices[(*f_iter)(0)].position;
        Vector3f v1 = m_vertices[(*f_iter)(1)].position;
        Vector3f v2 = m_vertices[(*f_iter)(2)].position;

        Vector3f n = (v1-v0).cross(v2-v0).normalized();

        m_vertices[(*f_iter)(0)].normal += n;
        m_vertices[(*f_iter)(1)].normal += n;
        m_vertices[(*f_iter)(2)].normal += n;
    }

    // pass 3: normalize
    for(VertexArray::iterator v_iter = m_vertices.begin() ; v_iter!=m_vertices.end() ; ++v_iter)
        v_iter->normal.normalize();
}

void Mesh::computeAABB()
{
    m_AABB.setNull();
    for(VertexArray::iterator v_iter = m_vertices.begin() ; v_iter!=m_vertices.end() ; ++v_iter)
        m_AABB.extend(v_iter->position);
}

void Mesh::buildBVH()
{
    if(m_BVH)
        delete m_BVH;
    m_BVH = new BVH;
    m_BVH->build(this, 10, 100);
}

void Mesh::drawGeometry() const
{
    if(!m_isInitialized)
    {
        m_isInitialized = true;
        // this is the first call to drawGeometry
        // => create the BufferObjects and copy the related data into them.
        glGenBuffers(1,&m_vertexBufferId);
        glBindBuffer(GL_ARRAY_BUFFER, m_vertexBufferId);
        glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex)*m_vertices.size(), m_vertices[0].position.data(), GL_STATIC_DRAW);

        glGenBuffers(1,&m_indexBufferId);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indexBufferId);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(FaceIndex)*m_faces.size(), m_faces[0].data(), GL_STATIC_DRAW);

        glGenVertexArrays(1,&m_vertexArrayId);
    }

    // bind the vertex array
    glBindVertexArray(m_vertexArrayId);

    glBindBuffer(GL_ARRAY_BUFFER, m_vertexBufferId);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indexBufferId);

    int vertex_loc   = m_shader->attrib("vtx_position");
    int normal_loc   = m_shader->attrib("vtx_normal");
    int texcoord_loc = m_shader->attrib("vtx_texcoord",false);

    // specify the vertex data
    if(vertex_loc>=0)
    {
        glVertexAttribPointer(vertex_loc, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
        glEnableVertexAttribArray(vertex_loc);
    }

    if(normal_loc>=0)
    {
        glVertexAttribPointer(normal_loc, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)sizeof(Vector3f));
        glEnableVertexAttribArray(normal_loc);
    }

    if(texcoord_loc>=0)
    {
        glVertexAttribPointer(texcoord_loc, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(2*sizeof(Vector3f)));
        glEnableVertexAttribArray(texcoord_loc);
    }

    // send the geometry
    glDrawElements(GL_TRIANGLES, 3*m_faces.size(), GL_UNSIGNED_INT, (void*)0);

    if(texcoord_loc>=0) glDisableVertexAttribArray(texcoord_loc);
    if(normal_loc>=0)   glDisableVertexAttribArray(normal_loc);
    if(vertex_loc>=0)   glDisableVertexAttribArray(vertex_loc);

    // release the vertex array
    glBindVertexArray(0);
}

long int Mesh::ms_itersection_count = 0;

bool Mesh::intersectFace(const Ray& ray, Hit& hit, int faceId) const
{
    ms_itersection_count++;
    Vector3f v0 = m_vertices[m_faces[faceId](0)].position;
    Vector3f v1 = m_vertices[m_faces[faceId](1)].position;
    Vector3f v2 = m_vertices[m_faces[faceId](2)].position;
    Vector3f e1 = v1 - v0;
    Vector3f e2 = v2 - v0;
    Eigen::Matrix3f M;
    M << -ray.direction, e1, e2;
    Vector3f tuv = M.inverse() * (ray.origin - v0);
    float t = tuv(0), u = tuv(1), v = tuv(2);
    if(t>0 && u>=0 && v>=0 && (u+v)<=1 && t<hit.t())
    {
        hit.setT(t);

        if(ray.shadowRay)
            return true;

        Vector3f n0 = m_vertices[m_faces[faceId](0)].normal;
        Vector3f n1 = m_vertices[m_faces[faceId](1)].normal;
        Vector3f n2 = m_vertices[m_faces[faceId](2)].normal;
        hit.setNormal(u*n1 + v*n2 + (1.-u-v)*n0);

        Vector2f tc0 = m_vertices[m_faces[faceId](0)].texcoord;
        Vector2f tc1 = m_vertices[m_faces[faceId](1)].texcoord;
        Vector2f tc2 = m_vertices[m_faces[faceId](2)].texcoord;
        hit.setTexcoord(u*tc1 + v*tc2 + (1.-u-v)*tc0);

        return true;
    }
    return false;
}

bool Mesh::intersect(const Ray& ray, Hit& hit) const
{
    if(m_BVH)
    {
        // use the BVH !!
        return m_BVH->intersect(ray, hit);
    }
    else
    {
        // brute force !!
        bool ret = false;
        float tMin, tMax;
        Normal3f normal;
        if( (!::intersect(ray, m_AABB, tMin, tMax,normal)) || tMin>hit.t())
            return false;

        for(uint i=0; i<m_faces.size(); ++i)
        {
            ret = ret | intersectFace(ray, hit, i);
        }
        return ret;
    }
}

std::string Mesh::toString() const {
    return tfm::format(
        "Mesh[\n"
        "  vertexCount = %i,\n"
        "  triangleCount = %i,\n"
        "  material = %s\n"
        "]",
        m_vertices.size(),
        m_faces.size(),
        m_material ? indent(m_material->toString()) : std::string("null")
    );
}

REGISTER_CLASS(Mesh, "mesh")
