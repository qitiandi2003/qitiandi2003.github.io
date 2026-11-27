// g++ -std=c++17 -O2 -Wall -Wextra -Wpedantic -Werror _qi-blog/tests/teaching.cpp -lcrypto $(pkg-config --cflags --libs opencv4) -o /tmp/qi-teaching-test
#include "browser.hpp"
#include <opencv2/imgproc.hpp>

int main(int argc,char**argv) {try {
    const std::string site=argc>1?argv[1]:"http://127.0.0.1:8982/";
    const bool audit=argc>2&&std::string(argv[2])=="--audit-svg";
    J figures=J::array(),lessons=J::array();
    for(const auto& item:fs::directory_iterator("_qi-blog/teaching")) {
        if(item.path().extension()!=".json")continue;
        std::ifstream source(item.path());const auto lesson=J::parse(source);
        lessons.push_back({{"slug",lesson.at("slug")},{"frames",lesson.at("frames").size()}});
        for(size_t i=0;i<lesson.at("frames").size();++i)figures.push_back("assets/teaching/"+lesson.at("slug").get<std::string>()+"-"+std::to_string(i+1)+".svg");
    }
    if(!audit)check(lessons.size()==50,"Every existing article needs a teaching example");
    Browser b;b.viewport(1280,960);
    b.cmd("Page.navigate",{{"url",site}});b.until("!!document.querySelector('.site-header')");
    const auto geometry=b.js(R"JS((async()=>{
      const issues=[],box=document.createElement('div');box.style='position:absolute;left:-10000px;top:0;width:640px';document.body.append(box);
      const files=)JS"+figures.dump()+R"JS(;
      for(const file of files){
        const response=await fetch(file);if(!response.ok){issues.push({file,error:'HTTP '+response.status});continue;}
        const xml=await response.text(),svg=new DOMParser().parseFromString(xml,'image/svg+xml').documentElement;
        if(svg.localName!=='svg'||svg.querySelector('parsererror')){issues.push({file,error:'invalid SVG'});continue;}
        const imported=document.importNode(svg,true);imported.style='width:640px;height:360px;display:block';
        box.replaceChildren(imported);await document.fonts.ready;
        const texts=[...box.querySelectorAll('text')].map(node=>({text:node.textContent,box:node.getBBox()}));
        for(const t of texts)if(t.box.x<1||t.box.y<1||t.box.x+t.box.width>639||t.box.y+t.box.height>359)issues.push({file,error:'clipped label',text:t.text,box:{x:t.box.x,y:t.box.y,width:t.box.width,height:t.box.height}});
        for(let i=0;i<texts.length;i++)for(let j=i+1;j<texts.length;j++){
          const a=texts[i].box,c=texts[j].box,w=Math.min(a.x+a.width,c.x+c.width)-Math.max(a.x,c.x),h=Math.min(a.y+a.height,c.y+c.height)-Math.max(a.y,c.y);
          if(w>3&&h>3&&w*h>50)issues.push({file,error:'overlapping labels',text:[texts[i].text,texts[j].text]});
        }
      }
      box.remove();return {files:files.length,issues};
    })())JS");
    std::ofstream("/tmp/qi-teaching-svg-audit.json")<<geometry.dump(2)<<'\n';
    std::cout<<"SVG figures: "<<geometry["files"]<<", layout issues: "<<geometry["issues"].size()<<std::endl;
    if(audit)return geometry["issues"].empty()?0:1;
    check(geometry["issues"].empty(),"Read /tmp/qi-teaching-svg-audit.json");

    b.cmd("Page.navigate",{{"url",site+"notes/opencv/"}});
    b.until("!!document.querySelector('[data-kernel][data-ready=true]')");
    b.js("document.querySelector('#sobel-lab').scrollIntoView({block:'center',behavior:'instant'})");
    cv::Mat row=(cv::Mat_<unsigned char>(1,5)<<30,30,30,110,110),source;
    cv::repeat(row,5,1,source);
    for(int reversed=0;reversed<2;++reversed)for(int axis=0;axis<2;++axis) {
        cv::Mat input=reversed?140-source:source.clone(),gradient;
        cv::Sobel(input,gradient,CV_32F,axis==0?1:0,axis==1?1:0,3);
        b.js("document.querySelector('[data-kernel-mode]').value='"+std::string(axis==0?"x":"y")+"';document.querySelector('[data-kernel-mode]').dispatchEvent(new Event('change'))");
        for(int step=0;step<9;++step) {
            const auto value=b.js("document.querySelector('[data-kernel-step]').value="+std::to_string(step)+";document.querySelector('[data-kernel-step]').dispatchEvent(new Event('input'));Number(document.querySelector('[data-kernel]').dataset.result)");
            check(value.get<float>()==gradient.at<float>(1+step/3,1+step%3),"Sobel demo disagrees with OpenCV");
        }
        if(axis==1&&reversed==0)b.click("[data-kernel-flip]");
    }
    b.click("[data-kernel-flip]");
    b.js("const s=document.querySelector('[data-kernel-mode]');s.value='x';s.dispatchEvent(new Event('change'));document.querySelector('[data-kernel-step]').value=0;document.querySelector('[data-kernel-step]').dispatchEvent(new Event('input'));document.querySelector('[data-kernel-step]').focus()");
    b.key("ArrowRight","ArrowRight",39);
    check(b.js("document.querySelector('[data-kernel]').dataset.result")=="320","keyboard window control");
    b.shot("teaching-sobel-desktop");
    b.click("[data-kernel-play]");
    b.until("document.querySelector('[data-kernel]').dataset.step==='2'");
    b.js("window.scrollTo({top:0,behavior:'instant'})");
    b.until("document.querySelector('[data-kernel-play]').getAttribute('aria-pressed')==='false'");
    std::cout<<"Sobel: 36 cells agree with OpenCV; keyboard, playback and offscreen pause passed"<<std::endl;

    for(const auto& lesson:lessons) {
        const auto slug=lesson["slug"].get<std::string>();
        b.viewport(390,844);b.cmd("Page.navigate",{{"url",site+"notes/"+slug+"/"}});
        b.until("location.pathname.endsWith('/notes/"+slug+"/')&&!!document.querySelector('[data-explainer][data-ready=true][data-step=\"0\"]')");
        const auto layout=b.js(R"JS((()=>{
          const e=document.querySelector('[data-explainer]');e.scrollIntoView({block:'start',behavior:'instant'});
          const r=e.getBoundingClientRect();return {overflow:document.documentElement.scrollWidth>innerWidth+1,left:r.left,right:r.right,visible:[...e.querySelectorAll('[data-frame]')].filter(f=>!f.hidden).length};
        })())JS");
        check(layout["overflow"]==false&&layout["left"].get<double>()>=0&&layout["right"].get<double>()<=390,"mobile overflow: "+slug);
        check(layout["visible"]==1,"multiple active frames: "+slug);
        const int count=lesson["frames"];
        for(int i=1;i<count;++i){b.click("[data-ex-next]");b.until("document.querySelector('[data-explainer]').dataset.step==='"+std::to_string(i)+"'");}
        check(b.js("document.querySelector('[data-ex-next]').disabled")==true,"end step limit: "+slug);
        b.click("[data-ex-prev]");b.until("document.querySelector('[data-explainer]').dataset.step==='"+std::to_string(count-2)+"'");
        b.click(".ex-question summary");check(b.js("document.querySelector('.ex-question').open")==true,"exercise disclosure: "+slug);
        if(slug=="slam"||slug=="word-embeddings"||slug=="ros2"||slug=="opencv")b.shot("teaching-mobile-"+slug);
        std::cout<<slug<<": steps, image decode, mobile layout and exercise passed"<<std::endl;
    }
    b.viewport(1280,960);b.cmd("Page.navigate",{{"url",site+"notes/opencv/"}});
    b.until("!!document.querySelector('[data-explainer][data-ready=true][data-step=\"0\"]')");
    b.js("document.querySelector('[data-explainer]').scrollIntoView({block:'center',behavior:'instant'})");
    b.click("[data-ex-play]");b.until("document.querySelector('[data-explainer]').dataset.step==='1'");
    b.click("[data-ex-play]");
    check(b.js("document.querySelector('[data-ex-play]').getAttribute('aria-pressed')")=="false","pause control");
    b.cmd("Emulation.setEmulatedMedia",{{"features",J::array({{{"name","prefers-reduced-motion"},{"value","reduce"}}})}});
    check(b.js("getComputedStyle(document.querySelector('[data-frame]:not([hidden])')).animationName")=="none","reduced-motion animation");
    b.cmd("Emulation.setScriptExecutionDisabled",{{"value",true}});
    b.cmd("Page.navigate",{{"url",site+"notes/slam/"}});
    b.until("location.pathname.endsWith('/notes/slam/')&&!!document.querySelector('[data-explainer]')");
    check(b.js("[...document.querySelectorAll('[data-frame]')].every(f=>!f.hidden)")==true,"no-JavaScript steps unavailable");
    std::cout<<"PASS: all 50 articles, original SVG layouts, OpenCV numeric comparison, mobile interaction, playback, reduced motion and static fallback"<<std::endl;
}catch(const std::exception& e){std::cerr<<e.what()<<std::endl;return 1;}}
