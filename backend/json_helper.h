#ifndef JSON_HELPER_H
#define JSON_HELPER_H

#include <string>
#include <vector>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <cctype>
#include "models.h"

namespace JsonHelper {

inline std::string escape(const std::string& s) {
    std::ostringstream o;
    for (char c : s) {
        switch (c) {
            case '"': o << "\\\""; break;
            case '\\': o << "\\\\"; break;
            case '\b': o << "\\b"; break;
            case '\f': o << "\\f"; break;
            case '\n': o << "\\n"; break;
            case '\r': o << "\\r"; break;
            case '\t': o << "\\t"; break;
            default:
                if (static_cast<unsigned char>(c) <= 0x1f) {
                    o << "\\u" << std::hex << std::setw(4) << std::setfill('0') << static_cast<int>(c);
                } else {
                    o << c;
                }
        }
    }
    return o.str();
}

inline std::string trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) return "";
    size_t last = str.find_last_not_of(" \t\r\n");
    return str.substr(first, (last - first + 1));
}

// Extract string value for a key from JSON object
inline std::string getStringField(const std::string& json, const std::string& key) {
    std::string searchKey = "\"" + key + "\"";
    size_t keyPos = json.find(searchKey);
    if (keyPos == std::string::npos) return "";

    size_t colonPos = json.find(':', keyPos + searchKey.length());
    if (colonPos == std::string::npos) return "";

    size_t valStart = json.find_first_not_of(" \t\r\n", colonPos + 1);
    if (valStart == std::string::npos) return "";

    if (json[valStart] == '"') {
        // String value
        std::string result;
        bool inEscape = false;
        for (size_t i = valStart + 1; i < json.length(); ++i) {
            char c = json[i];
            if (inEscape) {
                switch (c) {
                    case 'n': result += '\n'; break;
                    case 'r': result += '\r'; break;
                    case 't': result += '\t'; break;
                    case '"': result += '"'; break;
                    case '\\': result += '\\'; break;
                    default: result += c; break;
                }
                inEscape = false;
            } else if (c == '\\') {
                inEscape = true;
            } else if (c == '"') {
                break;
            } else {
                result += c;
            }
        }
        return result;
    } else {
        // Unquoted value (number or boolean or null)
        size_t valEnd = json.find_first_of(",}\r\n", valStart);
        if (valEnd == std::string::npos) valEnd = json.length();
        return trim(json.substr(valStart, valEnd - valStart));
    }
}

inline double getDoubleField(const std::string& json, const std::string& key, double defaultVal = 0.0) {
    std::string val = getStringField(json, key);
    if (val.empty() || val == "null") return defaultVal;
    try {
        return std::stod(val);
    } catch (...) {
        return defaultVal;
    }
}

inline int getIntField(const std::string& json, const std::string& key, int defaultVal = 0) {
    std::string val = getStringField(json, key);
    if (val.empty() || val == "null") return defaultVal;
    try {
        return std::stoi(val);
    } catch (...) {
        return defaultVal;
    }
}

inline std::string studentToJson(const Student& s) {
    std::ostringstream ss;
    ss << "{"
       << "\"id\":" << s.id << ","
       << "\"student_id\":\"" << escape(s.student_id) << "\","
       << "\"first_name\":\"" << escape(s.first_name) << "\","
       << "\"last_name\":\"" << escape(s.last_name) << "\","
       << "\"email\":\"" << escape(s.email) << "\","
       << "\"phone\":\"" << escape(s.phone) << "\","
       << "\"dob\":\"" << escape(s.dob) << "\","
       << "\"gender\":\"" << escape(s.gender) << "\","
       << "\"department\":\"" << escape(s.department) << "\","
       << "\"year\":\"" << escape(s.year) << "\","
       << "\"gpa\":" << std::fixed << std::setprecision(2) << s.gpa << ","
       << "\"status\":\"" << escape(s.status) << "\","
       << "\"address\":\"" << escape(s.address) << "\","
       << "\"created_at\":\"" << escape(s.created_at) << "\""
       << "}";
    return ss.str();
}

inline std::string studentsToJson(const std::vector<Student>& students) {
    std::ostringstream ss;
    ss << "[";
    for (size_t i = 0; i < students.size(); ++i) {
        if (i > 0) ss << ",";
        ss << studentToJson(students[i]);
    }
    ss << "]";
    return ss.str();
}

inline std::string statsToJson(const DashboardStats& st) {
    std::ostringstream ss;
    ss << "{"
       << "\"total_students\":" << st.total_students << ","
       << "\"active_students\":" << st.active_students << ","
       << "\"inactive_students\":" << st.inactive_students << ","
       << "\"graduated_students\":" << st.graduated_students << ","
       << "\"average_gpa\":" << std::fixed << std::setprecision(2) << st.average_gpa << ","
       << "\"departments\":{";
    bool first = true;
    for (const auto& pair : st.department_counts) {
        if (!first) ss << ",";
        first = false;
        ss << "\"" << escape(pair.first) << "\":" << pair.second;
    }
    ss << "}}";
    return ss.str();
}

inline std::string successResponse(const std::string& dataJson, const std::string& message = "") {
    std::ostringstream ss;
    ss << "{\"success\":true";
    if (!message.empty()) {
        ss << ",\"message\":\"" << escape(message) << "\"";
    }
    ss << ",\"data\":" << (dataJson.empty() ? "{}" : dataJson) << "}";
    return ss.str();
}

inline std::string errorResponse(const std::string& errorMsg) {
    return "{\"success\":false,\"error\":\"" + escape(errorMsg) + "\"}";
}

inline Student parseStudent(const std::string& json) {
    Student s;
    s.id = getIntField(json, "id", 0);
    s.student_id = getStringField(json, "student_id");
    s.first_name = getStringField(json, "first_name");
    s.last_name = getStringField(json, "last_name");
    s.email = getStringField(json, "email");
    s.phone = getStringField(json, "phone");
    s.dob = getStringField(json, "dob");
    s.gender = getStringField(json, "gender");
    s.department = getStringField(json, "department");
    s.year = getStringField(json, "year");
    s.gpa = getDoubleField(json, "gpa", 0.0);
    s.status = getStringField(json, "status");
    if (s.status.empty()) s.status = "Active";
    s.address = getStringField(json, "address");
    return s;
}

} // namespace JsonHelper

#endif // JSON_HELPER_H
