#include "config.h"

#include <cmath>
#include <limits>
#include <stdexcept>
#include <iostream>
#include <boost/filesystem.hpp>
#include <sstream>
#include <cstdlib>
#include <fstream>
#include <cfenv>
#include <X11/xpm.h>
#include <iomanip>
#include <FL/x.H>
#include "utils.hpp"
#include "florb.xpm"

#define CLIPLEFT   (1)  // 0001
#define CLIPRIGHT  (2)  // 0010
#define CLIPBOTTOM (4)  // 0100
#define CLIPTOP    (8)  // 1000

#define CIRCUMFERENCEKM (2*M_PI*6372.7982)

florb::point2d<double> florb::utils::wsg842merc(const florb::point2d<double> &wsg84)
{
    if ((wsg84.x() > 180.0) || (wsg84.x() < -180.0))
        throw std::out_of_range(_("Invalid longitude"));
    if ((wsg84.y() > 90.0) || (wsg84.y() < -90.0))
        throw std::out_of_range(_("Invalid latitude"));

    // Anything above 85 degrees N or S is infinity in the mercator projection,
    // clip here.
    double lat = wsg84.y();
    if (lat > 85.0)
        lat = 85.0;
    else if (lat < -85)
        lat = -85.0;

    // Project to Mercator (360 by 360 square)
    return florb::point2d<double>(
            180.0 + wsg84.x(),
            180.0 - ((180.0/M_PI) * log(tan(M_PI/4.0+lat*(M_PI/180.0)/2.0))));
}

florb::point2d<unsigned long> florb::utils::merc2px(unsigned int z, const florb::point2d<double> &merc)
{
    unsigned long dimxy = dim(z);

    return florb::point2d<unsigned long>(
        (unsigned long)(((double)(dimxy-1)/360.0) * merc.x()),
        (unsigned long)(((double)(dimxy-1)/360.0) * merc.y()));
}

florb::point2d<unsigned long> florb::utils::wsg842px(unsigned int z, const florb::point2d<double> &wsg84)
{
    // Mercator projection
    florb::point2d<double> merc(wsg842merc(wsg84));

    // Unit conversion
    return merc2px(z, merc);
}

florb::point2d<double> florb::utils::px2wsg84(unsigned int z, const florb::point2d<unsigned long> &px)
{
    // Get map dimensions
    unsigned long dimxy = dim(z);

    // Make sure the coordinate is on the map
    if ((px.x() > dimxy) || (px.y() > dimxy))
        throw std::out_of_range(_("Invalid pixel position"));

    // Unit conversion
    florb::point2d<double> mdeg(
            (360.0/((double)dimxy/(double)px.x())),
            (360.0/((double)dimxy/(double)px.y())));

    // Convert mercator to GPS coordinate
    return merc2wsg84(mdeg);
}

florb::point2d<double> florb::utils::merc2wsg84(const florb::point2d<double>& wsg84)
{
    return florb::point2d<double>( 
            wsg84.x() - 180.0,
            -((180.0/M_PI) * (2.0 * atan(exp((wsg84.y()-180.0)*M_PI/180.0)) - M_PI/2.0)));
}

florb::point2d<double> florb::utils::px2merc(unsigned int z, const florb::point2d<unsigned long> &px)
{
    unsigned long dimxy = dim(z);

    // Make sure the coordinate is on the map
    if ((px.x() >= dimxy) || (px.y() >= dimxy))
    {
        throw std::out_of_range(_("Invalid pixel position"));
    }

    double x = (px.x() == 0) ? 0.0 : (360.0/((double)(dimxy-1)/(double)px.x()));
    double y = (px.y() == 0) ? 0.0 : (360.0/((double)(dimxy-1)/(double)px.y()));
    return florb::point2d<double>(x,y);
}

unsigned long florb::utils::dim(unsigned int z)
{
    return pow(2.0, z) * 256;
}

double florb::utils::dist(const florb::point2d<double> &p1, const florb::point2d<double> &p2)
{
    if (p1 == p2)
        return 0.0;

    double d2r = (M_PI/180.0);
    double lon1 = p1.x()*d2r;
    double lat1 = p1.y()*d2r;
    double lon2 = p2.x()*d2r;
    double lat2 = p2.y()*d2r;

    double ret = (6378.388 * acos(sin(lat1) * sin(lat2) + cos(lat1) * cos(lat2) * cos(lon2 - lon1)));

    return (std::isnan(ret) > 0) ? 0.0 : ret;
}

