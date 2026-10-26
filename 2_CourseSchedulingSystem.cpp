#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <unordered_map>
#include <queue>
#include <algorithm>
#include <sstream>
#include <cctype>

using namespace std;

// ==========================================
// ROBUST INPUT VALIDATION HELPER
// ==========================================
int getValidInt() {
    int value;
    while (true) {
        if (cin >> value) {
            cin.ignore(10000, '\n'); // Clear the newline left in the buffer
            return value;
        } else {
            cout << "[!] Invalid input. Please enter a valid number: ";
            cin.clear();             // Clear the error flag
            cin.ignore(10000, '\n'); // Discard invalid input
        }
    }
}

// ==========================================
// COURSE STRUCTURE
// ==========================================
struct Course {
    string code;
    string name;
    int creditHour;
    int targetSemester; // 1 to 8 (Year 1 Sem 1 to Year 4 Sem 2)
};

// ==========================================
// MAIN SCHEDULING SYSTEM CLASS
// ==========================================
class ITSchedulingSystem {
private:
    unordered_map<string, Course> courses;
    unordered_map<string, vector<string>> graph;         // Prerequisite -> Dependent
    unordered_map<string, vector<string>> prerequisites; // Dependent -> Prerequisite

    const string COURSE_FILE = "courses.txt";
    const string PREREQ_FILE = "prereqs.txt";
    const int MAX_CREDITS_PER_SEM = 22; // Gondar University standard max

    // Helper to split strings for file reading
    vector<string> split(const string& str, char delimiter) {
        vector<string> tokens;
        string token;
        istringstream tokenStream(str);
        while (getline(tokenStream, token, delimiter)) {
            tokens.push_back(token);
        }
        return tokens;
    }

    // Helper to allow case-insensitive course lookups (e.g. math1011 -> MATH1011)
    string getExactCourseCode(const string& inputCode) {
        string lowerInput = inputCode;
        transform(lowerInput.begin(), lowerInput.end(), lowerInput.begin(), ::tolower);
        
        for (const auto& pair : courses) {
            string lowerKey = pair.first;
            transform(lowerKey.begin(), lowerKey.end(), lowerKey.begin(), ::tolower);
            if (lowerInput == lowerKey) {
                return pair.first; 
            }
        }
        return ""; // Not found
    }

public:
    ITSchedulingSystem() {
        loadData();
    }

    // =====================================
    // 1. FILE I/O & DATABASE MANAGEMENT
    // =====================================
    void loadData() {
        ifstream cFile(COURSE_FILE);
        ifstream pFile(PREREQ_FILE);

        // If files don't exist, generate the Gondar IT Curriculum automatically!
        if (!cFile.is_open() || !pFile.is_open()) {
            cout << "\n[!] Database not found. Generating Gondar University IT Curriculum...\n";
            seedGondarData();
            return;
        }

        string line;
        while (getline(cFile, line)) {
            vector<string> data = split(line, ';');
            if (data.size() == 4) {
                Course c = {data[0], data[1], stoi(data[2]), stoi(data[3])};
                courses[c.code] = c;
            }
        }
        cFile.close();

        while (getline(pFile, line)) {
            vector<string> data = split(line, ';');
            if (data.size() == 2) {
                string dependent = data[0];
                string prereq = data[1];
                graph[prereq].push_back(dependent);
                prerequisites[dependent].push_back(prereq);
            }
        }
        pFile.close();
    }

    void saveData() {
        ofstream cFile(COURSE_FILE);
        for (const auto& pair : courses) {
            cFile << pair.second.code << ";" << pair.second.name << ";" 
                  << pair.second.creditHour << ";" << pair.second.targetSemester << "\n";
        }
        cFile.close();

        ofstream pFile(PREREQ_FILE);
        for (const auto& pair : prerequisites) {
            string dependent = pair.first;
            for (const string& prereq : pair.second) {
                pFile << dependent << ";" << prereq << "\n";
            }
        }
        pFile.close();
    }

