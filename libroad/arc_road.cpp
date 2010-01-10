#include "arc_road.hpp"
#include <boost/bimap.hpp>

static mat3x3f axis_angle_matrix(const float theta, const vec3f &axis)
{
    const float c = std::cos(theta);
    const float s = std::sin(theta);
    mat3x3f res;
    res = axis[0]*axis[0] + (1.0-axis[0]*axis[0])*c, axis[0]*axis[1]*(1.0-c) -  axis[2]*s,  axis[0]*axis[2]*(1.0-c) +  axis[1]*s,
          axis[0]*axis[1]*(1.0-c) +  axis[2]*s, axis[1]*axis[1] + (1.0-axis[1]*axis[1])*c,  axis[1]*axis[2]*(1.0-c) -  axis[0]*s,
          axis[0]*axis[2]*(1.0-c) -  axis[1]*s, axis[1]*axis[2]*(1.0-c) +  axis[0]*s,  axis[2]*axis[2] + (1.0-axis[2]*axis[2])*c;
    return res;
}

static float cot_theta(const vec3f &nb, const vec3f &nf)
{
    float d = tvmet::dot(nb, nf);
    return std::sqrt( (1.0 + d) / (1.0 - d) );
}

static void circle_frame(vec3f &pos, vec3f &tan, const float theta, const mat4x4f &matrix, const float radius)
{
    const float c = std::cos(theta);
    const float s = std::sin(theta);
    const vec4f v(matrix*vec4f(radius*c, radius*s, 0.0, 1.0));
    pos[0] = v[0];
    pos[1] = v[1];
    pos[2] = v[2];
    const vec4f t(matrix * vec4f(-s, c, 0.0, 0.0));
    tan[0] = t[0];
    tan[1] = t[1];
    tan[2] = t[2];
}

static vec3f triangle_angles(const vec3f &pt0, const vec3f &pt1, const vec3f &pt2)
{
    const vec3f v01(tvmet::normalize(pt1 - pt0));
    const vec3f v02(tvmet::normalize(pt2 - pt0));
    const vec3f v12(tvmet::normalize(pt2 - pt1));

    const float a0 = std::acos(tvmet::dot(v01, v02));
    const float a1 = std::acos(tvmet::dot(-v01, v12));
    const float a2 = 2*M_PI - (a0 + a1);

    return vec3f(a0, a1, a2);
}

static void make_mesh(std::vector<vec3f> &vrts, std::vector<vec3i> &faces,
                      const std::vector<vec3f> &low, const std::vector<vec3f> &high)
{
    vrts.clear();
    faces.clear();

    std::vector<vec3f>::const_iterator low_itr  = low.begin();
    std::vector<vec3f>::const_iterator high_itr = high.begin();

    vrts.push_back(*low_itr++);
    vrts.push_back(*high_itr++);

    size_t base_low_idx  = vrts.size()-2;
    size_t base_high_idx = vrts.size()-1;

    const vec3f *low_cand_vrt  = (low_itr == low.end()   ? 0 : &(*low_itr++));
    const vec3f *high_cand_vrt = (high_itr == high.end() ? 0 : &(*high_itr++));

    bool pick_vrt;
    while(1)
    {
        if(!low_cand_vrt)
        {
            if(!high_cand_vrt)
                break;
            else
                pick_vrt = true;
        }
        else if(!high_cand_vrt)
            pick_vrt = false;
        else
        {
            const vec3f angles_low (triangle_angles(vrts[base_low_idx], vrts[base_high_idx], *low_cand_vrt));
            const vec3f angles_high(triangle_angles(vrts[base_low_idx], vrts[base_high_idx], *high_cand_vrt));
            if(*std::max_element(angles_low.begin(), angles_low.end()) > *std::max_element(angles_high.begin(), angles_high.end()))
                pick_vrt = false;
            else
                pick_vrt = true;
        }

        faces.push_back(vec3i(base_low_idx, base_high_idx, vrts.size()));

        if(pick_vrt)
        {
            vrts.push_back(*high_cand_vrt);
            base_high_idx = vrts.size()-1;
            high_cand_vrt = (high_itr == high.end() ? 0 : &(*high_itr++));
        }
        else
        {
            vrts.push_back(*low_cand_vrt);
            base_low_idx = vrts.size()-1;
            low_cand_vrt = (low_itr == low.end() ? 0 : &(*low_itr++));
        }
    }
}

