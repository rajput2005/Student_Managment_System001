#ifndef HTTP_SERVER_H
#define HTTP_SERVER_H

#include <string>
#include <map>
#include <winsock2.h>
#include "database.h"

struct HttpRequest {
    std::string method;
    std::string raw_url;
    std::string path;
    std::map<std::string, std::string> query_params;
    std::map<std::string, std::string> headers;
    std::string body;
};

struct HttpResponse {
    int status_code;
    std::string status_text;
    std::map<std::string, std::string> headers;
    std::string body;

    HttpResponse() : status_code(200), status_text("OK") {
        headers["Server"] = "EduPulse-Cpp-Server/1.0";
        headers["Access-Control-Allow-Origin"] = "*";
        headers["Access-Control-Allow-Methods"] = "GET, POST, PUT, DELETE, OPTIONS";
        headers["Access-Control-Allow-Headers"] = "Content-Type, Authorization";
    }

    std::string toString() const;
};

class HttpServer {
private:
    int port;
    std::string frontend_dir;
    Database& db;
    SOCKET listen_socket;
    bool running;

    static DWORD WINAPI handleClientThread(LPVOID lpParam);
    void handleClient(SOCKET client_socket);

    HttpRequest parseRequest(const std::string& raw);
    HttpResponse routeRequest(const HttpRequest& req);

    HttpResponse handleStaticFile(const std::string& path);
    HttpResponse handleApi(const HttpRequest& req);

    std::string getMimeType(const std::string& path);
    std::string urlDecode(const std::string& in);

public:
    HttpServer(int port, const std::string& frontend_dir, Database& db);
    ~HttpServer();

    bool start();
    void stop();
};

#endif // HTTP_SERVER_H
