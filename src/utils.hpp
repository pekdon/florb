#ifndef UTILS_HPP
#define UTILS_HPP

#include <ctime>
#include <vector>
#include <sstream>
#include <libintl.h>
#include <FL/Fl_Window.H>
#include <boost/lexical_cast.hpp>
#include "point.hpp"

#define _(STRING) gettext(STRING)

namespace florb
{
    class utils
    {
        public:
            static unsigned long dim(unsigned int z);

            static florb::point2d<double> wsg842merc(const florb::point2d<double> &wsg84);
            static florb::point2d<unsigned long> merc2px(unsigned int z, const florb::point2d<double> &merc);
            static florb::point2d<unsigned long> wsg842px(unsigned int z, const florb::point2d<double> &wsg84);
            static florb::point2d<double> px2wsg84(unsigned int z, const florb::point2d<unsigned long> &px);
            static florb::point2d<double> merc2wsg84(const florb::point2d<double>& wsg84);
            static florb::point2d<double> px2merc(unsigned int z, const florb::point2d<unsigned long> &px);

            static bool clipline(florb::point2d<double> &p1, florb::point2d<double> &p2, const florb::point2d<double> &r1, const florb::point2d<double> &r2, bool &p1clip, bool &p2clip); 

            static double dist(const florb::point2d<double> &p1, const florb::point2d<double> &p2);
            static double dist_merc(const florb::point2d<double> &p1, const florb::point2d<double> &p2);
            static double meters_per_pixel(unsigned int z, double lat);

            static time_t iso8601_2timet(const std::string& iso);
            static std::string timet2iso8601(time_t t);

            static std::vector<std::string> str_split(const std::string& str, const std::string& delimiter);
            static std::size_t str_count(const std::string& str, const std::string& token);
            static void str_replace(std::string& str, const std::string& s, const std::string& r);

            template <class T>
                static bool fromstr(const std::string& s, T& out)
                {
                    try {
                        out = boost::lexical_cast<T>(s);
                    } catch (const boost::bad_lexical_cast &e) {
                        return false;
                    }

                    return true;
                };

            static std::string pathsep();
            static std::string userdir();
            static std::string appdir();
            static bool mkdir(const std::string& path);
            static void rm(const std::string& path);
            static bool exists(const std::string& path);
            static std::string filestem(const std::string& path);
            static std::string extension(const std::string& path);
            static void touch(const std::string& path);
        private:
    };

};

#endif // UTILS_HPP
