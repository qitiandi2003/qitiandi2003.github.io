// Run from the repository root. See README.md.
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>
#include <set>
#include <nlohmann/json.hpp>

namespace fs = std::filesystem;
using json = nlohmann::json;
using std::string;

const fs::path destination = ".";
const string github = "https://github.com/qitiandi2003";
const string brand = "Qi’blog";

struct Article {
    string slug, title, summary, intro, topic, category, group, keywords, published;
    json sections, refs;
    string visual, figure_caption;
    size_t visual_section, communication_section;
    int minutes;
};

string html_escape(const string& s) {
    string out;
    for (char ch : s) {
        switch (ch) {
            case '&': out += "&amp;"; break;
            case '<': out += "&lt;"; break;
            case '>': out += "&gt;"; break;
            case '"': out += "&quot;"; break;
            case '\'': out += "&#39;"; break;
            default: out += ch;
        }
    }
    return out;
}

string prose(const string& s) {
    std::istringstream in(s);
    string part, out;
    bool code = false;
    while (std::getline(in, part, '`')) {
        out += code ? "<code>" + html_escape(part) + "</code>" : html_escape(part);
        code = !code;
    }
    return out;
}

void write(const fs::path& path, const string& data) {
    fs::create_directories(path.parent_path());
    std::ofstream file(path, std::ios::binary);
    file.exceptions(std::ios::failbit | std::ios::badbit);
    file << data;
}

string icon(const string& name) {
    string paths;
    if (name == "search") paths = R"(<circle cx="10.5" cy="10.5" r="6.6"/><path d="m16 16 4.5 4.5"/>)";
    else if (name == "close") paths = R"(<path d="m6 6 12 12M18 6 6 18"/>)";
    else if (name == "github") paths = R"(<path fill="currentColor" stroke="none" d="M12 .9a11.1 11.1 0 0 0-3.51 21.63c.55.1.76-.24.76-.54v-2.08c-3.09.67-3.74-1.31-3.74-1.31-.5-1.28-1.23-1.62-1.23-1.62-1.01-.69.08-.68.08-.68 1.11.08 1.69 1.14 1.69 1.14.99 1.7 2.59 1.21 3.22.92.1-.72.38-1.21.7-1.49-2.47-.28-5.06-1.23-5.06-5.49 0-1.21.43-2.21 1.14-2.98-.11-.28-.49-1.41.11-2.94 0 0 .93-.3 3.05 1.14a10.6 10.6 0 0 1 5.56 0c2.12-1.44 3.04-1.14 3.04-1.14.6 1.53.22 2.66.11 2.94.71.77 1.14 1.77 1.14 2.98 0 4.27-2.6 5.21-5.07 5.49.4.34.75 1.02.75 2.06v3.06c0 .3.21.65.77.54A11.1 11.1 0 0 0 12 .9z"/>)";
    else throw std::logic_error("Unknown icon: " + name);
    return R"(<svg aria-hidden="true" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="1.7" stroke-linecap="round" stroke-linejoin="round">)" + paths + "</svg>";
}

string wordmark(const string& base) {
    return "<a class='brand' href='" + base + "' aria-label='Qi’blog 首页'><span lang='en'><span class='brand-name'>Qi</span><span class='brand-mark'>’</span>blog</span></a>";
}

string header(const string& base, const string& page) {
    auto nav = [&](const string& id, const string& href, const string& label) {
        return "<a href='" + base + href + "'" + (id == page ? " aria-current='page'" : "") + ">" + label + "</a>";
    };
    return "<a class='skip' href='#main'>跳至正文</a><header class='site-header'><div class='header-inner wrap'>" + wordmark(base) +
        "<nav class='primary-nav' aria-label='主导航'>" + nav("archive", "notes/", "文章") + nav("topics", "topics/", "学习方向") + "</nav><div class='header-actions'><button class='search-button' type='button' data-open-search aria-haspopup='dialog' aria-label='搜索文章'>" +
        icon("search") + "<span>搜索</span></button><a class='github-button' href='" + github + "' aria-label='Qi 的 GitHub 主页'>" +
        icon("github") + "<span>GitHub</span></a></div></div></header>";
}

