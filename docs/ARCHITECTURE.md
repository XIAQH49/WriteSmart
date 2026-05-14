# WriteSmart 2.0 架构设计文档

## 一、项目概述

WriteSmart 是一款面向文学创作者的智能写作客户端。采用 C++ / Qt6 技术栈，以低内存占用和高性能为核心目标，提供三段式布局的写作工坊体验。

### 核心特性
- **三段式工作区**：左侧资源管理 → 中间写作编辑 → 右侧 AI 助手
- **可扩展 AI 引擎**：插件化 AI Provider，支持自定义 API
- **高性能编辑**：针对长文本优化的编辑引擎
- **低内存占用**：懒加载 + 对象池 + 增量渲染

---

## 二、分层架构

```
┌──────────────────────────────────────────────────────────────┐
│                      UI Layer (Qt Widgets)                    │
│  ┌──────────┐  ┌──────────────────┐  ┌───────────────────┐   │
│  │ Sidebar  │  │   EditorPanel    │  │    ChatPanel      │   │
│  │  Panel   │  │  (QPlainTextEdit │  │  (QScrollArea +   │   │
│  │          │  │   subclass)      │  │   ChatBubble)     │   │
│  └──────────┘  └──────────────────┘  └───────────────────┘   │
│                                                               │
│  MainWindow ── QSplitter (可拖拽调整三栏比例)                 │
├──────────────────────────────────────────────────────────────┤
│                    Application Layer                          │
│  ┌──────────┐  ┌──────────┐  ┌────────────────────────┐      │
│  │ AppCore  │  │  Config  │  │   SessionManager       │      │
│  │(生命周期)│  │(配置管理)│  │ (会话/项目管理)        │      │
│  └──────────┘  └──────────┘  └────────────────────────┘      │
├──────────────────────────────────────────────────────────────┤
│                       Core Layer                              │
│  ┌────────────────┐  ┌──────────────────────────────┐        │
│  │ Document Model │  │      AI Subsystem            │        │
│  │ ────────────── │  │  ┌────────────────────────┐  │        │
│  │ • Document     │  │  │ AIProvider (Interface) │  │        │
│  │ • Chapter      │  │  ├────────────────────────┤  │        │
│  │ • Paragraph    │  │  │ OpenAIProvider         │  │        │
│  │ • Character    │  │  │ ClaudeProvider         │  │        │
│  │ • Outline      │  │  │ CustomProvider ←── 插件│  │        │
│  └────────────────┘  │  └────────────────────────┘  │        │
│                       │  ┌────────────────────────┐  │        │
│                       │  │ AISession (会话管理)   │  │        │
│                       │  │ PromptTemplate (模板)  │  │        │
│                       │  └────────────────────────┘  │        │
│                       └──────────────────────────────┘        │
│                                                               │
│  ┌──────────────────────────────────────────────────┐        │
│  │              Plugin System                       │        │
│  │  PluginInterface → 动态加载 .dll/.so             │        │
│  │  • AI Provider 插件                              │        │
│  │  • 导出格式插件 (未来)                            │        │
│  └──────────────────────────────────────────────────┘        │
├──────────────────────────────────────────────────────────────┤
│                  Infrastructure Layer                         │
│  ┌──────────┐  ┌──────────┐  ┌──────────┐  ┌──────────┐     │
│  │ HTTP     │  │  JSON    │  │  Logger  │  │  Storage │     │
│  │ Client   │  │  Helper  │  │          │  │  (SQLite)│     │
│  │(QNetwork)│  │(QJson)   │  │(spdlog)  │  │          │     │
│  └──────────┘  └──────────┘  └──────────┘  └──────────┘     │
└──────────────────────────────────────────────────────────────┘
```

---

## 三、模块详细设计

### 3.1 UI 层 (`src/ui/`)

#### 3.1.1 MainWindow
- **职责**：应用程序主窗口，管理三段式布局
- **核心组件**：`QSplitter` (水平) 分割三个面板
- **默认比例**：左侧 220px | 中间 flex | 右侧 350px
- **状态持久化**：窗口大小、分割比例保存到 Config

```
┌─────────────┬────────────────────────┬─────────────┐
│   Sidebar   │     EditorPanel        │  ChatPanel  │
│  (文档树)    │   (写作编辑区)          │  (AI对话)   │
│             │                        │             │
│ • 章节目录   │  [标题]                │  ┌─────────┐│
│ • 人物卡片   │                        │  │用户消息  ││
│ • 笔记      │  正文内容...            │  ├─────────┤│
│ • 设定集    │                        │  │AI回复   ││
│             │                        │  ├─────────┤│
│             │                        │  │输入框   ││
└─────────────┴────────────────────────┴─────────────┘
```

#### 3.1.2 SidebarPanel（左侧边栏）
- 文档树 (`QTreeView` + 自定义 Model)
- Tab 切换：大纲 / 人物 / 设定 / 笔记
- 右键上下文菜单（新建/删除/重命名）
- 拖拽排序章节

#### 3.1.3 EditorPanel（中间编辑区）
- 基于 `QPlainTextEdit` 的增强编辑器
- 行号显示
- 语法高亮（Markdown 风格）
- 大纲导航联动
- 自动保存（防抖 2s）
- 字数统计

#### 3.1.4 ChatPanel（右侧 AI 对话栏）
- 消息列表 (`QScrollArea` + 自定义 `ChatBubble`)
- 流式输出支持（打字机效果）
- 消息复制 / 重新生成
- 会话历史管理
- 上下文注入（选中文本作为 Prompt）

---

### 3.2 核心层 (`src/core/`)

#### 3.2.1 文档模型 (`document/`)

