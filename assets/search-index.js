window.QI_ARTICLES = [
  {
    "keywords": "SLAM 视觉定位 关键帧 位姿 跟踪 回环 Tcw ORB-SLAM3 定位与建图",
    "slug": "slam",
    "summary": "从左右移动时物体的视差出发，理解相机怎样估计自身运动、恢复路标位置，并在绕回起点时校正轨迹。",
    "title": "视觉 SLAM 的定位与建图原理",
    "topic": "SLAM"
  },
  {
    "keywords": "ICP 点云配准 Open3D 点到面 残差 最近邻 fitness RMSE 定位与建图",
    "slug": "icp",
    "summary": "用两组桌角坐标手算一次配准，再看复杂场景中的旋转、初始位姿、点到面残差和几何退化。",
    "title": "ICP 怎样把两次扫描对齐",
    "topic": "ICP"
  },
  {
    "keywords": "SfM COLMAP 位姿 相机中心 四元数 重投影 图像匹配 轨迹 定位与建图",
    "slug": "colmap-camera-centers",
    "summary": "从两个坐标系出发推导相机中心 C = −Rᵀt，手算一次旋转，并把三维点投回像素检验方向。",
    "title": "从 COLMAP 相机位姿计算运动轨迹",
    "topic": "SfM"
  },
  {
    "keywords": "LiDAR IMU LIO 去畸变 扫描时间 状态传播 协方差 定位与建图",
    "slug": "livo-scan-time",
    "summary": "用一堵墙和一次匀速扫描解释点的采样时刻、IMU 运动估计，以及如何把一帧点云归到同一个参考时刻。",
    "title": "机器人一转弯，点云里的墙为什么弯了？",
    "topic": "雷达惯性里程计"
  },
  {
    "keywords": "概率体素地图 LiDAR 法向 残差协方差 点到面 退化 定位与建图",
    "slug": "voxel-plane-confidence",
    "summary": "从盒子里的点怎样确定一张平面讲起，用有误差的尺读数演算标准化残差，理解点到面约束的权重。",
    "title": "一小块点云能把墙面量得多准？",
    "topic": "概率体素地图"
  },
  {
    "keywords": "直接法 VIO 光度残差 相对增益 图块 梯度 曝光 里程计 定位与建图",
    "slug": "photometric-patches",
    "summary": "把一块小纹理在两帧间的亮度差写成误差，再用三个灰度读数看清纹理方向与曝光变化带来的歧义。",
    "title": "从一小块纹理估计相机运动",
    "topic": "直接法视觉里程计"
  },
  {
    "keywords": "HLoc NetVLAD SuperPoint LightGlue PnP RANSAC 检索 重定位 定位与建图",
    "slug": "visual-relocalization",
    "summary": "把图像检索、局部特征匹配与二维到三维的位姿估计串成一次定位，用重投影和独立深度复核候选地点。",
    "title": "用视觉重定位估计照片的拍摄位姿",
    "topic": "视觉重定位"
  },
  {
    "keywords": "LiDAR 地点识别 Scan Context ICP 全局定位 几何歧义 map odom 定位与建图",
    "slug": "lidar-place-ambiguity",
    "summary": "从一圈距离测量构造地点描述，区分候选检索、几何对齐与连续观察，理解什么时候只能保留多个可能位置。",
    "title": "走廊长得一样，激光雷达怎么认路？",
    "topic": "激光重定位"
  },
  {
    "keywords": "RTAB-Map 图优化 位姿图 回环 里程计 图约束 闭环 定位与建图",
    "slug": "rtabmap-pose-graph",
    "summary": "把走廊往返画成由关键帧和相对运动组成的图，手算两段里程的折中解，再辨别真假回环。",
    "title": "绕回起点后，RTAB-Map 怎样修正地图？",
    "topic": "RTAB-Map"
  },
  {
    "keywords": "多会话建图 点云拼接 位姿图 全局配准 因子图 鲁棒估计 定位与建图",
    "slug": "multi-session-graph",
    "summary": "用两次独立的楼层扫描建立跨图约束，计算一个二维变换，并通过未参与配准的结构检查拼接结果。",
    "title": "把两次扫描拼成一张地图",
    "topic": "多会话建图"
  },
  {
    "keywords": "补拍 增量重建 固定位姿 COLMAP 稀疏点云 高斯训练 新旧视图 增量训练 基线 定位与建图",
    "slug": "splat-incremental",
    "summary": "区分新照片定位、三角化和联合优化，从两段有重叠的拍摄理解怎样扩展模型并复查旧视角。",
    "title": "给已有的三维重建补几张照片",
    "topic": "增量重建"
  },
  {
    "keywords": "OctoMap 八叉树 体素 占据 自由空间 未知 语义 标注 定位与建图",
    "slug": "octomap-free-unknown",
    "summary": "沿一条测距射线手画三类空间，理解为什么空白点云无法直接表示通道，以及多次观测如何更新体素。",
    "title": "OctoMap 中的自由、占据与未知空间",
    "topic": "OctoMap"
  },
  {
    "keywords": "局部规划 滚动体素 动态障碍 射线 时间衰减 TTL 全局路径 定位与建图",
    "slug": "rolling-obstacle-map",
    "summary": "把一把临时搬到过道里的椅子写入局部地图，分清命中、射线清除、过期和移出滚动窗口各自表示什么。",
    "title": "椅子搬走以后，障碍地图怎样更新？",
    "topic": "动态障碍"
  },
  {
    "keywords": "数字孪生 导航 地面 坐标系 尺度 对齐 Nav2 footprint 通行 定位与建图",
    "slug": "twin-floor-frame",
    "summary": "利用已知距离、地面平面和前进方向恢复模型的米制参考系，再用独立量测核对地图里的位置与通行宽度。",
    "title": "三维模型里的“一米”有多长？",
    "topic": "数字孪生"
  },
  {
    "keywords": "A* 体素规划 地形 支撑 坡度 足迹 代价 启发函数 定位与建图",
    "slug": "grounded-astar",
    "summary": "先把体素地图变成机器人能够移动的图，再手算一次 A* 排序，比较绕行距离与贴近障碍的代价。",
    "title": "地面机器人怎样用 A* 绕开障碍",
    "topic": "地形规划"
  },
  {
    "keywords": "工业相机 采集 C++ OpenCV cv::Mat Bayer 行跨度 buffer 生命周期 内存 感知与标定",
    "slug": "camera-frame-buffer",
    "summary": "理解像素格式、行跨度和缓冲区所有权，排查花屏、偏色与跨线程图像串帧。",
    "title": "相机画面为什么会串帧？",
    "topic": "工业相机"
  },
  {
    "keywords": "OpenCV 计算机视觉 Sobel 边缘检测 液面 梯度 绿色通道 BGR 感知与标定",
    "slug": "opencv",
    "summary": "把一块图像压成一条亮度变化曲线，从峰的位置找液面，再把裁剪坐标换回原图。",
    "title": "用图像梯度检测瓶中液面",
    "topic": "OpenCV"
  },
  {
    "keywords": "自动定容 瓶身 刻度线 锁定 Sobel HoughLinesP GapStability OpenCV 稳定窗口 感知与标定",
    "slug": "scale-lock",
    "summary": "用候选线的位置窗口区分“短时抖动小”和“位置已经对准”。",
    "title": "怎样判断图像中的刻度已经稳定",
    "topic": "刻度检测"
  },
  {
    "keywords": "SGM SGBM 视差 双目 深度 RealSense 极线校正 基线 定点 感知与标定",
    "slug": "stereo-disparity-depth",
    "summary": "左右图像中同一点的横向偏移可以换算成深度。距离越远，一像素误差的代价越大。",
    "title": "双目相机怎样用视差计算深度",
    "topic": "SGM 双目深度"
  },
  {
    "keywords": "双目 工业相机 曝光 同步 时间戳 硬件触发 增益 运动模糊 感知与标定",
    "slug": "stereo-trigger-exposure",
    "summary": "区分触发、曝光、设备时间戳和到达时间，估算时间差怎样污染立体匹配。",
    "title": "双目同步要对齐哪一个时刻",
    "topic": "双目采集"
  },
  {
    "keywords": "CREStereo 左右一致性 双目匹配 视差掩码 置信度 亚像素 深度 感知与标定",
    "slug": "crestereo-confidence",
    "summary": "从左图出发找到右图，再从右图走回来，检查同一个像素能否回到起点。",
    "title": "用左右一致性检查双目匹配",
    "topic": "CREStereo"
  },
  {
    "keywords": "AprilTag 标签 PnP 平面标靶 相机位姿 距离 检测边长 重投影 感知与标定",
    "slug": "apriltag-pose-check",
    "summary": "四个检测角点提供图像位置，标记的真实边长为距离提供尺度。",
    "title": "用一张 AprilTag 测出相机的位置和朝向",
    "topic": "AprilTag"
  },
  {
    "keywords": "标定 相机 IMU Kalibr 外参 时间同步 坐标系 calibration 感知与标定",
    "slug": "calibration",
    "summary": "从一段转动和一个三维点开始，弄清联合标定要估计的旋转、平移与时差。",
    "title": "相机和 IMU 怎样对齐坐标与时间",
    "topic": "联合标定"
  },
  {
    "keywords": "Fast-Livo2 FAST-Calib FAST-Calib2 LiDAR 雷达 相机 外参 反光圆环 ArUco 圆心 RMSE 重投影 时间同步 感知与标定",
    "slug": "lidar-camera-calibration",
    "summary": "从一组三维圆心估计刚体变换，再用自然场景中的边缘检查投影。",
    "title": "怎样验证激光雷达与相机的联合标定",
    "topic": "雷达与相机标定"
  },
  {
    "keywords": "事件相机 Prophesee 事件流 极性 时间戳 时间窗 RGBD ROS2 感知与标定",
    "slug": "event-camera-windows",
    "summary": "读懂一个事件的四个字段，再选择时间窗，把异步观测积成可看的轮廓。",
    "title": "事件相机如何把亮度变化变成图像",
    "topic": "事件相机"
  },
  {
    "keywords": "3DGS 训练数据 相机轨迹 曝光时刻 相机外参 COLMAP Nerfstudio 重投影 图形渲染",
    "slug": "livo-gs-keyframes",
    "summary": "用一把椅子练习拍摄，理解照片重叠、视差、曝光时刻与相机位姿怎样共同影响重建。",
    "title": "三维重建的拍摄与关键帧",
    "topic": "3DGS 数据采集"
  },
  {
    "keywords": "Mesh 三角网格 拓扑 Open3D 孔洞 非流形 表面重建 几何验收 图形渲染",
    "slug": "mesh-inspection",
    "summary": "从四个角点拼出一块平板，弄清顶点、三角面、法向以及真实开口和采样空洞的区别。",
    "title": "点云与三角网格",
    "topic": "三角网格"
  },
  {
    "keywords": "3DGS GS 高斯泼溅 Gaussian Splatting 新视角 3D 重建 渲染 球谐 图形渲染",
    "slug": "gaussian-splatting",
    "summary": "一组三维色斑如何从不同角度画出同一间屋子？从一个像素的颜色混合开始，理解 3D Gaussian Splatting。",
    "title": "3D 高斯渲染入门",
    "topic": "3DGS"
  },
  {
    "keywords": "3DGS 伪影 高斯 透明度 多视角 MVS 遮挡 深度 光栅化 图形渲染",
    "slug": "multiview-floaters",
    "summary": "换一个观察位置，利用视差、遮挡和深度判断一块悬浮颜色的来源。",
    "title": "三维重建中的漂浮伪影",
    "topic": "多视角诊断"
  },
  {
    "keywords": "NLP 中文分词 FMM RMM 双向最大匹配 jieba TextRank 自定义词典 机器学习",
    "slug": "word-segmentation",
    "summary": "用“研究生命起源”比较两种查词方向，再看分词边界怎样影响搜索。",
    "title": "中文分词怎样确定词的边界",
    "topic": "中文分词"
  },
  {
    "keywords": "朴素贝叶斯 垃圾邮件 MultinomialNB TF-IDF SMOTE 数据泄漏 分类 机器学习",
    "slug": "bayes-mail",
    "summary": "用三个词在两类邮件里的计数算出类别分数，再检查模型为什么会误判从没见过的邮件。",
    "title": "用朴素贝叶斯识别邮件类别",
    "topic": "朴素贝叶斯"
  },
  {
    "keywords": "Word2Vec Gensim CBOW Skip-gram embedding 相似度 分布式表示 机器学习",
    "slug": "word-embeddings",
    "summary": "用同一句话造出两类训练题，再算一次余弦相似度，理解词向量中的“邻居”。",
    "title": "词向量如何从上下文中学出来",
    "topic": "词向量"
  },
  {
    "keywords": "RNN GRU Elman 姓氏 条件生成 字符模型 填充 字符串 分类 机器学习",
    "slug": "surname-rnn",
    "summary": "同一串字符既可以提供一个类别标签，也可以拆成一串“下一个字符”的预测题。",
    "title": "用字符 RNN 做分类与生成",
    "topic": "字符 RNN"
  },
  {
    "keywords": "机器翻译 Seq2Seq LSTM teacher forcing 德英 Multi30K 注意力 对齐 机器学习",
    "slug": "seq2seq-translation",
    "summary": "从德语短句的第一个英语词开始，理解编码器、解码器、教师强制和注意力各在哪里起作用。",
    "title": "Seq2Seq 翻译器怎样逐词生成译文",
    "topic": "Seq2Seq"
  },
  {
    "keywords": "BERT 中文 情感分析 Hugging Face 微调 分类 mask token Transformers 机器学习",
    "slug": "bert-sentiment",
    "summary": "跟随一条有转折的酒店评论，从分词、截断到两个类别分数，检查模型真正看到了什么。",
    "title": "用 BERT 判断一条评论的情感",
    "topic": "BERT 分类"
  },
  {
    "keywords": "GPT 自回归 因果语言模型 生成解码 top-k top-p temperature 重复惩罚 机器学习",
    "slug": "gpt-sampling",
    "summary": "把三个候选词的分数算成概率，观察温度、top-k 和 top-p 怎样改变续写。",
    "title": "GPT 怎样从下一个 token 生成一句话",
    "topic": "GPT 采样"
  },
  {
    "keywords": "ROS2 ROS 2 Jazzy turtlesim 节点 包 消息 话题 服务 动作 通信 ROS 图 CLI rosbag2 QoS 录包 回放 系统与通信",
    "slug": "ros2",
    "summary": "在 Jazzy 的海龟模拟器里搭一张通信图。先用话题控制速度，再调用服务、观察动作反馈，最后录下命令并验证回放。",
    "title": "用模拟海龟学 ROS 2",
    "topic": "ROS 2"
  },
  {
    "keywords": "RGB-D ROS 2 图像 深度 内参 时间戳 同步 里程计 TF 系统与通信",
    "slug": "rgbd-sensor-contract",
    "summary": "解释 RGB-D 输入的颜色、深度、内参、时间戳和坐标系，并用一个已知像素计算相机空间的位置。",
    "title": "RGB-D 相机的数据与时间对齐",
    "topic": "RGB-D 接口"
  },
  {
    "keywords": "TCP IP 计算机网络 字节序 半包 粘包 协议 socket 系统与通信",
    "slug": "network",
    "summary": "从一条 15 字节的控制消息入手，计算接收缓冲区何时能交付一帧，并核对字段的字节序。",
    "title": "TCP 发了两条消息，一次能读到几条？",
    "topic": "计算机网络"
  },
  {
    "keywords": "IPv4 CIDR 网络前缀 最长前缀匹配 路由表 默认路由 iproute2 dev src metric 系统与通信",
    "slug": "ipv4-route-selection",
    "summary": "用三个重叠的 IPv4 前缀推算出口，再用 Linux 的只读查询区分选路结果和端到端连通性。",
    "title": "IPv4 路由表与最长前缀匹配",
    "topic": "IPv4 路由"
  },
  {
    "keywords": "操作系统 Linux systemd oneshot RemainAfterExit waitpid EINTR 生命周期 进程 状态 信号 系统与通信",
    "slug": "oneshot-service",
    "summary": "区分服务单元、操作系统进程和任务留下的效果，解释 active (exited) 与一次性启动的关系。",
    "title": "读懂 systemd 的 active (exited)",
    "topic": "Linux 进程"
  },
  {
    "keywords": "自动定容 工作流 状态机 注液 dosing 完成事件 视觉测量 上位机 工程实践",
    "slug": "dosing-states",
    "summary": "让等待、找瓶、锁定刻度和跟踪液面拥有明确的输入、转移条件和完成动作。",
    "title": "用有限状态机描述视觉定容",
    "topic": "状态机"
  },
  {
    "keywords": "导航 目标检查 速度 容差 位姿 时间戳 Nav2 map odom 工程实践",
    "slug": "navigation-settling",
    "summary": "用位置、朝向、速度和时间窗口区分经过目标、停在目标附近，以及定位修正造成的假到达。",
    "title": "机器人到站了吗？",
    "topic": "导航状态确认"
  },
  {
    "keywords": "网页状态 持久化 前端 状态管理 刷新 多端 并发 竞态 版本 条件更新 请求 控制台 工程实践",
    "slug": "web-state",
    "summary": "从刷新和双端编辑两个场景出发，决定数据存在哪里、如何识别过期响应，以及何时只恢复画面。",
    "title": "刷新网页以后，哪些状态还在？",
    "topic": "网页状态"
  },
  {
    "keywords": "定容 录像 JPEG ffmpeg MP4 后台线程 异步 队列 retention 单元测试 观测 工程实践",
    "slug": "session-recording",
    "summary": "用每秒 25 帧的输入计算写盘积压，再区分采集时刻、视频播放速度和会话真正完成的时刻。",
    "title": "相机每秒拍 25 帧，硬盘跟得上吗？",
    "topic": "会话录像"
  },
  {
    "keywords": "异步任务 mesh 网格导出 进度 恢复 进程 PID 实验版本 作业管理 工程实践",
    "slug": "splat-mesh-job",
    "summary": "把请求受理、进程退出和文件可下载分开，用网格导出说明后台任务的状态模型。",
    "title": "点击导出以后，什么时候能下载文件？",
    "topic": "后台任务"
  },
  {
    "keywords": "配置 回归测试 C++ CTest 网络配置 备份 恢复 幂等 软件工程 工程实践",
    "slug": "config-regression",
    "summary": "用临时目录重现旧配置，依次验证首次生效、重复执行、失败与撤回后的结果。",
    "title": "改完配置，怎样确认它真的生效了？",
    "topic": "配置测试"
  },
  {
    "keywords": "模型清单 版本管理 静态打包 相对路径 检查点 C++ 文件系统 SHA-256 3DGS 数字孪生 工程实践",
    "slug": "splat-model-manifest",
    "summary": "让模型、相机、参数和坐标约定成套发布，换个目录再试一次即可发现遗留的路径依赖。",
    "title": "模型清单、相对路径与版本发布",
    "topic": "模型清单"
  },
  {
    "keywords": "C++ ROS 2 ament CMake 共享库 ABI 二进制 发布 依赖 overlay 工程实践",
    "slug": "ros-binary-package",
    "summary": "沿着头文件、链接符号和动态加载三个阶段，检查一个 C++ 库能否真正交给另一个工作空间使用。",
    "title": "ROS 2 与 CMake 的二进制包交付",
    "topic": "二进制交付"
  },
  {
    "keywords": "HTML 静态网站 相对路径 URL 子路径 目录 查询参数 锚点 部署 浏览器 CSS 工程实践",
    "slug": "static-pages",
    "summary": "用同一个资源名对照首页和文章页，分清相对引用、站点根路径、查询参数和页内锚点。",
    "title": "静态网站的目录与浏览器路径",
    "topic": "静态发布"
  }
];
