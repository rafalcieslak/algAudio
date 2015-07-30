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
#include "Canvas.hpp"
#include <algorithm>

namespace AlgAudio{

Canvas::Canvas(){

}

std::shared_ptr<Canvas> Canvas::CreateEmpty(){
  return std::shared_ptr<Canvas>( new Canvas() );
}

void Canvas::InsertModule(std::shared_ptr<Module> m){
  modules.emplace_back(m);
  m->canvas = shared_from_this();
}

void Canvas::RemoveModule(std::shared_ptr<Module> m){
  modules.erase(std::remove(modules.begin(), modules.end(), m), modules.end());
}

} // namespace AlgAudio
