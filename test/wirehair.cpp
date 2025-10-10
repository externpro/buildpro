#include <cstdint>
#include <cstring>
#include <memory>
#include <vector>

#include <gtest/gtest.h>
#include <wirehair/wirehair.h>

namespace
{
  // Helper function to create random data for testing
  std::vector<uint8_t> create_test_data(size_t size)
  {
    std::vector<uint8_t> data(size);
    for (size_t i = 0; i < size; ++i)
    {
      data[i] = static_cast<uint8_t>((i * 157) ^ (i >> 5));
    }
    return data;
  }

  // RAII wrapper for WirehairCodec to ensure proper cleanup
  class WirehairCodecWrapper
  {
  public:
    WirehairCodecWrapper() : codec_(nullptr) { }
    ~WirehairCodecWrapper()
    {
      if (codec_)
      {
        wirehair_free(codec_);
      }
    }

    WirehairCodec get() const { return codec_; }
    void set(WirehairCodec codec) { codec_ = codec; }

    operator bool() const { return codec_ != nullptr; }

    // Allow reuse of the codec object
    WirehairCodec release()
    {
      WirehairCodec temp = codec_;
      codec_ = nullptr;
      return temp;
    }

  private:
    WirehairCodec codec_;
  };

} // namespace

class WirehairTest : public ::testing::Test
{
protected:
  void SetUp() override
  {
    // Initialize wirehair library
    WirehairResult result = wirehair_init();
    ASSERT_EQ(result, Wirehair_Success)
      << "Failed to initialize wirehair: " << result;
  }

  void TearDown() override
  {
    // Nothing to do here
  }
};

TEST_F(WirehairTest, BasicEncodeDecode)
{
  // Test parameters
  const size_t kMessageBytes = 100000; // 100 KB
  const size_t kPacketSize = 1400;     // Standard MTU size minus overhead

  // Create test data
  std::vector<uint8_t> original_data = create_test_data(kMessageBytes);

  // Create encoder
  WirehairCodecWrapper encoder;
  encoder.set(wirehair_encoder_create(nullptr, // Use default memory allocator
                                      original_data.data(), // Original message
                                      kMessageBytes,        // Message length
                                      kPacketSize           // Packet size
                                      ));
  ASSERT_TRUE(encoder) << "Failed to create encoder";

  // Create decoder
  WirehairCodecWrapper decoder;
  decoder.set(wirehair_decoder_create(nullptr, // Use default memory allocator
                                      kMessageBytes, // Message length
                                      kPacketSize    // Packet size
                                      ));
  ASSERT_TRUE(decoder) << "Failed to create decoder";

  // Simulate packet loss (skip every 5th packet)
  unsigned block_id = 0;
  unsigned blocks_received = 0;
  bool decode_success = false;

  // Keep sending packets until we can decode
  std::vector<uint8_t> packet(kPacketSize);
  while (!decode_success &&
         blocks_received <
           kMessageBytes / kPacketSize * 2) // Limit to avoid infinite loop
  {
    block_id++;

    // Skip every 5th packet to simulate 20% packet loss
    if (block_id % 5 == 0)
    {
      continue;
    }

    // Encode a packet
    uint32_t write_len = 0;
    WirehairResult encode_result =
      wirehair_encode(encoder.get(), // Encoder
                      block_id,      // Block ID
                      packet.data(), // Output buffer
                      kPacketSize,   // Output buffer size
                      &write_len     // Actual written length
      );
    ASSERT_EQ(encode_result, Wirehair_Success)
      << "Encode failed: " << encode_result;

    // Attempt to decode
    WirehairResult decode_result =
      wirehair_decode(decoder.get(), // Decoder
                      block_id,      // Block ID
                      packet.data(), // Input buffer
                      write_len      // Input buffer length
      );

    blocks_received++;

    if (decode_result == Wirehair_Success)
    {
      decode_success = true;
      break;
    }
    else if (decode_result != Wirehair_NeedMore)
    {
      FAIL() << "Decode failed with error: " << decode_result;
    }
  }

  // Verify we could decode
  ASSERT_TRUE(decode_success)
    << "Failed to decode after " << blocks_received << " blocks";

  // Recover the original data
  std::vector<uint8_t> recovered_data(kMessageBytes);
  WirehairResult recover_result =
    wirehair_recover(decoder.get(),         // Decoder
                     recovered_data.data(), // Output buffer
                     kMessageBytes          // Output buffer size
    );
  ASSERT_EQ(recover_result, Wirehair_Success)
    << "Recovery failed: " << wirehair_result_string(recover_result);

  // Verify recovered data matches original
  ASSERT_EQ(recovered_data.size(), original_data.size());
  EXPECT_TRUE(std::memcmp(recovered_data.data(),
                          original_data.data(),
                          kMessageBytes) == 0)
    << "Recovered data does not match original data";
}

