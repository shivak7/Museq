#ifdef _WIN32
    #define NOMINMAX
#endif
#include "OggWriter.h"
#include "vorbis/vorbisenc.h"
#include "AudioRenderer.h"
#include <vector>
#include <algorithm>

void OggWriter::write(AudioRenderer& renderer, const Song& song, const std::string& file_path, float sample_rate, float quality) {
    std::vector<float> pcm_buffer = renderer.render(song, sample_rate);

    ogg_stream_state os;
    ogg_page         og;
    ogg_packet       op;
    vorbis_info      vi;
    vorbis_comment   vc;
    vorbis_dsp_state vd;
    vorbis_block     vb;

    vorbis_info_init(&vi);
    // Use 2 channels for stereo
    vorbis_encode_init_vbr(&vi, 2, sample_rate, quality);
    vorbis_analysis_init(&vd, &vi);
    vorbis_block_init(&vd, &vb);
    vorbis_comment_init(&vc);
    ogg_stream_init(&os, rand());

    ogg_packet header;
    ogg_packet header_comm;
    ogg_packet header_code;
    vorbis_analysis_headerout(&vd, &vc, &header, &header_comm, &header_code);
    ogg_stream_packetin(&os, &header);
    ogg_stream_packetin(&os, &header_comm);
    ogg_stream_packetin(&os, &header_code);

    FILE* ogg_file = fopen(file_path.c_str(), "wb");

    while (ogg_stream_flush(&os, &og)) {
        fwrite(og.header, 1, og.header_len, ogg_file);
        fwrite(og.body, 1, og.body_len, ogg_file);
    }

    // 5. Encode the PCM data in chunks for efficiency
    const int chunk_size = 1024;
    size_t offset = 0;
    while (offset < pcm_buffer.size()) {
        int to_write_frames = (std::min)((int)(pcm_buffer.size() - offset) / 2, chunk_size);
        if (to_write_frames <= 0) break;

        float** buffer = vorbis_analysis_buffer(&vd, to_write_frames);
        for (int i = 0; i < to_write_frames; ++i) {
            buffer[0][i] = pcm_buffer[offset + i*2];
            buffer[1][i] = pcm_buffer[offset + i*2 + 1];
        }
        vorbis_analysis_wrote(&vd, to_write_frames);
        offset += to_write_frames * 2;

        while (vorbis_analysis_blockout(&vd, &vb)) {
            vorbis_analysis(&vb, NULL);
            vorbis_bitrate_addblock(&vb);
            while (vorbis_bitrate_flushpacket(&vd, &op)) {
                ogg_stream_packetin(&os, &op);
                while (ogg_stream_pageout(&os, &og)) {
                    fwrite(og.header, 1, og.header_len, ogg_file);
                    fwrite(og.body, 1, og.body_len, ogg_file);
                }
            }
        }
    }

    // 6. Signal end of stream and flush remaining data
    vorbis_analysis_wrote(&vd, 0); 

    while (vorbis_analysis_blockout(&vd, &vb)) {
        vorbis_analysis(&vb, NULL);
        vorbis_bitrate_addblock(&vb);
        while (vorbis_bitrate_flushpacket(&vd, &op)) {
            ogg_stream_packetin(&os, &op);
            while (ogg_stream_pageout(&os, &og)) {
                fwrite(og.header, 1, og.header_len, ogg_file);
                fwrite(og.body, 1, og.body_len, ogg_file);
            }
        }
    }

    while (ogg_stream_flush(&os, &og)) {
        fwrite(og.header, 1, og.header_len, ogg_file);
        fwrite(og.body, 1, og.body_len, ogg_file);
    }

    fclose(ogg_file);
    ogg_stream_clear(&os);
    vorbis_block_clear(&vb);
    vorbis_dsp_clear(&vd);
    vorbis_comment_clear(&vc);
    vorbis_info_clear(&vi);
}
