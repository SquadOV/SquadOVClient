#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>

extern void log_packet(const AVFormatContext *fmt_ctx, const AVPacket *pkt);