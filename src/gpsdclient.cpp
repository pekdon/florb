#include "config.h"

#include "gpsdclient.hpp"
#include "utils.hpp"

florb::gpsdclient::gpsdclient(const std::string host, const std::string port) : 
    m_host(host),
    m_port(port),
    m_thread(NULL),
    m_exit(false),
    m_connected(false),
    m_mode(FIX_NONE),
    m_pos(0.0, 0.0),
    m_track(0.0)
{
    m_thread = new boost::thread(boost::bind(&florb::gpsdclient::worker, this));
    if (!m_thread)
        throw std::runtime_error(_("GPSd error"));
}

florb::gpsdclient::~gpsdclient()
{
    if (m_thread) 
    {
        exit(true);
        m_thread->join();
        delete m_thread;
    }
}

bool florb::gpsdclient::exit()
{
    m_mutex.lock();
    bool ret = m_exit;
    m_mutex.unlock();

    return ret;
}
void florb::gpsdclient::exit(bool e)
{
    m_mutex.lock();
    m_exit = e;
    m_mutex.unlock();
}

void florb::gpsdclient::connected(bool c)
{
    m_mutex.lock();

    if (!c)
    {
        m_mode = FIX_NONE;
        m_track = 0.0;
        m_pos.x(0.0);
        m_pos.y(0.0);
    }

    m_connected = c;

    m_mutex.unlock();
}

bool florb::gpsdclient::connected(void)
{
    m_mutex.lock();
    bool ret = m_connected;
    m_mutex.unlock();

    return ret;   
}

void florb::gpsdclient::mode(int m)
{
    m_mutex.lock();

    if (m == FIX_NONE)
    {
        m_track = 0.0;
        m_pos.x(0.0);
        m_pos.y(0.0);
    }

    m_mode = m;

    m_mutex.unlock();
}

int florb::gpsdclient::mode(void)
{
    m_mutex.lock();
    int ret = m_mode;
    m_mutex.unlock();

    return ret;   
}

void florb::gpsdclient::track(double t)
{
    m_mutex.lock();
    m_track = t;
    m_mutex.unlock();
}

double florb::gpsdclient::track(void)
{
    m_mutex.lock();
    double ret = m_track;
    m_mutex.unlock();

    return ret;   
}

void florb::gpsdclient::pos(const florb::point2d<double>& p)
{
    m_mutex.lock();
    m_pos = p;
    m_mutex.unlock();
}

void florb::gpsdclient::pos(double lon, double lat)
{
    m_mutex.lock();
    m_pos.x(lon);
    m_pos.y(lat);
    m_mutex.unlock();
}


florb::point2d<double> florb::gpsdclient::pos(void)
{
    m_mutex.lock();
    florb::point2d<double> ret(m_pos);
    m_mutex.unlock();

    return ret;   
}

void florb::gpsdclient::fire_event_gpsd(void)
{
    florb::gpsdclient::event_gpsd ge(m_connected, m_mode, m_pos, m_track);
    fire(&ge);
}

void florb::gpsdclient::worker(void)
{
#ifdef HAVE_LIBGPS
    int rc;
    for (;;)
    {
        rc = gps_open(m_host.c_str(), m_port.c_str(), &m_gpsdata);
        if (rc < 0)
            break;
        rc = gps_stream(&m_gpsdata, WATCH_ENABLE | WATCH_JSON, NULL);
        if (rc < 0)
            break;

        break;
    }

    // Update connection status
    if (rc == 0) {
        connected(true);
        fire_event_gpsd();
    }

    for (;;)
    {
        // Exit request
        if (exit())
            break;

        // Initialisation failure
        if (rc != 0)
            break;

        // Wait for data, 200ms timeout
        if (!gps_waiting(&m_gpsdata, 200000)) {
            // Error
            if (errno != 0)
                break;

            // Normal timeout
            continue;
        }

        // Read data
#if GPSD_API_MAJOR_VERSION >= 7
        if (gps_read(&m_gpsdata, NULL, 0) == -1) {
#else
        if (gps_read(&m_gpsdata) == -1) {
#endif
            // Error
            break;
        }

        // Handle received data
        handle_set();
    }

    // Try to shut down if the previous initialisation step didn't fail
    if (rc == 0)
    {
        gps_stream(&m_gpsdata, WATCH_DISABLE, NULL);
        gps_close(&m_gpsdata);
    }
#endif // HAVE_LIBGPS

    // Update connection status
    connected(false);
    fire_event_gpsd();
}

bool florb::gpsdclient::handle_set()
{
    bool ret = false;
#ifdef HAVE_LIBGPS

    for (;;) {

        // Mode update / Position update
        if (m_gpsdata.set & MODE_SET)
        {
            int m = FIX_NONE;
            if (m_gpsdata.fix.mode == MODE_2D) 
                m = FIX_2D;
            else if (m_gpsdata.fix.mode == MODE_3D) 
                m = FIX_3D;

            if (m_mode != m)
            {
                ret = true;
                mode(m);
            }

            // Handle latitude / longitude set
            if (m_gpsdata.set & LATLON_SET)
            {
                // Handle latitude / longitude
                if ((m_gpsdata.fix.latitude != m_pos.y()) ||
                    (m_gpsdata.fix.longitude != m_pos.x())) 
                {
                    pos(m_gpsdata.fix.longitude, m_gpsdata.fix.latitude);
                    ret = true;
                }
            }
        }

        // Handle track / course over ground info
        if (m_gpsdata.set & TRACK_SET)
        {
            if (m_track != m_gpsdata.fix.track) {
                track(m_gpsdata.fix.track);
                ret = true;
            }
        }

        // FIre notification event if necessary.
        if (ret)
            fire_event_gpsd();

        break;
    }
#endif // HAVE_LIBGPS
    return ret;
}

