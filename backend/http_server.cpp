#include "http_server.h"
#include "json_helper.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>

struct ClientContext {
    HttpServer* server;
    SOCKET client_sock;
};

std::string HttpResponse::toString() const {
    std::ostringstream ss;
    ss << "HTTP/1.1 " << status_code << " " << status_text << "\r\n";
    for (const auto& h : headers) {
        ss << h.first << ": " << h.second << "\r\n";
    }
    ss << "Content-Length: " << body.length() << "\r\n";
    ss << "Connection: close\r\n\r\n";
    ss << body;
    return ss.str();
}

HttpServer::HttpServer(int p, const std::string& fdir, Database& d)
    : port(p), frontend_dir(fdir), db(d), listen_socket(INVALID_SOCKET), running(false) {}

HttpServer::~HttpServer() {
    stop();
}

void HttpServer::stop() {
    running = false;
    if (listen_socket != INVALID_SOCKET) {
        closesocket(listen_socket);
        listen_socket = INVALID_SOCKET;
    }
}

std::string HttpServer::urlDecode(const std::string& in) {
    std::string out;
    out.reserve(in.length());
    for (size_t i = 0; i < in.length(); ++i) {
        if (in[i] == '%') {
            if (i + 2 < in.length()) {
                int hexVal;
                std::istringstream hexStream(in.substr(i + 1, 2));
                if (hexStream >> std::hex >> hexVal) {
                    out += static_cast<char>(hexVal);
                    i += 2;
                } else {
                    out += '%';
                }
            } else {
                out += '%';
            }
        } else if (in[i] == '+') {
            out += ' ';
        } else {
            out += in[i];
        }
    }
    return out;
}

std::string HttpServer::getMimeType(const std::string& path) {
    size_t dotPos = path.find_last_of('.');
    if (dotPos == std::string::npos) return "text/plain";
    std::string ext = path.substr(dotPos);
    for (auto& c : ext) c = tolower(c);

    if (ext == ".html" || ext == ".htm") return "text/html; charset=utf-8";
    if (ext == ".css") return "text/css; charset=utf-8";
    if (ext == ".js") return "application/javascript; charset=utf-8";
    if (ext == ".json") return "application/json; charset=utf-8";
    if (ext == ".png") return "image/png";
    if (ext == ".jpg" || ext == ".jpeg") return "image/jpeg";
    if (ext == ".gif") return "image/gif";
    if (ext == ".svg") return "image/svg+xml";
    if (ext == ".ico") return "image/x-icon";
    if (ext == ".woff") return "font/woff";
    if (ext == ".woff2") return "font/woff2";
    if (ext == ".ttf") return "font/ttf";
    return "application/octet-stream";
}

HttpRequest HttpServer::parseRequest(const std::string& raw) {
    HttpRequest req;
    size_t lineEnd = raw.find("\r\n");
    if (lineEnd == std::string::npos) return req;

    std::string requestLine = raw.substr(0, lineEnd);
    std::istringstream rls(requestLine);
    std::string protocol;
    rls >> req.method >> req.raw_url >> protocol;

    // Parse query params
    size_t qPos = req.raw_url.find('?');
    if (qPos != std::string::npos) {
        req.path = req.raw_url.substr(0, qPos);
        std::string qStr = req.raw_url.substr(qPos + 1);
        std::istringstream qs(qStr);
        std::string pair;
        while (std::getline(qs, pair, '&')) {
            size_t eqPos = pair.find('=');
            if (eqPos != std::string::npos) {
                std::string k = urlDecode(pair.substr(0, eqPos));
                std::string v = urlDecode(pair.substr(eqPos + 1));
                req.query_params[k] = v;
            } else if (!pair.empty()) {
                req.query_params[urlDecode(pair)] = "";
            }
        }
    } else {
        req.path = req.raw_url;
    }

    // Parse headers
    size_t cur = lineEnd + 2;
    while (cur < raw.length()) {
        size_t nextEnd = raw.find("\r\n", cur);
        if (nextEnd == std::string::npos || nextEnd == cur) {
            cur += 2;
            break; // Blank line before body
        }
        std::string headerLine = raw.substr(cur, nextEnd - cur);
        size_t colon = headerLine.find(':');
        if (colon != std::string::npos) {
            std::string key = headerLine.substr(0, colon);
            std::string val = headerLine.substr(colon + 1);
            // Trim
            size_t f = val.find_first_not_of(" \t");
            if (f != std::string::npos) val = val.substr(f);
            std::string lowerKey = key;
            for (auto& c : lowerKey) c = tolower(c);
            req.headers[lowerKey] = val;
        }
        cur = nextEnd + 2;
    }

    // Body
    if (cur < raw.length()) {
        req.body = raw.substr(cur);
    }

    return req;
}

