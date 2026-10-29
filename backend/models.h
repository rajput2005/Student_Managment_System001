#ifndef MODELS_H
#define MODELS_H

#include <string>
#include <vector>
#include <map>

struct Student {
    int id;
    std::string student_id;
    std::string first_name;
    std::string last_name;
    std::string email;
    std::string phone;
    std::string dob;
    std::string gender;
    std::string department;
    std::string year;
    double gpa;
    std::string status;
    std::string address;
    std::string created_at;

    Student() : id(0), gpa(0.0) {}
};

struct DashboardStats {
    int total_students;
    int active_students;
    int inactive_students;
    int graduated_students;
    double average_gpa;
    std::map<std::string, int> department_counts;

    DashboardStats() : total_students(0), active_students(0), inactive_students(0), graduated_students(0), average_gpa(0.0) {}
};

#endif // MODELS_H
