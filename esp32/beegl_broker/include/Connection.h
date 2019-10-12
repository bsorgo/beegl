#ifndef Connection_h
#define Connection_h

/*
  Connection.h - Connection header file
  
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

#include "Log.h"
#include "Settings.h"
#include "Service.h"


class ConnectionProvider
{
public:
  ConnectionProvider(Settings *settings) {m_settings = settings;};
  virtual Client *getClient() {return nullptr;};
  virtual void checkConnect() {};
  virtual bool setup() { return true;};
  virtual void shutdown() {};
  virtual void suspend() {};
  virtual void resume() {};
  virtual const char getInboundType() { return 0xFF;}; 
  virtual const char getOutboundType() { return 0xFF;};
  virtual const char* getName() { return {0x00};};


protected:
  Settings *m_settings;
  
};

class Connection
{

public:
  Connection(Service* service, Settings *settings);
  Client *getClient();
  void checkConnect();
  bool setup();
  void shutdown();
  void suspend();
  void resume();
  void addConnectionProvider(ConnectionProvider *connection);
  

private:
  int getOutboundConnectionProviders(ConnectionProvider** providers, char outboundTypeMask=0xFF);
  Settings *m_settings;
  Service *m_service;
  ConnectionProvider* m_connection[5];
  int connectionSize = 0;

  void webServerBind();
};



#endif