HttpResponse HttpServer::handleStaticFile(const std::string& path) {
    HttpResponse res;
    std::string cleanPath = (path == "/" || path.empty()) ? "/index.html" : path;
    
    // Prevent directory traversal
    if (cleanPath.find("..") != std::string::npos) {
        res.status_code = 403;
        res.status_text = "Forbidden";
        res.headers["Content-Type"] = "text/plain";
        res.body = "403 Forbidden";
        return res;
    }

    std::string fullPath = frontend_dir + cleanPath;
    std::ifstream file(fullPath.c_str(), std::ios::binary);
    if (!file.is_open()) {
        res.status_code = 404;
        res.status_text = "Not Found";
        res.headers["Content-Type"] = "text/plain";
        res.body = "404 Not Found: " + cleanPath;
        return res;
    }

    std::ostringstream ss;
    ss << file.rdbuf();
    res.body = ss.str();
    res.status_code = 200;
    res.status_text = "OK";
    res.headers["Content-Type"] = getMimeType(cleanPath);
    return res;
}

HttpResponse HttpServer::handleApi(const HttpRequest& req) {
    HttpResponse res;
    res.headers["Content-Type"] = "application/json; charset=utf-8";

    // 1. GET /api/stats
    if (req.method == "GET" && req.path == "/api/stats") {
        DashboardStats stats = db.getStats();
        res.status_code = 200;
        res.status_text = "OK";
        res.body = JsonHelper::successResponse(JsonHelper::statsToJson(stats));
        return res;
    }

    // 2. GET /api/students
    if (req.method == "GET" && req.path == "/api/students") {
        std::string search = req.query_params.count("search") ? req.query_params.at("search") : "";
        std::string dept = req.query_params.count("department") ? req.query_params.at("department") : "";
        std::string status = req.query_params.count("status") ? req.query_params.at("status") : "";
        std::string sortBy = req.query_params.count("sortBy") ? req.query_params.at("sortBy") : "id";
        std::string sortOrder = req.query_params.count("sortOrder") ? req.query_params.at("sortOrder") : "DESC";

        std::vector<Student> students = db.getStudents(search, dept, status, sortBy, sortOrder);
        res.status_code = 200;
        res.status_text = "OK";
        res.body = JsonHelper::successResponse(JsonHelper::studentsToJson(students));
        return res;
    }

    // 3. POST /api/students
    if (req.method == "POST" && req.path == "/api/students") {
        Student s = JsonHelper::parseStudent(req.body);
        if (s.student_id.empty() || s.first_name.empty() || s.last_name.empty() || s.email.empty() || s.department.empty()) {
            res.status_code = 400;
            res.status_text = "Bad Request";
            res.body = JsonHelper::errorResponse("Required fields missing: Student ID, First Name, Last Name, Email, and Department are mandatory.");
            return res;
        }

        std::string errMsg;
        int newId = db.addStudent(s, errMsg);
        if (newId <= 0) {
            res.status_code = 409;
            res.status_text = "Conflict";
            res.body = JsonHelper::errorResponse(errMsg.empty() ? "Failed to add student." : errMsg);
            return res;
        }

        s.id = newId;
        res.status_code = 201;
        res.status_text = "Created";
        res.body = JsonHelper::successResponse(JsonHelper::studentToJson(s), "Student record added successfully!");
        return res;
    }

    // 4. Individual Student routes: /api/students/<id>
    const std::string prefix = "/api/students/";
    if (req.path.length() > prefix.length() && req.path.compare(0, prefix.length(), prefix) == 0) {
        std::string idStr = req.path.substr(prefix.length());
        int id = 0;
        try {
            id = std::stoi(idStr);
        } catch (...) {
            res.status_code = 400;
            res.status_text = "Bad Request";
            res.body = JsonHelper::errorResponse("Invalid student ID format.");
            return res;
        }

        // GET /api/students/<id>
        if (req.method == "GET") {
            Student s;
            if (db.getStudentById(id, s)) {
                res.status_code = 200;
                res.status_text = "OK";
                res.body = JsonHelper::successResponse(JsonHelper::studentToJson(s));
            } else {
                res.status_code = 404;
                res.status_text = "Not Found";
                res.body = JsonHelper::errorResponse("Student not found.");
            }
            return res;
        }

        // PUT /api/students/<id>
        if (req.method == "PUT") {
            Student s = JsonHelper::parseStudent(req.body);
            if (s.student_id.empty() || s.first_name.empty() || s.last_name.empty() || s.email.empty() || s.department.empty()) {
                res.status_code = 400;
                res.status_text = "Bad Request";
                res.body = JsonHelper::errorResponse("Required fields missing: Student ID, First Name, Last Name, Email, and Department are mandatory.");
                return res;
            }

            std::string errMsg;
            if (db.updateStudent(id, s, errMsg)) {
                s.id = id;
                res.status_code = 200;
                res.status_text = "OK";
                res.body = JsonHelper::successResponse(JsonHelper::studentToJson(s), "Student record updated successfully!");
            } else {
                res.status_code = 409;
                res.status_text = "Conflict";
                res.body = JsonHelper::errorResponse(errMsg.empty() ? "Failed to update student." : errMsg);
            }
            return res;
        }

        // DELETE /api/students/<id>
        if (req.method == "DELETE") {
            std::string errMsg;
            if (db.deleteStudent(id, errMsg)) {
                res.status_code = 200;
                res.status_text = "OK";
                res.body = JsonHelper::successResponse("{}", "Student deleted successfully!");
            } else {
                res.status_code = 404;
                res.status_text = "Not Found";
                res.body = JsonHelper::errorResponse(errMsg.empty() ? "Student not found or deletion failed." : errMsg);
            }
            return res;
        }
    }

    // Fallback API route
    res.status_code = 404;
    res.status_text = "Not Found";
    res.body = JsonHelper::errorResponse("API endpoint not found: " + req.method + " " + req.path);
    return res;
}

