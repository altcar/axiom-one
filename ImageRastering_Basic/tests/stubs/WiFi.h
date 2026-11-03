#pragma once
#include <cassert>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <algorithm>
#include <string>
inline unsigned long test_ms=100;
inline bool test_connection=true, test_accept=true;
inline unsigned long millis(){return test_ms;}
struct IPAddress {};
constexpr int WIFI_AP=1;
struct WiFiMock {
  void mode(int){}
  bool softAP(const char*,const char*){return true;}
  IPAddress softAPIP()const{return {};}
  int softAPgetStationNum()const{return 1;}
};
inline WiFiMock WiFi;
struct WiFiClient {
  int descriptor=-1;
  WiFiClient()=default;
  explicit WiFiClient(int n):descriptor(n){}
  bool connected()const{return descriptor>=0 && test_connection;}
  explicit operator bool()const{return connected();}
  int fd()const{return descriptor;}
  void stop(){descriptor=-1;}
  void setNoDelay(bool){}
  size_t available(){return 0;}
  int read(){return -1;}
  size_t print(const char*){assert(false && "scan must not use synchronous print");return 0;}
  size_t write(const uint8_t*,size_t){assert(false && "scan must not use synchronous write");return 0;}
};
struct WiFiServer {
  explicit WiFiServer(uint16_t){}
  void begin(){}
  void setNoDelay(bool){}
  WiFiClient accept(){if(test_accept){test_accept=false;return WiFiClient(42);}return {};}
};