struct idx_sort
{
    idx_sort(const std::vector<float> &v) : vec(v)
    {
    }

    bool operator()(size_t x, size_t y) const
    {
        return vec[x] < vec[y];
    }

    const std::vector<float> &vec;
};

struct slack_idx_cmp
{
    slack_idx_cmp(const std::vector<float> &v) : vec(v)
    {
    }

    bool operator()(size_t x, size_t y) const
    {
        return vec[x] < vec[y];
    }

    const std::vector<float> &vec;
};

static float slack(const size_t idx, const std::vector<float> &lengths, const std::vector<float> &alphas)
{
    float low_slack;
    if(idx > 0)
        low_slack = lengths[idx] - alphas[idx-1] - alphas[idx];
    else
        low_slack = lengths[idx]                 - alphas[idx];

    float high_slack;
    if(idx < alphas.size()-1)
        high_slack = lengths[idx+1] - alphas[idx+1] - alphas[idx];
    else
        high_slack = lengths[idx+1]                 - alphas[idx];

    return std::min(low_slack, high_slack);
}

bool arc_road::initialize()
{
    const size_t N_pts  = points_.size();
    const size_t N_segs = N_pts - 1;
    const size_t N_arcs = N_pts - 2;

    normals_.resize(N_segs);
    std::vector<float> lengths(N_segs);
    for(size_t i = 1; i < N_pts; ++i)
    {
        normals_[i-1]    = points_[i] - points_[i-1];
        const float len  = std::sqrt(tvmet::dot(normals_[i-1], normals_[i-1]));
        if(len < FLT_EPSILON)
            return false;
        lengths[i-1]     = len;
        normals_[i-1]   /= len;
    }

    // Compute first pass for alphas
    std::vector<float> poly_lengths(N_segs);
    poly_lengths[0]      = lengths[0];
    for(size_t i = 0; i < N_segs; ++i)
        poly_lengths[i]  = lengths[i]/2;
    poly_lengths[N_arcs] = lengths[N_arcs];

    std::vector<size_t> indexes(N_segs);
    for(size_t i = 0; i < N_segs; ++i)
        indexes[i] = i;

    idx_sort isort(poly_lengths);
    std::sort(indexes.begin(), indexes.end(), isort);

    std::vector<float> alphas(N_arcs, 0);
    std::vector<bool>  set(N_arcs, false);

    BOOST_FOREACH(size_t idx, indexes)
    {
        if(idx != 0 && !set[idx-1])
        {
            alphas[idx-1] = poly_lengths[idx];
            set[idx-1]    = true;
        }
        if(idx != N_arcs && !set[idx])
        {
            alphas[idx] = poly_lengths[idx];
            set[idx]    = true;
        }
    }

    // Now do 2nd pass to remove excess slack
    for(size_t i = 0; i < N_segs; ++i)
        poly_lengths[i] = lengths[i];

    std::vector<float> slacks(N_arcs, 0);
    for(size_t i = 0; i < N_arcs; ++i)
        slacks[i] = slack(i, poly_lengths, alphas);
    idx_sort           ssort(slacks);

    indexes.resize(N_arcs);
    for(size_t i = 0; i < N_arcs; ++i)
        indexes[i] = i;
    for(std::vector<size_t>::iterator current = indexes.begin(); current != indexes.end(); ++current)
    {
        std::sort(current, indexes.end(), ssort);
        if(slacks[*current] > 0.0)
        {

            alphas[*current] += slacks[*current];
            slacks[*current]  = 0.0f;

            if(*current > 0)
            {
                const size_t prev_idx = *current - 1;
                slacks[prev_idx] = slack(prev_idx, poly_lengths, alphas);
            }
            if(*current < N_arcs-1)
            {
                const size_t next_idx = *current + 1;
                slacks[next_idx] = slack(next_idx, poly_lengths, alphas);
            }
        }
    }

    // Now compute actual helper data
    frames_.resize(N_arcs);
    radii_ .resize(N_arcs);
    arcs_  .resize(N_arcs);

    for(size_t i = 0; i < N_arcs; ++i)
    {
        const float alpha = alphas[i];
        radii_[i] = alpha * cot_theta(normals_[i], normals_[i+1]);

        const vec3f   laxis(tvmet::normalize(tvmet::cross(normals_[i+1], normals_[i])));
        const mat3x3f rot_pi2(axis_angle_matrix(M_PI_2, laxis));
        const vec3f   rm(rot_pi2*-normals_[i]);
        const vec3f   rp(tvmet::trans(rot_pi2)*normals_[i+1]);
        const vec3f   up(tvmet::cross(laxis,rm));
        const vec3f   tf(alpha * normals_[i+1] + radii_[i]*rp + points_[i+1]);

        frames_[i](0, 0) = -rm[0]; frames_[i](0, 1) = up[0]; frames_[i](0, 2) = laxis[0]; frames_[i](0, 3) = tf[0];
        frames_[i](1, 0) = -rm[1]; frames_[i](1, 1) = up[1]; frames_[i](1, 2) = laxis[1]; frames_[i](1, 3) = tf[1];
        frames_[i](2, 0) = -rm[2]; frames_[i](2, 1) = up[2]; frames_[i](2, 2) = laxis[2]; frames_[i](2, 3) = tf[2];
        frames_[i](3, 0) =   0.0f; frames_[i](3, 1) =  0.0f; frames_[i](3, 2) = 0.0f;     frames_[i](3, 3) =  1.0f;

        arcs_[i] = M_PI - std::acos(tvmet::dot(-normals_[i], normals_[i+1]));
    }

    arc_clengths_.resize(N_segs);
    arc_clengths_[0] = vec2f(0.0f, 0.0f);
    for(size_t i = 0; i < N_arcs; ++i)
        arc_clengths_[i+1] = arc_clengths_[i] + vec2f(radii_[i]*arcs_[i], arcs_[i]*copysign(1.0, frames_[i](2, 2)));

    for(size_t i = 0; i < N_arcs; ++i)
    {
        poly_lengths[i]   -= alphas[i];
        poly_lengths[i+1] -= alphas[i];
    }

    seg_clengths_.resize(N_pts);
    seg_clengths_[0] = 0.0f;
    for(size_t i = 1; i < N_pts; ++i)
        seg_clengths_[i] = seg_clengths_[i-1] + poly_lengths[i-1];

    return true;
}

