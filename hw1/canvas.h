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

        int i = y * xres_ + x;
        r_[i] = r;
        g_[i] = g;
        b_[i] = b;
    }

    void drawLine(float x1, float y1, float x2, float y2)
    {
        std::cout << "Drawing line from (" << x1 << ',' << y1 << ") to ("
            << x2 << ',' << y2 << ")\n";

        if (x1 > x2)
        {
            std::swap(x1, x2);
            std::swap(y1, y2);
        }

        // Convert to pixel coords
        int x1p = floor((x1 - xmin_) / (xmax_ - xmin_) * xres_);
        int y1p = floor((y1 - ymin_) / (ymax_ - ymin_) * yres_);
        int x2p = floor((x2 - xmin_) / (xmax_ - xmin_) * xres_);
        int y2p = floor((y2 - ymin_) / (ymax_ - ymin_) * yres_);
        std::cout << "Drawing line from pixels (" << x1p << ',' << y1p << ") to ("
            << x2p << ',' << y2p << ")\n";

        float slope = (y2 - y1) / (x2 - x1);
        if (fabs(slope) <= 1)
        {
            drawLineHelperX(x1p, x2p, y1p, y2p);
        }
        else
        {
            if (y1 > y2)
            {
                std::swap(x1p, x2p);
                std::swap(y1p, y2p);
            }
            drawLineHelperY(x1p, x2p, y1p, y2p);
        }

        std::cout << " ---- Done Drawing Line ---- \n";

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

    void drawLineHelperX(int x1, int x2, int y1, int y2)
    {
        std::cout << "Using xstep line helper\n";

        // Compute bresenham algo parameters
        int dy = y2 - y1;
        int ystep = dy > 0 ? 1 : -1;
        dy = dy > 0 ? dy : -dy;
        int dxdy = dy + x1 - x2;
        int F = dxdy;
        std::cout << "Bresenham parameters: dy = " << dy << " dxdy = " << dxdy << " F = " << F << '\n';

        int y = y1;
        for (int x = x1; x <= x2; x++)
        {
            std::cout << "Drawing pixel (" << x << ',' << y << ").  F = " << F << '\n';
            drawPixel(x, yres_ - y, 1, 1, 1);
            if (F < 0)
                if (dy >= 0) F += dy;
                else F -= dy;
            else
            {
                y += ystep;
                F += dxdy;
            }
        }
    }

    void drawLineHelperY(int x1, int x2, int y1, int y2)
    {
        std::cout << "Using ystep line helper\n";

        // Compute bresenham algo parameters
        int dx   = x2 - x1;
        int xstep = 1;
        if (dx < 0)
        {
            dx = -dx;
            xstep = -1;
        }
        int dydx = dx + y1 - y2;
        int F    = dydx;
        std::cout << "Bresenham parameters: dx = " << dx << " dydx = " << dydx << " F = " << F << '\n';

        int x = x1;
        for (int y = y1; y <= y2; y++)
        {
            std::cout << "Drawing pixel (" << x << ',' << y << ").  F = " << F << '\n';
            drawPixel(x, yres_ - y, 1, 1, 1);
            if (F < 0)
                F += dx;
            else
            {
                x += xstep;
                F += dydx;
            }
        }
    }
};

