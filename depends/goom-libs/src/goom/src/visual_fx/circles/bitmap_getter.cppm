module;

#include <algorithm>
#include <cstddef>

export module Goom.VisualFx.CirclesFx.BitmapGetter;

import Goom.Utils.Graphics.ImageBitmaps;
import Goom.Utils.Graphics.SmallImageBitmaps;
import Goom.Utils.Math.GoomRand;

export namespace GOOM::VISUAL_FX::CIRCLES
{

class IBitmapGetter
{
public:
  IBitmapGetter() noexcept                               = default;
  IBitmapGetter(const IBitmapGetter&) noexcept           = delete;
  IBitmapGetter(IBitmapGetter&&) noexcept                = delete;
  virtual ~IBitmapGetter()                               = default;
  auto operator=(const IBitmapGetter&) -> IBitmapGetter& = delete;
  auto operator=(IBitmapGetter&&) -> IBitmapGetter&      = delete;

  [[nodiscard]] virtual auto GetBitmap(size_t size) const noexcept
      -> const UTILS::GRAPHICS::ImageBitmap& = 0;
};

class BitmapGetter : public IBitmapGetter
{
public:
  static constexpr size_t MIN_DOT_DIAMETER = 5;
  static constexpr size_t MAX_DOT_DIAMETER = 21;

  BitmapGetter(const UTILS::MATH::GoomRand& goomRand,
               const UTILS::GRAPHICS::SmallImageBitmaps& smallBitmaps) noexcept;

  [[nodiscard]] auto GetBitmap(size_t size) const noexcept
      -> const UTILS::GRAPHICS::ImageBitmap& override;

  auto ChangeCurrentBitmap() noexcept -> void;

private:
  const UTILS::GRAPHICS::SmallImageBitmaps* m_smallBitmaps;
  UTILS::MATH::Weights<UTILS::GRAPHICS::SmallImageBitmaps::ImageNames> m_bitmapTypes;
  UTILS::GRAPHICS::SmallImageBitmaps::ImageNames m_currentBitmapName{
      m_bitmapTypes.GetRandomWeighted()};
};

} // namespace GOOM::VISUAL_FX::CIRCLES

module :private;

namespace GOOM::VISUAL_FX::CIRCLES
{

using UTILS::GRAPHICS::SmallImageBitmaps;

using enum SmallImageBitmaps::ImageNames;

static constexpr auto IMAGE_NAMES_CIRCLE_WEIGHT        = 05.0F;
static constexpr auto IMAGE_NAMES_SPHERE_WEIGHT        = 05.0F;
static constexpr auto IMAGE_NAMES_ORANGE_FLOWER_WEIGHT = 20.0F;
static constexpr auto IMAGE_NAMES_PINK_FLOWER_WEIGHT   = 20.0F;
static constexpr auto IMAGE_NAMES_RED_FLOWER_WEIGHT    = 20.0F;
static constexpr auto IMAGE_NAMES_WHITE_FLOWER_WEIGHT  = 10.0F;

BitmapGetter::BitmapGetter(const UTILS::MATH::GoomRand& goomRand,
                           const SmallImageBitmaps& smallBitmaps) noexcept
  : m_smallBitmaps{&smallBitmaps},
    m_bitmapTypes{
      goomRand,
      {
          {.key = CIRCLE,        .weight = IMAGE_NAMES_CIRCLE_WEIGHT},
          {.key = SPHERE,        .weight = IMAGE_NAMES_SPHERE_WEIGHT},
          {.key = ORANGE_FLOWER, .weight = IMAGE_NAMES_ORANGE_FLOWER_WEIGHT},
          {.key = PINK_FLOWER,   .weight = IMAGE_NAMES_PINK_FLOWER_WEIGHT},
          {.key = RED_FLOWER,    .weight = IMAGE_NAMES_RED_FLOWER_WEIGHT},
          {.key = WHITE_FLOWER,  .weight = IMAGE_NAMES_WHITE_FLOWER_WEIGHT},
        }
    }
{
}

auto BitmapGetter::GetBitmap(const size_t size) const noexcept
    -> const UTILS::GRAPHICS::ImageBitmap&
{
  return m_smallBitmaps->GetImageBitmap(m_currentBitmapName,
                                        std::clamp(size, MIN_DOT_DIAMETER, MAX_DOT_DIAMETER));
}

auto BitmapGetter::ChangeCurrentBitmap() noexcept -> void
{
  m_currentBitmapName = m_bitmapTypes.GetRandomWeighted();
}

} // namespace GOOM::VISUAL_FX::CIRCLES