float arc_road::length(const float offset) const
{
    return feature_base(2*frames_.size()+1, offset);
}

vec3f arc_road::point(const float t, const float offset, const vec3f &up) const
{
    vec3f pos;
    vec3f tan;

    float local;
    const size_t idx = locate_scale(t, offset, local);
    if(idx & 1)
    {
        const size_t real_idx = idx/2;
        circle_frame(pos, tan, local*arcs_[real_idx], frames_[real_idx], radii_[real_idx]);
        const vec3f left(tvmet::normalize(tvmet::cross(up, tan)));
        return vec3f(pos + left*offset);
    }
    else
    {
        const int real_idx = idx/2-1;

        if(real_idx < 0)
        {
            pos = points_.front();
            tan = normals_.front();
        }
        else
            circle_frame(pos, tan, arcs_[real_idx], frames_[real_idx], radii_[real_idx]);

        const vec3f left(tvmet::normalize(tvmet::cross(up, tan)));
        return vec3f(pos + left*offset + tan*local*feature_size(idx, offset));
    }
}

mat3x3f arc_road::frame(const float t, const float offset, const vec3f &up) const
{
    vec3f pos;
    vec3f tan;

    float local;
    const size_t idx = locate_scale(t, offset, local);
    if(idx & 1)
    {
        const size_t real_idx = idx/2;
        circle_frame(pos, tan, local*arcs_[real_idx], frames_[real_idx], radii_[real_idx]);
    }
    else
    {
        const int real_idx = idx/2-1;

        if(real_idx < 0)
        {
            pos = points_.front();
            tan = normals_.front();
        }
        else
            circle_frame(pos, tan, arcs_[real_idx], frames_[real_idx], radii_[real_idx]);

        pos += tan*local*feature_size(idx, offset);
    }

    const vec3f left  (tvmet::normalize(tvmet::cross(up, tan)));
    const vec3f new_up(tvmet::normalize(tvmet::cross(tan, left)));

    pos += left*offset;

    mat3x3f res;
    res(0, 0) = tan[0]; res(0, 1) = left[0]; res(0, 2) = new_up[0];
    res(1, 0) = tan[1]; res(1, 1) = left[1]; res(1, 2) = new_up[1];
    res(2, 0) = tan[2]; res(2, 1) = left[2]; res(2, 2) = new_up[2];
    return res;
}

