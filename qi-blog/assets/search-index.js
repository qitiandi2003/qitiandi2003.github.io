window.QI_ARTICLES = [
  {
    "keywords": "SLAM 视觉定位 关键帧 位姿 跟踪 回环 Tcw ORB-SLAM3 定位与建图",
    "slug": "slam",
    "summary": "沿着 ORB-SLAM3 的一帧图像，分清跟踪、建图与回环，再用一个三维点检验轨迹方向。",
    "title": "读懂 SLAM 轨迹里的相机位姿",
    "topic": "SLAM"
  },
  {
    "keywords": "ICP 点云配准 Open3D 点到面 残差 最近邻 fitness RMSE 定位与建图",
    "slug": "icp",
    "summary": "从一面平墙推导点到面残差，理解配准初值、几何退化和内点覆盖率之间的关系。",
    "title": "ICP 的误差很小，点云就对齐了吗？",
    "topic": "ICP"
  },
  {
    "keywords": "标定 相机 IMU Kalibr 外参 时间同步 坐标系 calibration 感知与标定",
    "slug": "calibration",
    "summary": "从 Kalibr 的两项输出入手，用一个空间点和一个时间戳核对外参与时差的方向。",
    "title": "相机和 IMU 怎样对上坐标与时间",
    "topic": "联合标定"
  },
  {
    "keywords": "OpenCV 计算机视觉 Sobel 边缘检测 液面 梯度 绿色通道 BGR 感知与标定",
    "slug": "opencv",
    "summary": "从自动定容项目的检测代码出发，拆开绿色通道、Sobel 梯度和列投影，观察一帧图像如何变成一个液面坐标。",
    "title": "用一条梯度曲线找到瓶中液面",
    "topic": "OpenCV"
  },
  {
    "keywords": "3DGS GS 高斯泼溅 Gaussian Splatting 新视角 3D 重建 渲染 球谐 图形渲染",
    "slug": "gaussian-splatting",
    "summary": "沿着一扇门框，追踪相机位姿、稀疏点云和屏幕上的像素，读懂 3D Gaussian Splatting 如何生成新视角。",
    "title": "从点云到高斯渲染",
    "topic": "3DGS"
  },
  {
    "keywords": "ROS2 ROS 2 rosbag2 QoS 机器人 录包 回放 TF 系统与通信",
    "slug": "ros2",
    "summary": "定位轨迹突然跳变时，沿着消息时间、坐标变换和订阅关系回放，查出异常最早出现在哪一层。",
    "title": "用 ROS 2 录包追踪第一次异常",
    "topic": "ROS 2"
  },
  {
    "keywords": "TCP IP 计算机网络 字节序 半包 粘包 协议 socket 系统与通信",
    "slug": "network",
    "summary": "沿着自动定容系统的控制通道，理解字节流、半帧、连续帧，以及解析器应该在什么时刻交付一条命令。",
    "title": "一条 15 字节指令如何穿过 TCP",
    "topic": "计算机网络"
  }
];
