#ifndef TEXTRENDERER_HPP
#define TEXTRENDERER_HPP
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
#include <memory>
#include <string>
#include <map>
#include "SDLHandle.hpp"
#include "Color.hpp"

struct _TTF_Font;
typedef _TTF_Font TTF_Font;
struct SDL_Color;

namespace AlgAudio{

class SDLTexture;
class SDLTextTexture;
class Window;

struct FontParrams{
  FontParrams(std::string n, int s) : name(n), size(s) {}
  std::string name;
  int size;
  bool operator<(const FontParrams& other) const{ return name<other.name || (name==other.name && size < other.size);}
};

// TODO: Instantiable class, SDLHandle tracking, font unloading on destruction
class TextRenderer{
  TextRenderer() = delete; // static class
public:
  static std::shared_ptr<SDLTextTexture> RenderShaded(std::weak_ptr<Window>, FontParrams, std::string);
  static std::shared_ptr<SDLTextTexture> Render(std::weak_ptr<Window>, FontParrams, std::string);
  static TTF_Font* GetFont(FontParrams);
  static TTF_Font* Preload(FontParrams);
private:
  // Temporarily it is assumed that any rendering will be performed only if
  // at least one window exists, thus it should be safe to assume that SDL
  // is always initialised.
  // SDLHandle handle;
  static std::map<FontParrams, TTF_Font*> fontbank;
};


} // namespace AlgAudio

#endif // TEXTRENDERER_HPP
