// g++ -std=c++17 -O2 -Wall -Wextra -Wpedantic -Werror _qi-blog/tests/reading-effects.cpp -lcrypto -o /tmp/qi-effects-test
#include "browser.hpp"

int main(int argc,char**argv) {try {
    const std::string site=argc>1?argv[1]:"http://127.0.0.1:8982/";
    Browser b;b.viewport(1440,960);
    b.cmd("Page.navigate",{{"url",site+"notes/ros2/"}});
    b.until("document.readyState==='complete'&&!!document.querySelector('.toc details.light-surface')");
    auto settled=[&]() {b.until("document.querySelector('.toc').getAnimations({subtree:true}).every(a=>a.playState!=='running')");};
    auto record=[&](int frames,const std::string& expression) {
        b.js("window.effectSamples=[];window.effectDone=false;(()=>{let last=performance.now();function sample(now){window.effectSamples.push("+expression+");last=now;if(window.effectSamples.length<"+std::to_string(frames)+")requestAnimationFrame(sample);else window.effectDone=true;}requestAnimationFrame(sample)})()");
    };
    check(b.js("getComputedStyle(document.querySelector('.toc')).backdropFilter.includes('blur(18px)')")==true,"Navigation has a real backdrop blur");
    check(b.js("getComputedStyle(document.body,'::before').animationName.includes('ambient-drift')")==true,"Desktop ambient light is enabled");
    const auto rect=b.js("(()=>{const r=document.querySelector('.toc details').getBoundingClientRect();return {x:r.x,y:r.y,width:r.width,height:r.height}})()");
    const double x=rect["x"].get<double>(),y=rect["y"].get<double>();
    b.cmd("Input.dispatchMouseEvent",{{"type","mouseMoved"},{"x",x+50},{"y",y+90}});
    b.until("getComputedStyle(document.querySelector('.toc details'),'::before').opacity==='1'");
    check(b.js("Math.abs(parseFloat(document.querySelector('.toc details').style.getPropertyValue('--light-x'))-50)<1")==true,"Light follows the real pointer position");
    b.shot("effects-glass-navigation");

    b.cmd("Performance.enable");
    const auto before=b.cmd("Performance.getMetrics");
    record(180,"now-last");
    for(int i=0;i<90;++i) {
        b.cmd("Input.dispatchMouseEvent",{{"type","mouseMoved"},{"x",x+45+40*std::sin(i*.12)},{"y",y+95+35*std::cos(i*.12)}});
        std::this_thread::sleep_for(16ms);
    }
    b.until("window.effectDone===true");
    const auto timing=b.js("(()=>{const a=window.effectSamples.slice(1).sort((a,b)=>a-b);return {frames:a.length,median:a[Math.floor(a.length*.5)],p95:a[Math.floor(a.length*.95)],max:a.at(-1)}})()");
    std::cout<<"Pointer and ambient-light frame intervals (ms): "<<timing.dump()<<std::endl;
    check(timing["p95"].get<double>()<40,"Pointer lighting produces sustained slow frames");
    const auto after=b.cmd("Performance.getMetrics");
    auto metric=[](const J& metrics,const std::string& name) {for(const auto& m:metrics.at("metrics"))if(m.at("name")==name)return m.at("value").get<double>();return 0.;};
    std::cout<<"During pointer sampling: layout "<<(metric(after,"LayoutDuration")-metric(before,"LayoutDuration"))*1000<<" ms, script "<<(metric(after,"ScriptDuration")-metric(before,"ScriptDuration"))*1000<<" ms"<<std::endl;
    b.cmd("Input.dispatchMouseEvent",{{"type","mouseMoved"},{"x",1000},{"y",90}});
    b.until("!document.querySelector('.toc details').style.getPropertyValue('--light-x')");
    b.click(".toc a[href='#s4']");
    b.until("document.querySelector('.toc [aria-current=location]')?.hash==='#s4'");settled();
    check(b.js(R"JS((()=>{
      const list=document.querySelector('.toc ol'),a=list.querySelector('[aria-current]'),style=getComputedStyle(list,'::before');
      return Math.abs(new DOMMatrixReadOnly(style.transform).m42-a.offsetTop)<1&&Math.abs(parseFloat(style.height)-a.offsetHeight)<1;
    })())JS")==true,"Animated chapter marker settles exactly on its link");
    b.shot("effects-active-section");

    b.click("[data-open-search]");
    b.until("document.querySelector('#search-dialog').open&&document.activeElement.id==='search-input'");
    check(b.js("getComputedStyle(document.querySelector('#search-dialog'),'::backdrop').backdropFilter.includes('blur')")==true,"Search backdrop stays blurred");
    check(b.js("getComputedStyle(document.body,'::before').animationPlayState.split(',').every(s=>s.trim()==='paused')")==true,"Background movement pauses behind search");
    b.cmd("Input.insertText",{{"text","ROS 2"}});
    b.until("document.querySelectorAll('#search-results a').length>0");
    b.shot("effects-search");
    b.key("Escape","Escape",27);
    b.until("!document.querySelector('#search-dialog').open");

    b.cmd("Emulation.setEmulatedMedia",{{"features",J::array({{{"name","prefers-reduced-motion"},{"value","reduce"}},{{"name","prefers-reduced-transparency"},{"value","reduce"}}})}});
    b.until("document.documentElement.classList.contains('motion-paused')");
    check(b.js("getComputedStyle(document.querySelector('.toc')).backdropFilter")=="none","Reduced transparency removes blur");
    check(b.js("getComputedStyle(document.querySelector('.toc ol'),'::before').transitionDuration")=="0s","Reduced motion removes chapter animation");
    check(b.js("getComputedStyle(document.querySelector('.toc details'),'::before').display")=="none","Reduced motion removes pointer light");
    b.cmd("Emulation.setEmulatedMedia",{{"features",J::array()}});

    b.viewport(390,844);b.cmd("Emulation.setTouchEmulationEnabled",{{"enabled",true}});
    b.cmd("Page.navigate",{{"url",site+"notes/ros2/"}});
    b.until("document.readyState==='complete'&&document.documentElement.classList.contains('reading-nav-ready')&&!document.querySelector('.toc details').open");
    check(b.js("!getComputedStyle(document.body,'::before').animationName.includes('ambient-drift')")==true,"Mobile avoids continuous ambient rendering");
    record(45,"document.querySelector('.toc').getBoundingClientRect().height");
    b.click(".toc summary");b.until("window.effectDone===true");
    if(b.js("CSS.supports('interpolate-size','allow-keywords')")==true) {
        check(b.js("new Set(effectSamples.map(n=>Math.round(n))).size>4")==true,"Mobile drawer interpolates rather than jumping");
    }
    settled();b.shot("effects-mobile-directory");
    b.click(".toc summary");settled();
    for(int i=0;i<4;++i)b.click(".toc summary");
    settled();
    check(b.js("!document.querySelector('.toc details').open&&document.querySelector('.toc').offsetHeight<70")==true,"Repeated toggles settle into a compact closed bar");
    b.click(".toc summary");
    b.until("document.querySelector('.toc a').getBoundingClientRect().bottom<document.querySelector('.toc').getBoundingClientRect().bottom");
    b.click(".toc a[href='#s1']");
    b.until("!document.querySelector('.toc details').open&&Math.abs(document.querySelector('#s1').getBoundingClientRect().top-160)<2");
    b.shot("effects-mobile-reading");
    std::cout<<"Glass and motion: real pointer tracking, moving marker, search/keyboard, reduced motion/transparency, mobile interpolation and rapid input passed"<<std::endl;
    return 0;
} catch(const std::exception& e) {std::cerr<<"FAIL: "<<e.what()<<'\n';return 1;}}
