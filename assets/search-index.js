window.QI_ARTICLES = [
  {
    "keywords": "3DGS GS 高斯泼溅 Gaussian Splatting 新视角 3D 重建 渲染 球谐 图形渲染 Splat-Studio 三维重建与数字孪生",
    "slug": "gaussian-splatting",
    "summary": "正面看是一条清晰的门框，换到斜侧方却可能出现双边。把同一处的配准点云和渲染图放在一起看，能分辨重影最早出现在哪一步。",
    "title": "门框边缘的高斯渲染",
    "topic": "3DGS"
  },
  {
    "keywords": "3DGS 训练数据 相机轨迹 曝光时刻 相机外参 COLMAP Nerfstudio 重投影 图形渲染 Fast-Livo2 激光、惯性与视觉融合",
    "slug": "livo-gs-keyframes",
    "summary": "从曝光时刻与相机坐标约定入手，在训练高斯模型之前，用门框角点检查照片与几何是否匹配。",
    "title": "一张训练照片的位姿从哪里来？",
    "topic": "3DGS 数据采集"
  },
  {
    "keywords": "Mesh 三角网格 拓扑 Open3D 孔洞 非流形 表面重建 几何验收 图形渲染 Splat-Studio 三维重建与数字孪生",
    "slug": "mesh-inspection",
    "summary": "彩色模型看着像一间完整的屋子。关掉纹理，沿墙面剖一刀，再看三角网格实际连接了什么。",
    "title": "门洞为何被一片三角形封住",
    "topic": "三角网格"
  },
  {
    "keywords": "3DGS 伪影 高斯 透明度 多视角 MVS 遮挡 深度 光栅化 图形渲染 Splat-Studio 三维重建与数字孪生 Coding_Skills 工程调试方法",
    "slug": "multiview-floaters",
    "summary": "在斜视图里跟踪一块伪影，用原图、深度和临时遮蔽区分输错的位姿、真实遮挡与高斯贡献。",
    "title": "寻找悬浮高斯的来源",
    "topic": "多视角诊断"
  },
  {
    "keywords": "SfM COLMAP 位姿 相机中心 四元数 重投影 图像匹配 轨迹 定位与建图 Splat-Studio 三维重建与数字孪生",
    "slug": "colmap-camera-centers",
    "summary": "轨迹里有了旋转和平移，画出来却穿过墙面。把世界原点和相机中心各代入一次，就能检验位姿方向。",
    "title": "COLMAP 位姿里的负号",
    "topic": "SfM"
  },
  {
    "keywords": "A* 体素规划 地形 支撑 坡度 足迹 代价 启发函数 定位与建图 Fast-Livo2 激光、惯性与视觉融合 VSLAM 视觉定位与机器人建图",
    "slug": "grounded-astar",
    "summary": "同一张三维体素图，对飞行器和地面机器人有不同的邻接关系。规划前要写清什么样的一步可走。",
    "title": "给 A* 的每一步找一块地面",
    "topic": "地形规划"
  },
  {
    "keywords": "ICP 点云配准 Open3D 点到面 残差 最近邻 fitness RMSE 定位与建图 VSLAM 视觉定位与机器人建图 Coding_Skills 工程调试方法",
    "slug": "icp",
    "summary": "两片点云沿平墙错开十厘米，点到面残差仍然可以为零。判断配准结果，还得看重叠区域里有没有能约束这次移动的边角。",
    "title": "ICP 残差为零，点云仍会错位",
    "topic": "ICP"
  },
  {
    "keywords": "LiDAR 地点识别 Scan Context ICP 全局定位 几何歧义 map odom 定位与建图 Fast-Livo2 激光、惯性与视觉融合",
    "slug": "lidar-place-ambiguity",
    "summary": "从地点描述子到几何配准，区分一个局部姿态能解释扫描和一张地图里只有一个可能地点。",
    "title": "两间相同的房间，雷达该选哪一个？",
    "topic": "激光重定位"
  },
  {
    "keywords": "LiDAR IMU LIO 去畸变 扫描时间 状态传播 协方差 FAST-LIVO2 定位与建图 Fast-Livo2 激光、惯性与视觉融合",
    "slug": "livo-scan-time",
    "summary": "用一束激光的采样时刻解释点云去畸变，再看 IMU 如何为一段扫描补全运动轨迹。",
    "title": "一帧点云，到底是哪一刻的？",
    "topic": "雷达惯性里程计"
  },
  {
    "keywords": "多会话建图 点云拼接 位姿图 全局配准 因子图 鲁棒估计 定位与建图 Fast-Livo2 激光、惯性与视觉融合",
    "slug": "multi-session-graph",
    "summary": "检查跨会话的相对姿态、图的坐标参照，以及重复走廊里可能自洽的错误对应。",
    "title": "两趟扫描怎样拼进同一张地图？",
    "topic": "多会话建图"
  },
  {
    "keywords": "OctoMap 八叉树 体素 占据 自由空间 未知 语义 标注 定位与建图 VSLAM 视觉定位与机器人建图",
    "slug": "octomap-free-unknown",
    "summary": "沿一次深度观测追踪射线，分清已经测到的自由空间、障碍表面和从未看见的区域。",
    "title": "点云的空白处，真能走过去吗？",
    "topic": "OctoMap"
  },
  {
    "keywords": "直接法 VIO 光度残差 相对增益 图块 梯度 曝光 里程计 定位与建图 Fast-Livo2 激光、惯性与视觉融合",
    "slug": "photometric-patches",
    "summary": "用两个像素值拆开图像运动与曝光变化，再用同一张图做一组可以复核的直接法实验。",
    "title": "图像亮了，里程计为什么会以为自己动了？",
    "topic": "直接法视觉里程计"
  },
  {
    "keywords": "局部规划 滚动体素 动态障碍 射线 时间衰减 TTL 全局路径 定位与建图 VSLAM 视觉定位与机器人建图",
    "slug": "rolling-obstacle-map",
    "summary": "保存的地图描述长期布局，滚动占据层记录最近看到的东西。用两束深度射线检查一把椅子的出现与消失。",
    "title": "搬走的椅子，什么时候从地图消失",
    "topic": "动态障碍"
  },
  {
    "keywords": "RTAB-Map 图优化 位姿图 回环 里程计 图约束 闭环 定位与建图 VSLAM 视觉定位与机器人建图",
    "slug": "rtabmap-pose-graph",
    "summary": "把相邻帧的里程计和跨时间的回环连进同一张位姿图，沿着一次走廊往返检查优化前后的变化。",
    "title": "走廊尽头的那条回环",
    "topic": "RTAB-Map"
  },
  {
    "keywords": "SLAM 视觉定位 关键帧 位姿 跟踪 回环 Tcw ORB-SLAM3 定位与建图 VSLAM 视觉定位与机器人建图",
    "slug": "slam",
    "summary": "相机沿世界 x 轴前进一米，世界到相机的变换里，平移量却是负一米。画轨迹之前，先弄清位姿文件保存的是哪个方向。",
    "title": "SLAM 轨迹里的相机中心",
    "topic": "SLAM"
  },
  {
    "keywords": "补拍 增量重建 固定位姿 COLMAP 稀疏点云 高斯训练 新旧视图 增量训练 基线 定位与建图 Splat-Studio 三维重建与数字孪生",
    "slug": "splat-incremental",
    "summary": "新照片能成功注册，只回答了位姿问题。训练后还得逐张比较旧视角，防止入口处改善的分数盖住走廊另一端的退化。",
    "title": "补拍一段走廊，旧视角还对吗",
    "topic": "增量重建"
  },
  {
    "keywords": "数字孪生 导航 地面 坐标系 尺度 对齐 Nav2 footprint 通行 定位与建图 Splat-Studio 三维重建与数字孪生",
    "slug": "twin-floor-frame",
    "summary": "从一段已知长度开始，把模型单位、地面法向、机器人朝向和窄门净宽统一到同一张地图上。",
    "title": "把机器人放进重建的房间",
    "topic": "数字孪生"
  },
  {
    "keywords": "HLoc NetVLAD SuperPoint LightGlue PnP RANSAC 检索 重定位 定位与建图 VSLAM 视觉定位与机器人建图",
    "slug": "visual-relocalization",
    "summary": "外观相似只能选出候选地点。把局部匹配接上地图中的三维点，才能估计相机的位置与朝向。",
    "title": "给一张照片找回地图坐标",
    "topic": "视觉重定位"
  },
  {
    "keywords": "概率体素地图 LiDAR 法向 残差协方差 点到面 退化 定位与建图 Fast-Livo2 激光、惯性与视觉融合",
    "slug": "voxel-plane-confidence",
    "summary": "从四个点拟合一块平面，理解点到面残差和地图不确定性如何共同约束位姿。",
    "title": "一块墙面能给定位多大权重？",
    "topic": "概率体素地图"
  },
  {
    "keywords": "配置 回归测试 C++ CTest 网络配置 备份 恢复 幂等 软件工程 工程实践 ssh-vpn 网络工具与服务管理 Coding_Skills 工程调试方法",
    "slug": "config-regression",
    "summary": "从一份包含旧设置的小样本出发，测试改动范围、重复运行和恢复后的内容。",
    "title": "为一次网络配置变更写回归测试",
    "topic": "配置测试"
  },
  {
    "keywords": "自动定容 工作流 状态机 注液 dosing 完成事件 视觉测量 上位机 工程实践 Bottle-Automatic-Volume-Adjustment-System 自动定容系统",
    "slug": "dosing-states",
    "summary": "开始、锁定刻度、检测液面、报告完成，对应不同的状态。把状态表和产生事件的那一行代码一起看，才能说清设备为什么停在这里。",
    "title": "定容结束的那一帧",
    "topic": "状态机"
  },
  {
    "keywords": "导航 目标检查 速度 容差 位姿 时间戳 Nav2 map odom 工程实践 Fast-Livo2 激光、惯性与视觉融合",
    "slug": "navigation-settling",
    "summary": "分别观察终点误差、机体运动和数据采集时间，理解导航中的到达条件。",
    "title": "机器人进了目标圈，什么时候算到达？",
    "topic": "导航状态确认"
  },
  {
    "keywords": "C++ ROS 2 ament CMake 共享库 ABI 二进制 发布 依赖 overlay 工程实践 VSLAM 视觉定位与机器人建图",
    "slug": "ros-binary-package",
    "summary": "从一个只依赖公开头文件的小程序出发，分别检验 CMake 包的导出信息、共享库和运行环境。",
    "title": "一个 ROS 2 包，离开源码树以后",
    "topic": "二进制交付"
  },
  {
    "keywords": "定容 录像 JPEG ffmpeg MP4 后台线程 异步 队列 retention 单元测试 观测 工程实践 Bottle-Automatic-Volume-Adjustment-System 自动定容系统",
    "slug": "session-recording",
    "summary": "一次检测结束后，JPEG 可能还在等待转码。录像器要保护正在收尾的会话，磁盘清理才不会抢先删掉唯一一份原始帧。",
    "title": "录像收尾时，谁还在写文件",
    "topic": "会话录像"
  },
  {
    "keywords": "异步任务 mesh 网格导出 进度 恢复 进程 PID 实验版本 作业管理 工程实践 Splat-Studio 三维重建与数字孪生",
    "slug": "splat-mesh-job",
    "summary": "后台日志里的百分比只能描述计算走到了哪一段。结果文件和版本登记齐全，网页才有可以下载的网格。",
    "title": "网格导出为什么停在 99%",
    "topic": "后台任务"
  },
  {
    "keywords": "模型清单 版本管理 静态打包 相对路径 检查点 C++ 文件系统 SHA-256 3DGS 数字孪生 工程实践 Splat-Studio 三维重建与数字孪生",
    "slug": "splat-model-manifest",
    "summary": "一个模型版本至少牵连相机、检查点、参数和坐标变换。把这些关系写在同一份清单里，换台机器也能核对渲染时究竟读了哪套数据。",
    "title": "重建项目搬走以后，模型在哪里",
    "topic": "模型清单"
  },
  {
    "keywords": "GitHub Pages 静态站 C++17 JSON 搜索索引 个人博客 相对路径 根目录迁移 重定向 旧书签 工程实践 qitiandi2003.github.io 博客与静态发布",
    "slug": "static-pages",
    "summary": "把一篇手记从子目录移到根目录，既要找对样式和图片，也要让收藏夹里的旧文章、分类和段落仍能打开。",
    "title": "把静态博客迁到域名根目录",
    "topic": "静态发布"
  },
  {
    "keywords": "Coding_Skills 工程验收 前端 状态管理 刷新 多端 竞态 版本 请求 日志 控制台 工程实践 Fast-Livo2 激光、惯性与视觉融合 Coding_Skills 工程调试方法",
    "slug": "web-state",
    "summary": "手机刷新后应该还看到刚才的地图，但一次按键或导航命令有自己的生命周期。恢复界面时，先分清哪些状态需要读回来。",
    "title": "刷新控制台，别重放一次操作",
    "topic": "网页状态"
  },
  {
    "keywords": "AprilTag 标签 PnP 平面标靶 相机位姿 距离 检测边长 重投影 感知与标定 VSLAM 视觉定位与机器人建图",
    "slug": "apriltag-pose-check",
    "summary": "AprilTag 提供编码身份和角点。用一张打印纸设计测量实验，检查边长、成像尺度和相机位姿。",
    "title": "打印的 AprilTag 能量出多远",
    "topic": "AprilTag"
  },
  {
    "keywords": "标定 相机 IMU Kalibr 外参 时间同步 坐标系 calibration 感知与标定 VSLAM 视觉定位与机器人建图",
    "slug": "calibration",
    "summary": "标定文件里相差 8 毫秒，应该加在哪一端？先拿 IMU 原点和一帧图像时间验算，再用转弯时的角速度峰核对。",
    "title": "Kalibr 外参和时差的方向",
    "topic": "联合标定"
  },
  {
    "keywords": "工业相机 采集 C++ OpenCV cv::Mat Bayer 行跨度 buffer 生命周期 内存 感知与标定 Fast-Livo2 激光、惯性与视觉融合 VSLAM 视觉定位与机器人建图 Bottle-Automatic-Volume-Adjustment-System 自动定容系统",
    "slug": "camera-frame-buffer",
    "summary": "相机预览偶尔花屏，可以从一帧的像素格式、行跨度，以及 SDK 回收缓冲区的时刻查起。",
    "title": "收到一帧图像以后，谁拥有这块内存？",
    "topic": "工业相机"
  },
  {
    "keywords": "CREStereo 左右一致性 双目匹配 视差掩码 置信度 亚像素 深度 感知与标定 VSLAM 视觉定位与机器人建图",
    "slug": "crestereo-confidence",
    "summary": "一根桌腿的边缘为什么有时会飘到背景？在学习式双目匹配中追踪一个像素的往返位置。",
    "title": "让左右视差互相对一次账",
    "topic": "CREStereo"
  },
  {
    "keywords": "事件相机 Prophesee 事件流 极性 时间戳 时间窗 RGBD ROS2 感知与标定 VSLAM 视觉定位与机器人建图",
    "slug": "event-camera-windows",
    "summary": "事件相机记录单个像素的亮度变化。选取时间窗，把异步事件投成图像，能直观看到延迟与轮廓拖尾的关系。",
    "title": "事件相机的时间窗",
    "topic": "事件相机"
  },
  {
    "keywords": "Fast-Livo2 FAST-Calib FAST-Calib2 LiDAR 雷达 相机 外参 反光圆环 ArUco 圆心 RMSE 重投影 时间同步 感知与标定 Fast-Livo2 激光、惯性与视觉融合",
    "slug": "lidar-camera-calibration",
    "summary": "标定板上的圆心都对齐了，换一面墙，还要检查激光边缘能不能落回图像。毫米和像素各回答一个问题。",
    "title": "圆心残差与门框上的投影",
    "topic": "雷达与相机标定"
  },
  {
    "keywords": "OpenCV 计算机视觉 Sobel 边缘检测 液面 梯度 绿色通道 BGR 感知与标定 Bottle-Automatic-Volume-Adjustment-System 自动定容系统",
    "slug": "opencv",
    "summary": "瓶身、刻度和反光都能形成强边缘。定容程序用锁定刻度时的图像作基线，在各列的梯度变化里寻找液面。",
    "title": "从梯度峰定位液面",
    "topic": "OpenCV"
  },
  {
    "keywords": "自动定容 瓶身 刻度线 锁定 Sobel HoughLinesP GapStability OpenCV 稳定窗口 感知与标定 Bottle-Automatic-Volume-Adjustment-System 自动定容系统",
    "slug": "scale-lock",
    "summary": "一条反光也可能在单帧里得到高分。定容程序把线的位置偏差放进时间窗口，等偏差的抖动足够小，才交给下一阶段。",
    "title": "刻度线何时算锁定",
    "topic": "刻度检测"
  },
  {
    "keywords": "SGM SGBM 视差 双目 深度 RealSense 极线校正 基线 定点 感知与标定 VSLAM 视觉定位与机器人建图",
    "slug": "stereo-disparity-depth",
    "summary": "在极线校正后的图像上，从像素位移推回深度，看看远处窗框为何比近处桌边更容易抖动。",
    "title": "一像素视差的深度误差",
    "topic": "SGM 双目深度"
  },
  {
    "keywords": "双目 工业相机 曝光 同步 时间戳 硬件触发 增益 运动模糊 感知与标定 Fast-Livo2 激光、惯性与视觉融合 VSLAM 视觉定位与机器人建图",
    "slug": "stereo-trigger-exposure",
    "summary": "左右图像在电脑上先后到达。用触发事件、设备时间戳和曝光时长判断，它们看见的是不是同一瞬间。",
    "title": "两台相机按下快门的那一刻",
    "topic": "双目采集"
  },
  {
    "keywords": "朴素贝叶斯 垃圾邮件 MultinomialNB TF-IDF SMOTE 数据泄漏 分类 机器学习 dev_skills 自然语言处理实验",
    "slug": "bayes-mail",
    "summary": "看清词频、类别先验和 TF-IDF 的作用，再用一次训练集与测试集的顺序检查评估是否可信。",
    "title": "朴素贝叶斯怎样给邮件打分",
    "topic": "朴素贝叶斯"
  },
  {
    "keywords": "BERT 中文 情感分析 Hugging Face 微调 分类 mask token Transformers 机器学习 dev_skills 自然语言处理实验",
    "slug": "bert-sentiment",
    "summary": "沿着 CSV、分词器和分类头检查中文情感微调，找出一条长评论在什么时候失去了末尾的转折。",
    "title": "BERT 会读到评论末尾的转折吗？",
    "topic": "BERT 分类"
  },
  {
    "keywords": "GPT 自回归 因果语言模型 生成解码 top-k top-p temperature 重复惩罚 机器学习 dev_skills 自然语言处理实验",
    "slug": "gpt-sampling",
    "summary": "从下一 token 的分布出发，读懂候选集、温度和长度限制，并用 token 边界准确取出续写。",
    "title": "同一句开头，GPT 为什么给出三种续写",
    "topic": "GPT 采样"
  },
  {
    "keywords": "机器翻译 Seq2Seq LSTM teacher forcing 德英 Multi30K 注意力 对齐 机器学习 dev_skills 自然语言处理实验",
    "slug": "seq2seq-translation",
    "summary": "跟随第一步译词的输入与标签，理解教师强制、定长状态，以及注意力需要补上的信息通路。",
    "title": "德英翻译器的第一个译词",
    "topic": "Seq2Seq"
  },
  {
    "keywords": "RNN GRU Elman 姓氏 条件生成 字符模型 填充 字符串 分类 机器学习 dev_skills 自然语言处理实验",
    "slug": "surname-rnn",
    "summary": "同一批字符可以用来预测数据集标签，也可以用来生成下一个字符。区别体现在序列如何错位和隐藏状态从何开始。",
    "title": "同一份姓氏，两种 RNN 任务",
    "topic": "字符 RNN"
  },
  {
    "keywords": "Word2Vec Gensim CBOW Skip-gram embedding 相似度 分布式表示 机器学习 dev_skills 自然语言处理实验",
    "slug": "word-embeddings",
    "summary": "对照餐饮评论里的 Skip-gram 和小说语料中的 CBOW，沿着训练目标解释词向量近邻。",
    "title": "词向量的邻居从哪里来",
    "topic": "词向量"
  },
  {
    "keywords": "NLP 中文分词 FMM RMM 双向最大匹配 jieba TextRank 自定义词典 机器学习 dev_skills 自然语言处理实验",
    "slug": "word-segmentation",
    "summary": "用一个小词典比较正向、逆向最大匹配，再观察自定义词典如何改变后续检索与关键词提取。",
    "title": "「研究生命起源」可以从哪里切开？",
    "topic": "中文分词"
  },
  {
    "keywords": "IPv4 CIDR 网络前缀 最长前缀匹配 路由表 默认路由 iproute2 dev src metric 系统与通信 ssh-vpn 网络工具与服务管理",
    "slug": "ipv4-route-selection",
    "summary": "用三条纸面路由判断出口，再对照 C++ 代码区分路由表、内核选路结果与端到端连通性。",
    "title": "默认路由怎样让位给更具体的地址",
    "topic": "IPv4 路由"
  },
  {
    "keywords": "TCP IP 计算机网络 字节序 半包 粘包 协议 socket 系统与通信 Bottle-Automatic-Volume-Adjustment-System 自动定容系统",
    "slug": "network",
    "summary": "上位机发出两条命令，接收端可能先读到 7 字节，再读到 23 字节。自动定容程序用一个缓冲区还原两条完整的 15 字节指令。",
    "title": "TCP 里的 15 字节控制帧",
    "topic": "计算机网络"
  },
  {
    "keywords": "操作系统 Linux systemd oneshot RemainAfterExit waitpid EINTR 生命周期 进程 状态 信号 系统与通信 ssh-vpn 网络工具与服务管理",
    "slug": "oneshot-service",
    "summary": "从一个 systemd oneshot 单元追到 C++ 的 waitpid，分别读懂单元状态、子进程退出状态和任务效果。",
    "title": "进程结束之后的 active 状态",
    "topic": "Linux 进程"
  },
  {
    "keywords": "RGB-D ROS 2 图像 深度 内参 时间戳 同步 里程计 TF 系统与通信 VSLAM 视觉定位与机器人建图",
    "slug": "rgbd-sensor-contract",
    "summary": "更换 RGB-D 传感器时，用同一张检查表核对彩色图、深度、内参和时钟，再画出里程计到地图的变换关系。",
    "title": "换一台相机，地图后端接收什么",
    "topic": "RGB-D 接口"
  },
  {
    "keywords": "ROS2 ROS 2 rosbag2 QoS 机器人 录包 回放 TF 系统与通信 Fast-Livo2 激光、惯性与视觉融合 VSLAM 视觉定位与机器人建图 Coding_Skills 工程调试方法",
    "slug": "ros2",
    "summary": "一分钟录到约 600 帧激光，并不能排除转弯时断过一小段。把扫描、坐标变换和定位输出对到同一个时间窗口，才看得出谁先出了问题。",
    "title": "转弯处的 ROS 2 录包",
    "topic": "ROS 2"
  }
];
