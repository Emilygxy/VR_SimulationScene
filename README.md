# VR_SimulationScene
## VR 模拟场景要求：
### 1. 设计并实现一个VR场景，使用OpenGL支持3DoF交互（头部旋转：偏航yaw、俯仰pitch）
实现基本的相机功能，并允许鼠标键盘实时交互 
### 2.在场景中渲染一个双面环形屏幕（模拟现实中的环绕显示器），屏幕实时显示动态纹理，模拟远程电脑的桌面画面。 
使用MRT 渲染显示器面板，渲染动态纹理 
### 3.场景需支持复杂交互（视角控制+位置移动）并满足高性能要求 
在1的基础上，增加交互功能，如：
- 视角控制：鼠标滚轮控制视角，手柄控制视角
- 位置移动：手柄控制位置移动
### 4. 最终输出的是模拟视差摄像头输出的3d场景画面，并使用着色器做好vr畸变
在2的基础上，增加vr畸变效果，如：
- 透视畸变：模拟现实中的透视畸变效果

## 项目文件结构
```
Project
├── CMakeLists.txt
├── 3rdParty
│   ├── glm
|── include
│   ├── utils
|        ├── CustomCamera.h
|        ├── Helper.h
│   ├── Shader.h
|── OpenGL
│   ├── include
│   ├── lib
│   ├── src
|        ├── glad.c
├── src
│   ├── utils
|        ├── CustomCamera.cpp
|        ├── Helper.cpp
│   ├── main.cpp
```