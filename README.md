# Simple HTTP Server in C++ (Windows)

这是一个基于 C++ 和 Winsock API 实现的简易多线程 HTTP 服务器，可用于本地测试静态网页，支持 HTML、CSS、JS、图片等文件类型。

---

## 🧩 功能 Features

- 支持 HTTP GET 请求
- 自动根据文件扩展名识别 MIME 类型
- 支持多线程同时处理多个客户端
- 支持配置文件（IP 地址、端口号、根目录）
- 返回 404 页面时自动生成友好提示页面

---

## 📁 项目结构 Project Structure

```text
.
├── src/
│   └── main.cpp           // 服务器主程序
├── config.ini             // 配置文件（IP、端口、网站目录）
├── static/                // 网站资源目录（index.html 等）
├── CMakeLists.txt         // CMake 构建文件（如使用 CLion）
└── README.md
```

---

## ⚙️ 配置文件 config.ini 示例

```init
0.0.0.0
8080
../static
```
说明：

1. 第 1 行：服务器监听的 IP 地址（如 `127.0.0.1` 或 `0.0.0.0`）
2. 第 2 行：端口号（例如 8080）
3. 第 3 行：网站根目录路径（用于存放 index.html 等静态资源）

---

## 🚀 构建与运行

### ✅ 使用 CLion（推荐）

1. 打开项目文件夹
2. CLion 会自动识别 CMake 工程，点击运行即可构建并启动服务器

### ✅ 命令行手动构建（使用 CMake）

```bash
mkdir build
cd build
cmake ..
cmake --build .
```
▶️ 运行服务器
```
./build/YourExecutableName.exe
```
默认监听地址为：
```
http://localhost:8080/
```
