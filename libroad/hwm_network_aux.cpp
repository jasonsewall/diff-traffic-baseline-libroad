#include "hwm_network.hpp"

namespace hwm
{
    network_aux::road_rev_map::lane_entry::lane_entry()
    {
    }

    network_aux::road_rev_map::lane_entry::lane_entry(const hwm::lane* l, const lane::road_membership *rm) :
        lane(l), membership(rm)
    {
    }

    void network_aux::road_rev_map::lane_cont::cairo_draw(cairo_t *c , const vec2f &interval, const float lane_widith, bool low_side, bool start_new) const
    {
        const float                  left  = begin()            ->first-0.5f*lane_width;
        const float                  right = boost::prior(end())->first+0.5f*lane_width;
        const lane::road_membership &rm    = *(begin()->second.membership);

        const float offset = low_side ? left : right;
        rm.parent_road->rep.svg_arc_path_center(interval, offset).cairo_draw(c, start_new);
    }

    void network_aux::road_rev_map::lane_cont::make_mesh(std::vector<vertex> &vrts, std::vector<vec3u> &fcs, const vec2f &interval, const float lane_width) const
    {
        const float                  left  = begin()            ->first-0.5f*lane_width;
        const float                  right = boost::prior(end())->first+0.5f*lane_width;
        const lane::road_membership &rm    = *(begin()->second.membership);
        rm.parent_road->rep.make_mesh(vrts, fcs, interval, vec2f(left, right), 0.005, true);
    }

    network_aux::road_rev_map::road_rev_map()
    {
        lane_map.insert(0.0f, lane_cont());
    }

    network_aux::road_rev_map::road_rev_map(const hwm::road *r) : road(r)
    {
        lane_map.insert(0.0f, lane_cont());
    }

    void network_aux::road_rev_map::add_lane(const hwm::lane *r, const lane::road_membership *rm)
    {
        vec2f      interval(rm->interval);
        lane_entry le(r, rm);
        if(interval[0] > interval[1])
            std::swap(interval[0], interval[1]);

        partition01<lane_cont>::iterator start(lane_map.find(interval[0]));
        const vec2f                      start_interval(lane_map.containing_interval(start));

        start = lane_map.split_interval(start, vec2f(interval[0], std::min(interval[1], start_interval[1])), start->second);

        start->second.insert(std::make_pair(le.membership->lane_position, le));
        if(interval[1] <= start_interval[1])
            return;

        partition01<lane_cont>::iterator end(lane_map.find(interval[1]));
        const vec2f                      end_interval(lane_map.containing_interval(end));

        end = lane_map.split_interval(end, vec2f(end_interval[0], interval[1]), end->second);
        end->second.insert(std::make_pair(le.membership->lane_position, le));

        for(partition01<lane_cont>::iterator current = boost::next(start); current != end; ++current)
            current->second.insert(std::make_pair(le.membership->lane_position, le));
    }

    void network_aux::road_rev_map::print() const
    {
        for(partition01<lane_cont>::const_iterator current = lane_map.begin(); current != lane_map.end(); ++current)
        {
            std::cout << lane_map.containing_interval(current) << ": ";
            BOOST_FOREACH(const lane_cont::value_type &le, current->second)
            {
                std::cout << le.second.membership->lane_position << " " << le.second.lane->id;
                if(le.second.membership->interval[0] < le.second.membership->interval[1])
                    std::cout << "(+) ";
                else
                    std::cout << "(-) ";

            }
            std::cout << std::endl;
        }
    }

    network_aux::network_aux(const network &n)
        : net(n)
    {
        BOOST_FOREACH(const road_pair &r, net.roads)
        {
            rrm[r.first] = road_rev_map(&(r.second));
        }

        BOOST_FOREACH(const lane_pair &l, net.lanes)
        {
            BOOST_FOREACH(const lane::road_membership::intervals::entry &rm, l.second.road_memberships)
            {
                strhash<road_rev_map>::type::iterator rev_itr(rrm.find(rm.second.parent_road->id));
                assert(rev_itr != rrm.end());
                rev_itr->second.add_lane(&(l.second), &(rm.second));
            }
        }

        BOOST_FOREACH(const intersection_pair &i, n.intersections)
        {
            intersection_geoms[i.first] = intersection_geometry(&(i.second));
        }
    }