    // =====================================
    // 2. ADMIN FUNCTIONS
    // =====================================
    void addCourse() {
        Course c;
        cout << "\n*******************************************************\n";
        cout << "                  ADD A NEW COURSE\n";
        cout << "*******************************************************\n";
        cout << "Enter Course Code (e.g., ITec1012): ";
        cin >> c.code;
        cin.ignore(10000, '\n'); // clear buffer before getline
        cout << "Enter Course Name: ";
        getline(cin, c.name);
        cout << "Enter Credit Hours: ";
        c.creditHour = getValidInt();
        cout << "Enter Standard Semester (1 to 8): ";
        c.targetSemester = getValidInt();

        courses[c.code] = c;
        saveData();
        cout << "\n[SUCCESS] Course saved to database!\n";
    }

    void addPrerequisite() {
        string pre, dependent;
        cout << "\n*******************************************************\n";
        cout << "               ADD PREREQUISITE RULE\n";
        cout << "*******************************************************\n";
        cout << "Enter Prerequisite Course Code: ";
        getline(cin, pre);
        cout << "Enter Dependent Course Code: ";
        getline(cin, dependent);

        pre = getExactCourseCode(pre);
        dependent = getExactCourseCode(dependent);

        if (pre == "" || dependent == "") {
            cout << "\n[ERROR] One or both course codes do not exist in the database!\n";
            return;
        }

        graph[pre].push_back(dependent);
        prerequisites[dependent].push_back(pre);
        saveData();
        cout << "\n[SUCCESS] Rule added successfully!\n";
    }

    void displayAllCourses() {
        cout << "\n=======================================================\n";
        cout << "          GONDAR UNIVERSITY IT DEPARTMENT COURSES\n";
        cout << "=======================================================\n";
        for (int i = 1; i <= 8; i++) {
            cout << "\n>>> Year " << ((i-1)/2)+1 << " Semester " << ((i-1)%2)+1 << " <<<\n";
            for (const auto& pair : courses) {
                if (pair.second.targetSemester == i) {
                    cout << "    - " << pair.second.code << " | " << pair.second.name 
                         << " (" << pair.second.creditHour << " Cr)\n";
                }
            }
        }
        cout << "\n";
    }

    bool dfsCycle(const string& node, unordered_map<string, bool>& visited, unordered_map<string, bool>& recStack) {
        visited[node] = true;
        recStack[node] = true;
        for (const string& neighbor : graph[node]) {
            if (!visited[neighbor] && dfsCycle(neighbor, visited, recStack))
                return true;
            else if (recStack[neighbor])
                return true;
        }
        recStack[node] = false;
        return false;
    }

    void detectCycles() {
        cout << "\n*******************************************************\n";
        cout << "               RUNNING CURRICULUM DIAGNOSTIC\n";
        cout << "*******************************************************\n";
        unordered_map<string, bool> visited;
        unordered_map<string, bool> recStack;
        for (const auto& pair : courses) {
            if (!visited[pair.first]) {
                if (dfsCycle(pair.first, visited, recStack)) {
                    cout << "\n[!] ERROR: Cycle Detected! The curriculum has an impossible prerequisite loop.\n";
                    return;
                }
            }
        }
        cout << "\n[OK] Curriculum is Valid. No infinite loops detected.\n";
    }

