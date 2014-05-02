#include <yaml-cpp/yaml.h>
#include <fstream>
#include "settings.hpp"

namespace YAML {
    template<>
        struct convert<cfg_tileserver> {
            static Node encode(const cfg_tileserver& rhs) {
                Node node;
                node["name"] = rhs.name();
                node["url"] = rhs.url();
                node["zmin"] = rhs.zmin();
                node["zmax"] = rhs.zmax();
                node["parallel"] = rhs.parallel();

                node["type"] = "PNG";
                if      (rhs.type() == image::PNG)
                    node["type"] = "PNG";
                else if (rhs.type() == image::JPG)
                    node["type"] = "JPG";

                return node;
            }

            static bool decode(const Node& node, cfg_tileserver& rhs) 
            {
                if((!node.IsMap()) || (node.size() == 0))
                    return true;
                
                if (node["name"])
                    rhs.name(node["name"].as<std::string>());

                if (node["url"])
                    rhs.url(node["url"].as<std::string>());

                if (node["zmin"])
                    rhs.zmin(node["zmin"].as<int>());
                
                if (node["zmax"])
                    rhs.zmax(node["zmax"].as<int>());
                
                if (node["parallel"])
                    rhs.parallel(node["parallel"].as<int>());

                if (node["type"])
                {
                    rhs.type(image::PNG);
                    std::string imgtype = node["type"].as<std::string>();
                    if      (imgtype.compare("PNG") == 0)
                        rhs.type(image::PNG);
                    else if (imgtype.compare("JPG") == 0)
                        rhs.type(image::JPG);
                }

                return true;
            }
        };

    template<>
        struct convert<cfg_cache> {
            static Node encode(const cfg_cache& rhs) {
                Node node;
                node["location"] = rhs.location();
                return node;
            }

            static bool decode(const Node& node, cfg_cache& rhs) 
            {
                if((!node.IsMap()) || (node.size() == 0))
                    return true;

                if (node["location"])
                    rhs.location(node["location"].as<std::string>());
                
                return true;
            }
        };

    template<>
        struct convert<cfg_gpsd> {
            static Node encode(const cfg_gpsd& rhs) {
                Node node;
                node["enabled"] = rhs.enabled();
                node["host"] = rhs.host();
                node["port"] = rhs.port();
                return node;
            }

            static bool decode(const Node& node, cfg_gpsd& rhs) 
            {
                if((!node.IsMap()) || (node.size() == 0))
                    return true;

                if (node["enabled"])
                    rhs.enabled(node["enabled"].as<bool>());
                
                if (node["host"])
                    rhs.host(node["host"].as<std::string>());
                
                if (node["port"])
                    rhs.port(node["port"].as<std::string>());
                
                return true;
            }
        };

    template<>
        struct convert<cfg_ui> {
            static Node encode(const cfg_ui& rhs) {
                Node node;
                node["markercolor"] = rhs.markercolor().rgb();
                node["markercolorselected"] = rhs.markercolorselected().rgb();
                node["trackcolor"] = rhs.trackcolor().rgb();
                node["selectioncolor"] = rhs.selectioncolor().rgb();
                node["gpscursorcolor"] = rhs.gpscursorcolor().rgb();
                node["tracklinewidth"] = rhs.tracklinewidth();
                return node;
            }

            static bool decode(const Node& node, cfg_ui& rhs) 
            {
                // Use defaults
                if((!node.IsMap()) || (node.size() == 0))
                {
                    return true;
                }

                if (node["markercolor"])
                    rhs.markercolor(color(node["markercolor"].as<unsigned int>()));
                
                if (node["markercolorselected"])
                    rhs.markercolorselected(color(node["markercolorselected"].as<unsigned int>()));

                if (node["trackcolor"])
                    rhs.trackcolor(color(node["trackcolor"].as<unsigned int>()));

                if (node["selectioncolor"])
                    rhs.selectioncolor(color(node["selectioncolor"].as<unsigned int>()));

                if (node["gpscursorcolor"])
                    rhs.gpscursorcolor(color(node["gpscursorcolor"].as<unsigned int>()));
                
                if (node["tracklinewidth"])
                    rhs.tracklinewidth(node["tracklinewidth"].as<unsigned int>());
                
                return true;
            }
        };
}

class yaml_node
{
    public:
        yaml_node(YAML::Node n) : m_node(n) {};
        yaml_node(const std::string& path)
        {
            // Touch cfgfile in case it doesn't exist
            utils::touch(path);
            m_node = YAML::LoadFile(path);
        }
        ~yaml_node() {};

        YAML::Node& get() { return m_node; };
    private:
        YAML::Node m_node;  
};

class yaml_iterator
{
    public:
        yaml_iterator(YAML::iterator i) : m_iter(i) {};
        ~yaml_iterator() {};

        YAML::iterator& get() { return m_iter; };
    private:
        YAML::iterator m_iter;  
};

node::iterator::iterator(const node& n, int be) 
{
    if (be <= 0)
        m_ref = new yaml_iterator(n.m_ref->get().begin());
    else
        m_ref = new yaml_iterator(n.m_ref->get().end());
}

