// g++ -std=c++17 -O2 -Wall -Wextra -Wpedantic -Werror _qi-blog/tests/reading-effects.cpp -lcrypto -o /tmp/qi-effects-test
#include "browser.hpp"

int main(int argc,char**argv) {try {
    const std::string site=argc>1?argv[1]:"http://127.0.0.1:8982/";
    Browser b;b.viewport(1440,960);
    b.cmd("Page.navigate",{{"url",site+"notes/ros2/"}});
    b.until("document.readyState==='complete'&&!!document.querySelector('.toc [aria-current=location]')");
    auto settled=[&]() {b.until("document.querySelector('.toc').getAnimations({subtree:true}).every(a=>a.playState!=='running')");};
    auto record=[&](int frames,const std::string& expression) {
        b.js("window.effectSamples=[];window.effectDone=false;(()=>{let last=performance.now();function sample(now){window.effectSamples.push("+expression+");last=now;if(window.effectSamples.length<"+std::to_string(frames)+")requestAnimationFrame(sample);else window.effectDone=true;}requestAnimationFrame(sample)})()");
    };
    check(b.js("getComputedStyle(document.querySelector('.toc')).backdropFilter.includes('blur(18px)')")==true,"Navigation has a real backdrop blur");
    check(b.js("getComputedStyle(document.body,'::before').animationName")=="none","Reading background remains still");
    b.click(".toc a[href='#s4']");
    b.until("document.querySelector('.toc [aria-current=location]')?.hash==='#s4'");settled();
    auto hover=[&](const std::string& selector) {
        const auto point=b.js("(()=>{const r=document.querySelector("+J(selector).dump()+").getBoundingClientRect();return [r.x+r.width/2,r.y+r.height/2]})()");
        b.cmd("Input.dispatchMouseEvent",{{"type","mouseMoved"},{"x",point[0]},{"y",point[1]}});
    };
    hover(".toc a[href='#s3']");
    b.until("document.querySelector('.toc a[href=\"#s3\"]').matches(':hover')");
    check(b.js(R"JS((()=>{
      const rows=[...document.querySelectorAll('.toc a')];
      const filled=rows.filter(a=>getComputedStyle(a).backgroundColor!=='rgba(0, 0, 0, 0)');
      return filled.length===1&&filled[0].hash==='#s4'&&getComputedStyle(filled[0]).borderInlineStartColor!=='rgba(0, 0, 0, 0)';
    })())JS")==true,"Only the current chapter is highlighted even while hovering another link");
    b.shot("effects-active-section");

    b.click("[data-open-search]");
    b.until("document.querySelector('#search-dialog').open&&document.activeElement.id==='search-input'");
    check(b.js("getComputedStyle(document.querySelector('#search-dialog'),'::backdrop').backdropFilter.includes('blur')")==true,"Search backdrop stays blurred");
    b.cmd("Input.insertText",{{"text","ROS 2"}});
    b.until("document.querySelectorAll('#search-results a').length>0");
    b.shot("effects-search");
    b.key("Escape","Escape",27);
    b.until("!document.querySelector('#search-dialog').open");

    const J reduced=J::array({{{"name","prefers-reduced-motion"},{"value","reduce"}},{{"name","prefers-reduced-transparency"},{"value","reduce"}}});
    b.cmd("Emulation.setEmulatedMedia",{{"features",reduced}});
    check(b.js("getComputedStyle(document.querySelector('.toc')).backdropFilter")=="none","Reduced transparency removes blur");
    check(b.js("getComputedStyle(document.querySelector('.toc [aria-current=location]')).textDecorationLine.includes('underline')")==true,"Current section stays marked without transparency");
    check(b.js("getComputedStyle(document.documentElement).scrollBehavior")=="auto","Reduced motion removes smooth scrolling");
    b.cmd("Emulation.setEmulatedMedia",{{"features",J::array()}});

    b.viewport(390,844);b.cmd("Emulation.setTouchEmulationEnabled",{{"enabled",true}});
    b.cmd("Page.navigate",{{"url",site+"notes/ros2/"}});
    b.until("document.readyState==='complete'&&document.documentElement.classList.contains('reading-nav-ready')&&!document.querySelector('.toc details').open");
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
    b.cmd("Emulation.setTouchEmulationEnabled",{{"enabled",false}});b.viewport(1440,960);
    b.cmd("Page.navigate",{{"url",site+"topics/"}});
    b.until("document.readyState==='complete'&&!!document.querySelector('.path-card')");
    hover(".path-card");
    b.until("getComputedStyle(document.querySelector('.path-card'),'::before').opacity==='1'");
    check(b.js("[...document.querySelectorAll('.path-card')].slice(1).every(a=>getComputedStyle(a,'::before').opacity==='0')")==true,"Hover light stays on the interacted card");
    b.shot("effects-card-focus");
    b.cmd("Emulation.setEmulatedMedia",{{"features",reduced}});
    check(b.js("getComputedStyle(document.querySelector('.path-card'),'::before').display")=="none","Reduced effects disable decorative hover light");
    std::cout<<"Current chapter, localized card lighting, search, reduced effects and mobile interactions passed"<<std::endl;
    return 0;
} catch(const std::exception& e) {std::cerr<<"FAIL: "<<e.what()<<'\n';return 1;}}
