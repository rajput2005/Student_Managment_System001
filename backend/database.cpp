#include "database.h"
#include <iostream>
#include <sstream>
#include <algorithm>

Database::Database(const std::string& path) : db(nullptr), db_path(path) {
    InitializeCriticalSection(&cs);
}

Database::~Database() {
    lock();
    if (db) {
        sqlite3_close(db);
        db = nullptr;
    }
    unlock();
    DeleteCriticalSection(&cs);
}

void Database::lock() {
    EnterCriticalSection(&cs);
}

void Database::unlock() {
    LeaveCriticalSection(&cs);
}

bool Database::init() {
    lock();
    int rc = sqlite3_open(db_path.c_str(), &db);
    if (rc != SQLITE_OK) {
        std::cerr << "[DB ERROR] Cannot open database: " << sqlite3_errmsg(db) << std::endl;
        unlock();
        return false;
    }

    const char* schema = 
        "CREATE TABLE IF NOT EXISTS students ("
        "  id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "  student_id TEXT UNIQUE NOT NULL,"
        "  first_name TEXT NOT NULL,"
        "  last_name TEXT NOT NULL,"
        "  email TEXT UNIQUE NOT NULL,"
        "  phone TEXT,"
        "  dob TEXT,"
        "  gender TEXT,"
        "  department TEXT NOT NULL,"
        "  year TEXT,"
        "  gpa REAL DEFAULT 0.0,"
        "  status TEXT DEFAULT 'Active',"
        "  address TEXT,"
        "  created_at DATETIME DEFAULT CURRENT_TIMESTAMP"
        ");";

    char* errMsg = nullptr;
    rc = sqlite3_exec(db, schema, nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
        std::cerr << "[DB ERROR] Schema creation failed: " << (errMsg ? errMsg : "Unknown error") << std::endl;
        sqlite3_free(errMsg);
        unlock();
        return false;
    }

    // Check count of records, seed if 0
    sqlite3_stmt* stmt = nullptr;
    rc = sqlite3_prepare_v2(db, "SELECT COUNT(*) FROM students", -1, &stmt, nullptr);
    int count = 0;
    if (rc == SQLITE_OK) {
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            count = sqlite3_column_int(stmt, 0);
        }
        sqlite3_finalize(stmt);
    }

    if (count == 0) {
        seedSampleData();
    }

    unlock();
    return true;
}

void Database::seedSampleData() {
    std::cout << "[DB] Seeding initial sample student records..." << std::endl;
    const char* seedSql = 
        "INSERT INTO students (student_id, first_name, last_name, email, phone, dob, gender, department, year, gpa, status, address) VALUES "
        "('STU-2024-001', 'Alexander', 'Wright', 'alex.wright@campus.edu', '+1 (555) 234-5678', '2003-04-15', 'Male', 'Computer Science', '3rd Year', 3.92, 'Active', '104 Innovation Way, Tech District'),"
        "('STU-2024-002', 'Sophia', 'Chen', 'sophia.chen@campus.edu', '+1 (555) 345-6789', '2004-08-22', 'Female', 'Data Science', '2nd Year', 3.85, 'Active', '205 Silicon Blvd, Metro City'),"
        "('STU-2024-003', 'Marcus', 'Johnson', 'marcus.j@campus.edu', '+1 (555) 456-7890', '2002-11-10', 'Male', 'Electrical Engineering', '4th Year', 3.68, 'Active', '78 University Crescent'),"
        "('STU-2024-004', 'Emily', 'Rodriguez', 'emily.r@campus.edu', '+1 (555) 567-8901', '2003-02-18', 'Female', 'Business Administration', '3rd Year', 3.75, 'Active', '312 Commerce Plaza, Suite 4B'),"
        "('STU-2024-005', 'Liam', 'Patel', 'liam.patel@campus.edu', '+1 (555) 678-9012', '2005-01-30', 'Male', 'Mechanical Engineering', '1st Year', 3.45, 'Active', '45 Campus Court, Dorm A'),"
        "('STU-2024-006', 'Olivia', 'Kim', 'olivia.kim@campus.edu', '+1 (555) 789-0123', '2001-09-05', 'Female', 'Computer Science', '4th Year', 3.98, 'Graduated', '89 Pioneer Park, Apt 12'),"
        "('STU-2024-007', 'Ethan', 'Davis', 'ethan.davis@campus.edu', '+1 (555) 890-1234', '2004-06-14', 'Male', 'Civil Engineering', '2nd Year', 3.20, 'Inactive', '510 Oak Ridge Drive'),"
        "('STU-2024-008', 'Aria', 'Sharma', 'aria.sharma@campus.edu', '+1 (555) 901-2345', '2003-12-03', 'Female', 'Data Science', '3rd Year', 3.89, 'Active', '12 Lakeview Avenue');";

    char* errMsg = nullptr;
    int rc = sqlite3_exec(db, seedSql, nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
        std::cerr << "[DB ERROR] Seed data insertion failed: " << (errMsg ? errMsg : "Unknown") << std::endl;
        sqlite3_free(errMsg);
    } else {
        std::cout << "[DB] Successfully seeded 8 students." << std::endl;
    }
}