mat4x4f arc_road::point_frame(const float t, const float offset, const vec3f &up) const
{
    vec3f pos;
    vec3f tan;

    float local;
    const size_t idx = locate_scale(t, offset, local);
    if(idx & 1)
    {
        const size_t real_idx = idx/2;
        circle_frame(pos, tan, local*arcs_[real_idx], frames_[real_idx], radii_[real_idx]);
    }
    else
    {
        const int real_idx = idx/2-1;

        if(real_idx < 0)
        {
            pos = points_.front();
            tan = normals_.front();
        }
        else
            circle_frame(pos, tan, arcs_[real_idx], frames_[real_idx], radii_[real_idx]);

        pos += tan*local*feature_size(idx, offset);
    }

    const vec3f left  (tvmet::normalize(tvmet::cross(up, tan)));
    const vec3f new_up(tvmet::normalize(tvmet::cross(tan, left)));

    pos += left*offset;

    mat4x4f res;
    res(0, 0) = tan[0];res(0, 1) = left[0];res(0, 2) = new_up[0];res(0, 3) = pos[0];
    res(1, 0) = tan[1];res(1, 1) = left[1];res(1, 2) = new_up[1];res(1, 3) = pos[1];
    res(2, 0) = tan[2];res(2, 1) = left[2];res(2, 2) = new_up[2];res(2, 3) = pos[2];
    res(3, 0) = 0.0f;  res(3, 1) = 0.0f;   res(3, 2) = 0.0f;     res(3, 3) = 1.0f;
    return res;
}

void arc_road::translate(const vec3f &o)
{
    BOOST_FOREACH(vec3f &pt, points_)
    {
        pt += o;
    }

    BOOST_FOREACH(mat4x4f &fr, frames_)
    {
        for(int i = 0; i < 3; ++i)
            fr(i, 3) += o[i];
    }
}

float arc_road::parameter_map(const float t, const float offset) const
{
    float center_local;
    size_t feature_idx = locate_scale(t, 0.0f, center_local);

    return length_at_feature(feature_idx, center_local, offset)/length(offset);
}

float arc_road::length_at_feature(const size_t i, const float p, const float offset) const
{
    return feature_base(i, offset) + p*feature_size(i, offset);
}

