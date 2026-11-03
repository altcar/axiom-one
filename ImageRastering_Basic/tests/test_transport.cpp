// Compile the actual transport against a deterministic nonblocking socket stub.
#include "../firmware/ImageRastering/RobotWifiAP.cpp"
#include <iostream>
#include <limits>
std::string wire_bytes;
size_t send_limit=512, send_calls=0;
int send_error=0;
unsigned long send_delay=0;
ssize_t send(int fd,const void *data,size_t len,int flags) {
  assert(fd==42 && flags==MSG_DONTWAIT && len<=512);
  ++send_calls; test_ms+=send_delay;
  if(send_error){errno=send_error;return -1;}
  const size_t n=std::min(len,send_limit);
  wire_bytes.append(static_cast<const char*>(data),n);
  return ssize_t(n);
}
RobotWifiAP_c connected_server() {
  test_ms=100;test_connection=true;test_accept=true;
  wire_bytes.clear();send_calls=0;send_limit=512;send_error=0;send_delay=0;
  RobotWifiAP_c s("test-only", "test-only", 80);
  assert(s.begin());s.update();assert(s.clientConnected());return s;
}
int main() {
  auto s=connected_server();
  std::string expected;
  send_limit=17;
  // Force partial writes and many ring wraparounds. Byte order and newlines
  // must survive; printf itself must never touch the socket.
  for(int i=0;i<160;++i) {
    const std::string line="record,"+std::to_string(i)+","+std::string(150,'x')+"\n";
    const size_t calls=send_calls;
    assert(s.printf("%s",line.c_str()));assert(send_calls==calls);
    expected+=line;
    while(s.pendingBytes()) {
      const size_t before=send_calls;
      s.update();assert(send_calls==before+1);++test_ms;
    }
    assert(s.telemetryOK());
  }
  assert(wire_bytes==expected);

  s=connected_server();assert(s.printf("first\nsecond\n"));
  const size_t queued=s.pendingBytes();send_error=EAGAIN;
  for(int i=0;i<10;++i) {
    s.update();assert(send_calls==size_t(i+1));assert(s.pendingBytes()==queued);
    assert(s.telemetryOK());test_ms+=10;
  }
  assert(!s.telemetryOK());s.update(); // latch timeout before accepting progress
  send_error=0;s.update();
  assert(wire_bytes=="first\nsecond\n" && s.pendingBytes()==0 && !s.telemetryOK());

  s=connected_server();send_error=EINTR;assert(s.printf("retry\n"));s.update();
  assert(s.telemetryOK() && s.pendingBytes()==6);
  send_error=ENOBUFS;s.update();assert(s.telemetryOK() && s.pendingBytes()==6);
  send_error=0;s.update();assert(wire_bytes=="retry\n");

  s=connected_server();assert(s.printf("pending\n"));send_error=EPIPE;s.update();
  assert(!s.telemetryOK() && s.pendingBytes()==8);

  s=connected_server();
  for(int i=0;i<18;++i) assert(s.printf("%s",std::string(200,'x').c_str()));
  assert(s.pendingBytes()==3600 && !s.telemetryOK());
  assert(s.printf("# stop,reason=telemetry_backpressure,row=1\n"));
  while(s.pendingBytes())s.update();
  assert(wire_bytes.find("# stop,reason=telemetry_backpressure,row=1\n")!=std::string::npos);

  s=connected_server();assert(!s.printf("%s",std::string(256,'x').c_str()));
  assert(s.pendingBytes()==0 && !s.telemetryOK()); // no truncated CSV fragment
  s=connected_server();
  for(int i=0;i<20;++i)assert(s.printf("%s",std::string(200,'x').c_str()));
  assert(!s.printf("%s",std::string(200,'y').c_str()));
  assert(s.pendingBytes()==4000 && !s.telemetryOK());
  while(s.pendingBytes())s.update();assert(wire_bytes==std::string(4000,'x'));

  s=connected_server();assert(s.printf("send-timing\n"));send_delay=671;s.update();
  assert(s.maxSendMs()==671); // report scheduler delay even with MSG_DONTWAIT

  s=connected_server();assert(s.printf("old-client\n"));test_connection=false;s.update();
  assert(!s.telemetryOK() && !s.clientConnected() && s.pendingBytes()==0);
  // A socket can already have been closed by the network read API. Never
  // send its queued records into a newly accepted connection.
  s=connected_server();assert(s.printf("old-client\n"));s.disconnectClient();
  test_accept=true;test_ms+=100;s.update();
  assert(s.clientConnected() && !s.telemetryOK() && s.pendingBytes()==0 && wire_bytes.empty());
  std::cout<<"PASS: actual transport queues whole records, preserves partial writes across wraparound, performs one nonblocking send per update, and reports stalls/overflow/disconnects without truncating CSV.\n";
}
