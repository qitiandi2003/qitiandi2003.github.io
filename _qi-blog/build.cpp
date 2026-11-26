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
#include <tuple>
#include <nlohmann/json.hpp>

namespace fs = std::filesystem;
using json = nlohmann::json;
using std::string;

const fs::path destination = ".";
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
        "<nav class='primary-nav' aria-label='主导航'>" + nav("home", "", "首页") + nav("archive", "notes/", "文章") + nav("repositories", "repositories/", "仓库") +
        nav("about", "about/", "关于") + "</nav><div class='header-actions'><button class='search-button' type='button' data-open-search aria-haspopup='dialog' aria-label='搜索文章'>" +
        icon("search") + "<span>搜索</span></button><a class='github-button' href='" + github + "' aria-label='Qi 的 GitHub 主页'>" +
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
        "<meta name='viewport' content='width=device-width,initial-scale=1'><meta name='theme-color' content='#f8f7f2'>"
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
    {"all","全部"}, {"geometry","定位与建图"}, {"vision","感知与标定"}, {"graphics","图形渲染"},
    {"systems","系统与通信"}, {"learning","机器学习"}, {"engineering","工程实践"}
};

const Article& lookup(const std::vector<Article>& articles, std::string_view slug) {
    const auto found = std::find_if(articles.begin(),articles.end(),[&](const Article& a){return a.slug == slug;});
    if (found == articles.end()) throw std::runtime_error("Missing article: " + string(slug));
    return *found;
}

bool has_cover(const Article& a) {
    return fs::is_regular_file(destination/"assets"/(a.slug + ".webp"));
}

string cover(const Article& a, const string& base) {
    return base + "assets/" + a.slug + ".webp";
}

string homepage(const std::vector<Article>& articles, const json& library) {
    const auto& feature = lookup(articles,"gaussian-splatting");
    std::ostringstream out;
    out << "<main id='main' class='wrap home'><section class='feature' aria-labelledby='feature-title'><div class='feature-content'>"
        << "<div class='feature-meta'>三维重建 <span>/</span> 3DGS</div><h1 id='feature-title'>门框边缘的<br>高斯渲染</h1>"
        << "<p>" << html_escape(feature.summary) << "</p><div class='feature-actions'><a class='feature-read' href='notes/" << feature.slug
        << "/'>阅读全文</a>" << duration(feature) << "</div></div><figure class='feature-figure'><img src='" << cover(feature,"")
        << "' width='1350' height='850' alt='建筑点云的轴测图，拱形门窗从银灰色采样点中显现。' fetchpriority='high'>"
        << "<figcaption><span>建筑点云习作</span><span>轴测视图</span></figcaption></figure></section>"
        << "<section id='articles' class='picks'><header class='section-heading'><h2>手记选读</h2><a href='notes/'>全部文章（" << articles.size()
        << "）</a></header><div class='pick-grid'>";
    for (const string slug : library.at("picks")) {
        const auto& a = lookup(articles,slug);
        out << "<article class='pick'><a href='notes/" << a.slug << "/'><img width='760' height='375' src='" << cover(a,"")
            << "' loading='lazy' alt=''><div class='card-meta'><span class='topic'>" << a.topic << "</span>" << duration(a) << "</div>"
            << "<h3>" << html_escape(a.title) << "</h3><p>" << html_escape(a.summary) << "</p></a></article>";
    }
    out << "</div></section></main>";
    return document("./","home",brand + " · 工程手记","Qi 的个人工程手记，记录三维视觉、机器人、机器学习与软件工程中的具体问题。",out.str());
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
    return document("../","archive","文章索引 · " + brand,"按主题浏览 Qi 的工程手记。",out.str());
}

string repositories(const std::vector<Article>& articles, const json& library) {
    std::ostringstream out;
    out << "<main id='main' class='wrap collection'><header class='collection-heading'><h1>仓库索引</h1></header>"
        << "<p class='collection-intro'>按项目查找工程笔记、原始代码和实验记录。</p>"
        << "<nav class='repo-nav' aria-label='跳转到仓库'>";
    for (const auto& repo : library.at("sources"))
        out << "<a href='#" << html_escape(repo.at("id")) << "'>" << html_escape(repo.at("repository")) << "</a>";
    out << "</nav>";
    for (const auto& repo : library.at("sources")) {
        const string url = repo.value("url","");
        out << "<section class='source' id='" << html_escape(repo.at("id")) << "'><div><h2>" << html_escape(repo.at("name")) << "</h2><p>"
            << html_escape(repo.at("summary")) << "</p>";
        if (!url.empty()) {
            out << "<a class='source-url' href='" << html_escape(url) << "'>" << icon("github") << html_escape(repo.at("repository")) << "</a><div class='module-links'>";
            for (const auto& link : repo.at("modules"))
                out << "<a href='" << html_escape(link.at("url")) << "'>" << html_escape(link.at("label")) << "</a>";
            out << "</div>";
        }
        out << "</div><ul>";
        for (const string slug : repo.at("notes")) {
            const auto& a = lookup(articles,slug);
            out << "<li><a href='../notes/" << a.slug << "/'>" << html_escape(a.title) << "</a><span>" << a.topic << "</span></li>";
        }
        out << "</ul></section>";
    }
    out << "</main>";
    return document("../","repositories","仓库索引 · " + brand,"按项目查找工程笔记、源码和参考资料。",out.str());
}

