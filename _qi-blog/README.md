# Qi‘blog

[访问博客](https://qitiandi2003.github.io/qi-blog/) · [文章索引](https://qitiandi2003.github.io/qi-blog/notes/) · [仓库索引](https://qitiandi2003.github.io/qi-blog/repositories/)

文章在 `_qi-blog/articles/`。每篇 JSON 保存标题、摘要、分类、关键词、日期、正文与参考来源。`library.json` 保存首页选读与项目的对应关系。

## 编辑与发布

在仓库根目录使用 C++17 编译器和 nlohmann/json 开发头文件构建。

```bash
g++ -std=c++17 -O2 _qi-blog/build.cpp -o /tmp/qi-blog-build
/tmp/qi-blog-build
python3 -m http.server --bind 127.0.0.1 8000
```

打开本机的 `http://127.0.0.1:8000/qi-blog/`。把源文件与生成的 `qi-blog/` 一起提交到 `master` 后，GitHub Pages 会更新公开站点。

新增文章时，创建与 slug 同名的 JSON，加入 `library.json` 对应项目。生成器会收录到文章索引、站内搜索和对应的项目页。文章可使用同名 WebP 插图；首页选读需要配图。

样式、搜索脚本与原创插图位于 `qi-blog/assets/`。标题字使用 Noto Serif CJK 的简体中文子集，授权条款随字体保存。
