#ifndef GOOM_DEBUG
#define GOOM_DEBUG
#endif

#include "catch2/catch.hpp"
#include "goom/goom_graphic.h"
#include "point2d.h"
#include "utils/math/mathutils.h"

#include <chrono>
#include <cstdint>
#include <memory>
#include <vector>

using GOOM::channel_limits;
using GOOM::MAX_ALPHA;
using GOOM::MAX_CHANNEL_VALUE_HDR;
using GOOM::MAX_COLOR_VAL;
using GOOM::MultiplyChannelColorByScalar;
using GOOM::MultiplyColorChannels;
using GOOM::Pixel;
using GOOM::PixelBuffer;
using GOOM::PixelChannelType;
using GOOM::PixelIntType;
using GOOM::UTILS::MATH::floats_equal;
using namespace std::chrono_literals;
using std::chrono::high_resolution_clock;
using std::chrono::microseconds;

constexpr size_t WIDTH = 1280;
constexpr size_t HEIGHT = 720;
constexpr PixelChannelType TEST_R = 200;
constexpr PixelChannelType TEST_G = 100;
constexpr PixelChannelType TEST_B = 50;

TEST_CASE("Pixels")
{
  SECTION("Pixel RGB")
  {
    const Pixel pixel{{TEST_R, TEST_G, TEST_B}};
    REQUIRE(floats_equal(static_cast<float>(TEST_R) / channel_limits<float>::max(), pixel.RFlt()));
    REQUIRE(floats_equal(static_cast<float>(TEST_G) / channel_limits<float>::max(), pixel.GFlt()));
    REQUIRE(floats_equal(static_cast<float>(TEST_B) / channel_limits<float>::max(), pixel.BFlt()));
  }
  SECTION("Pixel uint32_t RGB")
  {
    constexpr uint32_t RED = 299;
    constexpr uint32_t GREEN = 200;
    constexpr uint32_t BLUE = MAX_CHANNEL_VALUE_HDR + 10;
    constexpr uint32_t ALPHA = 256;
    const Pixel pixel{RED, GREEN, BLUE, ALPHA};
    REQUIRE(pixel.R() == RED);
    REQUIRE(pixel.G() == GREEN);
    REQUIRE(pixel.B() == MAX_CHANNEL_VALUE_HDR);
    REQUIRE(pixel.A() == 255);
  }
  SECTION("Pixel Set")
  {
    Pixel pixel{{TEST_R, TEST_G, TEST_B}};
    pixel.SetA(1);
    REQUIRE(1 == pixel.A());
    pixel.SetR(10);
    REQUIRE(10 == pixel.R());
    pixel.SetG(20);
    REQUIRE(20 == pixel.G());
    pixel.SetB(30);
    REQUIRE(30 == pixel.B());
  }
  SECTION("Pixel ==")
  {
    const Pixel pixel1{{TEST_R, TEST_G, TEST_B}};
    const Pixel pixel2{pixel1};
    const Pixel pixel3{{TEST_R + 1, TEST_G, TEST_B}};
    REQUIRE(pixel1 == pixel2);
    REQUIRE(!(pixel1 == pixel3));
    const Pixel pixelBlack{{0, 0, 0, 0}};
    REQUIRE(pixelBlack == Pixel::BLACK);
    const Pixel pixelWhite{{MAX_COLOR_VAL, MAX_COLOR_VAL, MAX_COLOR_VAL, MAX_ALPHA}};
    REQUIRE(pixelWhite == Pixel::WHITE);
  }
  SECTION("Pixel Multiply Scalar")
  {
    const Pixel pixel{{TEST_R, TEST_G, TEST_B}};
    const uint32_t rChannel = pixel.R();
    constexpr uint32_t SCALAR = 32;
    REQUIRE(MultiplyChannelColorByScalar(SCALAR, pixel.R()) ==
            ((SCALAR * rChannel) / MAX_COLOR_VAL));
  }
  SECTION("Pixel Multiply Channels")
  {
    const Pixel pixel{{TEST_R, TEST_G, TEST_B}};
    const uint32_t rChannel = pixel.R();
    const uint32_t gChannel = pixel.G();
    const uint32_t rgProduct = rChannel * gChannel;
    REQUIRE(MultiplyColorChannels(pixel.R(), pixel.G()) == rgProduct / MAX_COLOR_VAL);
  }
}

auto GetPixelCount(const PixelBuffer& buffer, const Pixel& pixel) -> uint32_t
{
  uint32_t count = 0;
  for (size_t y = 0; y < HEIGHT; ++y)
  {
    for (size_t x = 0; x < WIDTH; ++x)
    {
      if (buffer(x, y) == pixel)
      {
        count++;
      }
    }
  }
  return count;
}

