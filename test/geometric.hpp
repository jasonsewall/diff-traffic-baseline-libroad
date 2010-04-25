#ifndef _GEOMETRIC_HPP_
#define _GEOMETRIC_HPP_

#include <cstring>
#include <GL/gl.h>
#include "libroad/libroad_common.hpp"

inline bool rm_invert(double A[16], double Ainv[16])
{
    double swap_space[4];

    for(int i = 0; i < 4; ++i)
    {
        int maxi = i;
        for(int j = i+1; j < 4; ++j)
            if(std::abs(A[4*j + i]) > std::abs(A[4*maxi + i]))
                maxi = j;
        if(A[4*maxi + i] == 0.0f)
            return false;
        if(i != maxi)
        {
            std::memcpy(swap_space,       &(A[4*maxi+ 0]), sizeof(double)*4);
            std::memcpy(&(A[4*maxi + 0]), &(A[4*i   + 0]), sizeof(double)*4);
            std::memcpy(&(A[4*i + 0]),    swap_space,      sizeof(double)*4);

            std::memcpy(swap_space,          &(Ainv[4*maxi + 0]), sizeof(double)*4);
            std::memcpy(&(Ainv[4*maxi + 0]), &(Ainv[4*i    + 0]), sizeof(double)*4);
            std::memcpy(&(Ainv[4*i    + 0]), swap_space,          sizeof(double)*4);
        }

        const double iinv = 1.0f/A[4*i + i];
        A[4*i + i] = 1.0f;
        for(int j = i+1; j < 4; ++j)
            A[4*i + j] *= iinv;
        for(int j = 0; j < 4; ++j)
            Ainv[4*i + j] *= iinv;

        for(int j = 0; j < 4; ++j)
            if(A[4*j + i] != 0.0f && i != j)
            {
                double temp = A[4*j + i];
                A[4*j + i] = 0.0f;
                for(int k = i+1; k < 4; ++k)
                    A[4*j + k] -= A[4*i + k]*temp;
                for(int k = 0; k < 4; ++k)
                    Ainv[4*j + k] -= Ainv[4*i + k]*temp;
            }
    }

    return true;
}

inline void pick_ray(vec3f &origin, vec3f &dir, const float x, const float y, const int width, const int height, bool ortho = false)
{
    GLint view[4];
    glGetIntegerv(GL_VIEWPORT, view);
    GLdouble project[16];
    glGetDoublev(GL_PROJECTION_MATRIX, project);
    GLdouble model[16];
    glGetDoublev(GL_MODELVIEW_MATRIX, model);

    vec3d pt;
    vec3d o;

    GLdouble model2[16];
    GLdouble modelinv[16];

    for(int j = 0; j < 4; ++j)
        for(int i = 0; i < 4; ++i)
            model2[4*j+i] = model[4*i+j];

    memset(modelinv, 0, 16*sizeof(double));
    modelinv[4*0 + 0] = 1.0;
    modelinv[4*1 + 1] = 1.0;
    modelinv[4*2 + 2] = 1.0;
    modelinv[4*3 + 3] = 1.0;

    rm_invert(model2, modelinv);

    gluUnProject(x, height-y, 1.0, model, project, view, pt.data(), pt.data()+1, pt.data()+2);

    if(ortho)
        gluUnProject(x, height-y, -1.0, model, project, view, o.data(), o.data()+1, o.data()+2);
    else
    {
        o[0] = modelinv[4*0 + 3];
        o[1] = modelinv[4*1 + 3];
        o[2] = modelinv[4*2 + 3];
    }

    const vec3d v(tvmet::normalize(pt - o));
    for(int i = 0; i < 3; ++i)
    {
        origin[i] = o[i];
        dir[i]    = v[i];
    }
}

inline float ray_point_project(const vec3f &origin, const vec3f &dir, const vec3f &pt)
{
    // assumes dir is unit, so no divide by tvmet::dot(dir, dir);
    return tvmet::dot(dir, pt-origin);
}

inline vec3f ray_point_nearest(const vec3f &origin, const vec3f &dir, const vec3f &pt)
{
    return vec3f(origin + dir*ray_point_project(origin, dir, pt));
}

inline float ray_point_distance2(const vec3f &origin, const vec3f &dir, const vec3f &pt)
{
    const vec3f diff(pt - ray_point_nearest(origin, dir, pt));
    return tvmet::dot(diff, diff);
}

inline float ray_plane_intersection_param(const vec3f &origin, const vec3f &dir, const vec3f &normal, const float support)
{
    const float denom = tvmet::dot(dir, normal);
    assert(std::abs(denom) > 1e-8);

    return (-tvmet::dot(origin, normal) + support)/denom;
}

inline vec3f ray_plane_intersection(const vec3f &origin, const vec3f &dir, const vec3f &normal, const float support)
{
    return vec3f(origin + dir*ray_plane_intersection_param(origin, dir, normal, support));
}

#endif
