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
const string brand = "Qi‘blog";

struct Article {
    string slug, title, summary, intro, topic, category, group, keywords, published;
    json sections, refs;
    string figure_caption;
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
    else if (name == "clock") paths = R"(<circle cx="12" cy="12" r="8.5"/><path d="M12 6.5V12l3.2 2"/>)";
    else if (name == "close") paths = R"(<path d="m6 6 12 12M18 6 6 18"/>)";
    else if (name == "github") paths = R"(<path fill="currentColor" stroke="none" d="M12 .9a11.1 11.1 0 0 0-3.51 21.63c.55.1.76-.24.76-.54v-2.08c-3.09.67-3.74-1.31-3.74-1.31-.5-1.28-1.23-1.62-1.23-1.62-1.01-.69.08-.68.08-.68 1.11.08 1.69 1.14 1.69 1.14.99 1.7 2.59 1.21 3.22.92.1-.72.38-1.21.7-1.49-2.47-.28-5.06-1.23-5.06-5.49 0-1.21.43-2.21 1.14-2.98-.11-.28-.49-1.41.11-2.94 0 0 .93-.3 3.05 1.14a10.6 10.6 0 0 1 5.56 0c2.12-1.44 3.04-1.14 3.04-1.14.6 1.53.22 2.66.11 2.94.71.77 1.14 1.77 1.14 2.98 0 4.27-2.6 5.21-5.07 5.49.4.34.75 1.02.75 2.06v3.06c0 .3.21.65.77.54A11.1 11.1 0 0 0 12 .9z"/>)";
    else throw std::logic_error("Unknown icon: " + name);
    return R"(<svg aria-hidden="true" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="1.7" stroke-linecap="round" stroke-linejoin="round">)" + paths + "</svg>";
}

string wordmark(const string& base) {
    return "<a class='brand' href='" + base + "' aria-label='Qi‘blog 首页'><span lang='en'><em>Qi</em>‘blog</span></a>";
}

string header(const string& base, const string& page) {
    auto nav = [&](const string& id, const string& href, const string& label) {
        return "<a href='" + base + href + "'" + (id == page ? " aria-current='page'" : "") + ">" + label + "</a>";
    };
    return "<a class='skip' href='#main'>跳至正文</a><header class='site-header'><div class='header-inner wrap'>" + wordmark(base) +
        "<nav class='primary-nav' aria-label='主导航'>" + nav("home", "", "首页") + nav("archive", "notes/", "文章") + nav("topics", "topics/", "专题") +
        nav("about", "about/", "关于") + "</nav><div class='header-actions'><button class='search-button' type='button' data-open-search aria-haspopup='dialog' aria-label='搜索文章'>" +
        icon("search") + "<span>搜索</span></button><a class='github-button' href='" + github + "' aria-label='Qi 的 GitHub 主页'>" +
        icon("github") + "<span>GitHub 主页</span></a></div></div></header>";
}

string footer(const string& base) {
    return "<footer class='site-footer'><div class='wrap footer-inner'>" + wordmark(base) +
         "<span>© 2026 Qi</span><a href='" + base + "about/'>关于作者</a></div></footer>"
         "<dialog id='search-dialog' class='search-dialog' aria-labelledby='search-title'><div class='dialog-head'><h2 id='search-title'>搜索文章</h2>"
         "<button type='button' class='icon-button' aria-label='关闭搜索' data-close-search>" + icon("close") +
         "</button></div><div class='search-field'>" + icon("search") +
         "<input id='search-input' type='search' aria-label='搜索文章关键词' placeholder='输入关键词，例如 ICP 或点云' autocomplete='off'></div>"
         "<p id='search-status' role='status'></p><div id='search-results'></div></dialog>";
}