    static void write_intersection_mtl(std::ostream      &o,
                                       const std::string &ts_name)
    {
        o << boost::str(boost::format("newmtl %s\n"
                                      "ns 96.078431\n"
                                      "ka 0.0  0.0  0.0\n"
                                      "kd 0.0 0.0 0.0\n"
                                      "ks 0.5  0.5  0.5\n"
                                      "ni 1.0\n"
                                      "d  1.0\n") % ts_name);
    }

#if HAVE_CAIRO
    static void cairo_road(cairo_t *c, const network_aux::road_rev_map &rrm, const float lane_width, const bool closed)
    {
        cairo_new_path(c);
        for(partition01<network_aux::road_rev_map::lane_cont>::const_iterator current = rrm.lane_map.begin();
            current != rrm.lane_map.end();
            ++current)
        {
            const network_aux::road_rev_map::lane_cont &e = current->second;
            if(e.empty())
                continue;
            e.cairo_draw(c, rrm.lane_map.containing_interval(current), lane_width, false, !closed);
        }

        for(partition01<network_aux::road_rev_map::lane_cont>::const_reverse_iterator current = rrm.lane_map.rbegin();
            current != rrm.lane_map.rend();
            ++current)
        {
            const network_aux::road_rev_map::lane_cont &e = current->second;
            if(e.empty())
                continue;
            const vec2f cw(rrm.lane_map.containing_interval(current));
            e.cairo_draw(c, vec2f(cw[1], cw[0]), lane_width, true, !closed);
        }
        if(closed)
            cairo_close_path(c);
    }

    void network_aux::cairo_roads(cairo_t *c) const
    {
        cairo_set_line_cap(c, CAIRO_LINE_CAP_SQUARE);
        BOOST_FOREACH(const strhash<road_rev_map>::type::value_type &rrm_v, rrm)
        {
            cairo_road(c, rrm_v.second, net.lane_width, true);
            cairo_set_source_rgba(c, 237.0/255, 234.0/255, 186.0/255, 1.0);
            cairo_fill_preserve(c);
            cairo_stroke(c);
            cairo_road(c, rrm_v.second, net.lane_width, false);
            cairo_set_source_rgba(c, 135.0/255, 103.0/255, 61.0/255, 1.0);
            cairo_stroke(c);
        }

        BOOST_FOREACH(const strhash<intersection_geometry>::type::value_type &is_v, intersection_geoms)
        {
            is_v.second.cairo_draw(c, true);
            cairo_set_source_rgba(c, 237.0/255, 234.0/255, 186.0/255, 1.0);
            cairo_fill(c);
            is_v.second.cairo_draw(c, false);
            cairo_set_source_rgba(c, 135.0/255, 103.0/255, 61.0/255, 1.0);
            cairo_stroke(c);
        }
    }
#endif

    void network_aux::road_objs(std::ostream &os) const
    {
        const std::string mtlname("road.mtl");

        const bf::path texbase(bf::path(bf::current_path() / "tex"));

        std::ofstream mtllib(mtlname.c_str());
        write_intersection_mtl(mtllib, "intersection");

        os << "mtllib " << mtlname << std::endl;

        hwm::tex_db tdb(mtllib, texbase);

        BOOST_FOREACH(const strhash<road_rev_map>::type::value_type &rrm_v, rrm)
        {
            const hwm::road &r = *(rrm_v.second.road);

            size_t re_c = 0;
            for(partition01<road_rev_map::lane_cont>::const_iterator current = rrm_v.second.lane_map.begin();
                current != rrm_v.second.lane_map.end();
                ++current)
            {
                const road_rev_map::lane_cont &e = current->second;

                if(e.empty())
                    continue;

                std::vector<vertex> vrts;
                std::vector<vec3u>  fcs;
                e.make_mesh(vrts, fcs, rrm_v.second.lane_map.containing_interval(current), net.lane_width);

                const std::string oname(boost::str(boost::format("%s-%d") % r.id % re_c));

                const std::string texname(e.write_texture(tdb));
                mesh_to_obj(os,
                            oname,
                            texname,
                            vrts,
                            fcs);

                ++re_c;
            }
        }
    }

