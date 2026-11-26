# Qi’blog

[博客首页](https://qitiandi2003.github.io/) · [学习方向](https://qitiandi2003.github.io/topics/) · [文章索引](https://qitiandi2003.github.io/notes/)

在 `_qi-blog/articles/` 编辑文章 JSON。每篇包括摘要、序言、正文小节、关键词和延伸阅读。每张教学图由 `visual` 指向站内素材，`visualCaption` 解释如何读图，`visualSection` 可指定插图位于第几节。小节支持可选的 `code` 和 `codeTitle`，用于带标题的命令或代码块。

六个学习方向的课程顺序在 `_qi-blog/library.json` 的 `paths` 中。每个方向的 `notes` 列出全部 slug，`stages` 按顺序给出各阶段的名称、学习目标和起始 slug。生成器依此生成路线页、课程编号和前后课入口。插入新课时，在对应方向的 `notes` 中加入 slug，并准备独立的教学图与图注。

## 构建与发布

在仓库根目录，用 C++17 编译器和 nlohmann/json 开发头文件构建。

```bash
g++ -std=c++17 -O2 _qi-blog/build.cpp -o /tmp/qi-blog-build
/tmp/qi-blog-build
python3 -m http.server --bind 127.0.0.1 8000
```

打开 `http://127.0.0.1:8000/` 预览。将源文件、生成的静态页面和搜索索引一并推至 `master` 后，由 GitHub Pages 发布。程序化教学图保存在 `assets/lessons/`，数值演示在 `assets/diagrams.js`，ROS 2 通信演示在 `assets/ros-communication.js`。

首页配图来自本机 ROS 2 Lyrical 的 turtlesim 1.10.9 运行截图。仿真同时接收前进速度 3.3 与角速度 0.9 rad/s。课程命令按 ROS 2 Jazzy 文档编写，正文列出了两个发行版的接口区别。
