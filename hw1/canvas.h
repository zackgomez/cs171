#pragma once
#include <ostream>
#include <cmath>
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

    void drawPixel(unsigned x, unsigned y, float r, float g, float b)
    {
        if (x < 0 || x >= xres_ || y < 0 || y >= yres_)
            return;

        //std::cout << "Drawing pixel: (" << x << ',' << y << ")\n";

        int i = y * xres_ + x;
        r_[i] = r;
        g_[i] = g;
        b_[i] = b;
    }

    void drawLine(float x1, float y1, float x2, float y2)
    {
        //std::cout << "Drawing line from (" << x1 << ',' << y1 << ") to ("
            //<< x2 << ',' << y2 << ")\n";

        if (x1 > x2)
        {
            std::swap(x1, x2);
            std::swap(y1, y2);
        }
        else if (x1 == x2 && y2 > y1)
            std::swap(y1, y2);

        // Convert to pixel coords
        int x1p = floor((x1 - xmin_) / (xmax_ - xmin_) * xres_);
        int y1p = floor((ymax_ - y1) / (ymax_ - ymin_) * yres_);
        int x2p = floor((x2 - xmin_) / (xmax_ - xmin_) * xres_);
        int y2p = floor((ymax_ - y2) / (ymax_ - ymin_) * yres_);
        
        // Direction control
        bool xdir = true;
        int step = 1, ystep = 1; // These control direction, up/down

        // bresenham params
        int dv = y2p - y1p; 
        int dvdv = y2p - y1p + x1p - x2p;
        int F = dvdv;

        // Modify directions/parameters for different slopes
        // Negative slope
        if (y2p - y1p < 0)
        {
            ystep = -1;
            step = -1;
            if (x2p - x1p < y1p - y2p)
            {
                // step in y direction
                xdir = false;
                step = x2p > x1p ? 1 : -1;

                dv = x2p - x1p;
                dvdv = x2p - x1p + y2p - y1p;
                F = dvdv;
            }
            else
            {
                dv = y1p - y2p;
                dvdv = y1p - y2p + x1p - x2p;
                F = dvdv;
            }
        }
        // abs(slope) > 1
        if (x2p - x1p < y2p - y1p)
        {
            // step in y direction
            xdir = false;
            step = x2p > x1p ? 1 : -1;

            dv = x2p - x1p;
            dvdv = x2p - x1p + y1p - y2p;
            F = dvdv;
        }


        /*
        std::cout << "Drawing line from pixels (" << x1p << ',' << y1p << ") to ("
            << x2p << ',' << y2p << ")\n";

        std::cout << "Would draw line with parameters: xdir = " << xdir << " step = " << step
            << " ystep = " << ystep << " dv = " << dv << " dvdv = " << dvdv << " F = " << F << '\n';
            */

        // Do the actual drawing
        // NOTE there is some crazy shit going on in this for statement
        // continue going if stepping in +x direction and x <= xend
        //                   stepping in +y direction and y <= yend
        //                   stepping in -y direction and y >= yend
        // if +xstep, inc x
        // if -xstep, add ystep to y
        int x, y;
        for (x = x1p, y = y1p; xdir ? (x <= x2p) : (ystep > 0 ? y <= y2p : y >= y2p); xdir ? x++ : y += ystep)
        {
            //std::cout << "Trying (" << x << ',' << y << ") && F = " << F << " && ";
            drawPixel(x, y, 1.0, 1.0, 1.0);
            if (F < 0)
            {
                F += dv;
            }
            else
            {
                // Increment appropriate direction
                xdir ? y += step : x += step;
                F += dvdv;
            }
        }

        //std::cout << " ---- Done Drawing Line ---- \n";

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