string document(const string& base, const string& page, const string& title, const string& desc, const string& body) {
    return "<!doctype html>\n<html lang='zh-CN' data-base='" + base + "'><head><meta charset='utf-8'>"
        "<meta name='viewport' content='width=device-width,initial-scale=1'><meta name='theme-color' content='#f8f7f2'>"
        "<title>" + html_escape(title) + "</title><meta name='description' content='" + html_escape(desc) + "'>"
        "<link rel='icon' href='" + base + "assets/favicon.svg' type='image/svg+xml'><link rel='stylesheet' href='" +
        base + "assets/site.css?v=lessons-1'><script defer src='" + base + "assets/search-index.js?v=lessons-1'></script><script defer src='" + base +
        "assets/site.js?v=lessons-1'></script><noscript><style>.search-button,.filters{display:none}</style></noscript></head><body>" +
        header(base,page) + body + footer(base) + "</body></html>\n";
}

string duration(const Article& a) {
    return "<span class='duration'>" + icon("clock") + "约 " + std::to_string(a.minutes) + " 分钟</span>";
}

const std::vector<std::pair<string,string>> groups = {
    {"all","全部"}, {"geometry","定位与建图"}, {"vision","感知与标定"}, {"graphics","图形渲染"},
    {"systems","系统与通信"}, {"learning","机器学习"}, {"engineering","工程实践"}
};

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
    {"icp","registration","两组扫描点的配准过程","迭代次数","空心圆是固定的目标，实心点是移动的扫描。虚线表示当前的最近邻配对。这是二维点到点 ICP 示例。","第 0 次是初始位置。每次迭代重新寻找最近邻，再计算旋转和平移。",3,0,6,1,0},
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

string cover(const Article& a, const string& base) {
    if (fs::is_regular_file(destination/"assets"/(a.slug + ".webp"))) return base + "assets/" + a.slug + ".webp";
    if (const auto* d = diagram_for(a.slug)) return base + "assets/figures/" + d->kind + ".webp";
    return "";
}

string homepage(const std::vector<Article>& articles, const json& library) {
    const auto& feature = lookup(articles,"gaussian-splatting");
    std::ostringstream out;
    out << "<main id='main' class='wrap home'><section class='feature' aria-labelledby='feature-title'><div class='feature-content'>"
        << "<div class='feature-meta'>图形渲染 <span>/</span> 入门</div><h1 id='feature-title'>" << html_escape(feature.title) << "</h1>"
        << "<p>从几个拍摄角度出发，看一组有颜色、能透光的三维高斯怎样绘出新的视角。</p>"
        << "<div class='feature-actions'><a class='feature-read' href='notes/" << feature.slug << "/'>开始阅读</a>"
        << "<a class='diagram-link' href='notes/" << feature.slug << "/#diagram'>试试颜色混合</a></div></div>"
        << "<figure class='feature-figure'><img src='" << cover(feature,"") << "' width='1350' height='850' alt='由三维采样点勾勒出的建筑立面和拱窗' fetchpriority='high'>"
        << "<figcaption><span>建筑点云习作</span><span>轴测视图</span></figcaption></figure></section>"
        << "<section id='articles' class='picks'><header class='section-heading'><h2>从这里开始</h2><a href='notes/'>全部文章（" << articles.size()
        << "）</a></header><div class='pick-grid'>";
    for (const string slug : library.at("picks")) {
        const auto& a = lookup(articles,slug);
        out << "<article class='pick'><a href='notes/" << a.slug << "/'><div class='pick-image'><img width='760' height='375' src='" << cover(a,"")
            << "' loading='lazy' alt=''></div><div class='card-meta'><span class='topic'>" << a.topic << "</span>" << duration(a) << "</div>"
            << "<h3>" << html_escape(a.title) << "</h3><p>" << html_escape(a.summary) << "</p></a></article>";
    }
    out << "</div></section><section class='home-topics'><header class='section-heading'><h2>按主题学习</h2><a href='topics/'>阅读顺序</a></header><div class='topic-grid'>";
    for (const auto& path : library.at("paths")) {
        out << "<a href='topics/#" << html_escape(path.at("id")) << "'><h3>" << html_escape(path.at("name")) << "</h3><span>"
            << path.at("notes").size() << " 篇</span><p>" << html_escape(path.at("description")) << "</p></a>";
    }
    out << "</div></section></main>";
    return document("./","home",brand + " · 技术入门笔记","从第一条观测和一个小例子开始，学习计算机视觉、机器人、图形渲染、机器学习与网络。",out.str());
}

