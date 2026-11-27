小熊熊动画回归检查

在仓库根目录启动本地静态服务，然后编译并运行 C++ 测试。

python3 -m http.server 8982 --bind 127.0.0.1
g++ -std=c++17 -O2 -Wall -Wextra -Wpedantic -Werror _qi-blog/tests/pet-motion.cpp -lcrypto -o /tmp/qi-pet-test
/tmp/qi-pet-test http://127.0.0.1:8982/

依赖本机 google-chrome、nlohmann/json.hpp 与 OpenSSL 开发库。
浏览器使用独立的临时用户目录，测试结束后清理；不会读取个人 Chrome 会话。

pet-motion.html 用于放大和原尺寸预览。可选动作、拖动时间滑块或连续播放。
测试覆盖约 6500 个姿势的关节长度和可达性、循环接缝、食物与嘴的接触、
落脚漂移、连续刷新、实际绘制耗时、频繁打断动作、出入场、拖动记忆、搜索暂停、
减少动态效果、手机端布局、2 倍像素密度和图形上下文丢失后的静态显示。
相邻帧位移检查只统计纹理中可见的点，排除透明网格边缘。
性能阈值针对本机 60 Hz Chrome 自动化环境，不代表所有设备的性能。

资产生成由 _qi-blog/build-plush-rig.cpp 完成，输入是原有的两张奶油小熊素材图。
身体保持同一张纹理，袖子沿连续曲线变形。运行时不再加载姿势补帧图集。

文章分步图解检查

先编译并运行 _qi-blog/build.cpp，以 _qi-blog/teaching/ 中的内容生成 SVG 和文章页。
在本地静态服务运行时执行：

g++ -std=c++17 -O2 -Wall -Wextra -Wpedantic -Werror _qi-blog/tests/teaching.cpp -lcrypto $(pkg-config --cflags --libs opencv4) -o /tmp/qi-teaching-test
/tmp/qi-teaching-test http://127.0.0.1:8982/

检查 50 篇文章的图解切换、手机宽度、练习展开、播放/暂停、减少动态效果及无脚本回退。
152 张 SVG 用浏览器实际字体检查标注越界与重叠，问题报告在 /tmp/qi-teaching-svg-audit.json。
Sobel 演示的两个方向及明暗反转结果逐格与 OpenCV 的 C++ 计算比对。
只检查 SVG 排版时，在地址后增加 --audit-svg；也可传入已发布站点地址做线上验证。
数据格式说明见 _qi-blog/teaching-format.txt。示意图及算例保留在仓库中，可随正文维护。

阅读导航检查

g++ -std=c++17 -O2 -Wall -Wextra -Wpedantic -Werror _qi-blog/tests/reading-navigation.cpp -lcrypto -o /tmp/qi-reading-test
/tmp/qi-reading-test http://127.0.0.1:8982/

检查左侧目录、滚动高亮、锚点与浏览器返回、键盘焦点、手机折叠、窄屏与短屏、
减少动态效果和无脚本回退，并核对 50 篇文章的目录链接与正文顺序。

玻璃材质与动效检查

g++ -std=c++17 -O2 -Wall -Wextra -Wpedantic -Werror _qi-blog/tests/reading-effects.cpp -lcrypto -o /tmp/qi-effects-test
/tmp/qi-effects-test http://127.0.0.1:8982/

用真实鼠标事件检查当前章节高亮、单张卡片的悬停光晕、搜索与键盘操作、手机快速展开与跳转，
并检查减少动态效果和减少透明度设置。