TEST_F(WirehairTest, SmallMessageTest)
{
  // Test with a very small message
  const size_t kMessageBytes = 1024; // 1 KB
  const size_t kPacketSize = 128;    // Small packets

  // Create test data
  std::vector<uint8_t> original_data = create_test_data(kMessageBytes);

  // Create encoder
  WirehairCodecWrapper encoder;
  encoder.set(wirehair_encoder_create(
    nullptr, original_data.data(), kMessageBytes, kPacketSize));
  ASSERT_TRUE(encoder) << "Failed to create encoder for small message";

  // Create decoder
  WirehairCodecWrapper decoder;
  decoder.set(wirehair_decoder_create(nullptr, kMessageBytes, kPacketSize));
  ASSERT_TRUE(decoder) << "Failed to create decoder for small message";

  // Send all packets without loss to verify basic functionality
  unsigned blocks_needed = 0;
  bool decode_success = false;
  std::vector<uint8_t> packet(kPacketSize);

  for (unsigned block_id = 1; !decode_success; block_id++)
  {
    // Encode a packet
    uint32_t write_len = 0;
    WirehairResult encode_result = wirehair_encode(
      encoder.get(), block_id, packet.data(), kPacketSize, &write_len);
    ASSERT_EQ(encode_result, Wirehair_Success);

    // Decode the packet
    WirehairResult decode_result =
      wirehair_decode(decoder.get(), block_id, packet.data(), write_len);

    blocks_needed++;

    if (decode_result == Wirehair_Success)
    {
      decode_success = true;
      break;
    }
    else if (decode_result != Wirehair_NeedMore)
    {
      FAIL() << "Decode failed with error: " << decode_result;
    }

    // Prevent infinite loop
    ASSERT_LT(blocks_needed, 2 * kMessageBytes / kPacketSize)
      << "Too many blocks needed for decoding";
  }

  // Verify we could decode with reasonable overhead
  const unsigned original_blocks =
    (kMessageBytes + kPacketSize - 1) / kPacketSize;
  EXPECT_LE(blocks_needed, original_blocks + 2)
    << "Required too many blocks: " << blocks_needed << " vs original "
    << original_blocks;

  // Recover the original data
  std::vector<uint8_t> recovered_data(kMessageBytes);
  WirehairResult recover_result =
    wirehair_recover(decoder.get(), recovered_data.data(), kMessageBytes);
  ASSERT_EQ(recover_result, Wirehair_Success);

  // Verify recovered data matches original
  EXPECT_TRUE(std::memcmp(recovered_data.data(),
                          original_data.data(),
                          kMessageBytes) == 0);
}

TEST_F(WirehairTest, HighPacketLossTest)
{
  // Test parameters
  const size_t kMessageBytes = 50000; // 50 KB
  const size_t kPacketSize = 1000;    // 1 KB packets
  const int kLossRate = 50;           // 50% packet loss

  // Create test data
  std::vector<uint8_t> original_data = create_test_data(kMessageBytes);

  // Create encoder and decoder
  WirehairCodecWrapper encoder;
  encoder.set(wirehair_encoder_create(
    nullptr, original_data.data(), kMessageBytes, kPacketSize));
  ASSERT_TRUE(encoder);

  WirehairCodecWrapper decoder;
  decoder.set(wirehair_decoder_create(nullptr, kMessageBytes, kPacketSize));
  ASSERT_TRUE(decoder);

  // Simulate high packet loss
  unsigned block_id = 0;
  unsigned blocks_received = 0;
  bool decode_success = false;

  std::vector<uint8_t> packet(kPacketSize);
  while (!decode_success &&
         blocks_received <
           3 * kMessageBytes / kPacketSize) // Limit to avoid infinite loop
  {
    block_id++;

    // Skip packets based on loss rate
    if (block_id % 100 < kLossRate)
    {
      continue;
    }

    // Encode a packet
    uint32_t write_len = 0;
    WirehairResult encode_result = wirehair_encode(
      encoder.get(), block_id, packet.data(), kPacketSize, &write_len);
    ASSERT_EQ(encode_result, Wirehair_Success);

    // Attempt to decode
    WirehairResult decode_result =
      wirehair_decode(decoder.get(), block_id, packet.data(), write_len);

    blocks_received++;

    if (decode_result == Wirehair_Success)
    {
      decode_success = true;
      break;
    }
    else if (decode_result != Wirehair_NeedMore)
    {
      FAIL() << "Decode failed with error: "
             << wirehair_result_string(decode_result);
    }
  }

  // Verify we could decode despite high packet loss
  ASSERT_TRUE(decode_success)
    << "Failed to decode after " << blocks_received << " blocks with "
    << kLossRate << "% packet loss";

  // Recover and verify the original data
  std::vector<uint8_t> recovered_data(kMessageBytes);
  WirehairResult recover_result =
    wirehair_recover(decoder.get(), recovered_data.data(), kMessageBytes);
  ASSERT_EQ(recover_result, Wirehair_Success);

  EXPECT_TRUE(std::memcmp(recovered_data.data(),
                          original_data.data(),
                          kMessageBytes) == 0);
}
