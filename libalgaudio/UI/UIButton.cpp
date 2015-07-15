#include "UI/UIButton.hpp"
#include <iostream>
#include "TextRenderer.hpp"
#include "Theme.hpp"

namespace AlgAudio{

UIButton::UIButton(std::weak_ptr<Window> w, std::string t) : UIClickable(w), text(t){
  bg_color = Theme::Get("bg-button-neutral");
  text_color = Theme::Get("text-button");
  UpdateTexture();

  // UIClickable events
  on_pointed.Subscribe([&](bool){
    UpdateTexture();
  });
  on_pressed.Subscribe([&](bool){
    UpdateTexture();
  });
}

std::shared_ptr<UIButton> UIButton::Create(std::weak_ptr<Window> w, std::string text){
  std::shared_ptr<UIButton> res(new UIButton(w,text));
  return res;
}

Color UIButton::GetBgColor() const{
  if(pressed)
    return bg_color.Lighter(0.16);
  else if(pointed)
    return bg_color.Lighter(0.07);
  else
    return bg_color;
}

void UIButton::CustomDraw(DrawContext& c){
  c.SetColor(GetBgColor());
  c.DrawRect(0,0,c.width,c.height);

  c.SetColor(bg_color.Darker(0.15));
  c.DrawLine(0,0,c.width-1,0);
  c.DrawLine(0,0,0,c.height-1);
  c.DrawLine(c.width-1,0,c.width-1,c.height-1);
  c.DrawLine(0,c.height-1,c.width-1,c.height-1);
  c.DrawLine(1,1,c.width-2,1);
  c.DrawLine(1,1,1,c.height-2);
  c.DrawLine(c.width-2,1,c.width-2,c.height-2);
  c.DrawLine(1,c.height-2,c.width-2,c.height-2);

  c.SetColor(text_color);
  c.DrawTexture(
    texture,
    c.width/2  - texture->GetSize().width/2  + ((pressed)?2:0),
    c.height/2 - texture->GetSize().height/2 + ((pressed)?1:0)
  );
}

void UIButton::SetText(std::string t){
  text = t;
  UpdateTexture();
}

void UIButton::SetColors(Color text, Color background){
  text_color = text;
  bg_color = background;
  UpdateTexture();
  SetNeedsRedrawing();
}

void UIButton::UpdateTexture(){
  // Todo: Blended render, so that the same text texture can be used for any BG
  texture = TextRenderer::Render(window, FontParrams("Dosis-Regular",16), text, text_color, GetBgColor());
  SetRequestedSize(texture->GetSize() + Size2D(10,10));
  SetNeedsRedrawing();
}

} // namespace AlgAudio
