// g++ -std=c++17 -O2 -Wall -Wextra -Wpedantic -Werror _qi-blog/tests/reading-navigation.cpp -lcrypto -o /tmp/qi-reading-test
#include "browser.hpp"

int main(int argc,char**argv) {try {
    const std::string site=argc>1?argv[1]:"http://127.0.0.1:8982/";
    Browser b;
    auto load=[&](const std::string& path) {
        b.cmd("Page.navigate",{{"url",site+path}});
        b.until("document.readyState==='complete'&&document.documentElement.classList.contains('reading-nav-ready')");
    };
    auto positioned=[&](const std::string& id) {
        b.until("(()=>{const e=document.getElementById('"+id+"');return Math.abs(e.getBoundingClientRect().top-parseFloat(getComputedStyle(document.documentElement).scrollPaddingTop)-parseFloat(getComputedStyle(e).scrollMarginTop))<2})()");
    };
    auto at=[&](const std::string& id) {
        b.until("document.querySelector('.toc [aria-current=location]')?.hash==='#"+id+"'");
        positioned(id);
    };
    auto settled=[&]() {b.until("document.querySelector('.toc').getAnimations({subtree:true}).every(a=>a.playState!=='running')");};
    const std::string desktop=R"JS((()=>{
      const toc=document.querySelector('.toc').getBoundingClientRect(),body=document.querySelector('.prose').getBoundingClientRect(),title=document.querySelector('.article-heading').getBoundingClientRect();
      return toc.right+30<body.left&&Math.abs(body.width-720)<2&&Math.abs(title.left-body.left)<2&&document.documentElement.scrollWidth<=innerWidth;
    })())JS";
    b.viewport(1440,960);load("notes/ros2/");
    check(b.js(desktop)==true,"Desktop sidebar must leave a full-width reading column");
    check(b.js("document.querySelector('.toc details').open")==true,"Desktop directory opens by default");
    b.shot("reading-desktop-top");
    b.click(".toc a[href='#s4']");at("s4");
    check(b.js("location.hash")=="#s4","Anchor links retain a shareable URL");
    check(b.js("document.activeElement.id")=="s4","Navigation moves keyboard focus to the section");
    check(b.js("document.querySelector('.toc').getBoundingClientRect().top===92")==true,"Left navigation stays below the site header");
    b.shot("reading-desktop-section");
    b.js("document.querySelector('#s2').scrollIntoView({behavior:'instant'})");at("s2");
    b.click(".toc a[href='#s4']");at("s4");
    b.click(".toc a[href='#worked-example']");at("worked-example");
    b.js("history.back()");at("s4");
    std::cout<<"Desktop: left sidebar, full-width body, smooth anchors, scroll tracking, focus and back navigation passed"<<std::endl;

    for(int width:{1280,1100}) {
        b.viewport(width,900);
        check(b.js(desktop)==true,"Sidebar layout at width "+std::to_string(width));
    }
    b.viewport(1440,960);
    b.js("document.documentElement.style.fontSize='20px'");
    b.click(".toc a[href='#s5']");at("s5");
    b.js("document.documentElement.style.removeProperty('font-size')");
    b.viewport(1280,420);
    b.js("document.querySelector('#s8').scrollIntoView({behavior:'instant'})");at("s8");
    check(b.js(R"JS((()=>{
      const toc=document.querySelector('.toc'),a=toc.querySelector('[aria-current]'),r=a.getBoundingClientRect(),t=toc.getBoundingClientRect();
      return r.top>=t.top+toc.querySelector('summary').offsetHeight&&r.bottom<=t.bottom&&t.bottom<innerHeight;
    })())JS")==true,"Current section stays visible inside a short sidebar");
    b.shot("reading-short-screen");
    std::cout<<"Responsive desktop: 1100/1280/1440 px and independently scrolling sidebar on a 420 px tall screen passed"<<std::endl;

    b.viewport(390,844);load("notes/ros2/");
    check(b.js("!document.querySelector('.toc details').open")==true,"Mobile directory starts collapsed");
    check(b.js("document.documentElement.scrollWidth<=innerWidth")==true,"No mobile horizontal overflow");
    b.click(".toc summary");
    check(b.js("document.querySelector('.toc details').open")==true,"Mobile directory expands");
    settled();
    b.shot("reading-mobile-open");
    b.click(".toc a[href='#s4']");at("s4");
    check(b.js("!document.querySelector('.toc details').open")==true,"Mobile directory closes after choosing a section");
    check(b.js("document.querySelector('#s4').getBoundingClientRect().top>document.querySelector('.toc').getBoundingClientRect().bottom+8")==true,"Mobile anchor is not obscured by navigation");
    b.shot("reading-mobile-section");
    b.click(".toc summary");
    settled();
    b.js("document.querySelector('.toc a[href=\"#s3\"]').focus()");
    b.key("Enter","Enter",13);at("s3");
    check(b.js("document.activeElement.id")=="s3","Keyboard activation reaches the reading section");
    b.viewport(1280,900);
    b.until("document.querySelector('.toc details').open");
    b.viewport(390,844);load("notes/ros2/#s5");at("s5");
    b.cmd("Emulation.setEmulatedMedia",{{"features",J::array({{{"name","prefers-reduced-motion"},{"value","reduce"}}})}});
    check(b.js("getComputedStyle(document.documentElement).scrollBehavior")=="auto","Reduced-motion preference removes smooth scrolling");
    b.cmd("Emulation.setEmulatedMedia",{{"features",J::array()}});
    std::cout<<"Mobile: collapse/expand, sticky access, clear anchor positions, keyboard, deep links and reduced motion passed"<<std::endl;

    b.viewport(1440,960);load("notes/opencv/");
    b.click(".toc a[href='#sobel-lab']");at("sobel-lab");
    b.until("document.querySelector('[data-kernel]')?.dataset.ready==='true'");
    b.js("document.querySelector('[data-kernel-step]').value=1;document.querySelector('[data-kernel-step]').dispatchEvent(new Event('input'))");
    check(b.js("document.querySelector('[data-kernel]').dataset.result")=="320","Sobel interaction remains usable after navigation");
    b.shot("reading-opencv");
    std::ifstream input("_qi-blog/library.json");const auto library=J::parse(input);
    J slugs=J::array();for(const auto& path:library.at("paths"))for(const auto& slug:path.at("notes"))slugs.push_back(slug);
    const auto audit=b.js(R"JS((async()=>{
      const issues=[];
      for(const slug of )JS"+slugs.dump()+R"JS(){
        const response=await fetch('/notes/'+slug+'/');
        if(!response.ok){issues.push(slug+': HTTP '+response.status);continue;}
        const doc=new DOMParser().parseFromString(await response.text(),'text/html');
        let previous;
        for(const a of doc.querySelectorAll('.toc a')){
          const target=doc.getElementById(a.getAttribute('href').slice(1));
          if(!target||!target.closest('.prose')){issues.push(slug+': missing '+a.getAttribute('href'));continue;}
          if(previous&&!(previous.compareDocumentPosition(target)&Node.DOCUMENT_POSITION_FOLLOWING))issues.push(slug+': out-of-order '+a.getAttribute('href'));
          previous=target;
        }
        if(!doc.querySelector('.toc a[href="#worked-example"]'))issues.push(slug+': missing example');
      }
      return issues;
    })())JS");
    check(audit.empty(),audit.dump());
    std::cout<<"All 50 articles: navigation targets exist in reading order; OpenCV interaction passed"<<std::endl;

    b.cmd("Emulation.setScriptExecutionDisabled",{{"value",true}});
    b.cmd("Page.navigate",{{"url",site+"notes/ros2/"}});
    b.until("document.readyState==='complete'&&!!document.querySelector('.toc')&&!document.documentElement.classList.contains('reading-nav-ready')");
    check(b.js("document.querySelector('.toc details').open")==true,"Directory remains accessible without JavaScript");
    check(b.js("getComputedStyle(document.querySelector('.toc ol'),'::before').opacity")=="0","No-script directory does not display an uninitialized marker");
    b.click(".toc a[href='#s3']");
    positioned("s3");
    b.viewport(390,844);
    b.cmd("Page.navigate",{{"url",site+"notes/ros2/"}});
    b.until("document.readyState==='complete'&&!!document.querySelector('.toc')");
    b.click(".toc a[href='#s4']");
    positioned("s4");
    check(b.js("document.querySelector('.toc').getBoundingClientRect().bottom<0")==true,"No-script mobile directory does not cover the content");
    std::cout<<"No JavaScript: native directory and anchor fallback passed"<<std::endl;
    return 0;
} catch(const std::exception& e) {std::cerr<<"FAIL: "<<e.what()<<'\n';return 1;}}
