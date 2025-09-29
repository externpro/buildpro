#include <algorithm>
#include <cstdint>
#include <map>
#include <numeric>
#include <vector>

#include <fecpp/fecpp.h>
#include <gtest/gtest.h>

using fecpp::fec_code;
using std::size_t;
using std::uint8_t;

namespace
{

  // Create deterministic input of size K * block_size
  std::vector<uint8_t> make_input(size_t K, size_t block_size)
  {
    std::vector<uint8_t> in(K * block_size);
    for (size_t i = 0; i < in.size(); ++i)
    {
      in[i] = static_cast<uint8_t>((i * 131) ^ (i >> 3));
    }
    return in;
  }

} // namespace

TEST(FecppTest, EncodeDecodeRoundtrip)
{
  const size_t K = 4;            // number of primary shares
  const size_t N = 7;            // total shares generated
  const size_t block_size = 128; // bytes per share

  fec_code code(K, N);

  // Prepare input: K equal-sized blocks concatenated
  const std::vector<uint8_t> input = make_input(K, block_size);

  // Collect all N shares from encoder
  std::vector<std::vector<uint8_t>> shares(N, std::vector<uint8_t>(block_size));

  code.encode(
    input.data(),
    input.size(),
    [&](size_t share_id, size_t total_n, const uint8_t data[], size_t size)
    {
      ASSERT_EQ(total_n, N);
      ASSERT_EQ(size, block_size);
      ASSERT_LT(share_id, N);
      std::copy(data, data + size, shares[share_id].begin());
    });

  // Verify that the first K shares are identical to the input blocks
  // (systematic code)
  for (size_t i = 0; i < K; ++i)
  {
    const uint8_t* src = input.data() + i * block_size;
    EXPECT_TRUE(std::equal(src, src + block_size, shares[i].begin()));
  }

  // Simulate erasures: drop two shares including one primary and one parity
  // Ensure we still have at least K shares to decode
  std::vector<size_t> missing = {1, 5}; // arbitrary choices within [0, N)

  std::map<size_t, const uint8_t*> available;
  for (size_t i = 0; i < N; ++i)
  {
    if (std::find(missing.begin(), missing.end(), i) == missing.end())
    {
      available.emplace(i, shares[i].data());
    }
  }
  ASSERT_GE(available.size(), K);

  // Decode: collect recovered primary shares (indices 0..K-1)
  std::vector<std::vector<uint8_t>> recovered(K);
  for (auto& r : recovered)
    r.resize(block_size);

  code.decode(
    available,
    block_size,
    [&](size_t out_idx, size_t primary_k, const uint8_t data[], size_t size)
    {
      // decode() emits primary shares (systematic copies and/or reconstructed)
      ASSERT_EQ(primary_k, K);
      ASSERT_LT(out_idx, K);
      ASSERT_EQ(size, block_size);
      std::copy(data, data + size, recovered[out_idx].begin());
    });

  // Reassemble recovered input and compare with original
  std::vector<uint8_t> recovered_input;
  recovered_input.reserve(K * block_size);
  for (size_t i = 0; i < K; ++i)
  {
    recovered_input.insert(
      recovered_input.end(), recovered[i].begin(), recovered[i].end());
  }

  ASSERT_EQ(recovered_input.size(), input.size());
  EXPECT_TRUE(std::equal(input.begin(), input.end(), recovered_input.begin()));
}
