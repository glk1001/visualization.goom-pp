#pragma once

#include "draw/shape_drawers/text_drawer.h"
#include "utils/parallel_utils.h"

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace GOOM
{

namespace DRAW
{
class IGoomDraw;
}

namespace CONTROL
{

class GoomMessageDisplayer
{
public:
  GoomMessageDisplayer(DRAW::IGoomDraw& draw, const std::string& updateMessagesFontFile);

  void UpdateMessages(const std::vector<std::string>& msgLines);

private:
  UTILS::Parallel m_parallel{UTILS::GetNumAvailablePoolThreads()};
  DRAW::IGoomDraw* m_draw;
  std::string m_updateMessagesFontFile;

  static constexpr int32_t MSG_FONT_SIZE         = 9;
  static constexpr size_t DEFAULT_NUM_DISPLAYERS = 1;
  std::vector<DRAW::SHAPE_DRAWERS::TextDrawer> m_updateMessagesDisplayers{
      GetUpdateMessagesDisplayers(DEFAULT_NUM_DISPLAYERS, *m_draw, m_updateMessagesFontFile)};
  [[nodiscard]] static auto GetUpdateMessagesDisplayers(size_t numDisplayers,
                                                        DRAW::IGoomDraw& textOutput,
                                                        const std::string& updateMessagesFontFile)
      -> std::vector<DRAW::SHAPE_DRAWERS::TextDrawer>;
};

} // namespace CONTROL
} // namespace GOOM
