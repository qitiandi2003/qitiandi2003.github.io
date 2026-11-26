// Run from the repository root. See _qi-blog/README.md.
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>
#include <nlohmann/json.hpp>

namespace fs = std::filesystem;
using json = nlohmann::json;
using std::string;

const fs::path destination = "qi-blog";
const string github = "https://github.com/qitiandi2003";
const string brand = "Qi‘blog";

struct Article {
    string slug, title, summary, topic, category, group, keywords, published;
    json sections, refs;
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
            case 39: out += "&#39;"; break;
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
    else if (name == "book") paths = R"(<path d="M3 5.5c3.5-1.4 6.5-.8 9 1 3-1.8 5.8-2.3 9-1v13c-3.6-1.4-6.5-.8-9 1-2.7-1.8-5.7-2.3-9-1zM12 7v12"/>)";
    else if (name == "close") paths = R"(<path d="m6 6 12 12M18 6 6 18"/>)";
    else if (name == "layers") paths = R"(<path d="m3 8 9-5 9 5-9 5-9-5zm0 4 9 5 9-5m-18 4 9 5 9-5"/>)";
    else if (name == "github") paths = R"(<path fill="currentColor" stroke="none" d="M12 .9a11.1 11.1 0 0 0-3.51 21.63c.55.1.76-.24.76-.54v-2.08c-3.09.67-3.74-1.31-3.74-1.31-.5-1.28-1.23-1.62-1.23-1.62-1.01-.69.08-.68.08-.68 1.11.08 1.69 1.14 1.69 1.14.99 1.7 2.59 1.21 3.22.92.1-.72.38-1.21.7-1.49-2.47-.28-5.06-1.23-5.06-5.49 0-1.21.43-2.21 1.14-2.98-.11-.28-.49-1.41.11-2.94 0 0 .93-.3 3.05 1.14a10.6 10.6 0 0 1 5.56 0c2.12-1.44 3.04-1.14 3.04-1.14.6 1.53.22 2.66.11 2.94.71.77 1.14 1.77 1.14 2.98 0 4.27-2.6 5.21-5.07 5.49.4.34.75 1.02.75 2.06v3.06c0 .3.21.65.77.54A11.1 11.1 0 0 0 12 .9z"/>)";
    else throw std::logic_error("Unknown icon: " + name);
    return R"(<svg aria-hidden="true" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="1.7" stroke-linecap="round" stroke-linejoin="round">)" + paths + "</svg>";
}

string wordmark(const string& base) {
    return "<a class='brand' href='" + base + "' aria-label='Qi‘blog 首页'><span class='mark'>" +
           icon("layers") + "</span><span lang='en'>Qi<span class='quote'>‘</span>blog</span></a>";
}

string header(const string& base, const string& page) {
    auto nav = [&](const string& id, const string& href, const string& label) {
        return "<a href='" + base + href + "'" + (id == page ? " aria-current='page'" : "") + ">" + label + "</a>";
    };
    return "<a class='skip' href='#main'>跳至正文</a><header class='site-header'><div class='header-inner wrap'>" + wordmark(base) +
        "<nav class='primary-nav' aria-label='主导航'>" + nav("home", "", "首页") + nav("archive", "#articles", "文章索引") +
        nav("about", "about/", "关于") + "</nav><div class='header-actions'><button class='search-button' type='button' data-open-search aria-haspopup='dialog' aria-label='搜索文章'>" +
        icon("search") + "<span>搜索文章</span><kbd>/</kbd></button><a class='github-button' href='" + github + "' aria-label='Qi 的 GitHub 主页'>" +
        icon("github") + "<span>GitHub</span></a></div></div></header>";
}

string footer(const string& base) {
    return "<footer class='site-footer'><div class='wrap footer-inner'>" + wordmark(base) +
         "<span>© 2026 Qi</span><a href='" + github + "/qitiandi2003.github.io/tree/master/_qi-blog'>博客源码</a></div></footer>"
         "<dialog id='search-dialog' class='search-dialog' aria-labelledby='search-title'><div class='dialog-head'><h2 id='search-title'>搜索文章</h2>"
         "<button type='button' class='icon-button' aria-label='关闭搜索' data-close-search>" + icon("close") +
         "</button></div><div class='search-field'>" + icon("search") +
         "<input id='search-input' type='search' aria-label='搜索文章关键词' placeholder='输入关键词，例如 ICP 或点云' autocomplete='off'></div>"
         "<p id='search-status' role='status'></p><div id='search-results'></div></dialog>";
}