string articlepage(const Article& a, const Article& next, const json& library) {
    const string base = "../../";
    const bool cites_project = std::any_of(a.refs.begin(),a.refs.end(),[](const json& ref) {
        return ref.at("url").get_ref<const string&>().rfind(github + "/",0) == 0;
    });
    const string references_title = cites_project ? "源码与参考资料" : "参考资料";
    std::ostringstream out;
    out << "<main id='main' class='wrap reading-page'><nav class='crumbs' aria-label='面包屑'><a href='" << base << "notes/'>文章索引</a><span>/</span>"
        << "<a href='" << base << "notes/?group=" << a.group << "'>" << a.category << "</a></nav>"
        << "<header class='article-heading'><span class='section-label'>" << a.topic << "</span><h1>" << html_escape(a.title) << "</h1><p>" << html_escape(a.summary) << "</p>"
        << "<div class='byline'><span class='author'>Qi</span><i></i><time datetime='" << a.published << "'>" << a.published << "</time><i></i>" << duration(a) << "</div></header>"
        << "<div class='reading-layout'><article class='prose'>";
    if (has_cover(a)) out << "<div class='article-image'><img src='" << cover(a,base) << "' alt='' width='760' height='375'></div>";
    std::ostringstream toc;
    size_t id = 0;
    for (const auto& section : a.sections) {
        string key = "s" + std::to_string(++id), title = html_escape(section.at("title"));
        toc << "<li><a href='#" << key << "'>" << title << "</a></li>";
        out << "<section aria-labelledby='" << key << "'><h2 id='" << key << "'>" << title << "</h2>";
        for (const string p : section.at("paragraphs")) out << "<p>" << prose(p) << "</p>";
        out << "</section>";
    }
    out << "<section class='references' aria-labelledby='sources'><h2 id='sources'>" << references_title << "</h2><ul>";
    for (const auto& ref : a.refs) {
        out << "<li><a href=\"" << html_escape(ref.at("url")) << "\">" << html_escape(ref.at("label")) << "</a></li>";
    }
    out << "</ul><div class='origin-links'><span>相关项目</span>";
    for (const auto& project : library.at("sources")) {
        const auto& notes = project.at("notes");
        if (std::find(notes.begin(),notes.end(),a.slug) != notes.end())
            out << "<a href='../../repositories/#" << html_escape(project.at("id")) << "'>" << html_escape(project.at("name")) << "</a> ";
    }
    out << "</div></section><a class='next-note' href='../" << next.slug << "/'><span>接着读 · " << next.topic << "</span><strong>"
        << html_escape(next.title) << "</strong></a></article><aside class='toc'><h2>文章目录</h2><ol>"
        << toc.str() << "<li><a href='#sources'>" << references_title << "</a></li></ol></aside></div></main>";
    return document(base, "note", a.title + " · " + brand, a.summary, out.str());
}

string aboutpage() {
    const string body = "<main id='main' class='wrap about-page'><div class='about-panel'><h1>关于 Qi‘blog</h1>"
        "<div class='about-text'><p>我是 Qi，这里放的是我的工程手记。</p>"
        "<p>有时是一帧图像：一条亮边究竟是液面还是反光。有时是几行日志：机器人拐弯后，最先偏掉的是时间戳、坐标变换，还是地图上的轨迹。也有些问题来自读代码，比如一个词怎样变成向量，半条 TCP 消息该留在哪里。</p>"
        "<p>每篇手记都保留了用得上的源码或原始资料。想从某个项目开始，可以翻<a href='../repositories/'>仓库索引</a>。</p>"
        "<p class='about-links'><a href='" + github + "'>GitHub 主页</a></p></div></div></main>";
    return document("../","about","关于 · " + brand,"Qi 的个人工程手记。",body);
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
        articles.push_back({j.at("slug"),j.at("title"),j.at("summary"),j.at("topic"),j.at("category"),j.at("group"),j.at("keywords"),j.at("published"),
                            j.at("sections"),j.at("refs"),std::max(1,static_cast<int>((length + 279) / 280))});
    }
    std::sort(articles.begin(),articles.end(),[](const Article& a,const Article& b) {
        if (a.published != b.published) return a.published > b.published;
        return std::tie(a.category,a.slug) < std::tie(b.category,b.slug);
    });
    for (size_t i=0; i<articles.size(); ++i) {
        const auto& a=articles[i];
        string keywords = a.keywords + " " + a.category;
        for (const auto& repo : library.at("sources")) {
            const auto& notes = repo.at("notes");
            if (std::find(notes.begin(),notes.end(),a.slug) != notes.end())
                keywords += " " + repo.at("repository").get<string>() + " " + repo.at("name").get<string>();
        }
        index.push_back({{"slug",a.slug},{"title",a.title},{"summary",a.summary},{"topic",a.topic},{"keywords",keywords}});
        write(destination/"notes"/a.slug/"index.html",articlepage(a,articles[(i+1)%articles.size()],library));
    }
    write(destination/"index.html",homepage(articles,library));
    write(destination/"notes/index.html",archivepage(articles));
    write(destination/"repositories/index.html",repositories(articles,library));
    write(destination/"about/index.html",aboutpage());
    write(destination/"assets/search-index.js","window.QI_ARTICLES = " + index.dump(2) + ";\n");
    std::cout << "Built Qi‘blog with " << articles.size() << " articles.\n";
} catch (const std::exception& e) {
    std::cerr << e.what() << '\n';
    return 1;
}
