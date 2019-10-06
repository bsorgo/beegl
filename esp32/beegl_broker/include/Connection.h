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

#include <WiFiClient.h>

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
  virtual char getInboundType() { return 0x00;}; 
  virtual char getOutboundType() {return 0x00;};
  virtual const char* getName() { return EMPTY;};

protected:
  Settings *m_settings;
private:
  const char EMPTY[1] = ""; 

};

class Connection
{

public:
  Connection(Settings *settings);
  Client *getClient();
  void checkConnect();
  bool setup();
  void shutdown();
  void suspend();
  void resume();
  void addConnectionProvider(ConnectionProvider *connection);
  
private:
  Settings *m_settings;
  ConnectionProvider* m_connection[5];
  int connectionSize = 0;
};



#endif