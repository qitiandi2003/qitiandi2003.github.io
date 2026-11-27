// Original vector figures are built from the authored lesson data, not screenshots.
#include <cmath>
#include <map>

string teaching_color(const string& name) {
    static const std::map<string,string> colors={{"ink","#1d1d1f"},{"blue","#0071e3"},{"orange","#b05a15"},
        {"green","#267652"},{"muted","#6e6e73"},{"red","#b52d39"},{"paper","#f5f5f7"},
        {"paleBlue","#e8f2ff"},{"paleOrange","#fff2e4"},{"paleGreen","#e9f5ef"},{"line","#d2d2d7"},{"white","#ffffff"},{"none","none"}};
    const auto found=colors.find(name);
    if(found!=colors.end())return found->second;
    if(name.size()==7&&name[0]=='#'&&name.find_first_not_of("0123456789abcdefABCDEF",1)==string::npos)return name;
    throw std::runtime_error("Unknown teaching color: "+name);
}

string teaching_drawing(const json& commands) {
    std::ostringstream s;
    for(const auto& c:commands) {
        const string type=c.at(0);
        auto color=[&](size_t i,const string& fallback){return teaching_color(c.size()>i?c.at(i).get<string>():fallback);};
        auto number=[&](size_t i,double fallback){return c.size()>i?c.at(i).get<double>():fallback;};
        if(type=="rect")s<<"<rect x='"<<c.at(1)<<"' y='"<<c.at(2)<<"' width='"<<c.at(3)<<"' height='"<<c.at(4)
            <<"' rx='"<<number(7,8)<<"' fill='"<<color(5,"paper")<<"' stroke='"<<color(6,"none")<<"' stroke-width='2'/>";
        else if(type=="circle")s<<"<circle cx='"<<c.at(1)<<"' cy='"<<c.at(2)<<"' r='"<<c.at(3)<<"' fill='"<<color(4,"blue")<<"' stroke='"<<color(5,"none")<<"' stroke-width='3'/>";
        else if(type=="line"||type=="arrow") {
            s<<"<line x1='"<<c.at(1)<<"' y1='"<<c.at(2)<<"' x2='"<<c.at(3)<<"' y2='"<<c.at(4)<<"' stroke='"<<color(5,"line")
                <<"' stroke-width='"<<number(6,3)<<"'"<<(c.size()>7&&c.at(7)==true?" stroke-dasharray='7 6'":"")<<"/>";
            if(type=="arrow") {
                const double x=c.at(3),y=c.at(4),angle=std::atan2(y-c.at(2).get<double>(),x-c.at(1).get<double>());
                const double dx=11*std::cos(angle),dy=11*std::sin(angle);
                s<<"<path d='M "<<x<<' '<<y<<" L "<<x-dx-dy*.45<<' '<<y-dy+dx*.45<<" L "<<x-dx+dy*.45<<' '<<y-dy-dx*.45<<" Z' fill='"<<color(5,"blue")<<"'/>";
            }
        } else if(type=="text") {
            const string anchor=c.size()>6?c.at(6).get<string>():"start";
            if(anchor!="start"&&anchor!="middle"&&anchor!="end")throw std::runtime_error("Invalid text anchor");
            s<<"<text x='"<<c.at(1)<<"' y='"<<c.at(2)<<"' font-size='"<<number(4,28)<<"' fill='"<<color(5,"ink")
                <<"' text-anchor='"<<anchor<<"'"<<(c.size()>7&&c.at(7)==true?" font-weight='600'":"")<<'>'<<html_escape(c.at(3))<<"</text>";
        } else if(type=="path")s<<"<path d='"<<html_escape(c.at(1))<<"' stroke='"<<color(2,"blue")<<"' stroke-width='"<<number(3,3)<<"' fill='"<<color(4,"none")<<"'/>";
        else if(type=="polyline") {
            s<<"<polyline points='";
            for(const auto& p:c.at(1))s<<p.at(0)<<','<<p.at(1)<<' ';
            s<<"' fill='none' stroke='"<<color(2,"blue")<<"' stroke-width='"<<number(3,3)<<"'/>";
        } else throw std::runtime_error("Unknown drawing command: "+type);
    }
    return s.str();
}

json teaching_for(const string& slug) {
    const fs::path path=fs::path("_qi-blog/teaching")/(slug+".json");
    if(!fs::exists(path))return nullptr;
    std::ifstream file(path);
    const auto lesson=json::parse(file);
    if(lesson.at("slug")!=slug||lesson.at("frames").size()<3||lesson.at("frames").size()>4)
        throw std::runtime_error("Check teaching example: "+slug);
    size_t i=0;
    for(const auto& frame:lesson.at("frames")) {
        const string image="assets/teaching/"+slug+"-"+std::to_string(++i)+".svg";
        const string svg="<svg xmlns='http://www.w3.org/2000/svg' viewBox='0 0 640 360' width='640' height='360' role='img' aria-labelledby='title desc'>"
            "<title id='title'>"+html_escape(frame.at("label"))+"</title><desc id='desc'>"+html_escape(frame.at("alt"))+"</desc>"
            "<rect width='640' height='360' fill='#ffffff'/><g font-family='system-ui, -apple-system, Noto Sans CJK SC, sans-serif' stroke-linecap='round' stroke-linejoin='round'>"
            +teaching_drawing(lesson.value("base",json::array()))+teaching_drawing(frame.at("drawing"))+"</g></svg>\n";
        write(image,svg);
    }
    return lesson;
}

