#include "mesh.h"
#include "bvh.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdio>

using namespace std;
using namespace Eigen;
using namespace surface_mesh;

Mesh::~Mesh()
{
    if(_ready){
        glDeleteBuffers(2, _vbo);
        glDeleteVertexArrays(1,&_vao);
    }
    delete _bvh;
}

void Mesh::load(const string& filename)
{
    cout << "Loading: " << filename << endl;

    _halfEdgeMesh.read(filename);
    _halfEdgeMesh.update_face_normals();
    _halfEdgeMesh.update_vertex_normals();

    // vertex properties
    Surface_mesh::Vertex_property<Point> vertices = _halfEdgeMesh.get_vertex_property<Point>("v:point");
    Surface_mesh::Vertex_property<Normal> vnormals = _halfEdgeMesh.get_vertex_property<Normal>("v:normal");
    Surface_mesh::Vertex_property<Texture_coordinate> texcoords = _halfEdgeMesh.get_vertex_property<Texture_coordinate>("v:texcoord");
    Surface_mesh::Vertex_property<Color> colors = _halfEdgeMesh.get_vertex_property<Color>("v:color");

    // vertex iterator
    Surface_mesh::Vertex_iterator vit;

    Vector3f pos;
    Vector3f normal;
    Vector2f tex;
    Vector3f color;
    for(vit = _halfEdgeMesh.vertices_begin(); vit != _halfEdgeMesh.vertices_end(); ++vit)
    {
        pos = vertices[*vit];
        normal = vnormals[*vit];
        if(texcoords)
            tex = texcoords[*vit].head(2);
        if(colors)
            color = colors[*vit];
        else
            color = Vector3f(0.85f,0.85f,0.85f);

        _vertices.push_back(Vertex(pos,color,normal,tex));
    }

    // face iterator
    Surface_mesh::Face_iterator fit, fend = _halfEdgeMesh.faces_end();
    // vertex circulator
    Surface_mesh::Vertex_around_face_circulator fvit, fvend;
    Surface_mesh::Vertex v0, v1, v2;
    for (fit = _halfEdgeMesh.faces_begin(); fit != fend; ++fit)
    {
        fvit = fvend = _halfEdgeMesh.vertices(*fit);
        v0 = *fvit;
        ++fvit;
        v2 = *fvit;

        do{
            v1 = v2;
            ++fvit;
            v2 = *fvit;
            _indices.push_back(v0.idx());
            _indices.push_back(v1.idx());
            _indices.push_back(v2.idx());
        } while (++fvit != fvend);
    }

    updateNormals();
    updateBoundingBox();
    updateBVH();
}

void Mesh::init()
{
    glGenVertexArrays(1, &_vao);
    glGenBuffers(2, _vbo);

    updateVBO();

    _ready = true;
}

void Mesh::updateAll()
{
  updateNormals();
  updateBoundingBox();
  updateBVH();
  updateVBO();
}

void Mesh::updateNormals()
{
    // pass 1: set the normal to 0
    for(std::vector<Vertex>::iterator v_iter = _vertices.begin() ; v_iter!=_vertices.end() ; ++v_iter)
        v_iter->normal.setZero();

    // pass 2: compute face normals and accumulate
    for(int j=0; j<nbFaces(); ++j)
    {
        Vector3f v0 = _vertices[_indices[j*3+0]].position;
        Vector3f v1 = _vertices[_indices[j*3+1]].position;
        Vector3f v2 = _vertices[_indices[j*3+2]].position;

        Vector3f n = (v1-v0).cross(v2-v0).normalized();

        _vertices[_indices[j*3+0]].normal += n;
        _vertices[_indices[j*3+1]].normal += n;
        _vertices[_indices[j*3+2]].normal += n;
    }

    // pass 3: normalize
    for(std::vector<Vertex>::iterator v_iter = _vertices.begin() ; v_iter!=_vertices.end() ; ++v_iter)
        v_iter->normal.normalize();
}

void Mesh::updateVBO()
{
  glBindVertexArray(_vao);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _vbo[0]);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(int) * _indices.size(), _indices.data(),  GL_STATIC_DRAW);

  glBindBuffer(GL_ARRAY_BUFFER, _vbo[1]);
  glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex)*_vertices.size(), _vertices[0].position.data(), GL_STATIC_DRAW);

  glBindVertexArray(0);
}

