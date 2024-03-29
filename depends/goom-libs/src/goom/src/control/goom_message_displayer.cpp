//#undef NO_LOGGING

#include "goom_message_displayer.h"

#include "color/color_utils.h"
#include "draw/goom_draw.h"
#include "draw/shape_drawers/text_drawer.h"
#include "goom/goom_graphic.h"
#include "goom/goom_logger.h"
#include "goom/goom_types.h"
#include "goom/point2d.h"

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace GOOM::CONTROL
{

using COLOR::GetBrighterColor;
using DRAW::IGoomDraw;
using DRAW::SHAPE_DRAWERS::TextDrawer;

GoomMessageDisplayer::GoomMessageDisplayer(IGoomDraw& draw,
                                           const std::string& updateMessagesFontFile)
  : m_draw{&draw}, m_updateMessagesFontFile{updateMessagesFontFile}
{
}

auto GoomMessageDisplayer::GetUpdateMessagesDisplayers(const size_t numDisplayers,
                                                       IGoomDraw& textOutput,
                                                       const std::string& updateMessagesFontFile)
    -> std::vector<TextDrawer>
{

  const auto textColor    = GetBrighterColor(10.0F, WHITE_PIXEL);
  const auto getFontColor = [textColor]([[maybe_unused]] const size_t textIndexOfChar,
                                        [[maybe_unused]] const Point2dInt& point,
                                        [[maybe_unused]] const Dimensions& charDimensions)
  { return textColor; };

  static constexpr auto OUTLINE_GREY = 0xFA;
  const auto outlineColor =
      GetBrighterColor(2.0F,
                       Pixel{
                           {OUTLINE_GREY, OUTLINE_GREY, OUTLINE_GREY, MAX_ALPHA}
  });
  const auto getOutlineFontColor = [outlineColor]([[maybe_unused]] const size_t textIndexOfChar,
                                                  [[maybe_unused]] const Point2dInt& point,
                                                  [[maybe_unused]] const Dimensions& charDimensions)
  { return outlineColor; };

  auto updateMessagesDisplayers = std::vector<TextDrawer>{};

  for (auto i = 0U; i < numDisplayers; ++i)
  {
    auto& displayer = updateMessagesDisplayers.emplace_back(textOutput);

    displayer.SetFontFile(updateMessagesFontFile);
    displayer.SetFontSize(MSG_FONT_SIZE);
    displayer.SetOutlineWidth(1);
    displayer.SetAlignment(TextDrawer::TextAlignment::LEFT);
    displayer.SetFontColorFunc(getFontColor);
    displayer.SetOutlineFontColorFunc(getOutlineFontColor);
    displayer.SetParallelRender(false);
  }

  return updateMessagesDisplayers;
}

void GoomMessageDisplayer::UpdateMessages(const std::vector<std::string>& msgLines)
{
  if (msgLines.empty())
  {
    return;
  }

  const auto numberOfLinesInMessage = msgLines.size();
  if (numberOfLinesInMessage > m_updateMessagesDisplayers.size())
  {
    LogInfo("Changing number of displayers from {} to {}.",
            m_updateMessagesDisplayers.size(),
            numberOfLinesInMessage); // NOLINT
    m_updateMessagesDisplayers =
        GetUpdateMessagesDisplayers(numberOfLinesInMessage, *m_draw, m_updateMessagesFontFile);
  }

  static constexpr auto VERTICAL_SPACING = 10;
  static constexpr auto LINE_HEIGHT      = MSG_FONT_SIZE + VERTICAL_SPACING;
  static constexpr auto X_POS            = 30;
  static constexpr auto Y_START          = 10;

  const auto totalMessagesHeight = 20U + (LINE_HEIGHT * numberOfLinesInMessage);

  const auto displayMessage =
      [this, &totalMessagesHeight, &numberOfLinesInMessage, &msgLines](const size_t i)
  {
    const auto yPos = static_cast<int32_t>((Y_START + totalMessagesHeight) -
                                           ((numberOfLinesInMessage - i) * LINE_HEIGHT));
    m_updateMessagesDisplayers.at(i).SetText(msgLines[i]);
    m_updateMessagesDisplayers.at(i).Prepare();
    m_updateMessagesDisplayers.at(i).Draw({X_POS, yPos});
  };

  //m_parallel.ForLoop(numberOfLinesInMessage, displayMessage);
  for (auto i = 0U; i < numberOfLinesInMessage; ++i)
  {
    displayMessage(i);
  }
}

} // namespace GOOM::CONTROL