void arc_road::extract_arc(std::vector<vec3f> &result, const size_t i, const vec2f &in_range, const float offset, const float resolution, const vec3f &up) const
{
    assert(in_range[0] < in_range[1]);
    assert(in_range[0] >= 0.0f);
    assert(in_range[1] <= 1.0f);

    std::vector<std::pair<float,vec3f> > new_points;

    // add last point
    {
        vec3f pos;
        vec3f tan;

        circle_frame(pos, tan, in_range[1]*arcs_[i], frames_[i], radii_[i]);
        const vec3f left(tvmet::normalize(tvmet::cross(up, tan)));
        const vec3f pointend(pos + left*offset);

        new_points.push_back(std::make_pair(in_range[1]*arcs_[i], pointend));
    }

    // add first point
    {
        vec3f pos;
        vec3f tan;

        circle_frame(pos, tan, in_range[0]*arcs_[i], frames_[i], radii_[i]);

        const vec3f left(tvmet::normalize(tvmet::cross(up, tan)));
        const vec3f point0(pos + left*offset);

        bool add_this = true;
        if(!result.empty())
        {
            const vec3f diff(point0 - result.back());
            const float distance(std::sqrt(tvmet::dot(diff, diff)));
            add_this = (distance >= 1e-3);
        }

        if(add_this)
        {
            new_points.push_back(std::make_pair(in_range[0]*arcs_[i], point0));
            result.push_back(point0);
        }
        else
            new_points.push_back(std::make_pair(in_range[0]*arcs_[i], result.back()));
    }

    while(new_points.size() > 1)
    {
        const std::pair<float, vec3f> &front = new_points[new_points.size()-1];
        const std::pair<float, vec3f> &next  = new_points[new_points.size()-2];

        const vec3f diff(front.second - next.second);
        const float distance(std::sqrt(tvmet::dot(diff, diff)));
        if (distance > resolution)
        {
            assert(next.first - front.first > 1e-3);
            const float new_theta = (next.first + front.first)/2;
            vec3f pos;
            vec3f tan;
            circle_frame(pos, tan, new_theta, frames_[i], radii_[i]);

            const vec3f left(tvmet::normalize(tvmet::cross(up, tan)));
            new_points.push_back(std::make_pair(new_theta, pos + left*offset));
            std::swap(new_points[new_points.size()-1], new_points[new_points.size()-2]);
        }
        else
        {
            new_points.pop_back();
            result.push_back(new_points.back().second);
        }
    }
}

std::vector<vec3f> arc_road::extract_center(const vec2f &in_range, const float offset, const float resolution, const vec3f &up) const
{
    const vec2f new_range(parameter_map(in_range[0], offset), parameter_map(in_range[1], offset));
    return extract_line(new_range, offset, resolution, up);
}

std::vector<vec3f> arc_road::extract_line(const vec2f &in_range, const float offset, const float resolution, const vec3f &up) const
{
    std::vector<vec3f> result;
    result.clear();

    vec2f range(in_range);
    bool reversed = range[0] > range[1];
    if(reversed)
        std::swap(range[0], range[1]);

    float        start_local;
    size_t       start_arc;
    const size_t start_feature = locate_scale(range[0], offset, start_local);

    float        end_local;
    const size_t end_feature   = locate_scale(range[1], offset, end_local);

    if(start_feature & 1)
    {
        if(start_feature == end_feature)
            extract_arc(result, start_feature/2, vec2f(start_local, end_local), offset, resolution, up);
        else
            extract_arc(result, start_feature/2, vec2f(start_local, 1.0f),      offset, resolution, up);
        start_arc = start_feature/2 + 1;
    }
    else
    {
        vec3f pos, tan;
        const int real_idx = start_feature/2-1;
        if(real_idx < 0)
        {
            pos = points_.front();
            tan = normals_.front();
        }
        else
            circle_frame(pos, tan, arcs_[real_idx], frames_[real_idx], radii_[real_idx]);

        const vec3f left(tvmet::normalize(tvmet::cross(up, tan)));
        result.push_back(vec3f(pos + left*offset + tan*start_local*feature_size(start_feature, offset)));

        start_arc = start_feature/2;
    }
    const size_t end_arc = end_feature/2;
    for(size_t i = start_arc; i < end_arc; ++i)
        extract_arc(result, i, vec2f(0.0, 1.0), offset, resolution, up);

    if(end_feature & 1)
    {
        if(start_feature != end_feature)
            extract_arc(result, end_feature/2, vec2f(0.0, end_local), offset, resolution, up);
    }
    else
    {
        vec3f pos, tan;
        const int real_idx = end_feature/2-1;
        if(real_idx < 0)
        {
            pos = points_.front();
            tan = normals_.front();
        }
        else
            circle_frame(pos, tan, arcs_[real_idx], frames_[real_idx], radii_[real_idx]);

        const vec3f left(tvmet::normalize(tvmet::cross(up, tan)));
        const vec3f end_point = vec3f(pos + left*offset + tan*end_local*feature_size(end_feature, offset));

        const vec3f diff(end_point - result.back());
        const float distance(std::sqrt(tvmet::dot(diff, diff)));
        if(distance > 1e-3)
            result.push_back(end_point);
    }

    if(reversed)
        std::reverse(result.begin(), result.end());

    return result;
}

