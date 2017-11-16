#include "mesh.h"
#include "bvh.h"
#include "shader.h"

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

    read(filename);
    update_face_normals();
    update_vertex_normals();

    // vertex properties
    auto vpositions = get_vertex_property<Vector3f>("v:point");
    assert(vpositions);
    auto vnormals = get_vertex_property<Vector3f>("v:normal");
    assert(vnormals);

    auto texcoords = get_vertex_property<Vector2f>("v:texcoord");
    if(!texcoords)
    {
      texcoords = add_vertex_property<Vector2f>("v:texcoord");
      this->texcoords().setZero();
    }

    auto colors = get_vertex_property<Vector3f>("v:color");
    if(!colors)
    {
      colors = add_vertex_property<Vector3f>("v:color");
      this->colors().setOnes();
    }
    add_vertex_property<int>("v:mask");
    this->masks().fill(0);

    auto vcurvatures = add_vertex_property<float>("v:mean_curvature");
    for(auto v:vertices())
      vcurvatures[v] = 0;

    // face iterator
    Surface_mesh::Face_iterator fit, fend = faces_end();
    // vertex circulator
    Surface_mesh::Vertex_around_face_circulator fvit, fvend;
    Surface_mesh::Vertex v0, v1, v2;
    for (fit = faces_begin(); fit != fend; ++fit)
    {
        fvit = fvend = vertices(*fit);
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

void Mesh::createGrid(int m, int n)
{
  clear();
  _indices.clear();
  _indices.reserve(2*3*m*n);
  reserve(m*n, 3*m*n, 2*m*n);

  float dx = 1./float(m-1);
  float dy = 1./float(n-1);

  Eigen::Array<Surface_mesh::Vertex,Dynamic,Dynamic> ids(m,n);
  for(int i=0;i<m;++i)
    for(int j=0;j<n;++j)
      ids(i,j) = add_vertex(Point(double(i)*dx, double(j)*dy, 0.));

  add_vertex_property<Vector3f>("v:color");
  add_vertex_property<Vector3f>("v:normal");
  add_vertex_property<Vector2f>("v:texcoord");
  add_vertex_property<int>("v:mask");

  colors().fill(1);
  masks().fill(0);
  texcoords() = positions().topRows(2);

  for(int i=0; i<m-1; ++i) {
    for(int j=0; j<n-1; ++j) {
      Surface_mesh::Vertex v0,v1,v2,v3;
      v0 = ids(i+0,j+0);
      v1 = ids(i+1,j+0);
      v2 = ids(i+1,j+1);
      v3 = ids(i+0,j+1);
      add_triangle(v0,v1,v2);
      add_triangle(v0,v2,v3);

      _indices.push_back(v0.idx());
      _indices.push_back(v1.idx());
      _indices.push_back(v2.idx());

      _indices.push_back(v0.idx());
      _indices.push_back(v2.idx());
      _indices.push_back(v3.idx());
    }
  }
  updateNormals();
  updateBoundingBox();
  updateBVH();
}

void Mesh::init()
{
  glGenVertexArrays(1, &_vao);
  glGenBuffers(6, _vbo);
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
  update_face_normals();
  update_vertex_normals();
}

void Mesh::updateVBO()
{
  glBindVertexArray(_vao);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _vbo[VBO_IDX_FACE]);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(int) * _indices.size(), _indices.data(),  GL_STATIC_DRAW);

  Surface_mesh::Vertex_property<Vector3f> vertices = get_vertex_property<Vector3f>("v:point");
  int n_vertices = vertices.vector().size();
  if(vertices) {
    glBindBuffer(GL_ARRAY_BUFFER, _vbo[VBO_IDX_POSITION]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vector3f)*n_vertices, vertices.vector()[0].data(), GL_STATIC_DRAW);
  }

  Surface_mesh::Vertex_property<Vector3f> vnormals = get_vertex_property<Vector3f>("v:normal");
  if(vnormals) {
    glBindBuffer(GL_ARRAY_BUFFER, _vbo[VBO_IDX_NORMAL]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vector3f)*n_vertices, vnormals.vector()[0].data(), GL_STATIC_DRAW);
  }

  Surface_mesh::Vertex_property<Vector2f> texcoords = get_vertex_property<Vector2f>("v:texcoord");
  if(texcoords) {
    glBindBuffer(GL_ARRAY_BUFFER, _vbo[VBO_IDX_TEXCOORD]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vector2f)*n_vertices, texcoords.vector()[0].data(), GL_STATIC_DRAW);
  }

  Surface_mesh::Vertex_property<Vector3f> colors = get_vertex_property<Vector3f>("v:color");
  if(colors) {
    glBindBuffer(GL_ARRAY_BUFFER, _vbo[VBO_IDX_COLOR]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vector3f)*n_vertices, colors.vector()[0].data(), GL_STATIC_DRAW);
  }

  Surface_mesh::Vertex_property<int> masks = get_vertex_property<int>("v:mask");
  if(masks) {
    glBindBuffer(GL_ARRAY_BUFFER, _vbo[VBO_IDX_MASK]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(int)*n_vertices, masks.vector().data(), GL_STATIC_DRAW);
  }

  glBindVertexArray(0);
}

