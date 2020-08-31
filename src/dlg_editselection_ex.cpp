#include <FL/fl_ask.H>
#include "settings.hpp"
#include "unit.hpp"
#include "dlg_editselection.h"

void dlg_editselection::create_ex()
{
    // Set the window icon
    florb::utils::set_window_icon(m_window); 
}

void dlg_editselection::show_ex()
{
    florb::cfg_units cfgunits = florb::settings::get_instance()["units"].as<florb::cfg_units>();

    std::vector<florb::tracklayer::waypoint> waypoints;
    m_wgtmap->gpx_selection_get(waypoints);

    std::ostringstream os;
    os.precision(8);
    os.setf(std::ios::fixed, std::ios::floatfield);

    os.str("");
    os << waypoints[0].lon();
    m_txtin_lon->value(os.str().c_str());

    os.str("");
    os << waypoints[0].lat();
    m_txtin_lat->value(os.str().c_str());

    os.str("");
    os << _("Elevation");

    // Perform unit conversion for elevation and set the box label accordingly
    double ele = waypoints[0].elevation();
    std::string elelabel;
    switch (cfgunits.system_length())
    {
        case (florb::cfg_units::system::IMPERIAL):
        case (florb::cfg_units::system::NAUTICAL):
            ele = florb::unit::convert(florb::unit::length::M, florb::unit::length::FOOT, ele);
            os << " [" << florb::unit::sistr(florb::unit::FOOT) << "]";
            elelabel = os.str();
            m_box_elevation->label(elelabel.c_str());
            break;
        default:
            os << " [" << florb::unit::sistr(florb::unit::M) << "]";
            elelabel = os.str();
            m_box_elevation->label(elelabel.c_str());
            break;
    }

    os.str("");
    os << ele;
    m_txtin_ele->value(os.str().c_str());

    if (waypoints.size() > 1)
    {
        m_txtin_lon->value("-");
        m_txtin_lat->value("-");
        m_txtin_lon->deactivate();
        m_txtin_lat->deactivate();
        m_box_lon->deactivate();
        m_box_lat->deactivate();
    }
    else
    {
        m_txtin_lon->activate();
        m_txtin_lat->activate();
        m_box_lon->activate();
        m_box_lat->activate();
    }

    m_window->show();

    for (;;) {
        Fl_Widget *o = Fl::readqueue();
        if (!o) Fl::wait();
        else if (o == m_btn_ok)     
        {
            if (handle_ok_ex(waypoints))
                break;
        }
        else if (o == m_btn_cancel) {break;}
        else if (o == m_window)     {break;}
    }

    m_window->hide();
}

bool dlg_editselection::handle_ok_ex(std::vector<florb::tracklayer::waypoint>& waypoints)
{
    florb::cfg_units cfgunits = florb::settings::get_instance()["units"].as<florb::cfg_units>();

    // Perform unit conversion for elevaton if necessary
    double ele = 0;
    florb::utils::fromstr(m_txtin_ele->value(), ele);

    switch (cfgunits.system_length())
    {
        case (florb::cfg_units::system::IMPERIAL):
        case (florb::cfg_units::system::NAUTICAL):
            ele = florb::unit::convert(florb::unit::length::FOOT, florb::unit::length::M, ele);
            break;
        default:
            break;
    }

    if (waypoints.size() > 1)
    {
        // Update elevation only, common for every selected
        // waypoint
        std::vector<florb::tracklayer::waypoint>::iterator it;
        for (it=waypoints.begin();it!=waypoints.end();++it)
        {
            (*it).elevation(ele);
        }
    }
    else
    {
        double lat = 0, lon = 0;
        florb::utils::fromstr(m_txtin_lon->value(), lon);
        florb::utils::fromstr(m_txtin_lat->value(), lat);

        waypoints[0].lon(lon);
        waypoints[0].lat(lat);
        waypoints[0].elevation(ele);
    }

    try {
        m_wgtmap->gpx_selection_set(waypoints);
    } catch (std::out_of_range& e) {
        fl_alert("%s", e.what());
        return false;
    } catch (std::runtime_error& e) {
        fl_alert("%s", e.what());
        return false;
    }

    return true;
}