HttpResponse HttpServer::routeRequest(const HttpRequest& req) {
    if (req.method == "OPTIONS") {
        HttpResponse res;
        res.status_code = 204;
        res.status_text = "No Content";
        return res;
    }

    if (req.path.compare(0, 5, "/api/") == 0) {
        return handleApi(req);
    }

    return handleStaticFile(req.path);
}

DWORD WINAPI HttpServer::handleClientThread(LPVOID lpParam) {
    ClientContext* ctx = reinterpret_cast<ClientContext*>(lpParam);
    ctx->server->handleClient(ctx->client_sock);
    delete ctx;
    return 0;
}

void HttpServer::handleClient(SOCKET client_socket) {
    std::string rawRequest;
    char buffer[4096];
    int bytesRead = 0;
    size_t expectedBodyLength = 0;
    bool headersComplete = false;

    while ((bytesRead = recv(client_socket, buffer, sizeof(buffer) - 1, 0)) > 0) {
        buffer[bytesRead] = '\0';
        rawRequest.append(buffer, bytesRead);

        if (!headersComplete) {
            size_t headerEnd = rawRequest.find("\r\n\r\n");
            if (headerEnd != std::string::npos) {
                headersComplete = true;
                // Look for Content-Length
                std::string headerPart = rawRequest.substr(0, headerEnd);
                std::string lowerHeader = headerPart;
                for (auto& c : lowerHeader) c = tolower(c);
                size_t clPos = lowerHeader.find("content-length:");
                if (clPos != std::string::npos) {
                    size_t numStart = clPos + 15;
                    size_t numEnd = lowerHeader.find("\r\n", numStart);
                    std::string clStr = headerPart.substr(numStart, numEnd - numStart);
                    try {
                        expectedBodyLength = std::stoul(JsonHelper::trim(clStr));
                    } catch (...) {
                        expectedBodyLength = 0;
                    }
                }
                size_t currentBodyLen = rawRequest.length() - (headerEnd + 4);
                if (currentBodyLen >= expectedBodyLength) {
                    break;
                }
            }
        } else {
            size_t headerEnd = rawRequest.find("\r\n\r\n");
            size_t currentBodyLen = rawRequest.length() - (headerEnd + 4);
            if (currentBodyLen >= expectedBodyLength) {
                break;
            }
        }
    }

    if (!rawRequest.empty()) {
        HttpRequest req = parseRequest(rawRequest);
        HttpResponse res = routeRequest(req);

        std::cout << "[HTTP] " << res.status_code << " " << req.method << " " << req.raw_url << std::endl;

        std::string resStr = res.toString();
        send(client_socket, resStr.c_str(), static_cast<int>(resStr.length()), 0);
    }

    shutdown(client_socket, SD_BOTH);
    closesocket(client_socket);
}