double florb::utils::dist_merc(const florb::point2d<double> &p1, const florb::point2d<double> &p2)
{
    if (p1 == p2)
        return 0.0;

    double dstmerc = sqrt( pow(std::abs(p1.x()-p2.x()), 2.0) + pow(std::abs(p1.y()-p2.y()), 2.0)*0.9444444 );
    return (dstmerc * (CIRCUMFERENCEKM/360.0));
}

double florb::utils::meters_per_pixel(unsigned int z, double lat)
{
    return (CIRCUMFERENCEKM * cos(lat*(M_PI/180.0)) / pow(2.0,(z+8))) * 1000.0;
}

time_t florb::utils::iso8601_2timet(const std::string& iso)
{
    struct tm stm;
    strptime(iso.c_str(), "%FT%T%z", &stm);

    return mktime(&stm);
}

std::string florb::utils::timet2iso8601(time_t t)
{
    char buf[sizeof "2011-10-08T07:07:09Z"];
    strftime(buf, sizeof buf, "%FT%TZ", gmtime(&t));

    return std::string(buf);
}

std::string florb::utils::pathsep()
{
#if defined(WIN32) || defined(_WIN32) 
return std::string("\\");
#else 
return std::string("/");
#endif 
}

std::string florb::utils::userdir()
{
    char *home = getenv("HOME");
    if (!home)
        throw 0;

    return std::string(home);
}

std::string florb::utils::appdir()
{
    std::ostringstream oss;
    oss << userdir();
    oss << pathsep();
    oss << ".florb";

    return oss.str();
}

bool florb::utils::mkdir(const std::string& path)
{
    bool ret = true;

    try {
        boost::filesystem::create_directory(path);
    } catch (...) {
        ret = false;
    }

    return ret;
}

void florb::utils::rm(const std::string& path)
{
    boost::filesystem::remove_all(path);
}

bool florb::utils::exists(const std::string& path)
{
    return boost::filesystem::exists(path);
}

std::string florb::utils::filestem(const std::string& path)
{
    return boost::filesystem::path(path).stem().string();
}

std::string florb::utils::extension(const std::string& path)
{
    return boost::filesystem::path(path).extension().string();
}

void florb::utils::touch(const std::string& path)
{
    std::fstream f(path, std::ios::out|std::ios::app);
	f.close();
}

void florb::utils::set_window_icon(Fl_Window *w)
{
    fl_open_display();
    Pixmap p, mask;
    XpmCreatePixmapFromData(fl_display, DefaultRootWindow(fl_display), const_cast<char**>(florb_xpm), &p, &mask, NULL);
    w->icon((char *)p);
}

std::vector<std::string> florb::utils::str_split(const std::string& str, const std::string& delimiter)
{
    std::size_t offs = 0, p1 = 0, p2 = std::string::npos;
    std::size_t len = str.length();

    std::vector<std::string> ret;
    while ((offs < len) && (p2 = str.find(delimiter, offs)) != std::string::npos)
    {
        std::string token(str.substr(p1, p2-p1));

        if (token.length() > 0)
            ret.push_back(str.substr(p1, p2-p1));
    
        p1 = p2 + delimiter.length();
        offs = p1;
    }

    if ((p1 != len))
        ret.push_back(str.substr(p1, len-p1));

    return ret;
}

std::size_t florb::utils::str_count(const std::string& str, const std::string& token)
{
    if (token.length() == 0)
        return 0;

    std::size_t ret = 0, offs = 0;

    while ((offs = str.find(token, offs)) != std::string::npos)
    {
        offs += token.length();
        ret++;
    }

    return ret;
}

void florb::utils::str_replace(std::string& str, const std::string& s, const std::string& r)
{
    std::size_t offs = 0;
    while ((offs = str.find(s, 0)) != std::string::npos)
    {
        str.replace(offs, s.length(), r);
    }
}