string footer(const string& base) {
    return "<footer class='site-footer'><div class='wrap footer-inner'>" + wordmark(base) +
         "<span>© 2026 Qi</span><a href='" + base + "about/'>关于作者</a></div></footer>"
         "<dialog id='search-dialog' class='search-dialog' aria-label='搜索文章' data-index-src='" + base + "assets/search-index.js?v=reading-3'><div class='search-field'>" + icon("search") +
         "<input id='search-input' type='search' aria-label='搜索文章关键词' placeholder='搜索文章或主题' autocomplete='off' spellcheck='false'>"
         "<button type='button' class='search-clear' data-clear-search hidden>清空</button>"
         "<button type='button' class='search-close' aria-label='关闭搜索' data-close-search>" + icon("close") +
         "</button></div><div class='search-body'><div id='search-suggestions'><p class='search-label'>常用关键词</p>"
         "<div class='search-keywords'><button type='button' data-search-query='ROS 2'>ROS 2</button>"
         "<button type='button' data-search-query='SLAM'>SLAM</button><button type='button' data-search-query='OpenCV'>OpenCV</button>"
         "<button type='button' data-search-query='3DGS'>3DGS</button></div></div>"
         "<p id='search-status' role='status' aria-atomic='true'></p><ul id='search-results' aria-label='搜索结果' hidden></ul>"
         "<button type='button' class='search-more' data-more-search hidden></button>"
         "<button type='button' class='search-retry' data-retry-search hidden>重新加载搜索</button></div></dialog>";
}

string document(const string& base, const string& page, const string& title, const string& desc, const string& body) {
    return "<!doctype html>\n<html lang='zh-CN' data-base='" + base + "'><head><meta charset='utf-8'>"
        "<meta name='viewport' content='width=device-width,initial-scale=1'><meta name='theme-color' content='#ffffff'>"
        "<title>" + html_escape(title) + "</title><meta name='description' content='" + html_escape(desc) + "'>"
        "<link rel='icon' href='" + base + "assets/favicon.svg?v=wordmark-4' type='image/svg+xml'><link rel='stylesheet' href='" +
        base + "assets/site.css?v=perf-9'><script defer src='" + base + "assets/site.js?v=pet-21'></script>"
        "<noscript><style>.search-button,.filters{display:none}</style></noscript></head><body>" +
        header(base,page) + body + footer(base) + "</body></html>\n";
}

string duration(const Article& a) {
    return "<span class='duration'>约 " + std::to_string(a.minutes) + " 分钟</span>";
}

const Article& lookup(const std::vector<Article>& articles, std::string_view slug) {
    const auto found = std::find_if(articles.begin(),articles.end(),[&](const Article& a){return a.slug == slug;});
    if (found == articles.end()) throw std::runtime_error("Missing article: " + string(slug));
    return *found;
}

struct Diagram {
    string slug, kind, title, control, caption, initial_text;
    int section, min, max, step, initial;
};

const std::vector<Diagram> diagrams = {
    {"gaussian-splatting","gaussian","一个像素里的三层颜色","前景有效不透明度","观察两团高斯的交点。后方蓝色在这个像素上的有效不透明度固定为 50%。","前景贡献 60%，后景贡献 20%，背景贡献 20%。",3,0,100,5,60},
    {"stereo-disparity-depth","disparity","同一个点在两幅图里的位置","左右视差","两台相机已做极线校正，焦距为 600 像素，基线为 0.10 米。两图中的竖线标记同一个点。","视差 20 像素时，深度为 3.00 米。",2,6,60,1,20},
    {"icp","registration","两组扫描点的配准过程","迭代次数","空心圆是固定的目标，实心点是移动的扫描。虚线表示当前的最近邻配对。这是二维点到点 ICP 示例。","第 0 次是初始位置。每次迭代重新寻找最近邻，再计算旋转和平移。",4,0,6,1,0},
    {"network","framing","两条消息，分两次读","第一次读到的字节数","连续发送 A、B 两帧，每帧 15 字节。竖线表示第一次读取的边界，其余字节由第二次读取。","第一次读到 7 字节，可以交付 0 帧，暂存 7 字节。",3,1,30,1,7},
    {"gpt-sampling","sampling","三个候选词的概率","采样温度","假设“一杯”后面三个候选词茶、水、奶的模型分数为 2、1、0。这里保留全部候选。","温度为 1 时，三个概率约为 66.5%、24.5% 和 9.0%。",2,3,25,1,10},
    {"session-recording","queue","录制队列的积压","写入速度","相机每秒产生 25 帧，连续录制 4 秒。假设开始时队列为空、容量足够，忽略单帧处理耗时波动。","每秒能写 15 帧，4 秒共收到 100 帧，写出 60 帧，仍有 40 帧等待。",2,5,40,1,15}
};

