#ifndef LINE_H
#define LINE_H

#include "common.h"
#include "vector.h"
#include <nanogui/glutil.h>

class Point
{
public:
    static void draw(nanogui::GLShader* prg, const Point3f& p)
    {
        unsigned int vertexBufferId;
        glGenBuffers(1,&vertexBufferId);
        glBindBuffer(GL_ARRAY_BUFFER, vertexBufferId);
        glBufferData(GL_ARRAY_BUFFER, sizeof(Point3f), p.data(), GL_STATIC_DRAW);

        unsigned int vertexArrayId;
        glGenVertexArrays(1,&vertexArrayId);

        // bind the vertex array
        glBindVertexArray(vertexArrayId);

        glBindBuffer(GL_ARRAY_BUFFER, vertexBufferId);

        int vertex_loc = prg->attrib("vtx_position");
        glVertexAttribPointer(vertex_loc, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
        glEnableVertexAttribArray(vertex_loc);

        glPointSize(10.f);
        glDrawArrays(GL_POINTS,0,1);

        glDisableVertexAttribArray(vertex_loc);
        glBindVertexArray(0);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glDeleteBuffers(1, &vertexBufferId);
        glBindVertexArray(0);
        glDeleteVertexArrays(1, &vertexArrayId);
    }
};

class Line
{
public:
    static void draw(nanogui::GLShader* prg, const Point3f& p1, const Point3f& p2)
    {
        Point3f mPoints[2] = {p1, p2};

        unsigned int vertexBufferId;
        glGenBuffers(1,&vertexBufferId);
        glBindBuffer(GL_ARRAY_BUFFER, vertexBufferId);
        glBufferData(GL_ARRAY_BUFFER, sizeof(Point3f)*2, mPoints[0].data(), GL_STATIC_DRAW);

        unsigned int vertexArrayId;
        glGenVertexArrays(1,&vertexArrayId);

        // bind the vertex array
        glBindVertexArray(vertexArrayId);

        glBindBuffer(GL_ARRAY_BUFFER, vertexBufferId);

        int vertex_loc = prg->attrib("vtx_position");
        glVertexAttribPointer(vertex_loc, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
        glEnableVertexAttribArray(vertex_loc);

        glDrawArrays(GL_LINES,0,2);

        glDisableVertexAttribArray(vertex_loc);
        glBindVertexArray(0);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glDeleteBuffers(1, &vertexBufferId);
        glBindVertexArray(0);
        glDeleteVertexArrays(1, &vertexArrayId);
    }
};

class Box
{
public:
    static void draw(nanogui::GLShader* prg, const Eigen::AlignedBox3f& box)
    {
        Point3f mPoints[8] = { box.corner(Eigen::AlignedBox3f::BottomLeft),
                                       box.corner(Eigen::AlignedBox3f::BottomLeftCeil),
                                       box.corner(Eigen::AlignedBox3f::BottomRight),
                                       box.corner(Eigen::AlignedBox3f::BottomRightCeil),
                                       box.corner(Eigen::AlignedBox3f::TopLeft),
                                       box.corner(Eigen::AlignedBox3f::TopLeftCeil),
                                       box.corner(Eigen::AlignedBox3f::TopRight),
                                       box.corner(Eigen::AlignedBox3f::TopRightCeil)
                                     };

        unsigned int vertexBufferId, indexBufferId;
        glGenBuffers(1,&vertexBufferId);
        glBindBuffer(GL_ARRAY_BUFFER, vertexBufferId);
        glBufferData(GL_ARRAY_BUFFER, sizeof(Point3f)*8, mPoints[0].data(), GL_STATIC_DRAW);

        Eigen::Vector2i indices[12] = { Eigen::Vector2i(0,1), Eigen::Vector2i(2,3), Eigen::Vector2i(0,2), Eigen::Vector2i(1,3),
                                        Eigen::Vector2i(0,4), Eigen::Vector2i(1,5), Eigen::Vector2i(2,6), Eigen::Vector2i(3,7),
                                        Eigen::Vector2i(4,5), Eigen::Vector2i(6,7), Eigen::Vector2i(4,6), Eigen::Vector2i(5,7)};
        glGenBuffers(1,&indexBufferId);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferId);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Eigen::Vector2i)*12, indices[0].data(), GL_STATIC_DRAW);

        unsigned int vertexArrayId;
        glGenVertexArrays(1,&vertexArrayId);

        // bind the vertex array
        glBindVertexArray(vertexArrayId);

        glBindBuffer(GL_ARRAY_BUFFER, vertexBufferId);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferId);

        int vertex_loc = prg->attrib("vtx_position");
        glVertexAttribPointer(vertex_loc, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
        glEnableVertexAttribArray(vertex_loc);

        glDrawElements(GL_LINES, 24, GL_UNSIGNED_INT, (void*)0);

        glDisableVertexAttribArray(vertex_loc);
        glBindVertexArray(0);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glDeleteBuffers(1, &vertexBufferId);
        glBindVertexArray(0);
        glDeleteVertexArrays(1, &vertexArrayId);
    }
};

#endif // LINE_H