    struct road_winding
    {
        road_winding(const size_t rn, float th)
            : road_num(rn), theta(th)
        {}

        size_t road_num;
        float  theta;
    };

    struct circle_sort
    {
        bool operator()(const road_winding &l,
                        const road_winding &r) const
        {
            return l.theta < r.theta;
        }
    };

    typedef std::pair<float, network_aux::point_tan> offs_pt;

    static void gen_tan_points(offs_pt         &low, offs_pt &high,
                               const hwm::lane &l,
                               const bool       incomingp)
    {
        float param;
        float tan_sign;
        if(incomingp)
        {
            param    = 1.0f;
            tan_sign = 1.0f;
        }
        else
        {
            param    =  0.0f;
            tan_sign = -1.0f;
        }

        low.first = -tan_sign*lane_width/2;
        const mat4x4f low_mat(l.point_frame(param, low.first));
        for(int i = 0; i < 3; ++i)
        {
            low.second.point[i] = low_mat(i, 3);
            low.second.tan[i]   = tan_sign*low_mat(i, 0);
        }

        high.first = tan_sign*lane_width/2;
        const mat4x4f high_mat(l.point_frame(param, high.first));
        for(int i = 0; i < 3; ++i)
        {
            high.second.point[i] = high_mat(i, 3);
            high.second.tan[i]   = tan_sign*high_mat(i, 0);
        }
    }


    network_aux::road_store::road_store(const hwm::road *r, const bool as)
        : road(r), at_start(as)
    {}

    network_aux::road_store &network_aux::road_is_cnt::find(const hwm::road *r, const bool as)
    {
        BOOST_FOREACH(road_store &rw, *this)
        {
            if(rw.road == r && rw.at_start == as)
                return rw;
        }

        push_back(road_store(r, as));
        return back();
    }

    network_aux::intersection_geometry::intersection_geometry()
        : is(0)
    {}