const Diagram* diagram_for(const string& slug) {
    for (const auto& d : diagrams) if (d.slug == slug) return &d;
    return nullptr;
}

string diagram_html(const Diagram& d) {
    std::ostringstream out;
    out << "<figure id='diagram' class='lesson-diagram' data-kind='" << d.kind << "'>"
        << "<figcaption><strong>" << html_escape(d.title) << "</strong><p>" << html_escape(d.caption) << "</p></figcaption>"
        << "<div class='plot'><img class='poster' src='../../assets/figures/" << d.kind << ".webp' width='1120' height='452' loading='lazy' alt='"
        << html_escape(d.initial_text) << "'><canvas hidden aria-hidden='true'></canvas></div>"
        << "<div class='figure-control' hidden><label for='figure-value'>" << d.control << "</label>"
        << "<input type='range' id='figure-value' aria-describedby='figure-result' min='" << d.min << "' max='" << d.max
        << "' step='" << d.step << "' value='" << d.initial << "'><output for='figure-value'></output></div>"
        << "<p class='figure-result' id='figure-result'>" << html_escape(d.initial_text) << "</p></figure>";
    return out.str();
}

string lesson_card(const Article& a, const string& base, const string& number = "") {
    std::ostringstream out;
    out << "<article class='note-row' data-group='" << a.group << "'><a href='" << base << "notes/" << a.slug
        << "/'><div class='card-art'><img src='" << base << html_escape(a.visual) << "' loading='lazy' width='640' height='360' alt=''></div>"
        << "<div class='card-copy'><div class='card-meta'><span class='topic'>" << (number.empty() ? a.topic : number) << "</span>" << duration(a)
        << "</div><h3>" << html_escape(a.title) << "</h3><p>" << html_escape(a.summary) << "</p></div></a></article>";
    return out.str();
}

string path_cards(const json& paths, const string& base) {
    std::ostringstream out;
    out << "<div class='path-grid'>";
    for (const auto& path : paths) {
        const string id = path.at("id");
        out << "<a class='path-card' id='" << id << "' href='" << base << "topics/" << id << "/'>"
            << "<figure><img loading='lazy' src='" << base << html_escape(path.at("visual")) << "' width='640' height='360' alt=''></figure>"
            << "<div><span class='path-count'>" << path.at("notes").size() << " 篇</span><h3>" << html_escape(path.at("name")) << "</h3><p>" << html_escape(path.at("description")) << "</p></div></a>";
    }
    out << "</div>";
    return out.str();
}

string homepage(const std::vector<Article>& articles, const json& library) {
    std::ostringstream out;
    out << "<main id='main' class='home wrap'><section class='course-hero'><div class='hero-copy'><span class='section-label'>精选 · 机器人入门</span>"
        << "<h1>用模拟海龟<br>学 ROS 2</h1><p>从一个会画圆的小程序开始，认识节点、话题与服务，再录下一段运动，亲手回放。</p>"
        << "<a class='plain-link feature-read' href='notes/ros2/'>阅读这篇文章</a></div>"
        << "<figure class='hero-experiment'><a href='notes/ros2/' aria-label='阅读用模拟海龟学 ROS 2'><img src='assets/turtlesim-run.webp' "
        << "alt='模拟海龟运动留下的圆弧' width='1000' height='1000' fetchpriority='high'></a>"
        << "<figcaption>turtlesim 实录</figcaption></figure></section>"
        << "<div class='home-reading'><section class='shelf' id='articles'><header class='section-heading'><h2>文章选读</h2><a href='notes/'>全部文章</a></header>"
        << "<div class='note-list'>";
    for (const string slug : library.at("picks")) out << lesson_card(lookup(articles,slug),"");
    out << "</div></section><aside class='home-routes'><h2>学习方向</h2><nav aria-label='选择学习方向'>";
    for (const auto& path : library.at("paths")) {
        out << "<a href='topics/" << path.at("id").get<string>() << "/'><span>" << html_escape(path.at("name"))
            << "</span><span class='path-count'>" << path.at("notes").size() << " 篇</span></a>";
    }
    out << "</nav><a class='plain-link' href='topics/'>查看全部方向</a></aside></div></main>";
    return document("./","home",brand + " · 机器人、视觉与计算机学习笔记","循序学习视觉、机器人、三维渲染与计算机系统。",out.str());
}

