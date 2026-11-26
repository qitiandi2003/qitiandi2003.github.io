# Qi‘blog

[访问博客](https://qitiandi2003.github.io/qi-blog/) · [文章索引](https://qitiandi2003.github.io/qi-blog/#articles)

文章保存在 `_qi-blog/articles/`，每篇包含标题、摘要、主题、发布日期、正文和参考链接。浏览器读取预先生成的 HTML。搜索和分类筛选在访客设备上运行。

## 修改文章

编辑对应的 JSON。在仓库根目录用 C++17 编译器和 nlohmann/json 开发头文件重新生成页面。

```bash
g++ -std=c++17 -O2 _qi-blog/build.cpp -o /tmp/qi-blog-build
/tmp/qi-blog-build
python3 -m http.server --bind 127.0.0.1 8000
```

打开本机的 `http://127.0.0.1:8000/qi-blog/` 预览。将文章源文件与生成的 `qi-blog/` 一起提交到 `master` 后，GitHub Pages 会更新公开站点。

新增文章时，在生成器的文章顺序列表中加入 slug，并把同名 WebP 封面放在 `qi-blog/assets/`。网站样式与搜索交互也在该目录中。