    // =====================================
    // 3. STUDENT FUNCTIONS
    // =====================================
    void generateStudentSchedule() {
        int year, sem;
        cout << "\n*******************************************************\n";
        cout << "              ACADEMIC PROGRESS ENTRY\n";
        cout << "*******************************************************\n";
        cout << "Enter the highest Year you have COMPLETED (e.g., 1, 2, 3).\n";
        cout << "(Enter 0 if you are a new freshman): ";
        year = getValidInt();

        if (year >= 4) {
            cout << "\nCongratulations! You have already graduated.\n";
            return;
        }

        cout << "Enter the Semester you have COMPLETED (1 or 2).\n";
        cout << "(Enter 0 if you are a new freshman): ";
        sem = getValidInt();

        // Calculate absolute semester index (0 to 8)
        int completedSemNum = (year == 0) ? 0 : ((year - 1) * 2 + sem);
        int nextSemNum = completedSemNum + 1;

        // --- HANDLE FAILED COURSES ---
        vector<string> failedCourses;
        char failedChoice;
        cout << "\nDid you fail or drop any courses in the past? (y/n): ";
        cin >> failedChoice;
        cin.ignore(10000, '\n'); // clean buffer
        
        if (tolower(failedChoice) == 'y') {
            cout << "How many courses did you fail? ";
            int numFailed = getValidInt();
            
            for (int i = 0; i < numFailed; i++) {
                string fCode;
                cout << "  Enter failed course code " << (i+1) << ": ";
                getline(cin, fCode);
                
                string exactCode = getExactCourseCode(fCode);
                if(exactCode != "") {
                    failedCourses.push_back(exactCode);
                } else {
                    cout << "  [!] Course not found. Skipping...\n";
                }
            }
        }

        // --- FILTER ELIGIBILITY (GRAPH LOGIC) ---
        vector<string> completed;
        
        // 1. Auto-tag past courses as completed (unless failed)
        for (const auto& pair : courses) {
            string code = pair.first;
            int target = pair.second.targetSemester;
            
            if (target <= completedSemNum && find(failedCourses.begin(), failedCourses.end(), code) == failedCourses.end()) {
                completed.push_back(code);
            }
        }

        // 2. Calculate In-Degrees (Unmet Prerequisites) based on completed list
        unordered_map<string, int> indegree;
        for (const auto& pair : courses) {
            string code = pair.first;
            
            // Skip if already passed
            if (find(completed.begin(), completed.end(), code) != completed.end()) continue;
            
            int count = 0;
            for (const string& pre : prerequisites[code]) {
                if (find(completed.begin(), completed.end(), pre) == completed.end()) {
                    count++; // Prerequisite not yet passed
                }
            }
            indegree[code] = count;
        }

        // 3. Find Available Courses (indegree == 0 means all prereqs met)
        vector<string> available;
        for (const auto& pair : indegree) {
            if (pair.second == 0) {
                available.push_back(pair.first);
            }
        }

        // --- PREPARE DISPLAY ---
        vector<int> targetSemsToDisplay;
        if (completedSemNum == 6) {
            targetSemsToDisplay = {7, 8}; // Look-ahead rule for final year
        } else {
            targetSemsToDisplay = {nextSemNum};
        }

        cout << "\n\n=======================================================\n";
        cout << "            YOUR NEXT-STEP GRADUATION PATHWAY\n";
        cout << "=======================================================\n";
        
        string statusStr = (year == 0) ? "Freshman Enrolling" : "Year " + to_string(year) + ", Semester " + to_string(sem) + " Completed";
        cout << "[ACADEMIC STATUS: " << statusStr << "]\n\n";

        for (int displaySem : targetSemsToDisplay) {
            cout << ">>> Recommended Study: Year " << ((displaySem-1)/2)+1 << " Semester " << ((displaySem-1)%2)+1 << " <<<\n\n";
            int currentCredits = 0;
            int stepCount = 1; // Counter for clear output steps

            // Collect courses to show for this semester block
            vector<string> toPrint;
            
            for (const string& code : available) {
                bool isFailed = (find(failedCourses.begin(), failedCourses.end(), code) != failedCourses.end());
                
                // If it's a failed course, FORCE it into the very next semester block
                if (isFailed && displaySem == targetSemsToDisplay[0]) {
                    toPrint.push_back(code);
                } 
                // Otherwise, add if it naturally belongs to this semester
                else if (!isFailed && courses[code].targetSemester == displaySem) {
                    toPrint.push_back(code);
                }
            }

            // Sort them: Failed first, then by standard curriculum order
            sort(toPrint.begin(), toPrint.end(), [&](const string& a, const string& b) {
                return courses[a].targetSemester < courses[b].targetSemester;
            });

            // Print them checking the credit limits
            for (const string& code : toPrint) {
                if (currentCredits + courses[code].creditHour <= MAX_CREDITS_PER_SEM) {
                    currentCredits += courses[code].creditHour; // We still calculate it to enforce the rule
                    bool isFailed = (find(failedCourses.begin(), failedCourses.end(), code) != failedCourses.end());
                    
                    cout << "   Step " << stepCount++ << " ->  " << code << " | " << courses[code].name << " (" << courses[code].creditHour << " Cr)";
                    if (isFailed) cout << "  [URGENT RETAKE]";
                    cout << "\n";
                }
            }

            if (toPrint.empty()) {
                cout << "   (No available courses to schedule. Check prerequisites!)\n";
            }
            cout << "\n";
        }

        cout << "-------------------------------------------------------\n";
        if (failedCourses.empty()) {
            cout << "Note: You are currently on track for your degree.\n";
        } else {
            cout << "Note: Prioritize retaking failed courses to prevent prerequisite blocks later.\n";
        }
        cout << "=======================================================\n";
    }