TEST_CASE("PixelBuffers")
{
  SECTION("PixelBuffer copy")
  {
    PixelBuffer srceBuffer{WIDTH, HEIGHT};
    const Pixel testPixel{{TEST_R, TEST_G, TEST_B}};
    srceBuffer.Fill(testPixel);
    PixelBuffer destBuffer{WIDTH, HEIGHT};
    srceBuffer.CopyTo(destBuffer);
    REQUIRE(GetPixelCount(srceBuffer, testPixel) == GetPixelCount(destBuffer, testPixel));
  }

  SECTION("PixelBuffer copy time")
  {
    constexpr size_t NUM_LOOPS = 100;
    const Pixel testPixel{{TEST_R, TEST_G, TEST_B}};

    auto intSrceBuff = std::make_unique<std::vector<PixelIntType>>(WIDTH * HEIGHT);
    const PixelIntType intTestPixel = testPixel.Rgba();
    std::fill(intSrceBuff->begin(), intSrceBuff->end(), intTestPixel);
    auto intDestBuff = std::make_unique<std::vector<PixelIntType>>(WIDTH * HEIGHT);

    auto startTime = high_resolution_clock::now();
    for (size_t i = 0; i < NUM_LOOPS; ++i)
    {
      std::memmove(intDestBuff->data(), intSrceBuff->data(), WIDTH * HEIGHT * sizeof(PixelIntType));
    }
    auto finishTime = high_resolution_clock::now();
    const auto durationMemmove = std::chrono::duration_cast<microseconds>(finishTime - startTime);

    REQUIRE(std::count(intSrceBuff->cbegin(), intSrceBuff->cend(), intTestPixel) ==
    std::count(intDestBuff->cbegin(), intDestBuff->cend(), intTestPixel));

    auto srceBuffer = std::make_unique<PixelBuffer>(WIDTH, HEIGHT);
    srceBuffer->Fill(testPixel);
    auto destBuffer = std::make_unique<PixelBuffer>(WIDTH, HEIGHT);

    startTime = high_resolution_clock::now();
    for (size_t i = 0; i < NUM_LOOPS; ++i)
    {
      srceBuffer->CopyTo(*destBuffer);
    }
    finishTime = high_resolution_clock::now();
    const auto durationCopyTo = std::chrono::duration_cast<microseconds>(finishTime - startTime);

    REQUIRE(GetPixelCount(*srceBuffer, testPixel) == GetPixelCount(*destBuffer, testPixel));

    const auto tolerance = (durationCopyTo * 10) / 100;
    UNSCOPED_INFO("durationCopyTo.count() = " << durationCopyTo.count());
    UNSCOPED_INFO("durationMemmove.count() = " << durationMemmove.count());
    UNSCOPED_INFO("tolerance.count() = " << tolerance.count());
    REQUIRE(durationCopyTo < (durationMemmove + tolerance));
  }

  SECTION("PixelBuffer Get4RHBNeighbours")
  {
    PixelBuffer buffer{WIDTH, HEIGHT};
    const Pixel testPixel1{{TEST_R+0, TEST_G+0, TEST_B+0}};
    const Pixel testPixel2{{TEST_R+1, TEST_G+1, TEST_B+1}};
    const Pixel testPixel3{{TEST_R+2, TEST_G+2, TEST_B+2}};
    const Pixel testPixel4{{TEST_R+3, TEST_G+3, TEST_B+3}};
    buffer.Fill(Pixel::WHITE);

    constexpr size_t X = 20;
    constexpr size_t Y = 40;
    buffer(X, Y) = testPixel1;
    buffer(X+1, Y) = testPixel2;
    buffer(X, Y+1) = testPixel3;
    buffer(X+1, Y+1) = testPixel4;

    const std::array<Pixel, 4> pixel4RHBNeighbours = buffer.Get4RHBNeighbours(X, Y);

    REQUIRE(testPixel1 == pixel4RHBNeighbours[0]);
    REQUIRE(testPixel2 == pixel4RHBNeighbours[1]);
    REQUIRE(testPixel3 == pixel4RHBNeighbours[2]);
    REQUIRE(testPixel4 == pixel4RHBNeighbours[3]);
  }

  SECTION("PixelBuffer Get4RHBNeighbours Range Error")
  {
    PixelBuffer buffer{WIDTH, HEIGHT};

    REQUIRE_THROWS_AS(buffer.Get4RHBNeighbours(WIDTH - 1, HEIGHT - 1), PixelBuffer::RangeError);
    REQUIRE_THROWS_AS(buffer.Get4RHBNeighbours(WIDTH - 1, 40), PixelBuffer::RangeError);
    REQUIRE_THROWS_AS(buffer.Get4RHBNeighbours(10, HEIGHT - 1), PixelBuffer::RangeError);
  }

  SECTION("PixelBuffer RowIter")
  {
    const Pixel testPixel{{TEST_R, TEST_G, TEST_B}};
    PixelBuffer buffer{WIDTH, HEIGHT};
    buffer.Fill(Pixel::WHITE);
    constexpr size_t Y = 10;
    for (size_t x = 0; x < WIDTH; ++x)
    {
      buffer(x, Y) = testPixel;
    }

    uint32_t count = 0;
    const auto destRowIter = buffer.GetRowIter(Y);
    const auto destRowBegin = std::get<0>(destRowIter);
    const auto destRowEnd = std::get<1>(destRowIter);
    for (auto destRowBuff = destRowBegin; destRowBuff != destRowEnd; ++destRowBuff)
    {
      if (*destRowBuff == testPixel)
      {
        count++;
      }
    }
    REQUIRE(count == WIDTH);
  }
}