string document(const string& base, const string& page, const string& title, const string& desc, const string& body) {
    return "<!doctype html>\n<html lang='zh-CN' data-base='" + base + "'><head><meta charset='utf-8'>"
        "<meta name='viewport' content='width=device-width,initial-scale=1'><meta name='theme-color' content='#f8f9f5'>"
        "<title>" + html_escape(title) + "</title><meta name='description' content='" + html_escape(desc) + "'>"
        "<link rel='icon' href='" + base + "assets/favicon.svg' type='image/svg+xml'><link rel='stylesheet' href='" +
        base + "assets/site.css'><script defer src='" + base + "assets/search-index.js'></script><script defer src='" + base +
        "assets/site.js'></script><noscript><style>.search-button,.filters{display:none}</style></noscript></head><body>" +
        header(base,page) + body + footer(base) + "</body></html>\n";
}

string duration(const Article& a) {
    return "<span class='duration'>" + icon("clock") + "约 " + std::to_string(a.minutes) + " 分钟</span>";
}

const std::vector<std::pair<string,string>> groups = {
    {"all","全部"}, {"geometry","定位与建图"}, {"vision","感知与标定"}, {"graphics","图形渲染"}, {"systems","系统与通信"}
};

string cover(const Article& a, const string& base) {
    return base + "assets/" + a.slug + ".webp";
}

string homepage(const std::vector<Article>& articles, const Article& feature) {
    std::ostringstream out;
    out << "<main id='main' class='wrap home'><section class='feature' aria-labelledby='feature-title'>"
        << "<img class='feature-art' src='" << cover(feature,"") << "' alt='' fetchpriority='high'><div class='feature-content'>"
        << "<div class='feature-meta'><span class='feature-tag'>" << icon("book") << "精选手记</span>"
        << "<span class='feature-series' lang='en'>3D GAUSSIAN SPLATTING</span></div>"
        << "<h1 id='feature-title'>从点云到<br>高斯渲染<span class='title-stop' aria-hidden='true'>.</span></h1>"
        << "<p>跟着一扇门框，从相机位姿、稀疏点云，一路看到新视角里的第一个像素。</p>"
        << "<div class='feature-actions'><a class='feature-read' href='notes/" << feature.slug << "/'>" << icon("book") << "阅读文章</a>"
        << duration(feature) << "</div></div><span class='figure-caption'>建筑点云示意</span></section>"
        << "<section id='articles' class='archive' aria-labelledby='archive-title'><div class='archive-title-row'><div><span class='section-label' lang='en'>FIELD NOTES</span>"
        << "<h2 id='archive-title'>文章索引</h2></div><span id='filter-count' aria-live='polite'>共 " << articles.size() << " 篇</span></div>"
        << "<div class='filters' role='group' aria-label='按文章主题筛选'>";
    for (const auto& [key, name] : groups) {
        out << "<button type='button' data-filter='" << key << "' aria-pressed='" << (key == "all" ? "true" : "false") << "'>" << name << "</button>";
    }
    out << "</div><div class='article-grid'>";
    for (const auto& a : articles) {
        out << "<article class='card' data-group='" << a.group << "'><a href='notes/" << a.slug << "/' class='card-link'>"
            << "<div class='card-image'><img src='" << cover(a,"") << "' alt='' loading='lazy' width='760' height='375'></div>"
            << "<div class='card-copy'><div class='card-meta'><span class='topic'>" << a.topic << "</span>" << duration(a) << "</div>"
            << "<h3>" << html_escape(a.title) << "</h3><p>" << html_escape(a.summary) << "</p></div></a></article>";
    }
    out << "</div></section></main>";
    return document("./","home",brand + " · 视觉、机器人与系统工程手记","从公开项目与官方文档出发，探索 SLAM、点云配准、联合标定、OpenCV、3DGS、ROS 2 与计算机网络。",out.str());
}