node::iterator::~iterator()
{   
    delete m_ref;
}

bool node::iterator::operator==(iterator const& rhs) const 
{
    return (m_ref->get() == rhs.m_ref->get());
}

bool node::iterator::operator!=(iterator const& rhs) const 
{
    return !(m_ref->get() == rhs.m_ref->get());
}

node::iterator& node::iterator::operator++() 
{
    (m_ref->get())++;
    return *this;
}   

node::iterator node::iterator::operator++(int) 
{
    iterator tmp (*this);
    ++(*this);
    return tmp;
}

// Bidirectional iterators are not supported by YAML-Cpp
#if 0
iterator& node::iterator::operator--() 
{
    (m_ref.get())--;
    return *this;
}

iterator node::iterator::operator--(int) 
{
    iterator tmp (*this);
    --(*this);
    return tmp;
}
#endif

node node::iterator::operator* () const 
{
    yaml_node *tmp = new yaml_node(*(m_ref->get()));
    return node(tmp);
}

settings::settings()
{
    m_cfgfile = utils::appdir() + utils::pathsep() + "config";
    utils::mkdir(utils::appdir());

    m_rootnode = node(m_cfgfile);
    defaults(m_cfgfile);
}

settings::~settings()
{
    m_rootnode.serialize(m_cfgfile);
}

settings& settings::get_instance()
{
    static settings instance;
    return instance;
}

void settings::defaults(const std::string& path)
{
    // Tileserver default configuration
    cfg_tileserver cfgtileserver;
    if((!m_rootnode["tileservers"]) || (!m_rootnode["tileservers"].is_sequence()))
    {
        std::vector<cfg_tileserver> v;
        v.push_back(cfgtileserver);

        m_rootnode["tileservers"] = v;
    }

    // GPSd default configuration
    cfg_gpsd cfggpsd = m_rootnode["gpsd"].as<cfg_gpsd>();
    m_rootnode["gpsd"] = cfggpsd;

    // Cache default configuration
    cfg_cache cfgcache = m_rootnode["cache"].as<cfg_cache>();
    m_rootnode["cache"] = cfgcache;

    // UI default configuration
    cfg_ui cfgui = m_rootnode["ui"].as<cfg_ui>();
    m_rootnode["ui"] = cfgui;
}

node::node(const std::string& path)
{
    m_ref = new yaml_node(path);
}

node::node(const node& n)
{
    m_ref = new yaml_node(n.m_ref->get());
}

node::node()
{
    m_ref = new yaml_node(YAML::Node());
}

node::~node()
{
    if (m_ref)
        delete m_ref;
}

bool node::is_sequence()
{
    return m_ref->get().IsSequence();
}

node node::operator[] (const int idx)
{
    return node(new yaml_node(m_ref->get()[idx]));
}

node node::operator[] (const std::string &name)
{
    return node(new yaml_node(m_ref->get()[name]));
}

node& node::operator= (const node& n)
{
    if (m_ref)
        delete m_ref;

    m_ref = new yaml_node(n.m_ref->get());

    return *this;
}

template<typename T> node& node::operator= (const T& rhs)
{
    m_ref->get() = rhs;
    return *this;
}

node::operator bool() const 
{
    return m_ref->get();
}

template<typename T> T node::as() const
{
    return m_ref->get().as<T>();
}

template<typename T> void node::push_back(const T& rhs)
{
    m_ref->get().push_back<T>(rhs);
}

void node::push_back(const node& rhs)
{
    m_ref->get().push_back(rhs.m_ref->get());
}

void node::serialize(const std::string& path)
{
    std::ofstream fout(path.c_str(), std::fstream::in|std::fstream::out|std::fstream::trunc);
    fout << m_ref->get();
}

size_t node::size()
{
    return m_ref->get().size();
}

// This is necessary so YAML::Node template internals remain hidden from the
// header file
template int node::as<int>() const;
template std::string node::as<std::string>() const;
template cfg_tileserver node::as<cfg_tileserver>() const;
template std::vector< cfg_tileserver > node::as< std::vector<cfg_tileserver> >() const;
template bool node::as<bool>() const;
template cfg_cache node::as<cfg_cache>() const;
template cfg_gpsd node::as<cfg_gpsd>() const;
template cfg_ui node::as<cfg_ui>() const;

template void node::push_back<int>(const int& rhs);
template void node::push_back<std::string>(const std::string& rhs);
template void node::push_back<cfg_tileserver>(const cfg_tileserver& rhs);

template node& node::operator=<int> (const int& rhs);
template node& node::operator=< std::vector<int> > (const std::vector<int>& rhs);
template node& node::operator=<std::string> (const std::string& rhs);
template node& node::operator=< std::vector<cfg_tileserver> > (const std::vector<cfg_tileserver>& rhs);
template node& node::operator=<cfg_tileserver> (const cfg_tileserver& rhs);
template node& node::operator=<bool> (const bool& rhs);
template node& node::operator=<cfg_gpsd> (const cfg_gpsd& rhs);
template node& node::operator=<cfg_ui> (const cfg_ui& rhs);

