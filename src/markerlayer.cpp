#include <cmath>
#include <sstream>
#include "gfx.hpp"
#include "utils.hpp"
#include "settings.hpp"
#include "point.hpp"
#include "markerlayer.hpp"

florb::markerlayer::markerlayer()
{
    name(std::string("Marker"));
};

florb::markerlayer::~markerlayer()
{
};

size_t florb::markerlayer::add(const florb::point2d<double> &pmerc)
{
    size_t i = 0;
    for (;i<m_markers.size();i++)
    {
        bool found = false;

        std::vector<marker_internal>::iterator it;
        for (it=m_markers.begin();it!=m_markers.end();++it)
        {
            if ((*it).id == i)
            {
                found = true;
                break;
            }
        }

        if (!found)
            break;
    }

    add(pmerc, i);
    return i;
};

void florb::markerlayer::add(const florb::point2d<double> &pmerc, size_t id)
{
    marker_internal tmp;
    tmp.p = pmerc;
    tmp.id = id;

    m_markers.push_back(tmp);
    notify();
};

void florb::markerlayer::remove(size_t id)
{
    std::vector<marker_internal>::iterator it;
    for (it=m_markers.begin();it!=m_markers.end();++it)
    {
        if ((*it).id == id)
        {
            it = --(m_markers.erase(it));
        }
    } 

    notify();
};

void florb::markerlayer::clear() 
{
    m_markers.clear();
    notify();
};

void florb::markerlayer::notify()
{
    event_notify e;
    fire(&e);
}

bool florb::markerlayer::draw(const viewport &viewport, florb::canvas &os)
{
    florb::cfg_ui cfgui = florb::settings::get_instance()["ui"].as<florb::cfg_ui>();

    std::vector<marker_internal>::iterator it;
    for (it=m_markers.begin();it!=m_markers.end();++it)
    {
        florb::point2d<unsigned long> ppx(florb::utils::merc2px(viewport.z(), (*it).p));

        if (ppx.x() < viewport.x())
            continue;
        if (ppx.x() >= (viewport.x()+viewport.w()))
            continue;
        if (ppx.y() < viewport.y())
            continue;
        if (ppx.y() >= (viewport.y()+viewport.h()))
            continue;

        ppx[0] -= viewport.x();
        ppx[1] -= viewport.y();

        os.fgcolor(cfgui.gpscursorcolor());
        os.line(ppx.x()-12, ppx.y(), ppx.x()+12, ppx.y(), 1);
        os.line(ppx.x(), ppx.y()-12, ppx.x(), ppx.y()+12, 1);

        os.circle(ppx.x(), ppx.y(), 5);
        os.circle(ppx.x(), ppx.y(), 6);
    }

    return true;
};

