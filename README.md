# EduPulse - Full-Stack Student Management System

A high-performance, full-stack Student Management System built with a native **C++ backend**, **SQLite 3 database**, and a modern, responsive **HTML5 / CSS3 / Vanilla JavaScript** frontend hosted on **localhost:8080**.

---

## 🌟 Key Features

### 🖥️ Frontend (HTML5, Modern CSS3, JavaScript)
- **Responsive Dashboard**: Beautiful light/dark mode with smooth animations and glassmorphism touches.
- **Real-Time KPI Cards**: Total students, active enrollment count, average GPA meter, and department counts.
- **Interactive Data Table**:
  - Avatar badges with deterministic color gradients based on student name.
  - Formatted GPA badges (High / Good / Needs Attention).
  - Status pills (Active, Inactive, Graduated, Suspended).
  - Actions: **View Profile**, **Edit Record**, **Delete Record**.
- **Instant Search & Filtering**:
  - Live search across First Name, Last Name, Student ID / Roll Number, and Email.
  - Quick Department filter pills (Computer Science, Data Science, Electrical Eng, etc.).
  - Status and Multi-Column Sorting (Newest, Name, GPA, Roll No).
- **CRUD Modals**:
  - **Add Student Modal**: 2-column form with client-side validation and field error cues.
  - **Edit Student Modal**: Pre-populates all student attributes for instant updates.
  - **Student Profile View**: Comprehensive modal showing full profile, contact info, and academic history.
  - **Delete Confirmation Dialog**: Prevents accidental record removal.
- **Export to CSV**: Export the active or filtered student dataset to an Excel-ready `.csv` file.
- **Toast Notifications**: Non-intrusive floating feedback messages for every action.

### ⚙️ Backend (C++ & Windows Sockets)
- **Zero Third-Party Web Frameworks**: Built using native Windows Sockets (`winsock2.h`).
- **RESTful API Architecture**:
  - `GET /api/students` - Query students with search, filter, and sorting parameters.
  - `GET /api/students/:id` - Fetch single student record by primary ID.
  - `POST /api/students` - Create a new student with JSON payload.
  - `PUT /api/students/:id` - Update existing student record with JSON payload.
  - `DELETE /api/students/:id` - Delete student by ID.
  - `GET /api/stats` - Summary statistics for the dashboard.
- **Embedded Web Server**: Automatically serves static assets (`index.html`, `style.css`, `app.js`).
- **Threaded Connection Handling**: Uses Windows `CreateThread` for concurrency.
- **CORS & Options Handling**: Full pre-flight support for browser requests.

### 🗄️ Database (SQLite 3.54)
- **Persistent Local Database**: `students.db` stored directly in project folder.
- **Thread-Safe Access**: Protected via Win32 `CRITICAL_SECTION`.
- **Pre-Seeded Sample Data**: Includes 8 realistic student records on initial startup so the system is immediately usable.
- **Input Sanitization**: Prepared statements (`sqlite3_prepare_v2`, `sqlite3_bind_*`) prevent SQL injection.
- **Integrity Enforcement**: Unique constraints on Student ID / Roll Number and Email.

---

## 📁 Project Structure

```text
student-management-system/
│
├── backend/
│   ├── main.cpp            # Server initialization, Ctrl+C handler, entry point
│   ├── models.h            # Student & DashboardStats data structures
│   ├── json_helper.h       # Lightweight JSON parser & serializer in C++
│   ├── database.h          # Database class definition & query methods
│   ├── database.cpp        # SQLite CRUD implementation & seed records
│   ├── http_server.h       # Winsock HTTP server interface & routing
│   └── http_server.cpp     # HTTP request parser, file server & API endpoints
│
├── frontend/
│   ├── index.html          # Dashboard HTML UI & dialog templates
│   ├── style.css           # Styling with Dark/Light theme & responsive layout
│   └── app.js              # Frontend state, API client, table & modal logic
│
├── sqlite3.h               # SQLite 3.54 header
├── sqlite3.o               # Precompiled SQLite object
├── sqlite3_wrapper.c       # Source wrapper for SQLite compilation on MinGW
├── build.bat               # One-click Windows build script
├── run.bat                 # One-click Windows run script (opens browser)
└── README.md               # Documentation
```

---

## 🚀 How to Run the Website

### Option 1: Quick Run (Batch Script)
Simply double-click or run from PowerShell:
```powershell
.\run.bat
```
This automatically compiles `server.exe` if not already built, starts the C++ backend on `http://localhost:8080`, and opens it in your default browser.

### Option 2: Build and Run Manually

1. **Compile Backend**:
   ```powershell
   g++ -std=c++14 -O2 backend/main.cpp backend/database.cpp backend/http_server.cpp sqlite3.o -o server.exe -lws2_32
   ```

2. **Launch Server**:
   ```powershell
   .\server.exe
   ```

3. **Open in Browser**:
   Navigate to:
   ```
   http://localhost:8080
   ```

---

## 📡 API Reference

| Method | Endpoint | Description |
|---|---|---|
| `GET` | `/api/students` | Get all students (Supports `?search=`, `?department=`, `?status=`, `?sortBy=`, `?sortOrder=`) |
| `GET` | `/api/students/:id` | Get student details by ID |
| `POST` | `/api/students` | Create new student record (JSON body) |
| `PUT` | `/api/students/:id` | Update existing student record (JSON body) |
| `DELETE` | `/api/students/:id` | Delete student record |
| `GET` | `/api/stats` | Get dashboard summary metrics and department distribution |
| `GET` | `/` or `/index.html` | Serves frontend dashboard |
| `GET` | `/style.css` | Serves stylesheet |
| `GET` | `/app.js` | Serves frontend script |

---

## 💻 Tech Stack
- **Frontend**: HTML5, Modern CSS3 (Grid & Flexbox, CSS Variables), Vanilla ES6+ JavaScript.
- **Backend**: C++ (C++14), WinSock 2 API (`ws2_32.lib`).
- **Database**: SQLite 3.54 (Amalgamation embedded C engine).
- **Environment**: Windows localhost.
