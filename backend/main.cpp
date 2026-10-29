#include <iostream>
#include <winsock2.h>
#include <windows.h>
#include "database.h"
#include "http_server.h"

static HttpServer* g_server = nullptr;

BOOL WINAPI ConsoleHandler(DWORD signal) {
    if (signal == CTRL_C_EVENT || signal == CTRL_CLOSE_EVENT) {
        std::cout << "\n[INFO] Shutting down EduPulse server gracefully..." << std::endl;
        if (g_server) {
            g_server->stop();
        }
        return TRUE;
    }
    return FALSE;
}

int main(int argc, char* argv[]) {
    int port = 8080;
    if (argc > 1) {
        try {
            port = std::stoi(argv[1]);
        } catch (...) {
            port = 8080;
        }
    }

    // Set console code page to UTF-8 for clean output
    SetConsoleOutputCP(CP_UTF8);

    // Initialize Winsock
    WSADATA wsa;
    int rc = WSAStartup(MAKEWORD(2, 2), &wsa);
    if (rc != 0) {
        std::cerr << "[FATAL] WSAStartup failed with code: " << rc << std::endl;
        return 1;
    }

    std::cout << "[INIT] Initializing SQLite database (students.db)..." << std::endl;
    Database db("students.db");
    if (!db.init()) {
        std::cerr << "[FATAL] Failed to initialize SQLite database." << std::endl;
        WSACleanup();
        return 1;
    }
    std::cout << "[INIT] Database initialized successfully." << std::endl;

    // Frontend directory relative to working folder
    std::string frontendDir = "frontend";

    HttpServer server(port, frontendDir, db);
    g_server = &server;

    // Register Ctrl+C handler
    SetConsoleCtrlHandler(ConsoleHandler, TRUE);

    // Start server loop
    if (!server.start()) {
        std::cerr << "[FATAL] Failed to start HTTP server on port " << port << std::endl;
        WSACleanup();
        return 1;
    }

    WSACleanup();
    std::cout << "[INFO] Server shutdown complete." << std::endl;
    return 0;
}