// Cohen–Sutherland clipping algorithm
bool florb::utils::clipline(
        florb::point2d<double> &p1, 
        florb::point2d<double> &p2, 
        const florb::point2d<double> &r1, 
        const florb::point2d<double> &r2, 
        bool &p1clip, 
        bool &p2clip)
{
    bool ret = false;
    double xmin, xmax, ymin, ymax;
    p1clip = false;
    p2clip = false;

    if (r1.x() > r2.x())
    {
        xmin = r2.x();
        xmax = r1.x();
    }
    else
    {
        xmin = r1.x();
        xmax = r2.x();
    }

    if (r1.y() > r2.y())
    {
        ymin = r2.y();
        ymax = r1.y();
    }
    else
    {
        ymin = r1.y();
        ymax = r2.y();
    } 

    bool overflowa = false;

    // Catch arithmetic overflow when calculating m and n
    double n = 0, m = 0;
    for (;;)
    {
        std::feclearexcept(FE_OVERFLOW|FE_UNDERFLOW|FE_DIVBYZERO);
        
        m = (p2.y() - p1.y()) / (p2.x() - p1.x());
        if (std::fetestexcept(FE_OVERFLOW|FE_UNDERFLOW|FE_DIVBYZERO) != 0)
        {
            overflowa = true;
            break;
        }

        std::feclearexcept(FE_OVERFLOW|FE_UNDERFLOW|FE_DIVBYZERO);

        n = p1.y() - (m * p1.x());
        if (std::fetestexcept(FE_OVERFLOW|FE_UNDERFLOW|FE_DIVBYZERO) != 0)
        {
            overflowa = true;
            break;
        }

        break;
    }

    // Max 2 clipping operations per point and one last check operation. 
    for (int i=0;i<5;i++)
    {
        // Compute code for both points
        int code1 = 0, code2 = 0;
        
        if (p1.x() < xmin)
            code1 |= CLIPLEFT;
        if (p1.x() > xmax)
            code1 |= CLIPRIGHT;
        if (p1.y() < ymin)
            code1 |= CLIPTOP;
        if (p1.y() > ymax)
            code1 |= CLIPBOTTOM;

        if (p2.x() < xmin)
            code2 |= CLIPLEFT;
        if (p2.x() > xmax)
            code2 |= CLIPRIGHT;
        if (p2.y() < ymin)
            code2 |= CLIPTOP;
        if (p2.y() > ymax)
            code2 |= CLIPBOTTOM;

        // Both inside, draw line
        if ((code1 | code2) == 0)
        {
            ret = true;
            break;
        }

        // Both top, bottom, left or right outside, line need not be drawn
        if ((code1 & code2) != 0)
        {
            ret = false;
            break;
        }

        // Pick an endpoint for clipping
        florb::point2d<double> &ptmp = (code1 != 0) ? p1 : p2;
        int codetmp;
        if (code1 != 0)
        {
            codetmp = code1;
            p1clip = true;
        }
        else
        {
            codetmp = code2;
            p2clip = true;
        }

        // Clip top
        if (codetmp & CLIPTOP)
        {
            // This is what the following code does with just a little overflow
            // protection:
            // ptmp[0] = (ymin - n) / m; 

            double cx;
            for (;!overflowa;)
            {
                std::feclearexcept(FE_OVERFLOW|FE_UNDERFLOW|FE_DIVBYZERO);
                cx = (ymin - n) / m;

                if (std::fetestexcept(FE_OVERFLOW|FE_UNDERFLOW|FE_DIVBYZERO) != 0)
                {
                    overflowa = true;
                    break;
                }

                break;
            }

            // In case of an arithmetic exception we assume a vertical line and
            // keep the original x coordinate
            if (!overflowa)
                ptmp[0] = cx;

            ptmp[1] = ymin;
        }
        // Clip Bottom
        else if (codetmp & CLIPBOTTOM)
        {
            // This is what the following code does with just a little overflow
            // protection:
            // ptmp[0] = (ymax - n) / m; 

            double cx;
            for (;!overflowa;)
            {
                std::feclearexcept(FE_OVERFLOW|FE_UNDERFLOW|FE_DIVBYZERO);
                cx = (ymax - n) / m;

                if (std::fetestexcept(FE_OVERFLOW|FE_UNDERFLOW|FE_DIVBYZERO) != 0)
                {
                    overflowa = true;
                    break;
                }

                break;
            }

            // In case of an arithmetic exception we assume a vertical line and
            // keep the original x coordinate
            if (!overflowa)
                ptmp[0] = cx;

            ptmp[1] = ymax;
        }
        // Clip Left
        else if (codetmp & CLIPLEFT)
        {
            // Vertical line left outside
            if (overflowa)
            {
                ret = false;
                break;
            }
                
            ptmp[0] = xmin; 
            ptmp[1] = m * xmin + n;
        }
        // Clip Right
        else if (codetmp & CLIPRIGHT)
        {
            // Vertical line right outside
            if (overflowa)
            {
                ret = false;
                break;
            }

            ptmp[0] = xmax; 
            ptmp[1] = m * xmax + n;
        }
    }

    return ret;
}

