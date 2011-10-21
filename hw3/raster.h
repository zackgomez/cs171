#pragma once
#include <cassert>
#include "canvas.h"

static const Canvas *canv;

struct vertex
{
    float *data;
    unsigned num_data;
};
struct rasterCoord
{
    int x, y;
};

void initRaster(const Canvas *c)
{
    canv = c;
}

static int f(rasterCoord vert0, rasterCoord vert1, int x, int y)
{
    int x0 = vert0.x;
    int y0 = vert0.y;
    int x1 = vert1.x;
    int y1 = vert1.y;
    return (y0 - y1) * x + (x1 - x0) * y + x0 * y1 - x1 * y0;
}

// The fragment processor template needs to support function call notation
// with signature void (int, int, float*).  The two int arguments are the
// pixel coordinates and the float* is arbitrary data.
    template<typename fragmentProcessor>
void rasterizeTriangle(vertex verts[3], fragmentProcessor fp)
{
    // Make sure the vertices all have the same amount of data
    assert(verts[0].num_data == verts[1].num_data &&
            verts[0].num_data == verts[1].num_data);
    // Pixel coordinates for a bounding box
    int xMin, xMax, yMin, yMax;
    // High enough that it doesn't matter
    xMin = canv->getXRes() + 1; yMin = canv->getYRes() + 1;
    // Low enough that it doesn't matter
    xMax = yMax = -1;

    // Storage space to convert each vertex's x and y to pixel coordinates
    rasterCoord *coords = new rasterCoord[3];

    // do the conversion
    for (int i = 0; i < 3; i++)
    {
        coords[i].x = canv->getPixelX(verts[i].data[0]);
        coords[i].y = canv->getPixelY(verts[i].data[1]);
    }

    // find the bounding box
    for (int i = 0; i < 3; i++)
    {
        if (coords[i].x < xMin)
            xMin = coords[i].x;
        if (coords[i].y < yMin)
            yMin = coords[i].y;
        if (coords[i].x > xMax)
            xMax = coords[i].x;
        if (coords[i].y > yMax)
            yMax = coords[i].y;
    }

    // Clamp to canvas dimensions
    if (xMax > canv->getXRes()-1)
        xMax = canv->getXRes() - 1;
    if (yMax > canv->getYRes()-1)
        yMax = canv->getYRes() - 1;

    // normalizing values for the barycentric coordinates
    float fAlpha, fBeta, fGamma;

    // not sure exactly what's going on here, so read the textbook
    fAlpha = f(coords[1], coords[2], coords[0].x, coords[0].y);
    fBeta = f(coords[2], coords[0], coords[1].x, coords[1].y);
    fGamma = f(coords[0], coords[1], coords[2].x, coords[2].y);

    // check for zero denominators. if found, these indicate a degenerate
    // triangle which should not be drawn, so just return.
    if(fabs(fAlpha) < .0001 || fabs(fBeta) < .0001 || fabs(fGamma) < .0001)
        return;

    // Create storage- once
    int numData = verts[0].num_data;
    float *data = new float[numData];
    // go over every pixel in the bounding box
    for (int y = (int)((yMin > 0) ? yMin : 0); y < yMax; y++)
    {
        for (int x = (int)((xMin > 0) ? xMin : 0); x < xMax; x++)
        {

            // calculate the pixel's barycentric coordinates
            float alpha = f(coords[1], coords[2], x, y) / fAlpha;
            float beta = f(coords[2], coords[0], x, y) / fBeta;
            float gamma = f(coords[0], coords[1], x, y) / fGamma;

            // if the coordinates are positive, do the next check
            if (alpha >= 0 && beta >= 0 && gamma >= 0)
            {
                // interpolate all data
                for (int i = 0; i < numData; i++)
                {
                    data[i] = (alpha * verts[0].data[i] +
                            beta * verts[1].data[i] +
                            gamma * verts[2].data[i]);
                }

                // and finally, draw the pixel
                fp(x, y, data);
            }
        }
    }
    // Clean up
    delete data;
    delete coords;
}