    // =====================================
    // 4. AUTO-GENERATOR (Gondar Data)
    // =====================================
    void seedCourse(string code, string name, int cr, int sem) {
        courses[code] = {code, name, cr, sem};
    }
    void seedPrereq(string dependent, string pre) {
        graph[pre].push_back(dependent);
        prerequisites[dependent].push_back(pre);
    }

    void seedGondarData() {
        courses.clear(); graph.clear(); prerequisites.clear();
        
        // Y1 S1
        seedCourse("MATH1011", "Mathematics for Natural Sciences", 3, 1);
        seedCourse("FLEn1011", "Communicative English Language I", 3, 1);
        seedCourse("Phys1011", "General Physics", 3, 1);
        seedCourse("Psch1011", "General Psychology", 3, 1);
        seedCourse("LoCT1011", "Critical Thinking", 3, 1);
        seedCourse("SPSC1011", "Physical Fitness", 2, 1);
        seedCourse("GeES1011", "Geography of Ethiopia", 3, 1);
        seedCourse("Phil1011", "Intro to Logic", 3, 1);

        // Y1 S2
        seedCourse("FLEn1012", "Communicative English Language II", 3, 2);
        seedCourse("Anth1012", "Social Anthropology", 2, 2);
        seedCourse("MATH1041", "Applied Mathematics I", 3, 2);
        seedCourse("EmTe1012", "Emerging Technologies", 3, 2);
        seedCourse("MCiE1012", "Moral and Civic Education", 2, 2);
        seedCourse("CoSc1012", "Computer Programming", 3, 2);
        seedCourse("MGMT1012", "Entrepreneurship", 3, 2);

        // Y2 S1
        seedCourse("Gltr2015", "Global Trends", 2, 3);
        seedCourse("Incl2011", "Inclusiveness", 2, 3);
        seedCourse("Eco2013", "Economics", 3, 3);
        seedCourse("ITec2041", "Fundamentals of Programming II", 3, 3);
        seedCourse("ITEC2071", "Fundamentals of Database Systems", 3, 3);
        seedCourse("Stat2171", "Introduction to Statistics", 3, 3);
        seedCourse("Eeng2161", "Fundamentals of Electricity", 3, 3);

        // Y2 S2
        seedCourse("ITec2022", "Operating Systems", 3, 4);
        seedCourse("ITec2024", "Computer Organization and Arch", 3, 4);
        seedCourse("ITec2102", "Data Communication and Networks", 3, 4);
        seedCourse("ITec2052", "Data structure and Algorithms", 3, 4);
        seedCourse("Math-2182", "Discrete Mathematics", 3, 4);
        seedCourse("ITec2092", "Internet Programming I", 3, 4);

        // Y3 S1
        seedCourse("ITec3061", "System Analysis and Design", 3, 5);
        seedCourse("ITEC3121", "Multimedia Systems", 3, 5);
        seedCourse("ITec3051", "Object Oriented Programming Java", 3, 5);
        seedCourse("ITec3093", "Internet Programming II", 3, 5);
        seedCourse("ITec3071", "Advanced Database Systems", 3, 5);
        seedCourse("ITEC3031", "Computer Maintenance", 4, 5);
        
        // Y3 S2
        seedCourse("ITEC3102", "Intro to Distributed Systems", 3, 6);
        seedCourse("ITec3062", "IT Project Management", 3, 6);
        seedCourse("ITEC3082", "Information Storage and Retrieval", 3, 6);
        seedCourse("ITec3054", "Event-Driven Programming", 3, 6);
        seedCourse("ITec3058", "Advanced Programming", 4, 6);
        seedCourse("ITec3056", "Mobile Application Development", 3, 6);

        // Y4 S1
        seedCourse("ITec4121", "Artificial Intelligence", 3, 7);
        seedCourse("ITEC4133", "Information Assurance and Security", 3, 7);
        seedCourse("ITec4153A", "Final year Project I", 3, 7); 
        seedCourse("ITEC4111", "Network Design", 3, 7);

        // Y4 S2
        seedCourse("ITEC4112", "System and Network Administration", 3, 8);
        seedCourse("ITEC4113", "Network Device and Configuration", 3, 8);
        seedCourse("ITEC3104", "Wireless Networking", 3, 8);

        // Key Pre-requisite Logic
        seedPrereq("ITec2041", "CoSc1012");
        seedPrereq("ITec2022", "ITec2024");
        seedPrereq("ITec2052", "ITec2041");
        seedPrereq("ITec3051", "ITec2041");
        seedPrereq("ITec3093", "ITec2092");
        seedPrereq("ITec3071", "ITEC2071");
        seedPrereq("ITEC3031", "ITec2022");
        seedPrereq("ITEC3102", "ITec2102");
        seedPrereq("ITec3054", "ITec2052");
        seedPrereq("ITec3058", "ITec3051");
        seedPrereq("ITec3056", "ITec2102");
        seedPrereq("ITEC4133", "ITec2102");
        seedPrereq("ITEC4133", "ITec2022");
        seedPrereq("ITec4153A", "ITec3062");
        seedPrereq("ITEC4111", "ITec2102");
        seedPrereq("ITEC4112", "ITec2102");
        seedPrereq("ITEC4113", "ITEC4111");
        seedPrereq("ITEC3104", "ITec2102");

        saveData();
        cout << "[OK] Gondar University IT Curriculum loaded successfully.\n";
    }
};