static std::string colText(sqlite3_stmt* stmt, int col) {
    const unsigned char* txt = sqlite3_column_text(stmt, col);
    return txt ? reinterpret_cast<const char*>(txt) : "";
}

static Student extractStudentFromRow(sqlite3_stmt* stmt) {
    Student s;
    s.id = sqlite3_column_int(stmt, 0);
    s.student_id = colText(stmt, 1);
    s.first_name = colText(stmt, 2);
    s.last_name = colText(stmt, 3);
    s.email = colText(stmt, 4);
    s.phone = colText(stmt, 5);
    s.dob = colText(stmt, 6);
    s.gender = colText(stmt, 7);
    s.department = colText(stmt, 8);
    s.year = colText(stmt, 9);
    s.gpa = sqlite3_column_double(stmt, 10);
    s.status = colText(stmt, 11);
    s.address = colText(stmt, 12);
    s.created_at = colText(stmt, 13);
    return s;
}

std::vector<Student> Database::getStudents(
    const std::string& search,
    const std::string& department,
    const std::string& status,
    const std::string& sortBy,
    const std::string& sortOrder
) {
    std::vector<Student> list;
    lock();
    if (!db) {
        unlock();
        return list;
    }

    std::ostringstream sql;
    sql << "SELECT id, student_id, first_name, last_name, email, phone, dob, gender, department, year, gpa, status, address, created_at FROM students WHERE 1=1";

    if (!search.empty()) {
        sql << " AND (first_name LIKE ? OR last_name LIKE ? OR student_id LIKE ? OR email LIKE ?)";
    }
    if (!department.empty() && department != "All") {
        sql << " AND department = ?";
    }
    if (!status.empty() && status != "All") {
        sql << " AND status = ?";
    }

    // Whitelist valid sort columns
    std::string safeSort = "id";
    if (sortBy == "student_id") safeSort = "student_id";
    else if (sortBy == "first_name") safeSort = "first_name";
    else if (sortBy == "department") safeSort = "department";
    else if (sortBy == "gpa") safeSort = "gpa";
    else if (sortBy == "created_at") safeSort = "created_at";

    std::string safeOrder = (sortOrder == "ASC" || sortOrder == "asc") ? "ASC" : "DESC";
    sql << " ORDER BY " << safeSort << " " << safeOrder;

    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(db, sql.str().c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "[DB ERROR] Query prepare failed: " << sqlite3_errmsg(db) << std::endl;
        unlock();
        return list;
    }

    int bindIdx = 1;
    if (!search.empty()) {
        std::string pattern = "%" + search + "%";
        sqlite3_bind_text(stmt, bindIdx++, pattern.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, bindIdx++, pattern.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, bindIdx++, pattern.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, bindIdx++, pattern.c_str(), -1, SQLITE_TRANSIENT);
    }
    if (!department.empty() && department != "All") {
        sqlite3_bind_text(stmt, bindIdx++, department.c_str(), -1, SQLITE_TRANSIENT);
    }
    if (!status.empty() && status != "All") {
        sqlite3_bind_text(stmt, bindIdx++, status.c_str(), -1, SQLITE_TRANSIENT);
    }

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        list.push_back(extractStudentFromRow(stmt));
    }
    sqlite3_finalize(stmt);

    unlock();
    return list;
}

bool Database::getStudentById(int id, Student& student) {
    lock();
    if (!db) {
        unlock();
        return false;
    }

    const char* sql = "SELECT id, student_id, first_name, last_name, email, phone, dob, gender, department, year, gpa, status, address, created_at FROM students WHERE id = ?";
    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        unlock();
        return false;
    }

    sqlite3_bind_int(stmt, 1, id);

    bool found = false;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        student = extractStudentFromRow(stmt);
        found = true;
    }
    sqlite3_finalize(stmt);

    unlock();
    return found;
}

bool Database::getStudentByStudentId(const std::string& studentId, Student& student) {
    lock();
    if (!db) {
        unlock();
        return false;
    }

    const char* sql = "SELECT id, student_id, first_name, last_name, email, phone, dob, gender, department, year, gpa, status, address, created_at FROM students WHERE student_id = ?";
    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        unlock();
        return false;
    }

    sqlite3_bind_text(stmt, 1, studentId.c_str(), -1, SQLITE_TRANSIENT);

    bool found = false;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        student = extractStudentFromRow(stmt);
        found = true;
    }
    sqlite3_finalize(stmt);

    unlock();
    return found;
}