string archivepage(const std::vector<Article>& articles) {
    std::ostringstream out;
    out << "<main id='main' class='wrap collection'><header class='collection-heading'><h1>文章索引</h1>"
        << "<span id='filter-count' aria-live='polite'>共 " << articles.size() << " 篇</span></header>"
        << "<div class='filters' role='group' aria-label='按文章主题筛选'>";
    for (const auto& [key,name] : groups) {
        if (key != "all" && std::none_of(articles.begin(),articles.end(),[&](const Article& a){return a.group==key;})) continue;
        out << "<button type='button' data-filter='" << key << "' aria-pressed='" << (key=="all" ? "true" : "false") << "'>" << name << "</button>";
    }
    out << "</div><div class='note-list' id='articles'>";
    for (const auto& a : articles) {
        out << "<article class='note-row' data-group='" << a.group << "'><a href='" << a.slug << "/'><div class='card-meta'><span class='topic'>"
            << a.topic << "</span>" << duration(a) << "</div><h2>" << html_escape(a.title) << "</h2><p>" << html_escape(a.summary) << "</p></a></article>";
    }
    out << "</div></main>";
    return document("../","archive","文章索引 · " + brand,"按主题查找视觉、机器人和计算机系统的学习笔记。",out.str());
}

string topics_page(const std::vector<Article>& articles, const json& library) {
    std::ostringstream out;
    out << "<main id='main' class='wrap collection'><header class='collection-heading'><h1>学习专题</h1></header>"
        << "<p class='collection-intro'>选择一个感兴趣的方向。每组文章从基础概念读起，逐步进入具体问题。</p>"
        << "<nav class='topic-nav' aria-label='专题导航'>";
    for (const auto& path : library.at("paths"))
        out << "<a href='#" << html_escape(path.at("id")) << "'>" << html_escape(path.at("name")) << "</a>";
    out << "</nav>";
    for (const auto& path : library.at("paths")) {
        out << "<section class='source' id='" << html_escape(path.at("id")) << "'><div><h2>" << html_escape(path.at("name"))
            << "</h2><p>" << html_escape(path.at("description")) << "</p></div><ol>";
        for (const string slug : path.at("notes")) {
            const auto& a = lookup(articles,slug);
            out << "<li><a href='../notes/" << a.slug << "/'>" << html_escape(a.title) << "</a><span>" << duration(a) << "</span></li>";
        }
        out << "</ol></section>";
    }
    out << "</main>";
    return document("../","topics","学习专题 · " + brand,"按学习顺序阅读六个方向的入门文章。",out.str());
}

