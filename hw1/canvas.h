#pragma once
#include <ostream>

#include <iostream>

class Canvas
{
public:
    // Default constructor
    Canvas() :
        r_(NULL),
        g_(NULL),
        b_(NULL)
    {}

    Canvas(float xmin, float xmax, float ymin, float ymax, unsigned xres, unsigned yres) :
        xmin_(xmin),
        xmax_(xmax),
        ymin_(ymin),
        ymax_(ymax),
        xres_(xres),
        yres_(yres)
    {
        r_ = new float[xres * yres];
        b_ = new float[xres * yres];
        g_ = new float[xres * yres];

        // Initialize to black
        for (unsigned i = 0; i < xres * yres; i++)
        {
            r_[i] = b_[i] = g_[i] = 0.0f;
        }
    }

    ~Canvas()
    {
        delete[] r_;
        delete[] g_;
        delete[] b_;
    }

    void drawPoint(float x, float y, float r, float g, float b)
    {
        // Don't draw the point if it's not in range
        if (x < xmin_ || x > xmax_ || y < ymin_ || y > ymax_)
            return;
        int xi = (x - xmin_) / (xmax_ - xmin_) * xres_;
        int yi = (y - ymin_) / (ymax_ - ymin_) * yres_;
        int i = xi*yres_ + yi;
        r_[i] = r;
        g_[i] = g;
        b_[i] = b;
    }

    void drawLine(float x1, float y1, float x2, float y2)
    {
        std::cout << "Drawing line from (" << x1 << ',' << y1 << ") to ("
            << x2 << ',' << y2 << ")\n";

        // TODO actually draw using drawPoint

    }

    void display(std::ostream &os, unsigned maxintensity)
    {
        // Output PPM header
        os << "P3\n";
        os << xres_ << ' ' << yres_ << '\n';
        os << maxintensity << '\n';

        // Now the pixel data
        for (unsigned i = 0; i < xres_ * yres_; i++)
        {
            os << static_cast<unsigned>(r_[i] * maxintensity) << ' ';
            os << static_cast<unsigned>(g_[i] * maxintensity) << ' ';
            os << static_cast<unsigned>(b_[i] * maxintensity) << '\n';
        }
    }


private:
    float xmin_, xmax_;
    float ymin_, ymax_;
    unsigned xres_, yres_;

    float *r_;
    float *g_;
    float *b_;
};