int Database::addStudent(const Student& s, std::string& errorMsg) {
    lock();
    if (!db) {
        errorMsg = "Database not connected";
        unlock();
        return -1;
    }

    const char* sql = 
        "INSERT INTO students (student_id, first_name, last_name, email, phone, dob, gender, department, year, gpa, status, address) "
        "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)";

    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        errorMsg = sqlite3_errmsg(db);
        unlock();
        return -1;
    }

    sqlite3_bind_text(stmt, 1, s.student_id.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, s.first_name.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, s.last_name.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 4, s.email.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 5, s.phone.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 6, s.dob.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 7, s.gender.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 8, s.department.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 9, s.year.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_double(stmt, 10, s.gpa);
    sqlite3_bind_text(stmt, 11, s.status.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 12, s.address.c_str(), -1, SQLITE_TRANSIENT);

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        errorMsg = sqlite3_errmsg(db);
        if (errorMsg.find("UNIQUE") != std::string::npos) {
            if (errorMsg.find("student_id") != std::string::npos) {
                errorMsg = "A student with this Student ID (" + s.student_id + ") already exists.";
            } else if (errorMsg.find("email") != std::string::npos) {
                errorMsg = "A student with this Email (" + s.email + ") already exists.";
            }
        }
        sqlite3_finalize(stmt);
        unlock();
        return -1;
    }

    int newId = static_cast<int>(sqlite3_last_insert_rowid(db));
    sqlite3_finalize(stmt);
    unlock();
    return newId;
}

bool Database::updateStudent(int id, const Student& s, std::string& errorMsg) {
    lock();
    if (!db) {
        errorMsg = "Database not connected";
        unlock();
        return false;
    }

    const char* sql = 
        "UPDATE students SET student_id = ?, first_name = ?, last_name = ?, email = ?, phone = ?, "
        "dob = ?, gender = ?, department = ?, year = ?, gpa = ?, status = ?, address = ? WHERE id = ?";

    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        errorMsg = sqlite3_errmsg(db);
        unlock();
        return false;
    }

    sqlite3_bind_text(stmt, 1, s.student_id.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, s.first_name.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, s.last_name.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 4, s.email.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 5, s.phone.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 6, s.dob.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 7, s.gender.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 8, s.department.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 9, s.year.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_double(stmt, 10, s.gpa);
    sqlite3_bind_text(stmt, 11, s.status.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 12, s.address.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 13, id);

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        errorMsg = sqlite3_errmsg(db);
        if (errorMsg.find("UNIQUE") != std::string::npos) {
            if (errorMsg.find("student_id") != std::string::npos) {
                errorMsg = "Another student with Student ID (" + s.student_id + ") already exists.";
            } else if (errorMsg.find("email") != std::string::npos) {
                errorMsg = "Another student with Email (" + s.email + ") already exists.";
            }
        }
        sqlite3_finalize(stmt);
        unlock();
        return false;
    }

    sqlite3_finalize(stmt);
    unlock();
    return true;
}

bool Database::deleteStudent(int id, std::string& errorMsg) {
    lock();
    if (!db) {
        errorMsg = "Database not connected";
        unlock();
        return false;
    }

    const char* sql = "DELETE FROM students WHERE id = ?";
    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        errorMsg = sqlite3_errmsg(db);
        unlock();
        return false;
    }

    sqlite3_bind_int(stmt, 1, id);
    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        errorMsg = sqlite3_errmsg(db);
        sqlite3_finalize(stmt);
        unlock();
        return false;
    }

    int changes = sqlite3_changes(db);
    sqlite3_finalize(stmt);
    unlock();

    if (changes == 0) {
        errorMsg = "Student record not found";
        return false;
    }
    return true;
}

DashboardStats Database::getStats() {
    DashboardStats st;
    lock();
    if (!db) {
        unlock();
        return st;
    }

    // Totals and status counts
    const char* sqlTotals = 
        "SELECT "
        "  COUNT(*), "
        "  SUM(CASE WHEN status = 'Active' THEN 1 ELSE 0 END), "
        "  SUM(CASE WHEN status = 'Inactive' THEN 1 ELSE 0 END), "
        "  SUM(CASE WHEN status = 'Graduated' THEN 1 ELSE 0 END), "
        "  AVG(gpa) "
        "FROM students;";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sqlTotals, -1, &stmt, nullptr) == SQLITE_OK) {
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            st.total_students = sqlite3_column_int(stmt, 0);
            st.active_students = sqlite3_column_int(stmt, 1);
            st.inactive_students = sqlite3_column_int(stmt, 2);
            st.graduated_students = sqlite3_column_int(stmt, 3);
            st.average_gpa = sqlite3_column_double(stmt, 4);
        }
        sqlite3_finalize(stmt);
    }

    // Department breakdown
    const char* sqlDept = "SELECT department, COUNT(*) FROM students GROUP BY department;";
    if (sqlite3_prepare_v2(db, sqlDept, -1, &stmt, nullptr) == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            std::string dept = colText(stmt, 0);
            int count = sqlite3_column_int(stmt, 1);
            st.department_counts[dept] = count;
        }
        sqlite3_finalize(stmt);
    }

    unlock();
    return st;
}