string archivepage(const std::vector<Article>& articles, const json& library) {
    std::ostringstream out;
    out << "<main id='main' class='wrap collection'><header class='collection-heading'><h1>文章</h1><a class='plain-link' href='../topics/'>按方向浏览</a></header>"
        << "<div class='filters' role='group' aria-label='按主题筛选'><button class='all-filter' data-filter='all' type='button' aria-pressed='true'>全部文章</button>";
    for (const auto& path : library.at("paths")) {
        const string group = path.at("id");
        out << "<button data-filter='" << group << "' type='button' aria-pressed='false'>" << html_escape(path.at("name")) << "</button>";
    }
    out << "</div><h2 class='results-title' id='filter-count' aria-live='polite'>共 " << articles.size() << " 篇</h2><div class='note-list' id='articles'>";
    for (const auto& a : articles) out << lesson_card(a,"../");
    out << "</div></main>";
    return document("../","archive","文章索引 · " + brand,"按配图和主题浏览所有学习笔记。",out.str());
}

string topics_page(const json& library) {
    const string body="<main id='main' class='wrap collection'><header class='collection-heading'><h1>学习方向</h1></header>"
        "<p class='collection-intro'>选一个感兴趣的方向，查看相关知识与建议的学习顺序。</p>"
        +path_cards(library.at("paths"),"../")+"</main>";
    return document("../","topics","学习方向 · "+brand,"按六个学习方向浏览文章和分阶段的学习路线。",body);
}

string path_page(const json& path, const std::vector<Article>& articles) {
    const string name=path.at("name"), group=path.at("id");
    const auto& notes=path.at("notes");
    const auto& stages=path.at("stages");
    std::ostringstream out;
    out << "<main id='main' class='wrap path-page'><nav class='crumbs' aria-label='面包屑'><a href='../'>学习方向</a><span>/</span>" << html_escape(name) << "</nav>"
        << "<header class='path-head'><div><span class='section-label'>"
        << stages.size() << " 个阶段 · " << notes.size() << " 篇</span><h1>" << html_escape(name) << "</h1><p>" << html_escape(path.at("description"))
        << "</p><a class='plain-link' href='../../notes/" << notes[0].get<string>() << "/'>从第一课开始</a></div></header><details class='stage-menu'><summary>查看学习阶段</summary><nav class='stage-nav' aria-label='跳至学习阶段'>";
    for (size_t j=0;j<stages.size();++j) out << "<a href='#stage-" << j+1 << "'>" << j+1 << " · " << html_escape(stages[j].at("name")) << "</a>";
    out << "</nav></details>";
    size_t i=0;
    for (size_t j=0;j<stages.size();++j) {
        const auto& stage=stages[j];
        if (i==notes.size() || stage.at("begin")!=notes[i]) throw std::runtime_error("Invalid stage in "+group);
        out << "<section class='stage' id='stage-" << j+1 << "'><div class='stage-heading'><span class='section-label'>阶段 " << j+1 << "</span><h2>"
            << html_escape(stage.at("name")) << "</h2><p>" << html_escape(stage.at("description")) << "</p></div><div class='note-list'>";
        do {
            out << lesson_card(lookup(articles,notes[i].get_ref<const string&>()),"../../","第 "+std::to_string(i+1)+" 课");
            ++i;
        } while(i<notes.size() && (j+1==stages.size() || notes[i]!=stages[j+1].at("begin")));
        out << "</div></section>";
    }
    out << "</main>";
    return document("../../","topics",name+" · 学习路线 · "+brand,path.at("description"),out.str());
}

string communication_lab() {
    return R"(<section class='ros-lab' id='communication' aria-labelledby='lab-title'><h3 id='lab-title'>试试三种通信方式</h3>
<p>给同一只模拟海龟发送三种不同的指令，观察哪些需要应答，哪些会持续返回进度。</p>
<div class='ros-controls' hidden><fieldset class='ros-modes'><legend>通信类型</legend>
<label><input type='radio' name='ros-mode' value='topic' checked>话题</label><label><input type='radio' name='ros-mode' value='service'>服务</label><label><input type='radio' name='ros-mode' value='action'>动作</label></fieldset>
<div class='ros-link'><div><span>发送端</span><strong data-role='client'></strong></div><div class='ros-wire'><i></i></div><div><span>接收端</span><strong>/turtlesim</strong></div></div>
<p class='ros-channel'></p><p class='ros-state' aria-live='polite'></p><div class='ros-actions'><button type='button' class='button' data-ros-step>下一步</button><button type='button' class='quiet-button' data-ros-cancel hidden>请求取消</button></div></div>
<noscript><p>话题连续发送速度。服务发送一次生成请求，再取得新海龟的名字。旋转动作先申请目标，执行期间汇报角度，最后返回结果。</p></noscript></section>)";
}

