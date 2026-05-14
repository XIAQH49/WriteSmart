# WriteSmart 2.0

> 智能文学创作客户端 — 以 AI 为翼，让创作更专注

## 简介

WriteSmart 2.0 是一款面向文学创作者的桌面写作工具。采用 **C++ / Qt 6** 开发，以极低的内存占用和流畅的性能，提供三段式写作工作区体验：

| 左侧边栏 | 中间编辑区 | 右侧 AI 对话 |
|:---:|:---:|:---:|
| 章节目录、人物卡片、设定集 | Markdown 写作编辑 | 大模型实时辅助 |

## 特性

- **三段式工作区**：结构管理 → 专注写作 → AI 辅助，一气呵成
- **自定义 API**：支持 OpenAI / Claude / 自定义兼容端点
- **流式对话**：AI 回复打字机效果，实时呈现
- **大文档优化**：分块加载，10万字工程流畅编辑
- **插件扩展**：AI Provider 插件化，第三方可扩展
- **轻量高效**：Qt Widgets 原生渲染，启动快、内存省

## 截图

<!-- TODO: 添加截图 -->

## 技术栈

| 层次 | 技术 |
|------|------|
| 语言 | C++20 |
| UI 框架 | Qt 6.9 (Widgets) |
| 网络 | QNetworkAccessManager |
| JSON | QJsonDocument |
| 构建 | CMake 3.20+ |
| 编译器 | MinGW GCC 14+ / MSVC 2022+ |

## 快速开始

### 环境要求

- Windows 10/11（macOS / Linux 可自行适配）
- Qt 6.5+ (Widgets + Network 模块)
- CMake 3.20+
- MinGW GCC 14+ 或 MSVC 2022+

### 构建

```bash
# 克隆仓库
git clone https://github.com/XIAQH49/WriteSmart.git
cd WriteSmart2.0

# 配置 (MinGW)
mkdir build && cd build
cmake -G "MinGW Makefiles" \
      -DCMAKE_PREFIX_PATH=D:/Qt/6.9.1/mingw_64 \
      -DCMAKE_BUILD_TYPE=Release \
      ..

# 编译
cmake --build . --config Release

# 运行
./WriteSmart.exe
```

## 目录结构

```
WriteSmart2.0/
├── CMakeLists.txt              # 根构建文件
├── README.md                   # 项目说明
├── docs/
│   └── ARCHITECTURE.md         # 架构设计文档
├── src/
│   ├── main.cpp                # 程序入口
│   ├── app/                    # 应用层（生命周期、配置）
│   ├── ui/                     # UI 层
│   │   ├── MainWindow          # 主窗口（三段式布局）
│   │   ├── panels/             # 三大面板
│   │   │   ├── SidebarPanel    # 左侧边栏
│   │   │   ├── EditorPanel     # 中间编辑区
│   │   │   └── ChatPanel       # 右侧 AI 对话
│   │   ├── widgets/            # 可复用组件
│   │   └── themes/             # 主题管理
│   ├── core/                   # 核心逻辑
│   │   ├── document/           # 文档模型
│   │   ├── ai/                 # AI 子系统
│   │   │   └── providers/      # AI Provider 实现
│   │   └── plugin/             # 插件系统
│   ├── network/                # 网络层（HTTP/SSE）
│   └── utils/                  # 工具函数
├── resources/
│   ├── icons/                  # 图标资源
│   ├── styles/                 # QSS 样式表
│   └── templates/              # Prompt 模板
└── tests/                      # 单元测试
```

## 开发指南

详见 [架构设计文档](docs/ARCHITECTURE.md)。

### 核心接口

所有可扩展点都通过抽象接口定义：

- **`AIProvider`** — AI 服务提供者接口，实现此接口即可接入新的大模型
- **`IPlugin`** — 插件基类接口，支持动态加载
- **`Document`** — 文档数据模型，与 UI 解耦

## License

MIT License

## 作者

XIAQH49
