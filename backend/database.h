#ifndef DATABASE_H
#define DATABASE_H

#include <string>
#include <vector>
#include <windows.h>
#include "../sqlite3.h"
#include "models.h"

class Database {
private:
    sqlite3* db;
    std::string db_path;
    CRITICAL_SECTION cs;

    void lock();
    void unlock();
    void seedSampleData();

public:
    Database(const std::string& path = "students.db");
    ~Database();

    bool init();
    
    // CRUD Operations
    std::vector<Student> getStudents(
        const std::string& search = "",
        const std::string& department = "",
        const std::string& status = "",
        const std::string& sortBy = "id",
        const std::string& sortOrder = "DESC"
    );

    bool getStudentById(int id, Student& student);
    bool getStudentByStudentId(const std::string& studentId, Student& student);
    int addStudent(const Student& student, std::string& errorMsg);
    bool updateStudent(int id, const Student& student, std::string& errorMsg);
    bool deleteStudent(int id, std::string& errorMsg);

    DashboardStats getStats();
};

#endif // DATABASE_H