void Mesh::updateBoundingBox()
{
  _bbox.setNull();
  for(std::vector<Vertex>::iterator v_iter = _vertices.begin() ; v_iter!=_vertices.end() ; ++v_iter)
      _bbox.extend(v_iter->position);
}

void Mesh::updateBVH()
{
    if(_bvh)
        delete _bvh;
    _bvh = new BVH;
    _bvh->build(this, 10, 100);
}

void Mesh::display(Shader *shader)
{
    if (!_ready)
        init();

    glBindVertexArray(_vao);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _vbo[0]);
    glBindBuffer(GL_ARRAY_BUFFER, _vbo[1]);

    int vertex_loc = shader->getAttribLocation("vtx_position");
    if(vertex_loc>=0) {
        glVertexAttribPointer(vertex_loc, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
        glEnableVertexAttribArray(vertex_loc);
    }

    int color_loc = shader->getAttribLocation("vtx_color");
    if(color_loc>=0){
        glVertexAttribPointer(color_loc, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(sizeof(Vector3f)));
        glEnableVertexAttribArray(color_loc);
    }

    int normal_loc = shader->getAttribLocation("vtx_normal");
    if(normal_loc>=0){
        glVertexAttribPointer(normal_loc, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(2*sizeof(Vector3f)));
        glEnableVertexAttribArray(normal_loc);
    }

    int texCoord_loc = shader->getAttribLocation("vtx_texcoord");
    if(texCoord_loc>=0){
        glVertexAttribPointer(texCoord_loc, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(3*sizeof(Vector3f)));
        glEnableVertexAttribArray(texCoord_loc);
    }

    int mask_loc = shader->getAttribLocation("vtx_mask");
    if(mask_loc>=0){
        glVertexAttribPointer(mask_loc, 1, GL_INT, GL_FALSE, sizeof(Vertex), (void*)(3*sizeof(Vector3f)+sizeof(Vector2f)));
        glEnableVertexAttribArray(mask_loc);
    }

    glDrawElements(GL_TRIANGLES, _indices.size(),  GL_UNSIGNED_INT, 0);

    if(vertex_loc>=0)
        glDisableVertexAttribArray(vertex_loc);
    if(color_loc>=0)
        glDisableVertexAttribArray(color_loc);
    if(normal_loc>=0)
        glDisableVertexAttribArray(normal_loc);
    if(texCoord_loc>=0)
        glDisableVertexAttribArray(texCoord_loc);
    if(mask_loc>=0)
        glDisableVertexAttribArray(mask_loc);

    glBindVertexArray(0);
}

bool Mesh::intersectFace(const Ray& ray, Hit& hit, int faceId) const
{
    Vector3f v0 = _vertices[_indices[faceId*3+0]].position;
    Vector3f v1 = _vertices[_indices[faceId*3+1]].position;
    Vector3f v2 = _vertices[_indices[faceId*3+2]].position;
    Vector3f e1 = v1 - v0;
    Vector3f e2 = v2 - v0;
    Eigen::Matrix3f M;
    M << -ray.direction, e1, e2;
    Vector3f tuv = M.inverse() * (ray.origin - v0);
    float t = tuv(0), u = tuv(1), v = tuv(2);
    if(t>0 && u>=0 && v>=0 && (u+v)<=1 && t<hit.t())
    {
        hit.setT(t);

        hit.setFaceId(faceId);
        hit.setBaryCoords(Vector3f(u,v,1.-u-v));

        return true;
    }
    return false;
}

bool Mesh::intersect(const Ray& ray, Hit& hit) const
{
    if(_bvh)
    {
        // use the BVH !!
        return _bvh->intersect(ray, hit);
    }
    else
    {
        // brute force !!
        bool ret = false;
        float tMin, tMax;
        Normal3f normal;
        if( (!::intersect(ray, _bbox, tMin, tMax,normal)) || tMin>hit.t())
            return false;

        for(uint i=0; i<nbFaces(); ++i)
        {
            ret = ret | intersectFace(ray, hit, i);
        }
        return ret;
    }
}