```
Document
├── metadata: title, author, created, modified
├── chapters: list<Chapter>
│   └── paragraphs: list<Paragraph>
│       └── text, style, annotations
├── characters: list<Character>
│   └── name, description, relationships
├── outline: tree<OutlineNode>
│   └── title, level, linkedChapter
└── notes: list<Note>
```

**性能优化**：
- 大文档分块加载（Chapter 粒度）
- `QString` 使用 COW (Copy-on-Write)
- 撤销/重做使用 Command 模式，限制历史深度

#### 3.2.2 AI 子系统 (`ai/`)

**核心接口 - AIProvider**（可扩展关键）：

```cpp
class AIProvider {
public:
    virtual ~AIProvider() = default;
    virtual QString name() const = 0;
    virtual QStringList models() const = 0;
    virtual void chat(const ChatRequest& req, ChatCallback callback) = 0;
    virtual void chatStream(const ChatRequest& req, StreamCallback callback) = 0;
    virtual bool configure(const QJsonObject& config) = 0;
};
```

**内置 Provider**：
| Provider | 说明 |
|----------|------|
| `OpenAIProvider` | OpenAI / 兼容 API（如 DeepSeek、通义千问） |
| `ClaudeProvider` | Anthropic Claude API |
| `CustomProvider` | 完全自定义（用户配置 endpoint/headers/body 模板） |

**会话管理**：
- `AISession`：一次对话会话 = 一个 System Prompt + 多条消息
- `PromptTemplate`：可复用的提示词模板（润色/扩写/大纲...）
- 上下文窗口管理：自动裁剪历史消息以适应 token 限制

#### 3.2.3 插件系统 (`plugin/`)

```cpp
// 第三方可实现的插件接口
class IPlugin {
public:
    virtual ~IPlugin() = default;
    virtual QString id() const = 0;
    virtual QString name() const = 0;
    virtual QString version() const = 0;
    virtual bool initialize() = 0;
    virtual void shutdown() = 0;
};

// AI Provider 插件工厂
class IAIProviderPlugin : public IPlugin {
public:
    virtual std::unique_ptr<AIProvider> createProvider() = 0;
};
```

**加载机制**：
1. 扫描 `plugins/` 目录
2. `QLibrary` 动态加载 `.dll`
3. 调用 `createPlugin()` 工厂函数
4. 注册到 `PluginManager`

---

### 3.3 网络层 (`src/network/`)

- `HttpClient`：封装 `QNetworkAccessManager`
  - 同步/异步请求
  - 超时控制（默认 30s）
  - 重试机制（指数退避）
  - TLS/SSL 支持
- `StreamHandler`：SSE (Server-Sent Events) 解析
  - 增量 JSON 解析
  - 断线重连

### 3.4 工具层 (`src/utils/`)

- `JsonHelper`：JSON 序列化/反序列化辅助
- `Logger`：分级日志（基于 spdlog 或 qDebug）
- `StringUtils`：字数统计、Markdown 解析辅助

---

## 四、数据流

### 4.1 AI 对话流程

```
用户输入文本
    │
    ▼
ChatPanel::sendMessage()
    │
    ▼
AISession::send(userMessage)
    │
    ├── 构建 ChatRequest (system + history + user)
    │
    ▼
AIProvider::chatStream(request, callback)
    │
    ├── HttpClient::post(url, body, headers)
    │       │
    │       └── QNetworkReply (readyRead 信号)
    │               │
    │               └── StreamHandler::parse(chunk)
    │                       │
    │                       └── callback.onToken(token)
    │
    ▼
ChatPanel::appendToken(token)  // 打字机效果
    │
    ▼
ChatPanel::onComplete()  // 流结束
```

### 4.2 文档保存流程

```
EditorPanel::onTextChanged()
    │
    ├── 防抖计时器 (2s)
    │
    ▼
DocumentManager::saveDocument(doc)
    │
    ├── 写入本地文件 (.wrs 格式 / Markdown)
    └── 更新 Document::modified 时间戳
```

---

## 五、可扩展性设计

### 5.1 AI Provider 扩展
用户可通过 UI 配置自定义 API：
```
API 地址: https://api.example.com/v1/chat
API Key:  sk-xxxx
模型名:   my-model
请求模板:
{
  "model": "{model}",
  "messages": [{messages}],
  "temperature": {temperature}
}
响应路径: choices[0].message.content
```

### 5.2 插件扩展点（规划中）
- 导出格式插件（PDF/EPUB/DOCX）
- 语法检查插件
- 自定义 Prompt 模板包
- 主题/皮肤插件

### 5.3 未来扩展方向
- 多人协作（WebSocket 同步）
- 版本历史（Git-like diff）
- 语音输入
- 本地模型支持（Ollama）

---

## 六、性能策略

| 策略 | 实现方式 |
|------|---------|
| 大文档分块 | 按 Chapter 粒度 `mmap` 加载 |
| UI 虚拟化 | `QTreeView` + Model/View 分离 |
| 对象池 | AI Provider 实例复用 |
| 异步 IO | `QNetworkAccessManager` 异步请求 |
| 内存监控 | 定期检查 RSS，超过阈值触发 GC |
| 编译优化 | `-O2 -flto`，模板最小化 |

---

## 七、构建系统

```
CMake 3.20+
Qt 6.5+ (Widgets, Network, Core)
编译器: MinGW GCC 14+ / MSVC 2022+
```

构建命令：
```bash
mkdir build && cd build
cmake -G "MinGW Makefiles" -DCMAKE_PREFIX_PATH=D:/Qt/6.9.1/mingw_64 ..
cmake --build . --config Release
```
