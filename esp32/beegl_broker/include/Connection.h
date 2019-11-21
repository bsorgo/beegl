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

#define STR_INBOUNDMODE "inM"
#define STR_OUTBOUNDMODE "outM"

#define TAG_CONNECTION "CONNECTION"

namespace beegl
{
class Connection;

class ConnectionProvider : public ISettingsHandler
{
public:
  ConnectionProvider(Connection *connection, Settings *settings);
  virtual Client *getClient() const { return nullptr; };
  virtual void checkConnect(){};
  virtual bool setup() { return true; };
  virtual void shutdown(){};
  virtual void suspend(){};
  virtual void resume(){};
  virtual const char getInboundType() { return 0x00; };
  virtual const char getOutboundType() { return 0x00; };
  virtual const char compatibleInboundType() { return 0x00;}
  virtual const char *getName() { return "No connection";};

protected:
  Connection *m_connection;
};



class Connection : public ISettingsHandler, public IShutdownHandler
{

public:
  Connection(Service *service, Settings *settings);
  Client *getClient();
  void checkConnect();
  bool setup();
  void shutdown();
  void suspend();
  void resume();
  int registerConnectionProvider(ConnectionProvider *connection);

  void readSettings(const JsonObject &source) override;
  void writeSettings(JsonObject &target, const JsonObject &input) override;

  void onShutdown() override;

  void setOutboundMode(const char outboundMode)
  {
    m_outboundMode = outboundMode;
  }
  char getOutboundMode()
  {
    return m_outboundMode;
  }

  void setInboundMode(const char inboundMode)
  {
    m_inboundMode = inboundMode;
  }
  char getInboundMode()
  {
    return m_inboundMode;
  }

private:
  int getOutboundConnectionProviders(ConnectionProvider **providers, char outboundTypeMask = 0xFF);
  int getInboundConnectionProviders(std::vector<std::pair<char, String>>& list, char inboundTypeMask = 0xFF);
  Service *m_service;
  ConnectionProvider *m_connection[5];
  int connectionSize = 0;
  void webServerBind();

  char m_outboundMode = 0x1;
  char m_inboundMode = 0x0;
};
} // namespace beegl
#endif