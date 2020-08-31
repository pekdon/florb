#include <FL/fl_ask.H>
#include <sstream>
#include <iostream>
#include "utils.hpp"
#include "shell.hpp"
#include "dlg_garminul.h"

void dlg_garminul::create_ex()
{
    // Set the window icon
    florb::utils::set_window_icon(m_window); 
}

void dlg_garminul::show_ex()
{
    m_choice_device->clear();

    florb::shell sh;
    sh.run("gpsbabel -i garmin -f usb:-1");
    std::string tmp;
    while(sh.readln(tmp))
    {
        // Very basic gpsbabel output validation (check first item integer)
        int conv;
        if (!florb::utils::fromstr(florb::utils::str_split(tmp, " ")[0], conv))
            break;

        m_choice_device->add(tmp.c_str());
    }

    sh.wait();

    if (m_choice_device->size() == 0)
    {
        m_choice_device->add(_("No devices found"));
        m_btn_upload->deactivate();
        m_input_title->deactivate();
        m_choice_device->deactivate();
    }
    else
    {
        m_btn_upload->activate();
        m_input_title->activate();
        m_choice_device->activate();
    }

    m_choice_device->value(0);
    m_input_title->value(m_name.c_str());

    // Show the window
    m_window->show();
    for (;;) {
        Fl_Widget *o = Fl::readqueue();
        if (!o) Fl::wait();
        else if (o == m_btn_ok)     {break;}
        else if (o == m_window)     {break;}
    }

    m_window->hide();
}

void dlg_garminul::cb_btn_upload_ex(Fl_Widget *widget)
{
    m_btn_upload->deactivate();
    m_btn_ok->deactivate();
    m_input_title->deactivate();
    m_choice_device->deactivate();

    std::ostringstream os;
    int idx = m_choice_device->value();
    os << "gpsbabel -t -i gpx -f " << m_path << " -x track,title=\"" << m_input_title->value() << "\" -o garmin -F usb:" << idx;

    florb::shell sh;
    sh.run(os.str());

    std::string line;
    Fl::check();
    bool ok = sh.wait();

    m_btn_upload->activate();
    m_btn_ok->activate();
    m_input_title->activate();
    m_choice_device->activate();

    if (ok)
        fl_message("%s", _("Upload OK"));
    else
        fl_message("%s", _("Upload Failed!"));
}

void dlg_garminul::cb_input_title_ex(Fl_Widget *widget)
{
    if (strlen(m_input_title->value()) <= 0)
    {
        m_btn_upload->deactivate();
    }
    else
    {
        m_btn_upload->activate();
    }
}
