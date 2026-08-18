#include <gtest/gtest.h>

#ifdef __cplusplus
extern "C"
{
#endif
#include <ffmpeg/libavcodec/avcodec.h>
#include <ffmpeg/libavdevice/avdevice.h>
#include <ffmpeg/libavfilter/avfilter.h>
#include <ffmpeg/libavformat/avformat.h>
#include <ffmpeg/libavutil/avutil.h>
#include <ffmpeg/libavutil/opt.h>
#include <ffmpeg/libavutil/time.h>
#include <ffmpeg/libswresample/swresample.h>
#include <ffmpeg/libswscale/swscale.h>
#ifdef __cplusplus
}
#endif

// Test fixture for FFmpeg tests
class FFmpegTestFixture : public ::testing::Test
{
protected:
  static void SetUpTestSuite()
  {
    // Initialize FFmpeg libraries once before any tests run
#if LIBAVFORMAT_VERSION_MAJOR < 58
    av_register_all();
#endif
    avformat_network_init();
#if LIBAVFILTER_VERSION_MAJOR < 7
    avfilter_register_all();
#endif
    avdevice_register_all();
  }

  static void TearDownTestSuite()
  {
    // Cleanup FFmpeg resources after all tests complete
    // Note: Most FFmpeg resources don't require explicit cleanup in modern
    // versions
  }
};

// Use the test fixture for all FFmpeg tests
typedef FFmpegTestFixture FFmpegTest;

// Test case for basic FFmpeg library initialization and version checks
TEST_F(FFmpegTest, LibraryVersions)
{
  // Test that all libraries are properly linked by checking their versions
  EXPECT_GT(avcodec_version() >> 16 & 0xFF, 0);
  EXPECT_GT(avformat_version() >> 16 & 0xFF, 0);
  EXPECT_GT(avfilter_version() >> 16 & 0xFF, 0);
  EXPECT_GT(avdevice_version() >> 16 & 0xFF, 0);
  EXPECT_GT(swresample_version() >> 16 & 0xFF, 0);
  EXPECT_GT(swscale_version() >> 16 & 0xFF, 0);
}

// Test case for AVCodec functionality
TEST_F(FFmpegTest, CodecSupport)
{
  // Check if H.264 decoder is available
  const AVCodec* h264_codec = avcodec_find_decoder(AV_CODEC_ID_H264);
  EXPECT_NE(h264_codec, nullptr) << "H.264 codec not found";

  // Check if MP3 decoder is available
  const AVCodec* mp3_codec = avcodec_find_decoder(AV_CODEC_ID_MP3);
  EXPECT_NE(mp3_codec, nullptr) << "MP3 codec not found";
}

// Test case for AVFormat functionality
TEST_F(FFmpegTest, FormatSupport)
{
  // Check format support using the appropriate API for this FFmpeg version
#if LIBAVFORMAT_VERSION_MAJOR < 58
  AVInputFormat* input_format = av_iformat_next(nullptr);
#else
  const AVInputFormat* input_format = nullptr;
  void* iter = nullptr;
  input_format = av_demuxer_iterate(&iter);
#endif
  EXPECT_NE(input_format, nullptr) << "No input formats found";

  // Check if common formats are supported
  AVOutputFormat* mp4_format = av_guess_format("mp4", nullptr, nullptr);
  EXPECT_NE(mp4_format, nullptr) << "MP4 format not supported";
}

// Test case for AVFilter functionality
TEST_F(FFmpegTest, FilterSupport)
{
  // Initialize filter system if required by this FFmpeg version
#if LIBAVFILTER_VERSION_MAJOR < 7
  avfilter_register_all();
#endif

  // Check if common filters are available
  const AVFilter* scale_filter = avfilter_get_by_name("scale");
  EXPECT_NE(scale_filter, nullptr) << "Scale filter not found";

  const AVFilter* hflip_filter = avfilter_get_by_name("hflip");
  EXPECT_NE(hflip_filter, nullptr) << "Horizontal flip filter not found";
}

// Test case for SWScale functionality
TEST_F(FFmpegTest, ScaleSupport)
{
  // Check if scaling is supported
  SwsContext* sws_ctx = sws_getContext(640,
                                       480,
                                       AV_PIX_FMT_YUV420P,
                                       320,
                                       240,
                                       AV_PIX_FMT_RGB24,
                                       SWS_BILINEAR,
                                       nullptr,
                                       nullptr,
                                       nullptr);
  EXPECT_NE(sws_ctx, nullptr) << "Failed to create scaling context";
  if (sws_ctx)
  {
    sws_freeContext(sws_ctx);
  }
}

// Test case for SWResample functionality
TEST_F(FFmpegTest, ResampleSupport)
{
  // Check if resampling is supported
  SwrContext* swr_ctx = swr_alloc();
  EXPECT_NE(swr_ctx, nullptr) << "Failed to allocate resampling context";

  if (swr_ctx)
  {
    av_opt_set_int(swr_ctx, "in_channel_layout", AV_CH_LAYOUT_STEREO, 0);
    av_opt_set_int(swr_ctx, "out_channel_layout", AV_CH_LAYOUT_STEREO, 0);
    av_opt_set_int(swr_ctx, "in_sample_rate", 44100, 0);
    av_opt_set_int(swr_ctx, "out_sample_rate", 48000, 0);
    av_opt_set_sample_fmt(swr_ctx, "in_sample_fmt", AV_SAMPLE_FMT_FLT, 0);
    av_opt_set_sample_fmt(swr_ctx, "out_sample_fmt", AV_SAMPLE_FMT_S16, 0);

    int ret = swr_init(swr_ctx);
    EXPECT_GE(ret, 0) << "Failed to initialize resampling context";

    swr_free(&swr_ctx);
  }
}

// Test case for AVDevice functionality (if available)
TEST_F(FFmpegTest, DeviceSupport)
{
  // Initialize device support
  avdevice_register_all();

  // Check for input devices using the older API
  AVInputFormat* input_device = av_input_video_device_next(nullptr);

  // This is just a basic check that the API works
  // The actual device availability depends on the system
  (void)input_device; // Suppress unused variable warning
  SUCCEED();
}

// Test case for AVUtil functionality
TEST_F(FFmpegTest, UtilFunctions)
{
  // Test memory allocation
  void* buffer = av_malloc(1024);
  EXPECT_NE(buffer, nullptr) << "Memory allocation failed";

  // Test memory reallocation
  void* new_buffer = av_realloc(buffer, 2048);
  EXPECT_NE(new_buffer, nullptr) << "Memory reallocation failed";

  // Free the memory
  if (new_buffer)
  {
    av_free(new_buffer);
  }
  else if (buffer)
  {
    av_free(buffer);
  }

  // Test time functions
  int64_t timestamp = av_gettime();
  // Check that the timestamp is reasonable (after 2000-01-01)
  EXPECT_GT(timestamp, 946684800000000) << "Invalid timestamp";
}
