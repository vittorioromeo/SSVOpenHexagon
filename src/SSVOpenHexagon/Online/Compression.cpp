// Copyright (c) 2013-2015 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#include "SSVOpenHexagon/Global/Common.hpp"
#include "SSVOpenHexagon/Online/Compression.hpp"

using namespace std;

namespace hg
{
	string getZLibCompress(const string& mStr, int mCompressionlevel)
	{
		z_stream zs; memset(&zs, 0, sizeof(zs));

		if(deflateInit(&zs, mCompressionlevel) != Z_OK) throw(runtime_error("deflateInit failed while compressing."));

		zs.next_in = reinterpret_cast<Bytef*>(const_cast<char*>(mStr.data()));
		zs.avail_in = mStr.size();

		int ret; char outbuffer[32768]; string outstring;

		do
		{
			zs.next_out = reinterpret_cast<Bytef*>(outbuffer);
			zs.avail_out = sizeof(outbuffer);

			ret = deflate(&zs, Z_FINISH);

			if(outstring.size() < zs.total_out) outstring.append(outbuffer, zs.total_out - outstring.size());
		}
		while(ret == Z_OK);

		deflateEnd(&zs);

		if(ret != Z_STREAM_END)
		{
			ostringstream oss;
			oss << "Exception during zlib compression: (" << ret << ") " << zs.msg;
			throw(runtime_error(oss.str()));
		}

		return outstring;
	}

	string getZLibDecompress(const string& mStr)
	{
		z_stream zs; memset(&zs, 0, sizeof(zs));

		if(inflateInit(&zs) != Z_OK) throw(runtime_error("inflateInit failed while decompressing."));

		zs.next_in = reinterpret_cast<Bytef*>(const_cast<char*>(mStr.data()));
		zs.avail_in = mStr.size();

		int ret; char outbuffer[32768]; string outstring;

		do
		{
			zs.next_out = reinterpret_cast<Bytef*>(outbuffer);
			zs.avail_out = sizeof(outbuffer);

			ret = inflate(&zs, 0);

			if(outstring.size() < zs.total_out) outstring.append(outbuffer, zs.total_out - outstring.size());
		}
		while(ret == Z_OK);

		inflateEnd(&zs);

		if(ret != Z_STREAM_END)
		{
			ostringstream oss;
			oss << "Exception during zlib decompression: (" << ret << ") " << zs.msg;
			throw(runtime_error(oss.str()));
		}

		return outstring;
	}
}
