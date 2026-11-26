#pragma once
#include <nlohmann/json.hpp>
#include <openssl/evp.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <thread>
#include <vector>
#include <stdexcept>
#include <cstdlib>
#include <cmath>
using J=nlohmann::json;
namespace fs=std::filesystem;
using namespace std::chrono_literals;
static void check(bool ok, const std::string& why) {if(!ok)throw std::runtime_error(why);}
class Browser {
    pid_t pid; int send_fd, recv_fd, seq=0;
    std::string session, pending,profile;
public:
    Browser() {
        char tmp[]="/tmp/qi-motion-chrome-XXXXXX";profile=mkdtemp(tmp);
        int tx[2],rx[2];check(pipe(tx)==0&&pipe(rx)==0,"pipe");
        pid=fork();check(pid!=-1,"fork");
        if(pid==0){
            int a=fcntl(tx[0],F_DUPFD,16),b=fcntl(rx[1],F_DUPFD,16);
            dup2(a,3);dup2(b,4);
            int nullfd=open("/dev/null",O_RDWR);dup2(nullfd,1);dup2(nullfd,2);
            close_range(5,~0u,0);
            std::string arg="--user-data-dir="+profile;
            execlp("google-chrome","google-chrome","--headless","--no-sandbox","--disable-gpu","--disable-background-networking","--remote-debugging-pipe",arg.c_str(),"about:blank",(char*)nullptr);_exit(1);
        }
        close(tx[0]);close(rx[1]);send_fd=tx[1];recv_fd=rx[0];
        auto id=cmd("Target.createTarget",{{"url","about:blank"}})["targetId"];
        session=cmd("Target.attachToTarget",{{"targetId",id},{"flatten",true}})["sessionId"].get<std::string>();
        cmd("Page.enable");cmd("Runtime.enable");
    }
    ~Browser(){close(send_fd);close(recv_fd);kill(pid,SIGTERM);waitpid(pid,nullptr,0);std::error_code ec;fs::remove_all(profile,ec);}
    J cmd(std::string method,J params=J::object()){
        J request={{"id",++seq},{"method",method},{"params",params}};
        if(!session.empty())request["sessionId"]=session;
        auto msg=request.dump();msg+='\0';size_t n=0;
        while(n<msg.size()){ssize_t sent=write(send_fd,msg.data()+n,msg.size()-n);check(sent>0,"write Chrome");n+=sent;}
        for(;;){
            size_t split=pending.find('\0');
            if(split==pending.npos){char buf[65536];auto got=read(recv_fd,buf,sizeof buf);check(got>0,"read Chrome "+method);pending.append(buf,got);continue;}
            auto result=J::parse(pending.substr(0,split));pending.erase(0,split+1);
            if(result.value("method","")=="Runtime.exceptionThrown")throw std::runtime_error(result.dump());
            if(result.value("id",-1)!=seq)continue;
            check(!result.contains("error"),result.dump());return result["result"];
        }
    }
    J js(std::string exp){auto result=cmd("Runtime.evaluate",{{"expression",exp},{"awaitPromise",true},{"returnByValue",true}});check(!result.contains("exceptionDetails"),exp+" "+result.dump());return result["result"].value("value",J(nullptr));}
    void until(std::string exp){for(int i=0;i<440;i++){if(js(exp)==true)return;std::this_thread::sleep_for(50ms);}throw std::runtime_error(exp);}
    void viewport(int w,int h){cmd("Emulation.setDeviceMetricsOverride",{{"width",w},{"height",h},{"deviceScaleFactor",1},{"mobile",w<=390}});}
    void click(std::string sel){auto point=js("(()=>{const e=document.querySelector("+J(sel).dump()+");if(!e)return null;e.scrollIntoView({block:'center',behavior:'instant'});let r=e.getBoundingClientRect();return [r.x+r.width/2,r.y+r.height/2]})()");check(point.is_array(),"missing "+sel);
    for(auto t:{"mousePressed","mouseReleased"})cmd("Input.dispatchMouseEvent",{{"type",t},{"button","left"},{"clickCount",1},{"x",point[0]},{"y",point[1]}});}
    void key(std::string name,std::string code,int vk,int mods=0){for(auto t:{"keyDown","keyUp"})cmd("Input.dispatchKeyEvent",{{"type",t},{"key",name},{"code",code},{"windowsVirtualKeyCode",vk},{"modifiers",mods}});}
    void shot(std::string name){auto data=cmd("Page.captureScreenshot",{{"format","png"},{"fromSurface",true}})["data"].get<std::string>();std::vector<unsigned char> buf(data.size());int len=EVP_DecodeBlock(buf.data(),(const unsigned char*)data.data(),data.size());while(!data.empty()&&data.back()=='='){--len;data.pop_back();}std::ofstream("/tmp/qi-quiet-"+name+".png",std::ios::binary).write((char*)buf.data(),len);}
};
