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