bool HttpServer::start() {
    listen_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listen_socket == INVALID_SOCKET) {
        std::cerr << "[SERVER ERROR] Failed to create socket: " << WSAGetLastError() << std::endl;
        return false;
    }

    // Reuse address
    BOOL opt = TRUE;
    setsockopt(listen_socket, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<char*>(&opt), sizeof(opt));

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serverAddr.sin_port = htons(static_cast<u_short>(port));

    if (bind(listen_socket, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "[SERVER ERROR] Bind failed on port " << port << " (Code: " << WSAGetLastError() << ")" << std::endl;
        closesocket(listen_socket);
        listen_socket = INVALID_SOCKET;
        return false;
    }

    if (listen(listen_socket, SOMAXCONN) == SOCKET_ERROR) {
        std::cerr << "[SERVER ERROR] Listen failed: " << WSAGetLastError() << std::endl;
        closesocket(listen_socket);
        listen_socket = INVALID_SOCKET;
        return false;
    }

    running = true;
    std::cout << "\n========================================================" << std::endl;
    std::cout << "  EduPulse Student Management Backend (C++ & SQLite)" << std::endl;
    std::cout << "  Server listening at: http://localhost:" << port << std::endl;
    std::cout << "  API Base URL:        http://localhost:" << port << "/api/students" << std::endl;
    std::cout << "  Frontend UI:         http://localhost:" << port << "/" << std::endl;
    std::cout << "  Press Ctrl+C to terminate the server." << std::endl;
    std::cout << "========================================================\n" << std::endl;

    while (running) {
        sockaddr_in clientAddr;
        int clientLen = sizeof(clientAddr);
        SOCKET clientSock = accept(listen_socket, reinterpret_cast<sockaddr*>(&clientAddr), &clientLen);

        if (clientSock == INVALID_SOCKET) {
            if (running) {
                std::cerr << "[SERVER WARN] Accept failed: " << WSAGetLastError() << std::endl;
            }
            break;
        }

        ClientContext* ctx = new ClientContext{ this, clientSock };
        HANDLE hThread = CreateThread(NULL, 0, handleClientThread, ctx, 0, NULL);
        if (hThread) {
            CloseHandle(hThread); // Detach thread
        } else {
            // Fallback to synchronous handling
            handleClient(clientSock);
            delete ctx;
        }
    }

    return true;
}