string teaching_html(const json& lesson) {
    std::ostringstream out;
    const string slug=lesson.at("slug");
    out<<"<figure class='explainer' id='worked-example' data-explainer><figcaption><span class='figure-eyebrow'>分步图解</span><h3>"
        <<html_escape(lesson.at("title"))<<"</h3><p>"<<prose(lesson.at("lead"))<<"</p></figcaption><div class='ex-frames'>";
    size_t i=0;
    for(const auto& frame:lesson.at("frames")) {
        const string image="../../assets/teaching/"+slug+"-"+std::to_string(++i)+".svg";
        out<<"<div class='ex-frame' data-frame><img src='"<<image<<"' width='640' height='360' loading='lazy' decoding='async' alt='"<<html_escape(frame.at("alt"))
            <<"'><div class='ex-caption'><h4>"<<html_escape(frame.at("label"))<<"</h4><a href='"<<image<<"' class='ex-zoom' aria-label='查看大图："<<html_escape(frame.at("label"))<<"'>查看大图</a></div><p>"<<prose(frame.at("body"))<<"</p></div>";
    }
    out<<"</div><div class='ex-controls' hidden><button type='button' data-ex-prev>上一步</button><button type='button' data-ex-play aria-pressed='false'>播放</button>"
        <<"<button type='button' data-ex-next>下一步</button><output aria-live='polite' aria-atomic='true'></output></div><p class='ex-takeaway'>"<<prose(lesson.at("takeaway"))
        <<"</p><details class='ex-question'><summary>小练习："<<prose(lesson.at("question"))<<"</summary><p>"<<prose(lesson.at("answer"))<<"</p></details></figure>";
    return out.str();
}

string sobel_lab() {
    std::ostringstream out;
    out<<"<figure class='kernel-lab' id='sobel-lab' data-kernel><figcaption><span class='figure-eyebrow'>动手试一试</span><h3>让 Sobel 在像素格上走一遍</h3>"
        <<"<p>五行像素都从暗变亮。蓝框内的九个数分别乘以核里的权重，再相加，填入右侧对应格。这里步长为 1，不填充边界，坐标从 0 开始。</p></figcaption>"
        <<"<div class='kernel-panels'><div class='kernel-input'><h4>输入 · 5 × 5</h4><div class='pixel-grid input-grid' role='img' aria-label='五行相同的灰度值，每行依次是 30、30、30、110、110'>";
    for(int i=0;i<25;++i)out<<"<span style='background:rgb("<<(i%5<3?"30,30,30":"110,110,110")<<")' aria-hidden='true'>"<<(i%5<3?30:110)<<"</span>";
    out<<"<i class='kernel-window' aria-hidden='true'></i></div></div><div><h4>权重 · 3 × 3</h4><div class='pixel-grid weight-grid' role='img' aria-label='横向 Sobel 核：负一、零、一；负二、零、二；负一、零、一'>";
    for(int n:{-1,0,1,-2,0,2,-1,0,1})out<<"<span aria-hidden='true'>"<<n<<"</span>";
    out<<"</div></div><div><h4>输出 · 3 × 3</h4><div class='pixel-grid output-grid' role='img' aria-label='横向梯度每行是 0、320、320'>";
    for(int i=0;i<9;++i)out<<"<span aria-hidden='true'>"<<(i%3?320:0)<<"</span>";
    out<<"</div></div></div><div class='kernel-controls' hidden><div class='kernel-toolbar'><button type='button' data-kernel-play aria-pressed='false'>播放</button>"
        <<"<label>求导方向<select data-kernel-mode><option value='x'>横向 Gx</option><option value='y'>纵向 Gy</option></select></label>"
        <<"<button type='button' data-kernel-flip aria-pressed='false'>反转明暗</button></div><label class='kernel-position'>当前窗口<input data-kernel-step type='range' min='0' max='8' value='0' aria-describedby='kernel-result'><output>1 / 9</output></label></div>"
        <<"<div class='kernel-calculation'><span>九个乘积，逐项相加</span><p data-products>30 × (−1) + 30 × 0 + 30 × 1 + 30 × (−2) + 30 × 0 + 30 × 2 + 30 × (−1) + 30 × 0 + 30 × 1 = 0</p></div>"
        <<"<p class='kernel-result' id='kernel-result' aria-live='polite'>当前九个像素都是 30，正负权重抵消，导数为 0。向右移动一格后，窗口跨过明暗边界，结果变成 320。</p></figure>";
    return out.str();
}
