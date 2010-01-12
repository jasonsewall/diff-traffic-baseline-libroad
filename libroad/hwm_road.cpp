#include "hwm_network.hpp"

namespace hwm
{
    bool road::check() const
    {
        return !id.empty() && rep.check();
    }

    void road::translate(const vec3f &o)
    {
        rep.translate(o);
    }

    void road::bounding_box(vec3f &low, vec3f &high) const
    {
        return rep.bounding_box(low, high);
    }
}
