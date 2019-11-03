/*
  LogManagement.h - Log management interface 
  
  This file is part of the BeeGl distribution (https://github.com/bsorgo/beegl).
  Copyright (c) 2019 Bostjan Sorgo
  
  This program is free software: you can redistribute it and/or modify  
  it under the terms of the GNU General Public License as published by  
  the Free Software Foundation, version 3.
 
  This program is distributed in the hope that it will be useful, but 
  WITHOUT ANY WARRANTY; without even the implied warranty of 
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU 
  General Public License for more details.
 
  You should have received a copy of the GNU General Public License 
  along with this program. If not, see <http://www.gnu.org/licenses/>.

*/

#ifndef LogManagement_h
#define LogManagement_h

#include "Log.h"
#include "Settings.h"
#include "Service.h"
namespace beegl
{
class LogManagement : public ISettingsHandler
{
public:
  LogManagement(Settings *settings, Service *service);

private:
  Service *m_server;
  void webServerBind();
};
} // namespace beegl
#endif