module;

#include <cstddef>
#include <functional>
#include <map>
#include <memory>
#include <string>

export module Goom.Utils.Graphics.SmallImageBitmaps;

import Goom.Utils.Graphics.ImageBitmaps;
import Goom.Lib.GoomTypes;

export namespace GOOM::UTILS::GRAPHICS
{

class SmallImageBitmaps
{
public:
  enum class ImageNames : UnderlyingEnumType
  {
    CIRCLE,
    SPHERE,
    PINK_FLOWER,
    RED_FLOWER,
    ORANGE_FLOWER,
    WHITE_FLOWER,
  };
  static constexpr size_t MIN_IMAGE_SIZE = 3;
  static constexpr size_t MAX_IMAGE_SIZE = 21;

  explicit SmallImageBitmaps(const std::string& resourcesDirectory);
  SmallImageBitmaps(const SmallImageBitmaps&) noexcept = delete;
  SmallImageBitmaps(SmallImageBitmaps&&) noexcept      = delete;
  ~SmallImageBitmaps() noexcept;
  auto operator=(const SmallImageBitmaps&) noexcept -> SmallImageBitmaps& = delete;
  auto operator=(SmallImageBitmaps&&) noexcept -> SmallImageBitmaps&      = delete;

  [[nodiscard]] auto GetImageBitmap(ImageNames name, size_t res) const -> const ImageBitmap&;
  // void AddImageBitmap(const std::string& name, size_t res);

private:
  std::string m_resourcesDirectory;
  std::map<std::string, std::unique_ptr<const ImageBitmap>, std::less<>> m_bitmapImages;
  [[nodiscard]] auto GetImageBitmapPtr(ImageNames name, size_t sizeOfImageSquare) const
      -> std::unique_ptr<const ImageBitmap>;
  static auto GetImageKey(ImageNames name, size_t sizeOfImageSquare) -> std::string;
  [[nodiscard]] auto GetImageFilename(ImageNames name,
                                      size_t sizeOfImageSquare) const -> std::string;
};

} // namespace GOOM::UTILS::GRAPHICS