string articlepage(const Article& a, const Article* previous, const Article* next, size_t position, const json& path) {
    const string base="../../", route=base+"topics/"+a.group+"/";
    const auto* diagram=diagram_for(a.slug);
    std::ostringstream out,toc;
    out << "<div class='reading-progress' aria-hidden='true'><span></span></div><main id='main' class='wrap reading-page'>"
        << "<nav class='crumbs' aria-label='面包屑'><a href='../../topics/'>学习方向</a><span>/</span><a href='" << route << "'>" << a.category << "</a></nav>"
        << "<header class='article-heading'><h1>"
        << html_escape(a.title) << "</h1><p>" << html_escape(a.summary) << "</p><div class='byline'><span class='author'>Qi</span><i></i><time datetime='" << a.published
        << "'>" << a.published << "</time><i></i>" << duration(a) << "<i></i><span>第 " << position+1 << " / " << path.at("notes").size()
        << " 课</span></div></header>";
    for (size_t i=0; i<a.sections.size(); ++i) {
        toc << "<li><a href='#s" << i+1 << "'>" << html_escape(a.sections[i].at("title")) << "</a></li>";
        if (diagram && i+1==size_t(diagram->section))
            toc << "<li class='diagram-toc'><a href='#diagram'>互动 · " << html_escape(diagram->title) << "</a></li>";
        if (a.slug=="ros2" && i+1==a.communication_section)
            toc << "<li class='diagram-toc'><a href='#communication'>互动 · 三种通信方式</a></li>";
    }
    out << "<div class='reading-layout'><aside class='toc'><details><summary>文章目录</summary><ol><li><a href='#preface'>序言</a></li>" << toc.str()
        << "<li><a href='#sources'>延伸阅读</a></li></ol></details></aside><article class='prose'>"
        << "<section class='preface' aria-labelledby='preface'><h2 id='preface'>序言</h2><p>" << prose(a.intro) << "</p></section>";
    size_t id=0;
    for(const auto& section : a.sections) {
        const string key="s"+std::to_string(++id),title=html_escape(section.at("title"));
        out << "<section aria-labelledby='" << key << "'><h2 id='" << key << "'>" << title << "</h2>";
        for(const string p : section.at("paragraphs")) out << "<p>" << prose(p) << "</p>";
        if (section.contains("code")) out << "<figure class='terminal'><figcaption>" << html_escape(section.value("codeTitle",string("终端示例")))
            << "</figcaption><pre tabindex='0'><code>" << html_escape(section.at("code")) << "</code></pre></figure>";
        if (id==a.visual_section)
            out << "<figure class='article-image'><img src='" << base << html_escape(a.visual) << "' width='640' height='360' loading='lazy' alt=''>"
                << "<figcaption><span class='caption-text'>" << prose(a.figure_caption) << "</span><a class='figure-zoom' href='" << base
                << html_escape(a.visual) << "'>放大图解</a></figcaption></figure>";
        if (diagram && id==size_t(diagram->section)) {
            out << diagram_html(*diagram);
        }
        if (a.slug=="ros2" && id==a.communication_section) {
            out << communication_lab();
        }
        out << "</section>";
    }
    out << "<section class='references' aria-labelledby='sources'><h2 id='sources'>延伸阅读</h2><ul>";
    for(const auto& ref:a.refs) out << "<li><a href='" << html_escape(ref.at("url")) << "'>" << html_escape(ref.at("label")) << "</a></li>";
    auto chapter_link=[&](const Article* link,const string& label){
        out << "<a class='next-note' href='" << (link?"../"+link->slug+"/":route) << "'><span>" << label << "</span><strong>"
            << html_escape(link?link->title:path.at("name").get<string>()) << "</strong></a>";
    };
    out << "</ul></section><nav class='chapter-links' aria-label='课程阅读顺序'>";
    chapter_link(previous,previous?"上一课":"学习路线");
    chapter_link(next,next?"下一课":"返回学习路线");
    out << "</nav></article></div></main>";
    if(diagram) out << "<script defer src='../../assets/diagrams.js?v=perf-9'></script>";
    if(a.slug=="ros2") out << "<script defer src='../../assets/ros-communication.js?v=reading-3'></script>";
    return document(base,"note",a.title+" · "+brand,a.summary,out.str());
}