// ==========================================
// MAIN UI AND MENUS
// ==========================================
int main() {
    ITSchedulingSystem system;
    int roleChoice;

    do {
        cout << "\n=======================================================\n";
        cout << "      GONDAR UNIVERSITY IT COURSE SCHEDULING SYSTEM\n";
        cout << "=======================================================\n";
        cout << "  1. Login as Admin / Registrar\n";
        cout << "  2. Login as Student\n";
        cout << "  0. Exit System\n";
        cout << "-------------------------------------------------------\n";
        cout << "Select Role: ";
        roleChoice = getValidInt();

        if (roleChoice == 1) {
            string pass;
            cout << "Enter Admin Password (default is 'admin'): ";
            cin >> pass;
            cin.ignore(10000, '\n'); // clean buffer

            if (pass == "admin") {
                int adminChoice;
                do {
                    cout << "\n=======================================================\n";
                    cout << "                  [ ADMIN DASHBOARD ]\n";
                    cout << "=======================================================\n";
                    cout << "  1. View All Curriculum Courses\n";
                    cout << "  2. Add New Course\n";
                    cout << "  3. Add Course Prerequisite Rule\n";
                    cout << "  4. Verify Graph Validity (Detect Cycles)\n";
                    cout << "  0. Logout\n";
                    cout << "-------------------------------------------------------\n";
                    cout << "Choice: ";
                    adminChoice = getValidInt();

                    switch (adminChoice) {
                        case 1: system.displayAllCourses(); break;
                        case 2: system.addCourse(); break;
                        case 3: system.addPrerequisite(); break;
                        case 4: system.detectCycles(); break;
                        case 0: cout << "Logging out...\n"; break;
                        default: cout << "[!] Invalid Choice!\n";
                    }
                } while (adminChoice != 0);
            } else {
                cout << "\n[!] Incorrect Password.\n";
            }
        } 
        else if (roleChoice == 2) {
            int studentChoice;
            do {
                cout << "\n=======================================================\n";
                cout << "                 [ STUDENT DASHBOARD ]\n";
                cout << "=======================================================\n";
                cout << "  1. View My Next-Step Academic Plan\n";
                cout << "  2. View Department Course List\n";
                cout << "  0. Logout\n";
                cout << "-------------------------------------------------------\n";
                cout << "Choice: ";
                studentChoice = getValidInt();

                switch (studentChoice) {
                    case 1: system.generateStudentSchedule(); break;
                    case 2: system.displayAllCourses(); break;
                    case 0: cout << "Logging out...\n"; break;
                    default: cout << "[!] Invalid Choice!\n";
                }
            } while (studentChoice != 0);
        }

    } while (roleChoice != 0);

    return 0;
}