string articlepage(const Article& a, const Article* next) {
    const string base = "../../";
    const auto* diagram = diagram_for(a.slug);
    std::ostringstream out, toc;
    out << "<div class='reading-progress' aria-hidden='true'><span></span></div><main id='main' class='wrap reading-page'>"
        << "<nav class='crumbs' aria-label='面包屑'><a href='../../topics/'>学习专题</a><span>/</span>"
        << "<a href='../../topics/#" << a.group << "'>" << a.category << "</a></nav>"
        << "<header class='article-heading'><span class='section-label'>" << a.topic << "</span><h1>" << html_escape(a.title) << "</h1>"
        << "<div class='byline'><span class='author'>Qi</span><i></i><time datetime='" << a.published << "'>" << a.published << "</time><i></i>" << duration(a)
        << "</div></header><div class='reading-layout'><article class='prose'><section class='preface' aria-labelledby='preface'><h2 id='preface'>序言</h2><p>" << prose(a.intro) << "</p></section>";
    if (!diagram && !cover(a,base).empty())
        out << "<figure class='article-image'><img src='" << cover(a,base) << "' loading='lazy' alt='' width='760' height='375'>"
            << "<figcaption>" << html_escape(a.figure_caption) << "</figcaption></figure>";
    int id = 0;
    for (const auto& section : a.sections) {
        const string key = "s" + std::to_string(++id), title = html_escape(section.at("title"));
        toc << "<li><a href='#" << key << "'>" << title << "</a></li>";
        out << "<section aria-labelledby='" << key << "'><h2 id='" << key << "'>" << title << "</h2>";
        for (const string p : section.at("paragraphs")) out << "<p>" << prose(p) << "</p>";
        if (diagram && id == diagram->section) {
            out << diagram_html(*diagram);
            toc << "<li class='diagram-toc'><a href='#diagram'>图解 · " << html_escape(diagram->title) << "</a></li>";
        }
        out << "</section>";
    }
    out << "<section class='references' aria-labelledby='sources'><h2 id='sources'>延伸阅读</h2><ul>";
    for (const auto& ref : a.refs)
        out << "<li><a href=\"" << html_escape(ref.at("url")) << "\">" << html_escape(ref.at("label")) << "</a></li>";
    out << "</ul></section>";
    if (next) out << "<a class='next-note' href='../" << next->slug << "/'><span>本专题下一篇</span><strong>" << html_escape(next->title) << "</strong></a>";
    else out << "<a class='next-note' href='../../topics/'><span>本专题完</span><strong>浏览其他学习专题</strong></a>";
    out << "</article><aside class='toc'><details open><summary>文章目录</summary><ol><li><a href='#preface'>序言</a></li>"
        << toc.str() << "<li><a href='#sources'>延伸阅读</a></li></ol></details></aside></div></main>";
    if (diagram) out << "<script defer src='../../assets/diagrams.js?v=1'></script>";
    return document(base,"note",a.title + " · " + brand,a.summary,out.str());
}

string aboutpage() {
    const string body = "<main id='main' class='wrap about-page'><div class='about-panel'><h1>关于 Qi‘blog</h1>"
        "<div class='about-text'><p>我是 Qi。这个博客写给刚开始学视觉、机器人和计算机系统的朋友。</p>"
        "<p>我喜欢从能观察到的现象入手：交替闭上左右眼，近处的手指会移动；两张扫描图叠在一起，墙面贴合了，墙角却可能还差几厘米。把现象画出来，给它添上一两个数字，许多术语就有了着落。</p>"
        "<p>第一次来，可以按<a href='../topics/'>学习专题</a>慢慢读。已经有了具体问题，可以在<a href='../notes/'>文章索引</a>里找关键词。</p></div></div></main>";
    return document("../","about","关于 · " + brand,"关于 Qi 和这份技术入门笔记。",body);
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
                            j.at("sections"),j.at("refs"),j.value("figureCaption",string{}),std::max(1,static_cast<int>((length + 279) / 280))});
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
            write(destination/"notes"/a.slug/"index.html",articlepage(a,next));
            ordered.push_back(a);
            index.push_back({{"slug",a.slug},{"title",a.title},{"summary",a.summary},{"topic",a.topic},{"keywords",a.keywords + " " + a.category}});
        }
    }
    if (ordered.size() != articles.size()) throw std::runtime_error("A lesson is missing from the reading paths.");
    write(destination/"index.html",homepage(articles,library));
    write(destination/"notes/index.html",archivepage(ordered));
    write(destination/"topics/index.html",topics_page(articles,library));
    write(destination/"about/index.html",aboutpage());
    write(destination/"assets/search-index.js","window.QI_ARTICLES = " + index.dump(2) + ";\n");
    std::cout << "Built Qi‘blog with " << articles.size() << " articles.\n";
} catch (const std::exception& e) {
    std::cerr << e.what() << '\n';
    return 1;
}