string aboutpage() {
    const string body = "<main id='main' class='wrap about-page'><div class='about-panel'><header class='author-profile'>"
        "<img class='author-avatar' src='../assets/author-avatar.jpg' width='144' height='144' alt='Qi 的头像' decoding='async'>"
        "<div><p class='section-label'>关于作者</p><h1>我是 Qi</h1><p class='author-focus'>具身智能</p></div></header>"
        "<div class='about-text'><p>我关注 SLAM、高斯渲染、数字孪生、机器学习和计算机视觉（CV）。</p>"
        "<p>这个博客用来整理学习和实践中的笔记，把概念、推导和实现过程慢慢写清楚。</p>"
        "<p>第一次来，可以先选一个<a href='../topics/'>学习方向</a>，再按路线慢慢读。已经有了具体问题，可以在<a href='../notes/'>文章索引</a>里找关键词。</p></div></div></main>";
    return document("../","about","关于 Qi · " + brand,"Qi 的个人介绍：具身智能、SLAM、高斯渲染、数字孪生、机器学习与计算机视觉。",body);
}

int main() try {
    std::ifstream catalog("_qi-blog/library.json");
    const auto library = json::parse(catalog);
    std::vector<Article> articles;
    json index = json::array();
    for (const auto& entry : fs::directory_iterator("_qi-blog/articles")) {
        if (entry.path().extension() != ".json") continue;
        std::ifstream file(entry.path());
        const auto j = json::parse(file);
        if (j.at("slug") != entry.path().stem().string()) throw std::runtime_error("Incorrect slug: " + entry.path().string());
        size_t length = 0;
        for (const auto& s : j.at("sections")) for (const string p : s.at("paragraphs"))
            length += std::count_if(p.begin(), p.end(), [](unsigned char c){ return (c & 0xc0) != 0x80; });
        const string intro = j.at("intro");
        length += std::count_if(intro.begin(),intro.end(),[](unsigned char c){return (c & 0xc0) != 0x80;});
        articles.push_back({j.at("slug"),j.at("title"),j.at("summary"),intro,j.at("topic"),j.at("category"),j.at("group"),j.at("keywords"),j.at("published"),
                            j.at("sections"),j.at("refs"),j.at("visual"),j.at("visualCaption"),j.value("visualSection",size_t{1}),j.value("communicationSection",size_t{5}),std::max(1,static_cast<int>((length + 279) / 280))});
    }
    std::vector<Article> ordered;
    std::set<string> assigned;
    for (const auto& path : library.at("paths")) {
        const auto& slugs = path.at("notes");
        for (size_t i = 0; i < slugs.size(); ++i) {
            const auto& a = lookup(articles,slugs.at(i).get_ref<const string&>());
            if (path.at("id") != a.group || !assigned.insert(a.slug).second)
                throw std::runtime_error("Check topic membership: " + a.slug);
            const Article* next = i + 1 == slugs.size() ? nullptr : &lookup(articles,slugs.at(i+1).get_ref<const string&>());
            if (!fs::is_regular_file(a.visual) || a.figure_caption.empty() || a.visual_section<1 || a.visual_section>a.sections.size())
                throw std::runtime_error("Check teaching illustration for "+a.slug);
            const Article* previous=i ? &lookup(articles,slugs.at(i-1).get_ref<const string&>()) : nullptr;
            write(destination/"notes"/a.slug/"index.html",articlepage(a,previous,next,i,path));
            ordered.push_back(a);
            index.push_back({{"slug",a.slug},{"title",a.title},{"summary",a.summary},{"topic",a.topic},{"keywords",a.keywords + " " + a.category}});
        }
        write(destination/"topics"/path.at("id").get<string>()/"index.html",path_page(path,articles));
    }
    if (ordered.size() != articles.size()) throw std::runtime_error("A lesson is missing from the reading paths.");
    write(destination/"index.html",homepage(articles,library));
    write(destination/"notes/index.html",archivepage(ordered,library));
    write(destination/"topics/index.html",topics_page(library));
    write(destination/"about/index.html",aboutpage());
    write(destination/"assets/search-index.js","window.QI_ARTICLES = " + index.dump(2) + ";\n");
    std::cout << "Built Qi’blog with " << articles.size() << " articles.\n";
} catch (const std::exception& e) {
    std::cerr << e.what() << '\n';
    return 1;
}
