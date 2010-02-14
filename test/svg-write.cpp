#include "libroad/hwm_network.hpp"

int main(int argc, char *argv[])
{
    std::cerr << libroad_package_string() << std::endl;
    if(argc < 3)
    {
        std::cerr << "Usage: " << argv[0] << " <input network> <output svg>" << std::endl;
        return 1;
    }
    hwm::network net(hwm::load_xml_network(argv[1], vec3f(1.0, 1.0, 15.0f)));
    net.build_intersections();
    net.build_fictitious_lanes();
    net.auto_scale_memberships();
    net.center();
    std::cerr << "HWM net loaded successfully" << std::endl;

    if(net.check())
        std::cerr << "HWM net checks out" << std::endl;
    else
    {
        std::cerr << "HWM net doesn't check out" << std::endl;
        exit(1);
    }

    net.svg_write(argv[2], hwm::network::SVG_ROADS);

    return 0;
}