void Mesh::updateBoundingBox()
{
  _bbox.setNull();
  Surface_mesh::Vertex_property<Vector3f> vertices = get_vertex_property<Vector3f>("v:point");
  for(const auto& p : vertices.vector())
      _bbox.extend(p);
}

void Mesh::updateBVH()
{
    if(_bvh)
        delete _bvh;
    _bvh = new BVH;
    _bvh->build(this, 10, 100);
}

void Mesh::draw(const Shader &shader)
{
  if (!_ready)
      init();

  glBindVertexArray(_vao);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _vbo[VBO_IDX_FACE]);


  int vertex_loc = shader.getAttribLocation("vtx_position");
  auto vertices = get_vertex_property<Vector3f>("v:point");
  if(vertex_loc>=0 && vertices) {
    glBindBuffer(GL_ARRAY_BUFFER, _vbo[VBO_IDX_POSITION]);
    glVertexAttribPointer(vertex_loc, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glEnableVertexAttribArray(vertex_loc);
  }

  int color_loc = shader.getAttribLocation("vtx_color");
  auto colors = get_vertex_property<Vector3f>("v:color");
  if(color_loc>=0 && colors){
    glBindBuffer(GL_ARRAY_BUFFER, _vbo[VBO_IDX_COLOR]);
    glVertexAttribPointer(color_loc, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glEnableVertexAttribArray(color_loc);
  }

  int normal_loc = shader.getAttribLocation("vtx_normal");
  auto vnormals = get_vertex_property<Vector3f>("v:normal");
  if(normal_loc>=0 && vnormals){
    glBindBuffer(GL_ARRAY_BUFFER, _vbo[VBO_IDX_NORMAL]);
    glVertexAttribPointer(normal_loc, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glEnableVertexAttribArray(normal_loc);
  }

  int texCoord_loc = shader.getAttribLocation("vtx_texcoord");
  auto texcoords = get_vertex_property<Vector2f>("v:texcoord");
  if(texCoord_loc>=0 && texcoords){
    glBindBuffer(GL_ARRAY_BUFFER, _vbo[VBO_IDX_TEXCOORD]);
    glVertexAttribPointer(texCoord_loc, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glEnableVertexAttribArray(texCoord_loc);
  }

  int mask_loc = shader.getAttribLocation("vtx_mask");
  auto masks = get_vertex_property<int>("v:mask");
  if(mask_loc>=0 && masks){
    glBindBuffer(GL_ARRAY_BUFFER, _vbo[VBO_IDX_MASK]);
    glVertexAttribPointer(mask_loc, 1, GL_INT, GL_FALSE, 0, (void*)0);
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
  auto vertices = get_vertex_property<Vector3f>("v:point").vector();
  Vector3f v0 = vertices[_indices[faceId*3+0]];
  Vector3f v1 = vertices[_indices[faceId*3+1]];
  Vector3f v2 = vertices[_indices[faceId*3+2]];
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
      hit.setBaryCoords(Vector3f(1.-u-v,u,v));

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

        for(uint i=0; i<n_faces(); ++i)
        {
            ret = ret | intersectFace(ray, hit, i);
        }
        return ret;
    }
}