    network_aux::intersection_geometry::intersection_geometry(const hwm::intersection *in_is)
        : is(in_is)
    {
        BOOST_FOREACH(const hwm::lane *incl, is->incoming)
        {
            const hwm::lane::road_membership *rm       = &(boost::prior(incl->road_memberships.end())->second);
            const hwm::road                  *r        = rm->parent_road;
            const bool                        at_start = (rm->interval[0] > rm->interval[1]);
            road_store                       &ent(ric.find(r, at_start));

            offs_pt low; offs_pt high;
            gen_tan_points(low, high, *incl, true);

            const float oriented_sign = at_start ? 1.0 : -1.0;
            low.first                 = oriented_sign*(low.first + rm->lane_position);
            high.first                = oriented_sign*(high.first + rm->lane_position);
            ent.ipt.insert(low);
            ent.ipt.insert(high);
        }

        BOOST_FOREACH(const hwm::lane *outl, is->outgoing)
        {
            const hwm::lane::road_membership *rm       = &(outl->road_memberships.begin()->second);
            const hwm::road                  *r        = rm->parent_road;
            const bool                        at_start = (rm->interval[0] < rm->interval[1]);
            road_store                       &ent(ric.find(r, at_start));

            offs_pt low; offs_pt high;
            gen_tan_points(low, high, *outl, false);

            const float oriented_sign = at_start ? 1.0 : -1.0;
            low.first                 = oriented_sign*(low.first + rm->lane_position);
            high.first                = oriented_sign*(high.first + rm->lane_position);
            ent.ipt.insert(low);
            ent.ipt.insert(high);
        }

        std::vector<road_winding> rw_sort;
        for(size_t road_num = 0; road_num < ric.size(); ++road_num)
        {
            const road_store &r = ric[road_num];

            vec2f avg(0.0f);
            BOOST_FOREACH(const offs_pt &op, r.ipt)
            {
                const vec3f svec(op.second.point - is->center);
                avg[0] += svec[0];
                avg[1] += svec[1];
            }
            rw_sort.push_back(road_winding(road_num, std::fmod(std::atan2(avg[1], avg[0])+M_PI, 2*M_PI)));
        }

        std::sort(rw_sort.begin(), rw_sort.end(), circle_sort());
        {
            road_is_cnt temp;
            temp.reserve(rw_sort.size());
            BOOST_FOREACH(const road_winding &rw, rw_sort)
            {
                temp.push_back(ric[rw.road_num]);
            }
            ric.swap(temp);
        }

        for(size_t i = 0; i < ric.size(); ++i)
        {
            const road_store &rs      = ric[i];
            const road_store &rs_next = ric[(i+1)%ric.size()];
            const point_tan  &start   = boost::prior(rs.ipt.end())->second;
            const point_tan  &end     = rs_next.ipt.begin()->second;

            connecting_arcs.push_back(arc_road());
            connecting_arcs.back().initialize_from_polyline(0.0f, from_tan_pairs(start.point,
                                                                                 start.tan,
                                                                                 end.point,
                                                                                 end.tan,
                                                                                 0.0f));
        }
    }

    void network_aux::intersection_geometry::cairo_draw(cairo_t *c, bool closed) const
    {
        cairo_new_path(c);
        BOOST_FOREACH(const arc_road &ar, connecting_arcs)
        {
            ar.svg_arc_path(vec2f(0, 1), 0.0f).cairo_draw(c, !closed);
        }
        if(closed)
            cairo_close_path(c);
    }

    void network_aux::intersection_geometry::intersection_obj(std::ostream &os) const
    {
        std::vector<vertex> vrts;
        for(size_t i = 0; i < ric.size(); ++i)
        {
            const road_store &rs = ric[i];
            BOOST_FOREACH(const offs_pt &op, rs.ipt)
            {
                vrts.push_back(vertex(op.second.point, vec3f(0, 0, 1), vec2f(0.0, 0.0)));
            }

            connecting_arcs[i].extract_line(vrts, vec2f(0.0f, 1.0f), 0.0f, 0.01);
        }

        vrts.push_back(vertex(is->center, vec3f(0, 0, 1), vec2f(0.0, 0.0)));
        const unsigned int center(static_cast<unsigned int>(vrts.size()-1));
        std::vector<vec3u> fcs;
        for(unsigned int i = 0; i < center; ++i)
            fcs.push_back(vec3u(center, i, (i+1) % center));

        mesh_to_obj(os, is->id, "intersection_mtl", vrts, fcs);
    }

    void network_aux::network_obj(const std::string &path) const
    {
        bf::path full_out_path(path);
        bf::path dir(full_out_path.parent_path());
        bf::path tex_dir(full_out_path.parent_path() / "tex");

        if(!dir.empty())
            bf::create_directory(dir);
        bf::remove_all(tex_dir);
        bf::create_directory(tex_dir);

        const bf::path now(bf::current_path());

        if(!dir.empty())
            bf::current_path(dir);
        std::ofstream out(full_out_path.filename().c_str());
        out << "s 1\n";
        road_objs(out);

        typedef strhash<intersection_geometry>::type::value_type ig_pair;
        BOOST_FOREACH(const ig_pair &ig, intersection_geoms)
        {
            ig.second.intersection_obj(out);
        }

        bf::current_path(now);
    }
}