void arc_road::make_mesh(std::vector<vec3f> &vrts, std::vector<vec3i> &faces,
                         const vec2f &range,
                         const vec2f &offsets, const float resolution) const
{
    ::make_mesh(vrts, faces, extract_line(range, offsets[0], resolution), extract_line(range, offsets[1], resolution));
}

float arc_road::feature_base(const size_t i, const float offset) const
{
    if(i & 1)
    {
        int seg_idx = i/2+1;
        int arc_idx = i/2;

        return seg_clengths_[seg_idx] + arc_clengths_[arc_idx][0] + offset*arc_clengths_[arc_idx][1];
    }
    else
    {
        if(i == 0)
            return seg_clengths_[0];

        int seg_idx = i/2;
        int arc_idx = i/2;

        return seg_clengths_[seg_idx] + arc_clengths_[arc_idx][0] + offset*arc_clengths_[arc_idx][1];
    }
}

float arc_road::feature_size(const size_t i, const float offset) const
{
    return feature_base(i+1, offset) - feature_base(i, offset);
}

size_t arc_road::locate(const float t, const float offset) const
{
    const float scaled_t = t*length(offset);

    size_t low = 0;
    size_t high = 2*frames_.size()+2;
    while (low < high)
    {
        const size_t mid = low + ((high - low) / 2);
        float lookup = feature_base(mid, offset);

        if (lookup < scaled_t)
            low = mid + 1;
        else
            high = mid;
    }
    if(low > 0)
        --low;
    while(low < 2*frames_.size()+2 && feature_size(low, offset) == 0)
        ++low;

    return low;
}

size_t arc_road::locate_scale(const float t, const float offset, float &local) const
{
    const float scaled_t = t*length(offset);

    size_t low = 0;
    size_t high = 2*frames_.size()+2;
    while (low < high)
    {
        const size_t mid = low + ((high - low) / 2);
        float lookup = feature_base(mid, offset);

        if (lookup < scaled_t)
            low = mid + 1;
        else
            high = mid;
    }
    if(low > 0)
        --low;
    while(low < 2*frames_.size()+2 && feature_size(low, offset) == 0)
        ++low;

    float lookup = feature_base(low, offset);
    float base   = feature_size(low, offset);

    local = (scaled_t - lookup) / base;

    return low;
}

bool arc_road::check() const
{
    const size_t N_pts       = points_.size();
    return frames_.size()   == N_pts-2
    && radii_.size()        == N_pts-2
    && arcs_.size()         == N_pts-2
    && seg_clengths_.size() == N_pts
    && arc_clengths_.size() == N_pts-1
    && normals_.size()       == N_pts-1;
}
