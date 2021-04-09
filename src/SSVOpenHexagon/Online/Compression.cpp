// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#include "SSVOpenHexagon/Online/Compression.hpp"

#include <zlib.h>

#include <sstream>
#include <cstring>

namespace hg {

std::string getZLibCompress(const std::string& mStr)
{
    z_stream zs;
    std::memset(&zs, 0, sizeof(zs));

    if(deflateInit(&zs, Z_BEST_COMPRESSION) != Z_OK)
    {
        throw std::runtime_error("deflateInit failed while compressing.");
    }

    zs.next_in = reinterpret_cast<Bytef*>(const_cast<char*>(mStr.data()));
    zs.avail_in = mStr.size();

    int ret;
    char outbuffer[32768];
    std::string outstring;

    do
    {
        zs.next_out = reinterpret_cast<Bytef*>(outbuffer);
        zs.avail_out = sizeof(outbuffer);

        ret = deflate(&zs, Z_FINISH);

        if(outstring.size() < zs.total_out)
        {
            outstring.append(outbuffer, zs.total_out - outstring.size());
        }
    } while(ret == Z_OK);

    deflateEnd(&zs);

    if(ret != Z_STREAM_END)
    {
        std::ostringstream oss;
        oss << "Exception during zlib compression: (" << ret << ") " << zs.msg;
        throw std::runtime_error(oss.str());
    }

    return outstring;
}

std::string getZLibDecompress(const std::string& mStr)
{
    z_stream zs;
    std::memset(&zs, 0, sizeof(zs));

    if(inflateInit(&zs) != Z_OK)
    {
        throw std::runtime_error("inflateInit failed while decompressing.");
    }

    zs.next_in = reinterpret_cast<Bytef*>(const_cast<char*>(mStr.data()));
    zs.avail_in = mStr.size();

    int ret;
    char outbuffer[32768];
    std::string outstring;

    do
    {
        zs.next_out = reinterpret_cast<Bytef*>(outbuffer);
        zs.avail_out = sizeof(outbuffer);

        ret = inflate(&zs, 0);

        if(outstring.size() < zs.total_out)
        {
            outstring.append(outbuffer, zs.total_out - outstring.size());
        }
    } while(ret == Z_OK);

    inflateEnd(&zs);

    if(ret != Z_STREAM_END)
    {
        std::ostringstream oss;
        oss << "Exception during zlib decompression: (" << ret << ") "
            << zs.msg;
        throw std::runtime_error(oss.str());
    }

    return outstring;
}

} // namespace hg
