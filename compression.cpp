#include "compression.h"
#include <zlib.h>

bool GzipHelper::isGzip(const std::vector<uint8_t>& data) {
    return data.size() >= 2 && data[0] == 0x1F && data[1] == 0x8B;
}

bool GzipHelper::decompressGzip(const std::vector<uint8_t>& input, std::vector<uint8_t>& output) {
    z_stream strm{};
    strm.next_in = const_cast<Bytef*>(input.data());
    strm.avail_in = input.size();

    if (inflateInit2(&strm, 16 + MAX_WBITS) != Z_OK) return false;

    //10MB max expected uncompressed size
    output.resize(10 * 1024 * 1024);
    strm.next_out = output.data();
    strm.avail_out = output.size();

    int ret = inflate(&strm, Z_FINISH);
    inflateEnd(&strm);
    output.resize(strm.total_out);

    return ret == Z_STREAM_END;
}

bool GzipHelper::compressGzip(const std::vector<uint8_t>& input, std::vector<uint8_t>& output) {
    z_stream strm{};
    strm.next_in = const_cast<Bytef*>(input.data());
    strm.avail_in = input.size();

    if (deflateInit2(&strm, Z_DEFAULT_COMPRESSION, Z_DEFLATED, 16 + MAX_WBITS, 8, Z_DEFAULT_STRATEGY) != Z_OK) {
        return false;
    }

    output.resize(deflateBound(&strm, input.size()));
    strm.next_out = output.data();
    strm.avail_out = output.size();

    int ret = deflate(&strm, Z_FINISH);
    deflateEnd(&strm);
    
    if (ret != Z_STREAM_END) {
        return false;
    }
    
    output.resize(strm.total_out);
    return true;
}
