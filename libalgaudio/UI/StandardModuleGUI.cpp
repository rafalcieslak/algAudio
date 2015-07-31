/*
This file is part of AlgAudio.

AlgAudio, Copyright (C) 2015 CeTA - Audiovisual Technology Center

AlgAudio is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as
published by the Free Software Foundation, either version 3 of the
License, or (at your option) any later version.

AlgAudio is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with AlgAudio.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "UI/StandardModuleGUI.hpp"
#include "ModuleTemplate.hpp"
#include <SDL2/SDL.h>
#include "Theme.hpp"

namespace AlgAudio{

std::shared_ptr<StandardModuleGUI> StandardModuleGUI::CreateEmpty(std::shared_ptr<Window> w){
  return std::shared_ptr<StandardModuleGUI>( new StandardModuleGUI(w) );
}
std::shared_ptr<StandardModuleGUI> StandardModuleGUI::CreateFromXML(std::shared_ptr<Window> w, std::string xml_data, std::shared_ptr<ModuleTemplate> templ){
  auto ptr = std::shared_ptr<StandardModuleGUI>( new StandardModuleGUI(w) );
  ptr->LoadFromXML(xml_data, templ);
  return ptr;
}
std::shared_ptr<StandardModuleGUI> StandardModuleGUI::CreateFromTemplate(std::shared_ptr<Window> w, std::shared_ptr<ModuleTemplate> templ){
  auto ptr = std::shared_ptr<StandardModuleGUI>( new StandardModuleGUI(w) );
  ptr->LoadFromTemplate( templ);
  return ptr;
}

void StandardModuleGUI::CommonInit(){
  main_margin = UIMarginBox::Create(window,0,6,0,6);
  main_box = UIVBox::Create(window);
  inlets_box = UIHBox::Create(window);
  outlets_box = UIHBox::Create(window);
  caption = UILabel::Create(window, "", 16);
  caption->SetTextColor("standardbox-caption");

  main_box->Insert(inlets_box, UIBox::PackMode::TIGHT);
  main_box->Insert(caption, UIBox::PackMode::TIGHT);
  main_box->Insert(outlets_box, UIBox::PackMode::TIGHT);
  main_margin->Insert(main_box);
  main_margin->parent = shared_from_this();

   inlets_box->SetCustomSize(Size2D(0,4));
  outlets_box->SetCustomSize(Size2D(0,4));

  SetBackColor(Theme::Get("standardbox-bg"));
}

void StandardModuleGUI::LoadFromXML(std::string xml_data, std::shared_ptr<ModuleTemplate> templ){
  CommonInit();
  std::cout << "Building GUI from XML " << std::endl;
  caption->SetText(templ->name);
  UpdateMinimalSize();
}
void StandardModuleGUI::LoadFromTemplate(std::shared_ptr<ModuleTemplate> templ){
  CommonInit();
  std::cout << "Building GUI from template" << std::endl;
  caption->SetText(templ->name);

  for(auto& i : templ->inlets){
    auto inlet = IOConn::Create(window, i, VertAlignment_TOP, Theme::Get("standardbox-inlet"));
    inlets_box->Insert(inlet,UIBox::PackMode::WIDE);
    inlet->on_press.SubscribeForever([this, i](bool b){
      on_inlet_pressed.Happen(i, b);
    });
    inlets.push_back(inlet);
  }
  for(auto& o : templ->outlets){
    auto outlet = IOConn::Create(window, o, VertAlignment_BOTTOM, Theme::Get("standardbox-outlet"));
    outlets_box->Insert(outlet,UIBox::PackMode::WIDE);
    outlet->on_press.SubscribeForever([this, o](bool b){
      on_outlet_pressed.Happen(o, b);
    });
    outlets.push_back(outlet);
  }
  UpdateMinimalSize();
}

void StandardModuleGUI::OnChildRequestedSizeChanged(){
  UpdateMinimalSize();
}
void StandardModuleGUI::OnChildVisibilityChanged(){
  UpdateMinimalSize();
}

void StandardModuleGUI::UpdateMinimalSize(){
  SetMinimalSize(main_margin->GetRequestedSize());
}

void StandardModuleGUI::CustomDraw(DrawContext& c){
  // TODO: Store the color instead of getting it every time
  Color border_color = Theme::Get("standardbox-border");

  if(highlight) c.SetColor(border_color.Lighter(0.2));
  else c.SetColor(border_color);
  int w = c.Size().width;
  int h = c.Size().height;
  c.DrawLine(0,0,w-1,0);
  c.DrawLine(0,0,0,h-1);
  c.DrawLine(w-1,0,w-1,h-1);
  c.DrawLine(0,h-1,w-1,h-1);
  c.DrawLine(1,1,w-2,1);
  c.DrawLine(1,1,1,h-2);
  c.DrawLine(w-2,1,w-2,h-2);
  c.DrawLine(1,h-2,w-2,h-2);

  c.Push(Point2D(0,0),c.Size());
  main_margin->Draw(c);
  c.Pop();
}

void StandardModuleGUI::CustomResize(Size2D s){
  main_margin->Resize(s);
}

void StandardModuleGUI::SetHighlight(bool h){
  highlight = h;

  if(highlight) SetBackColor(Theme::Get("standardbox-bg").Lighter(0.03));
  else  SetBackColor(Theme::Get("standardbox-bg"));

  Color border_color = Theme::Get("standardbox-border");
  if(highlight) border_color = border_color.Lighter(0.2);
  for(auto& i :  inlets) i->SetBorderColor(border_color);
  for(auto& o : outlets) o->SetBorderColor(border_color);

  SetNeedsRedrawing();
}

std::shared_ptr<StandardModuleGUI::IOConn> StandardModuleGUI::IOConn::Create(std::weak_ptr<Window> w, std::string id, VertAlignment align, Color c){
  return std::shared_ptr<IOConn>( new IOConn(w, id, align, c) );
}

StandardModuleGUI::IOConn::IOConn(std::weak_ptr<Window> w, std::string id_, VertAlignment align_, Color c)
  : UIWidget(w), id(id_), align(align_), main_color(c), border_color(c){
  SetMinimalSize(Size2D(20,12));
  on_pointed.SubscribeForever([this](bool){
    SetNeedsRedrawing();
  });
  on_motion.SubscribeForever([this](Point2D pos){
    bool new_inside = pos.IsInside(GetRectPos(),Size2D(20,12));
    if(inside != new_inside){
      inside = new_inside;
      SetNeedsRedrawing();
    }
  });
};

Point2D StandardModuleGUI::IOConn::GetRectPos() const{
  int x = current_size.width/2 - 10;
  int y = 0;
  if(align == VertAlignment_TOP) y = 0;
  if(align == VertAlignment_CENTERED) y = current_size.height/2 - 6;
  if(align == VertAlignment_BOTTOM) y = current_size.height - 12;
  return Point2D(x,y);
}

bool StandardModuleGUI::IOConn::CustomMousePress(bool down, short b,Point2D pos){
  if(b == SDL_BUTTON_LEFT && pos.IsInside(GetRectPos(),Size2D(20,12))){
    on_press.Happen(down);
    return true;
  }
  return false;
}

void StandardModuleGUI::IOConn::CustomDraw(DrawContext& c){
  Point2D p = GetRectPos();
  const int x = p.x, y = p.y;
  if(pointed && inside) c.SetColor(main_color.Lighter(0.2));
  else c.SetColor(main_color);
  c.DrawRect(x,y,20,12);
  c.SetColor(border_color);
  if(align != VertAlignment_TOP) c.DrawLine(x,y,x+20,y);
  c.DrawLine(x,y,x,y+12);
  c.DrawLine(x+20,y,x+20,y+12);
  if(align != VertAlignment_BOTTOM) c.DrawLine(x,y+11,x+20,y+11);
}
void StandardModuleGUI::IOConn::SetBorderColor(Color c){
  border_color = c;
  SetNeedsRedrawing();
}

}
