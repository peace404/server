
#include <csignal>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <thread>

#ifdef _WIN32
#include <Ws2tcpip.h>
#else
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#define SOCKET int
#define INVALID_SOCKET (-1)
#define SOCKET_ERROR (-1)
#define closesocket close
typedef struct sockaddr *LPSOCKADDR;
#endif

#define buf_size 4096

std::string get_mime_type(const std::string &filepath) {
    size_t dotPos = filepath.find_last_of('.');
    if (dotPos == std::string::npos)
        return "application/octet-stream"; // 默认二进制类型

    std::string ext = filepath.substr(dotPos + 1);
    std::map<std::string, std::string> mime_types{
            {"html", "text/html"},
            {"css", "text/css"},
            {"js", "application/javascript"},
            {"jpg", "image/jpg"},
            {"jpeg", "image/jpeg"},
            {"png", "image/png"},
            {"gif", "image/gif"},
            {"txt", "text/plain"},
            {"ico", "image/x-icon"},
            // 根据需要添加
    };

    if (mime_types.count(ext)) {
        return mime_types[ext];
    }
    return "application/octet-stream"; // 未知类型时返回默认
}

void send_http(const SOCKET &clientSock, const std::string &filepath) {
    std::ifstream file(filepath, std::ios::binary); // 使用二进制模式打开文件
    if (!file.is_open()) {
        // 如果文件未找到，返回 404 错误
        std::cout << "404 NOT FOUND!" << std::endl;
        std::string errorResp =
                "HTTP/1.1 404 Not Found\r\n"
                "Content-Type: text/html\r\n\r\n"
                "<html>"
                "<head><title>404 Not Found</title></head>"
                "<body style='text-align:center; font-family:sans-serif;'>"
                "<h1 style='color:red;'>404 Not Found</h1>"
                "<p>Sorry, the page you are looking for does not exist.</p>"
                "<a href='/' style='color:blue;'>Return to Home</a>"
                "</body>"
                "</html>";
        send(clientSock, errorResp.c_str(), errorResp.size(), 0);
        return;
    }

    // 获取文件内容
    std::cout << "200 OK" << std::endl;
    file.seekg(0, std::ios::end);
    size_t fileSize = file.tellg();
    file.seekg(0, std::ios::beg);

    std::string mime_type = get_mime_type(filepath);
    std::ostringstream oss;
    oss << "HTTP/1.1 200 OK\r\n"
        << "Server: VerySimpleServer\r\n"
        << "Content-Type: " << mime_type << "\r\n"
        << "Content-Length: " << fileSize << "\r\n\r\n";
    std::string header = oss.str();
    send(clientSock, header.c_str(), static_cast<int>(header.size()), 0);

    // 发送文件内容
    char buffer[buf_size];
    while (file.read(buffer, sizeof(buffer))) {
        send(clientSock, buffer, file.gcount(), 0);
    }
    // 发送最后剩余的字节
    send(clientSock, buffer, file.gcount(), 0);
    file.close();
}

void handle_client(SOCKET sessionSocket, sockaddr_in clientAddr,
                   const std::string &root_dir) {
    std::string request;
    char revBuff[buf_size]{};
    int rtn;
    while ((rtn = recv(sessionSocket, revBuff, sizeof(revBuff), 0)) > 0) {
        request.append(revBuff, rtn);
        if (request.find("\r\n\r\n") != std::string::npos) {
            break;
        }
    }
    if (rtn <= 0) {
        std::cout << "Client disconnected." << std::endl;
        return;
    }
    std::istringstream req_stream(request);
    std::string method, path;
    req_stream >> method >> path;
    if (path.empty() || path == "/") {
        path = "/index.html";
    }
    std::string full_path = root_dir + path;
    std::cout << "client ip: " << inet_ntoa(clientAddr.sin_addr)
              << ", port: " << ntohs(clientAddr.sin_port)
              << ", method: " << method << ", path: " << path << std::endl;
    send_http(sessionSocket, full_path);
    closesocket(sessionSocket);
    std::cout << "Connection closed." << std::endl;
}

bool load_config(const std::string &filename, int &port, std::string &ip,
                 std::string &root_dir) {
    std::ifstream infile(filename);
    if (!infile.is_open())
        return false;
    std::getline(infile, ip);
    infile >> port;
    infile.ignore();
    std::getline(infile, root_dir);
    return true;
}

int main() {
    std::string ipaddr = "127.0.0.1";
    int port = 8080;
    std::string main_directory = "../static";
    // Load configuration file
    if (!load_config("../config.ini", port, ipaddr, main_directory)) {
        std::cerr << "Failed to load config.ini. Using default settings."
                  << std::endl;
    }
#ifdef _WIN32
    WSADATA wsaData;
    // 初始化Winsock环境
    if (WSAStartup(MAKEWORD(2, 2), &wsaData)) {
        std::cerr << "Winsock startup failed with error!\n";
        return 1;
    }
#endif

    // create socket
    SOCKET srvSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (srvSocket == INVALID_SOCKET) {
        std::cerr << "Socket creation failed" << std::endl;
        return 1;
    }

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    inet_pton(AF_INET, ipaddr.c_str(), &serverAddr.sin_addr);
    // bind
    if (bind(srvSocket, (LPSOCKADDR)&serverAddr, sizeof(serverAddr)) ==
        SOCKET_ERROR) {
        std::cerr << "Bind failed" << std::endl;
        return 1;
    }
    // listen
    if (listen(srvSocket, SOMAXCONN) == SOCKET_ERROR) {
        std::cerr << "listen failed!\n";
        return 1;
    }
    std::cout << "HTTP Server running at http://" << ipaddr << ":" << port
              << ", serving directory: " << main_directory << std::endl;
    while (true) {
        sockaddr_in clientAddr{};
        socklen_t addrLen = sizeof(clientAddr);
        SOCKET sessionSocket = accept(srvSocket, (LPSOCKADDR)&clientAddr, &addrLen);
        if (INVALID_SOCKET == sessionSocket) {
            std::cout << "accept failed!\n" << std::endl;
#ifdef _WIN32
            WSACleanup();
#endif
            return 1;
        }
        std::cout << "Accepted a new client connection.\n";
        // 创建新线程处理客户端请求
        std::thread(handle_client, sessionSocket, clientAddr, main_directory)
                .detach();
    }
}
