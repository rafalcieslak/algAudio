#ifndef __unix
  #include <winsock2.h>
#endif
#include "SCLang.hpp"
#include <lo/lo.h>
#include <lo/lo_cpp.h>
#include <iostream>
#include "SCLangSubprocess.hpp"
#include "ModuleTemplate.hpp"

namespace AlgAudio{

std::unique_ptr<SCLangSubprocess> SCLang::subprocess;
std::set<std::string> SCLang::installed_templates;
Signal<std::string> SCLang::on_line_received;
Signal<> SCLang::on_start_completed;
lo::Address addr("");
bool SCLang::osc_debug = false;

void SCLang::Start(std::string command){
  if(subprocess) return;
  subprocess = std::make_unique<SCLangSubprocess>(command);
  subprocess->on_any_line_received.Subscribe([&](std::string l){
    on_line_received.Happen(l);
  });
  subprocess->on_started.SubscribeOnce([](){ SCLang::Start2(); });
  subprocess->Start();
}
void SCLang::Start2(){
  // The SC dir should be in current directory.
  SetOSCDebug(osc_debug);
  // TODO: Check if the directories and files exist.
  SendInstruction("(\"sc/main.sc\").loadPaths;");
  subprocess->SendInstruction("NetAddr.localAddr.port.postln;", [&](std::string port){
    port = Utilities::SplitString(port,"\n")[1];
    std::cout << "SCLang is using port " << port << std::endl;
    // The constructor for lo::Address is messed up. Creating an lo_address
    // manually and binding it to lo::Address fixes the issue.
    lo_address a = lo_address_new("localhost", port.c_str());
    addr = lo::Address(a,true);
  });
  on_start_completed.Happen();
}

void SCLang::Restart(std::string command){
  Stop();
  Start(command);
}
void SCLang::Stop(){
  subprocess.reset(); // Resets the unique_ptr, not the process.
}
void SCLang::SendInstruction(std::string i){
  if(subprocess) subprocess->SendInstruction(i);
}
void SCLang::SendOSCSimple(std::string a){
  if(!subprocess) return;
  addr.send(a);
}
void SCLang::SendOSC(const std::string &path, const std::string &tag, ...)
{
    va_list q;
    va_start(q, tag);
    lo_message m = lo_message_new();
    std::string t = tag + "$$";
    lo_message_add_varargs(m, t.c_str(), q);
    addr.send(path, m);
    lo_message_free(m);
}

void SCLang::Poll(){
  if(subprocess) subprocess->TriggerSignals();
}
void SCLang::SetOSCDebug(bool enabled){
  if(enabled) SendInstruction("OSCFunc.trace(true);");
  else SendInstruction("OSCFunc.trace(false);");
  osc_debug = enabled;
}
void SCLang::InstallTemplate(const ModuleTemplate& t){
  if(!t.has_sc_code) return;
  SendOSC("/algaudioSC/installtemplate", "ss", t.GetFullID().c_str(), t.sc_code.c_str());
  installed_templates.insert(t.GetFullID());
}
bool SCLang::WasInstalled(const std::string& s){
  auto it = installed_templates.find(s);
  return (it != installed_templates.end());
}

} // namespace AlgAudio
