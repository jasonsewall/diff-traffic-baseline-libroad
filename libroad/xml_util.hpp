#ifndef _XML_UTIL_HPP_
#define _XML_UTIL_HPP_

#include <boost/lexical_cast.hpp>
#include <glibmm/ustring.h>
#include <libxml++/libxml++.h>
#include <libxml++/parsers/textreader.h>
#include <map>

typedef Glib::ustring str;

template <typename T>
inline bool get_attribute(T &res, xmlpp::TextReader &reader, const str &eltname)
{
    str val(reader.get_attribute(eltname));

    if(val.empty())
        return false;
    res = boost::lexical_cast<T>(val);
    return true;
}

inline bool is_opening_element(const xmlpp::TextReader &reader, const str &name)
{
    return (reader.get_node_type() == xmlpp::TextReader::Element
            && reader.get_name() == name);
}

inline bool is_closing_element(const xmlpp::TextReader &reader, const str &name)
{
    return (reader.get_node_type() == xmlpp::TextReader::EndElement
            && reader.get_name() == name);
}

template <class closure, typename T>
inline bool read_map(closure &c, std::map<str, T> &themap, xmlpp::TextReader &reader, const str &item_name, const str &container_name)
{
    bool ret;
    do
    {
        ret = reader.read();
        if(ret != 1)
            return false;

        if(reader.get_node_type() == xmlpp::TextReader::Element)
        {
            if(reader.get_name() == item_name)
            {
                T new_item;
                if(!c.xml_read(new_item, reader))
                    return false;

                themap[new_item.id] = new_item;
            }
            else
                return false;
        }
    }
    while(ret && !is_closing_element(reader, container_name));

    return ret;
}

// inline bool read_leaf_text(std::string &res, xmlTextReaderPtr reader, const char *endtag)
// {
//     bool read_text = false;

//     do
//     {
//         int ret = xmlTextReaderRead(reader);
//         if(ret != 1)
//             return false;

//         if(xmlTextReaderNodeType(reader) == XML_READER_TYPE_TEXT)
//         {
//             res.append((const char *) xmlTextReaderConstValue(reader));
//             read_text = true;
//         }
//     }
//     while(!is_closing_element(reader, endtag));

//     return read_text;
// }
#endif