string articlepage(const Article& a, const Article& next) {
    const string base = "../../";
    std::ostringstream out;
    out << "<main id='main' class='wrap reading-page'><nav class='crumbs' aria-label='面包屑'><a href='" << base << "#articles'>文章索引</a><span>/</span>"
        << "<a href='" << base << "?group=" << a.group << "#articles'>" << a.category << "</a></nav>"
        << "<header class='article-heading'><span class='section-label'>" << a.topic << "</span><h1>" << html_escape(a.title) << "</h1><p>" << html_escape(a.summary) << "</p>"
        << "<div class='byline'><span class='author'>Qi</span><i></i><time datetime='" << a.published << "'>" << a.published << "</time><i></i>" << duration(a) << "</div></header>"
        << "<div class='reading-layout'><article class='prose'><div class='article-image'><img src='" << cover(a,base) << "' alt='' width='760' height='375'></div>";
    std::ostringstream toc;
    size_t id = 0;
    for (const auto& section : a.sections) {
        string key = "s" + std::to_string(++id), title = html_escape(section.at("title"));
        toc << "<li><a href='#" << key << "'>" << title << "</a></li>";
        out << "<section aria-labelledby='" << key << "'><h2 id='" << key << "'>" << title << "</h2>";
        for (const string p : section.at("paragraphs")) out << "<p>" << prose(p) << "</p>";
        out << "</section>";
    }
    out << "<section class='references' aria-labelledby='sources'><h2 id='sources'>源码与参考资料</h2><ul>";
    for (const auto& ref : a.refs) {
        out << "<li><a href=\"" << html_escape(ref.at("url")) << "\">" << html_escape(ref.at("label")) << "</a></li>";
    }
    out << "</ul></section><a class='next-note' href='../" << next.slug << "/'><span>接着读 · " << next.topic << "</span><strong>"
        << html_escape(next.title) << "</strong></a></article><aside class='toc'><h2>文章目录</h2><ol>"
        << toc.str() << "<li><a href='#sources'>源码与参考资料</a></li></ol></aside></div></main>";
    return document(base, "note", a.title + " · " + brand, a.summary, out.str());
}

string aboutpage() {
    const string base = "../";
    const string body = "<main id='main' class='wrap about-page'><div class='about-panel'><span class='section-label' lang='en'>ABOUT</span>"
        "<h1>关于 Qi‘blog</h1><p class='about-lead'>把一次工程排查，写成一篇能跟着验证的手记。</p>"
        "<div class='about-text'><h2>这些笔记从哪里来</h2><p>我是 Qi。这里记录视觉、机器人与系统工程里的学习问题：相机走过了哪里，两帧点云怎样对齐，一条控制指令又怎样抵达设备。</p>"
        "<p>有些文章从仓库里的一段代码讲起，有些从论文里的公式开始。每篇文末都列出源码或原始资料，方便回到实现核对细节。</p>"
        "<h2>从仓库开始读</h2><ul><li><a href='" + github + "/Bottle-Automatic-Volume-Adjustment-System'>自动定容系统</a><p>从液面检测看图像梯度，从设备通信看 TCP 字节流。</p></li>"
        "<li><a href='" + github + "/Coding_Skills'>Coding_Skills</a><p>沿着录包和点云复核笔记，追踪一次观测异常最早出现的位置。</p></li></ul>"
        "<p class='about-links'><a href='" + github + "'>GitHub 主页</a><a href='/archives/'>2023 年旧站归档</a></p></div></div></main>";
    return document(base, "about", "关于 · " + brand, "Qi 的技术手记与公开工程仓库。",body);
}

int main() try {
    std::vector<Article> articles;
    json index = json::array();
    for (const string slug : {"slam","icp","calibration","opencv","gaussian-splatting","ros2","network"}) {
        std::ifstream file(fs::path("_qi-blog/articles") / (slug + ".json"));
        auto j = json::parse(file);
        size_t length = 0;
        for (const auto& s : j.at("sections")) for (const string p : s.at("paragraphs"))
            length += std::count_if(p.begin(), p.end(), [](unsigned char c){ return (c & 0xc0) != 0x80; });
        Article a{j.at("slug"),j.at("title"),j.at("summary"),j.at("topic"),j.at("category"),j.at("group"),j.at("keywords"),j.at("published"),
                  j.at("sections"),j.at("refs"),std::max(1,static_cast<int>((length + 279) / 280))};
        index.push_back({{"slug",slug},{"title",a.title},{"summary",a.summary},{"topic",a.topic},{"keywords",a.keywords + " " + a.category}});
        articles.push_back(a);
    }
    const auto feature = std::find_if(articles.begin(), articles.end(), [](const Article& a){return a.slug == "gaussian-splatting";});
    write(destination/"index.html", homepage(articles,*feature));
    for (size_t i = 0; i < articles.size(); ++i)
        write(destination/"notes"/articles[i].slug/"index.html",articlepage(articles[i],articles[(i+1)%articles.size()]));
    write(destination/"about/index.html",aboutpage());
    write(destination/"assets/search-index.js","window.QI_ARTICLES = " + index.dump(2) + ";\n");
    std::cout << "Built Qi‘blog with " << articles.size() << " articles.\n";
} catch (const std::exception& e) {
    std::cerr << e.what() << '\n';
    return 1;
}
