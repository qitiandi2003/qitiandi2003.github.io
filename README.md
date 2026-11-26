# Qi‘blog

[博客首页](https://qitiandi2003.github.io/) · [学习专题](https://qitiandi2003.github.io/topics/) · [文章索引](https://qitiandi2003.github.io/notes/)

文章保存在 `_qi-blog/articles/`。每篇 JSON 包含标题、首页摘要、序言、分类、关键词、日期、正文和参考文献。首页选读与六个专题的阅读顺序由 `_qi-blog/library.json` 决定。

## 编辑与发布

在仓库根目录使用 C++17 编译器和 nlohmann/json 开发头文件构建。

```bash
g++ -std=c++17 -O2 _qi-blog/build.cpp -o /tmp/qi-blog-build
/tmp/qi-blog-build
python3 -m http.server --bind 127.0.0.1 8000
```

在浏览器打开 `http://127.0.0.1:8000/`。将源文件、生成的页面和搜索索引一起推送到 `master`，GitHub Pages 会从根目录发布。

新增文章时，创建与 slug 同名的 JSON，并在 `library.json` 对应专题的 `notes` 数组中插入 slug。顺序同时决定专题目录和文章底部的下一篇。文章引用的论文或官方文档放在 `refs`。站点导航提供作者的 GitHub 个人主页入口。

文章可使用 `assets/` 下与 slug 同名的 WebP 配图和 `figureCaption` 图注。交互图解由生成器添加到相应章节，绘图程序在 `assets/diagrams.js`，静态替代图在 `assets/figures/`。样式、搜索和标题字体位于 `assets/`。标题字使用 Noto Serif CJK 的简体中文子集，授权条款随字体保存。
