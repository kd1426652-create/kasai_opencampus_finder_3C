#define NOMINMAX

#include "DxLib.h"
#include <windows.h>
#include <commdlg.h>

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <cstring>
#include <cstdio>
#include <algorithm>

#pragma comment(lib, "Comdlg32.lib")

using namespace std;

// ============================================================================
// ?? (?) - Personal One: System Setup, Global Variables & Window Settings
// ????????? Member 1 (System Setup, Global Variables & Main Loop)
// ============================================================================

const int SW = 900;
const int SH = 780;

const char* DATA_FILE = "schools.txt";

const int ITEMS_PER_PAGE = 4;

struct School
{
    string name;
    string location;
    string course;
    string openDate;
    string rating;
    string description;

    string imagePath;
    int imageHandle = -1;
};

vector<School> schools;
vector<int> filteredIndices;

enum Page
{
    LOGIN_PAGE,
    ADMIN_PAGE,
    STUDENT_PAGE,
    SCHOOL_FORM_PAGE,
    SCHOOL_DETAIL_PAGE
};

Page currentPage = LOGIN_PAGE;
Page formReturnPage = ADMIN_PAGE;

bool isAdmin = false;
bool isEditMode = false;

int selectedSchool = -1;
int activeField = 0;
int listPage = 0;

// Search Data (?? (?) ???????????)
string searchQuery = "";
bool searchFieldActive = false;

// Form Data (?? (?) ???????????)
string formFields[6];
string fieldNames[6] =
{
    "School Name",
    "Location",
    "Courses",
    "Open Campus Date",
    "Rating",
    "Description"
};
string formImagePath = "";
string formOriginalImagePath = "";
int formImageHandle = -1;

// Login Data (?? (?) ???????????)
string email = "";
string password = "";
int loginField = 0;
string loginMessage = "";

// ============================================================================
// ?? (?) - Personal One: UI Components & Utility Functions
// ?????????: Member 2 (Mouse System, Button UI, Text Wrapping & Image Drawing)
// ============================================================================

bool mouseClicked = false;

void UpdateMouse()
{
    static int previousMouse = 0;

    int mouse = GetMouseInput();

    mouseClicked =
        (mouse & MOUSE_INPUT_LEFT) &&
        !(previousMouse & MOUSE_INPUT_LEFT);

    previousMouse = mouse;
}

bool IsInside(int x, int y, int w, int h)
{
    int mx, my;

    GetMousePoint(&mx, &my);

    return mx >= x && mx <= x + w &&
        my >= y && my <= y + h;
}

bool Button(int x, int y, int w, int h, const char* text)
{
    bool hover = IsInside(x, y, w, h);

    int color = hover
        ? GetColor(65, 115, 180)
        : GetColor(40, 75, 125);

    DrawBox(
        x, y,
        x + w, y + h,
        color,
        TRUE
    );

    DrawBox(
        x, y,
        x + w, y + h,
        GetColor(85, 110, 145),
        FALSE
    );

    int textWidth =
        GetDrawStringWidth(text, (int)strlen(text));

    DrawString(
        x + (w - textWidth) / 2,
        y + (h - 16) / 2,
        text,
        GetColor(255, 255, 255)
    );

    return hover && mouseClicked;
}

void DrawAppText(int x, int y, const string& text)
{
    DrawString(
        x, y,
        text.c_str(),
        GetColor(235, 238, 245)
    );
}

void DrawWrappedText(
    int x,
    int y,
    const string& text,
    int maxWidth,
    int lineHeight = 23
)
{
    stringstream ss(text);

    string word;
    string line;

    int currentY = y;

    while (ss >> word)
    {
        string test = line.empty()
            ? word
            : line + " " + word;

        int width = GetDrawStringWidth(
            test.c_str(),
            (int)test.length()
        );

        if (width > maxWidth && !line.empty())
        {
            DrawAppText(x, currentY, line);

            currentY += lineHeight;
            line = word;
        }
        else
        {
            line = test;
        }
    }

    if (!line.empty())
        DrawAppText(x, currentY, line);
}

void DrawSchoolImage(
    int x,
    int y,
    int w,
    int h,
    int handle
)
{
    DrawBox(
        x, y,
        x + w, y + h,
        GetColor(45, 52, 70),
        TRUE
    );

    if (handle != -1)
    {
        DrawExtendGraph(
            x, y,
            x + w, y + h,
            handle,
            TRUE
        );
    }
    else
    {
        const char* text = "NO IMAGE";

        int textWidth =
            GetDrawStringWidth(text, (int)strlen(text));

        DrawString(
            x + (w - textWidth) / 2,
            y + h / 2 - 8,
            text,
            GetColor(150, 155, 170)
        );
    }

    DrawBox(
        x, y,
        x + w, y + h,
        GetColor(90, 100, 120),
        FALSE
    );
}

void ReleaseSchoolImages()
{
    for (int i = 0; i < (int)schools.size(); i++)
    {
        if (schools[i].imageHandle != -1)
        {
            DeleteGraph(schools[i].imageHandle);

            schools[i].imageHandle = -1;
        }
    }
}

void ReleaseFormImage()
{
    if (formImageHandle != -1)
    {
        DeleteGraph(formImageHandle);
        formImageHandle = -1;
    }
}

// ============================================================================
// ?? (?) - Personal One (????): Background & Text Cleaning
// ?????????: Member 1
// ============================================================================

void DrawBackground()
{
    DrawBox(
        0, 0, SW, SH,
        GetColor(20, 25, 40),
        TRUE
    );

    DrawString(
        30, 25,
        "KANSAI IT SCHOOL MANAGEMENT SYSTEM",
        GetColor(255, 255, 255)
    );

    DrawLine(
        30, 60,
        870, 60,
        GetColor(75, 90, 115)
    );
}

void DrawPageTitle(const char* title)
{
    DrawString(
        330, 85,
        title,
        GetColor(255, 210, 110)
    );
}

string CleanText(string value)
{
    for (int i = 0; i < (int)value.length(); i++)
    {
        if (value[i] == '|')
            value[i] = '/';

        if (value[i] == '\n' || value[i] == '\r')
            value[i] = ' ';
    }

    return value;
}

// ============================================================================
// ?? (?) - Personal One: Data Management & File I/O (File & Image Picker)
// ?????????: Member 3 (Save/Load Schools, Filter & Windows Image Picker)
// ============================================================================

bool SaveSchools()
{
    ofstream file(DATA_FILE);

    if (!file.is_open())
        return false;

    for (int i = 0; i < (int)schools.size(); i++)
    {
        file
            << CleanText(schools[i].name) << "|"
            << CleanText(schools[i].location) << "|"
            << CleanText(schools[i].course) << "|"
            << CleanText(schools[i].openDate) << "|"
            << CleanText(schools[i].rating) << "|"
            << CleanText(schools[i].description) << "|"
            << CleanText(schools[i].imagePath)
            << endl;
    }

    file.close();

    return true;
}

void UpdateFilter()
{
    filteredIndices.clear();

    string queryLower = searchQuery;
    transform(queryLower.begin(), queryLower.end(), queryLower.begin(), ::tolower);

    for (int i = 0; i < (int)schools.size(); i++)
    {
        string nameLower = schools[i].name;
        string locLower = schools[i].location;
        string courseLower = schools[i].course;

        transform(nameLower.begin(), nameLower.end(), nameLower.begin(), ::tolower);
        transform(locLower.begin(), locLower.end(), locLower.begin(), ::tolower);
        transform(courseLower.begin(), courseLower.end(), courseLower.begin(), ::tolower);

        if (queryLower.empty() ||
            nameLower.find(queryLower) != string::npos ||
            locLower.find(queryLower) != string::npos ||
            courseLower.find(queryLower) != string::npos)
        {
            filteredIndices.push_back(i);
        }
    }
}

void LoadSchools()
{
    schools.clear();

    ifstream file(DATA_FILE);

    if (!file.is_open())
    {
        School s1;

        s1.name = "ECC Computer Academy";
        s1.location = "Osaka Umeda";
        s1.course = "Programming, AI, Game Development";
        s1.openDate = "2026/11/15";
        s1.rating = "4.8";
        s1.description = "IT and Programming School";
        s1.imagePath = "";

        schools.push_back(s1);

        School s2;

        s2.name = "HAL Osaka";
        s2.location = "Near Osaka Station";
        s2.course = "IT Engineering, Cyber Security";
        s2.openDate = "2026/11/22";
        s2.rating = "4.7";
        s2.description = "Professional IT College";
        s2.imagePath = "";

        schools.push_back(s2);

        SaveSchools();
    }
    else
    {
        string line;

        while (getline(file, line))
        {
            stringstream ss(line);

            School s;

            getline(ss, s.name, '|');
            getline(ss, s.location, '|');
            getline(ss, s.course, '|');
            getline(ss, s.openDate, '|');
            getline(ss, s.rating, '|');
            getline(ss, s.description, '|');
            getline(ss, s.imagePath, '|');

            s.imageHandle = -1;

            if (!s.name.empty())
                schools.push_back(s);
        }

        file.close();
    }

    UpdateFilter();
}

void LoadSchoolImages()
{
    for (int i = 0; i < (int)schools.size(); i++)
    {
        if (!schools[i].imagePath.empty())
        {
            schools[i].imageHandle =
                LoadGraph(schools[i].imagePath.c_str());
        }
    }
}

bool IsValidImageExtension(const char* path)
{
    const char* ext = strrchr(path, '.');

    if (ext == NULL)
        return false;

    return
        _stricmp(ext, ".jpg") == 0 ||
        _stricmp(ext, ".jpeg") == 0 ||
        _stricmp(ext, ".png") == 0;
}

bool ChooseSchoolImage()
{
    OPENFILENAMEA ofn = {};

    char fileName[MAX_PATH] = "";

    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = GetActiveWindow();

    ofn.lpstrFile = fileName;
    ofn.nMaxFile = MAX_PATH;

    ofn.lpstrFilter =
        "Image Files (*.jpg;*.jpeg;*.png)\0"
        "*.jpg;*.jpeg;*.png\0"
        "All Files (*.*)\0"
        "*.*\0";

    ofn.nFilterIndex = 1;

    ofn.Flags =
        OFN_FILEMUSTEXIST |
        OFN_PATHMUSTEXIST |
        OFN_NOCHANGEDIR;

    ofn.lpstrTitle = "Select School Image";

    if (!GetOpenFileNameA(&ofn))
        return false;

    if (!IsValidImageExtension(fileName))
    {
        loginMessage = "Invalid image format!";
        return false;
    }

    CreateDirectoryA("images", NULL);

    static unsigned int counter = 0;

    const char* ext = strrchr(fileName, '.');

    char destination[MAX_PATH];

    bool copied = false;

    for (int i = 0; i < 100; i++)
    {
        counter++;

        sprintf_s(
            destination,
            "images\\school_%llu_%u%s",
            GetTickCount64(),
            counter,
            ext
        );

        if (CopyFileA(fileName, destination, TRUE))
        {
            copied = true;
            break;
        }
    }

    if (!copied)
    {
        loginMessage = "Cannot copy image!";
        return false;
    }

    if (!formImagePath.empty() &&
        formImagePath != formOriginalImagePath)
    {
        DeleteFileA(formImagePath.c_str());
    }

    ReleaseFormImage();

    formImagePath = destination;

    formImageHandle =
        LoadGraph(formImagePath.c_str());

    if (formImageHandle == -1)
    {
        DeleteFileA(formImagePath.c_str());

        formImagePath = formOriginalImagePath;

        if (!formImagePath.empty())
        {
            formImageHandle =
                LoadGraph(formImagePath.c_str());
        }

        loginMessage = "Cannot load image!";

        return false;
    }

    loginMessage = "Image selected successfully!";

    return true;
}

// ============================================================================
// ?? (?) - Personal One: Pagination & List Control
// ?????????: Member 6 (Student View, Search, List & Detail Page)
// ============================================================================

int GetTotalPages()
{
    if (filteredIndices.empty())
        return 1;

    return ((int)filteredIndices.size() + ITEMS_PER_PAGE - 1)
        / ITEMS_PER_PAGE;
}

void NormalizeListPage()
{
    int totalPages = GetTotalPages();

    if (listPage < 0)
        listPage = 0;

    if (listPage >= totalPages)
        listPage = totalPages - 1;
}

// ============================================================================
// ?? (?) - Personal One: Admin Dashboard & School Form Management (CRUD)
// ?????????: Member 5 (Add, Edit, Delete Schools & Admin Dashboard)
// ============================================================================

void StartAddSchool()
{
    isEditMode = false;

    selectedSchool = -1;

    for (int i = 0; i < 6; i++)
        formFields[i] = "";

    ReleaseFormImage();

    formImagePath = "";
    formOriginalImagePath = "";

    activeField = 0;

    formReturnPage = ADMIN_PAGE;

    loginMessage = "";

    currentPage = SCHOOL_FORM_PAGE;
}

void StartEditSchool()
{
    if (selectedSchool < 0 ||
        selectedSchool >= (int)schools.size())
        return;

    isEditMode = true;

    School& s = schools[selectedSchool];

    formFields[0] = s.name;
    formFields[1] = s.location;
    formFields[2] = s.course;
    formFields[3] = s.openDate;
    formFields[4] = s.rating;
    formFields[5] = s.description;

    ReleaseFormImage();

    formOriginalImagePath = s.imagePath;
    formImagePath = s.imagePath;

    if (!formImagePath.empty())
    {
        formImageHandle =
            LoadGraph(formImagePath.c_str());
    }

    activeField = 0;

    formReturnPage = SCHOOL_DETAIL_PAGE;

    loginMessage = "";

    currentPage = SCHOOL_FORM_PAGE;
}

void CancelSchoolForm()
{
    ReleaseFormImage();

    if (!formImagePath.empty() &&
        formImagePath != formOriginalImagePath)
    {
        DeleteFileA(formImagePath.c_str());
    }

    formImagePath = "";
    formOriginalImagePath = "";

    currentPage = formReturnPage;
}

void SaveSchoolForm()
{
    if (formFields[0].empty())
    {
        loginMessage = "School Name is required!";
        return;
    }

    School s;

    s.name = formFields[0];
    s.location = formFields[1];
    s.course = formFields[2];
    s.openDate = formFields[3];
    s.rating = formFields[4];
    s.description = formFields[5];

    s.imagePath = formImagePath;
    s.imageHandle = -1;

    int index = -1;

    if (isEditMode &&
        selectedSchool >= 0 &&
        selectedSchool < (int)schools.size())
    {
        School& oldSchool = schools[selectedSchool];

        string oldPath = oldSchool.imagePath;

        if (oldSchool.imageHandle != -1)
        {
            DeleteGraph(oldSchool.imageHandle);
        }

        if (!oldPath.empty() &&
            oldPath != s.imagePath)
        {
            DeleteFileA(oldPath.c_str());
        }

        schools[selectedSchool] = s;

        index = selectedSchool;
    }
    else
    {
        schools.push_back(s);

        index = (int)schools.size() - 1;
    }

    schools[index].imageHandle = formImageHandle;

    formImageHandle = -1;

    formImagePath = "";
    formOriginalImagePath = "";

    if (!SaveSchools())
    {
        loginMessage = "Cannot save schools.txt!";
        return;
    }

    UpdateFilter();

    currentPage = formReturnPage;
}

void DeleteSelectedSchool()
{
    if (selectedSchool < 0 ||
        selectedSchool >= (int)schools.size())
        return;

    School& s = schools[selectedSchool];

    string oldPath = s.imagePath;

    if (s.imageHandle != -1)
    {
        DeleteGraph(s.imageHandle);
    }

    schools.erase(
        schools.begin() + selectedSchool
    );

    if (!oldPath.empty())
        DeleteFileA(oldPath.c_str());

    selectedSchool = -1;

    UpdateFilter();
    NormalizeListPage();

    SaveSchools();

    currentPage = STUDENT_PAGE;
}

// ============================================================================
// ?? (?) - Personal One: Authentication & Login System
// ?????????: Member 4 (Login/Logout, Input Handling & Login UI)
// ============================================================================

void TryLogin()
{
    if (email == "admin" && password == "admin")
    {
        isAdmin = true;

        selectedSchool = -1;
        listPage = 0;
        searchQuery = "";
        UpdateFilter();

        loginMessage = "";

        currentPage = ADMIN_PAGE;
    }
    else if (email == "student" && password == "student")
    {
        isAdmin = false;

        selectedSchool = -1;
        listPage = 0;
        searchQuery = "";
        UpdateFilter();

        loginMessage = "";

        currentPage = STUDENT_PAGE;
    }
    else
    {
        loginMessage = "Invalid Username or Password!";
    }
}

void Logout()
{
    email = "";
    password = "";

    loginField = 0;

    selectedSchool = -1;
    listPage = 0;
    searchQuery = "";
    UpdateFilter();

    isAdmin = false;

    loginMessage = "";

    currentPage = LOGIN_PAGE;
}

void HandleLoginInput()
{
    int mx, my;

    GetMousePoint(&mx, &my);

    if (mouseClicked)
    {
        if (mx >= 270 && mx <= 630 &&
            my >= 260 && my <= 305)
        {
            loginField = 0;
        }

        if (mx >= 270 && mx <= 630 &&
            my >= 370 && my <= 415)
        {
            loginField = 1;
        }
    }

    int ch;

    while ((ch = GetInputChar(TRUE)) != 0)
    {
        if (ch >= 32 && ch <= 126)
        {
            if (loginField == 0 && email.length() < 60)
                email += (char)ch;

            if (loginField == 1 && password.length() < 40)
                password += (char)ch;
        }
    }

    static bool previousBack = false;

    bool back = CheckHitKey(KEY_INPUT_BACK);

    if (back && !previousBack)
    {
        if (loginField == 0 && !email.empty())
            email.pop_back();

        if (loginField == 1 && !password.empty())
            password.pop_back();
    }

    previousBack = back;

    static bool previousEnter = false;

    bool enter = CheckHitKey(KEY_INPUT_RETURN);

    if (enter && !previousEnter)
        TryLogin();

    previousEnter = enter;
}

void DrawLoginPage()
{
    DrawBackground();

    DrawString(
        375, 125,
        "USER LOGIN",
        GetColor(255, 215, 120)
    );

    DrawString(
        270, 230,
        "Username:",
        GetColor(255, 255, 255)
    );

    DrawBox(
        270, 260,
        630, 305,
        loginField == 0
        ? GetColor(65, 80, 110)
        : GetColor(40, 45, 65),
        TRUE
    );

    DrawAppText(285, 275, email);

    DrawString(
        270, 340,
        "Password:",
        GetColor(255, 255, 255)
    );

    DrawBox(
        270, 370,
        630, 415,
        loginField == 1
        ? GetColor(65, 80, 110)
        : GetColor(40, 45, 65),
        TRUE
    );

    string hiddenPassword(password.length(), '*');

    DrawAppText(285, 385, hiddenPassword);

    if (Button(350, 480, 200, 50, "LOGIN"))
    {
        TryLogin();
    }

    DrawString(
        260, 570,
        "ADMIN   : admin / admin",
        GetColor(180, 190, 210)
    );

    DrawString(
        260, 600,
        "STUDENT : student / student",
        GetColor(180, 190, 210)
    );

    DrawString(
        260, 650,
        "Click username or password box and type.",
        GetColor(150, 160, 180)
    );

    if (!loginMessage.empty())
    {
        DrawAppText(270, 700, loginMessage);
    }
}

// ============================================================================
// ?? (?) - Personal One (????): Admin Dashboard UI
// ?????????: Member 5
// ============================================================================

void DrawAdminPage()
{
    DrawBackground();

    DrawPageTitle("ADMIN DASHBOARD");

    DrawFormatString(
        100, 180,
        GetColor(255, 255, 255),
        "Total Schools: %d",
        (int)schools.size()
    );

    if (Button(100, 260, 250, 55, "ADD NEW SCHOOL"))
    {
        StartAddSchool();
        return;
    }

    if (Button(100, 340, 250, 55, "MANAGE SCHOOLS"))
    {
        selectedSchool = -1;
        listPage = 0;
        searchQuery = "";
        UpdateFilter();

        currentPage = STUDENT_PAGE;
        return;
    }

    if (Button(100, 420, 250, 55, "LOGOUT"))
    {
        Logout();
        return;
    }

    DrawString(
        430, 280,
        "ADMIN FUNCTIONS",
        GetColor(255, 210, 110)
    );

    DrawString(
        430, 320,
        "- Add new schools",
        GetColor(220, 220, 220)
    );

    DrawString(
        430, 355,
        "- Edit school information",
        GetColor(220, 220, 220)
    );

    DrawString(
        430, 390,
        "- Delete schools",
        GetColor(220, 220, 220)
    );

    DrawString(
        430, 425,
        "- Upload school images",
        GetColor(220, 220, 220)
    );

    DrawString(
        430, 460,
        "- View school details",
        GetColor(220, 220, 220)
    );
}

// ============================================================================
// ?? (?) - Personal One (????): Search Bar, School List & Detail Page
// ?????????: Member 6
// ============================================================================

void HandleSearchInput()
{
    int mx, my;
    GetMousePoint(&mx, &my);

    if (mouseClicked)
    {
        searchFieldActive = (mx >= 50 && mx <= 450 && my >= 110 && my <= 145);
    }

    if (!searchFieldActive) return;

    int ch;
    while ((ch = GetInputChar(TRUE)) != 0)
    {
        if (ch >= 32 && ch <= 126)
        {
            if (searchQuery.length() < 40)
            {
                searchQuery += (char)ch;
                listPage = 0;
                UpdateFilter();
            }
        }
    }

    static bool previousBack = false;
    bool back = CheckHitKey(KEY_INPUT_BACK);

    if (back && !previousBack)
    {
        if (!searchQuery.empty())
        {
            searchQuery.pop_back();
            listPage = 0;
            UpdateFilter();
        }
    }

    previousBack = back;
}

void DrawSchoolList()
{
    DrawBackground();

    DrawPageTitle(
        isAdmin ? "MANAGE SCHOOLS" : "SCHOOL SEARCH"
    );

    HandleSearchInput();

    // Search Box UI
    DrawString(50, 90, "Search (Name, Location, Course):", GetColor(200, 210, 230));
    DrawBox(
        50, 110,
        450, 145,
        searchFieldActive ? GetColor(65, 80, 110) : GetColor(35, 43, 60),
        TRUE
    );
    DrawBox(
        50, 110,
        450, 145,
        GetColor(90, 100, 120),
        FALSE
    );
    DrawAppText(60, 120, searchQuery.empty() && !searchFieldActive ? "Type to search..." : searchQuery);

    if (!searchQuery.empty())
    {
        if (Button(460, 110, 80, 35, "CLEAR"))
        {
            searchQuery = "";
            listPage = 0;
            UpdateFilter();
        }
    }

    NormalizeListPage();

    DrawFormatString(
        550, 120,
        GetColor(180, 195, 220),
        "Found: %d schools",
        (int)filteredIndices.size()
    );

    int startIndex = listPage * ITEMS_PER_PAGE;
    int endIndex = startIndex + ITEMS_PER_PAGE;

    if (endIndex > (int)filteredIndices.size())
        endIndex = (int)filteredIndices.size();

    for (int i = startIndex; i < endIndex; i++)
    {
        int schoolIdx = filteredIndices[i];
        School& s = schools[schoolIdx];

        int y = 160 + (i - startIndex) * 105;

        DrawBox(
            50, y,
            850, y + 95,
            GetColor(35, 43, 60),
            TRUE
        );

        DrawSchoolImage(
            65, y + 10,
            100, 75,
            s.imageHandle
        );

        DrawFormatString(
            185, y + 12,
            GetColor(255, 255, 255),
            "%d. %s",
            schoolIdx + 1,
            s.name.c_str()
        );

        DrawAppText(
            185, y + 40,
            "Location: " + s.location
        );

        DrawAppText(
            185, y + 65,
            "Course: " + s.course
        );

        if (Button(700, y + 27, 120, 40, "SELECT"))
        {
            selectedSchool = schoolIdx;
            currentPage = SCHOOL_DETAIL_PAGE;
            return;
        }
    }

    // Pagination
    DrawFormatString(
        390, 620,
        GetColor(200, 210, 230),
        "Page %d / %d",
        listPage + 1,
        GetTotalPages()
    );

    if (Button(280, 675, 120, 45, "PREVIOUS"))
    {
        if (listPage > 0)
            listPage--;

        return;
    }

    if (Button(420, 675, 120, 45, "NEXT"))
    {
        if (listPage + 1 < GetTotalPages())
            listPage++;

        return;
    }

    if (isAdmin)
    {
        if (Button(600, 675, 110, 45, "BACK"))
        {
            selectedSchool = -1;
            currentPage = ADMIN_PAGE;
            return;
        }
    }

    if (Button(740, 675, 120, 45, "LOGOUT"))
    {
        Logout();
        return;
    }

    if (filteredIndices.empty())
    {
        DrawString(
            350, 380,
            "No schools found matching your search.",
            GetColor(180, 180, 180)
        );
    }
}

void DrawSchoolDetailPage()
{
    DrawBackground();

    DrawPageTitle("SCHOOL DETAILS");

    if (selectedSchool < 0 ||
        selectedSchool >= (int)schools.size())
    {
        selectedSchool = -1;
        currentPage = STUDENT_PAGE;
        return;
    }

    School& s = schools[selectedSchool];

    DrawSchoolImage(
        60, 135,
        330, 255,
        s.imageHandle
    );

    DrawWrappedText(
        420, 150,
        s.name,
        420,
        28
    );

    DrawLine(
        420, 205,
        850, 205,
        GetColor(80, 95, 120)
    );

    DrawString(
        420, 230,
        "SCHOOL RATING",
        GetColor(255, 210, 110)
    );

    DrawFormatString(
        420, 260,
        GetColor(100, 220, 150),
        "%s / 5.0",
        s.rating.c_str()
    );

    DrawString(
        60, 425,
        "LOCATION",
        GetColor(255, 210, 110)
    );

    DrawWrappedText(
        60, 455,
        s.location,
        350
    );

    DrawString(
        480, 425,
        "OPEN CAMPUS DATE",
        GetColor(255, 210, 110)
    );

    DrawWrappedText(
        480, 455,
        s.openDate,
        350
    );

    DrawString(
        60, 515,
        "AVAILABLE COURSES",
        GetColor(255, 210, 110)
    );

    DrawWrappedText(
        60, 545,
        s.course,
        350
    );

    DrawString(
        480, 515,
        "DESCRIPTION",
        GetColor(255, 210, 110)
    );

    DrawWrappedText(
        480, 545,
        s.description,
        350
    );

    if (Button(60, 690, 140, 45, "BACK"))
    {
        currentPage = STUDENT_PAGE;
        return;
    }

    if (isAdmin)
    {
        if (Button(230, 690, 140, 45, "EDIT"))
        {
            StartEditSchool();
            return;
        }

        if (Button(400, 690, 140, 45, "DELETE"))
        {
            DeleteSelectedSchool();
            return;
        }
    }

    if (Button(740, 690, 120, 45, "LOGOUT"))
    {
        Logout();
        return;
    }
}

// ============================================================================
// ?? (?) - Personal One (????): School Form Input & Form UI
// ?????????: Member 5
// ============================================================================

void HandleFormInput()
{
    int mx, my;

    GetMousePoint(&mx, &my);

    if (mouseClicked)
    {
        for (int i = 0; i < 6; i++)
        {
            int y = 125 + i * 60;

            if (mx >= 220 && mx <= 660 &&
                my >= y + 20 && my <= y + 55)
            {
                activeField = i;
            }
        }
    }

    int ch;

    while ((ch = GetInputChar(TRUE)) != 0)
    {
        if (ch >= 32 && ch <= 126)
        {
            if (formFields[activeField].length() < 120)
            {
                formFields[activeField] += (char)ch;
            }
        }
    }

    static bool previousBack = false;

    bool back = CheckHitKey(KEY_INPUT_BACK);

    if (back && !previousBack)
    {
        if (!formFields[activeField].empty())
            formFields[activeField].pop_back();
    }

    previousBack = back;
}

void DrawSchoolForm()
{
    DrawBackground();

    DrawPageTitle(
        isEditMode ? "EDIT SCHOOL" : "ADD NEW SCHOOL"
    );

    for (int i = 0; i < 6; i++)
    {
        int y = 125 + i * 60;

        DrawString(
            50, y + 25,
            fieldNames[i].c_str(),
            GetColor(235, 235, 245)
        );

        DrawBox(
            220, y + 20,
            660, y + 55,
            activeField == i
            ? GetColor(65, 80, 110)
            : GetColor(40, 45, 62),
            TRUE
        );

        DrawAppText(
            232, y + 29,
            formFields[i]
        );
    }

    DrawString(
        50, 535,
        "School Image:",
        GetColor(255, 255, 255)
    );

    if (Button(220, 525, 180, 45, "CHOOSE IMAGE"))
    {
        ChooseSchoolImage();
    }

    DrawSchoolImage(
        700, 145,
        160, 150,
        formImageHandle
    );

    DrawString(
        700, 305,
        formImagePath.empty()
        ? "No image selected"
        : "Image selected",
        GetColor(100, 220, 150)
    );

    DrawString(
        50, 600,
        "Click a field and type. Use Backspace to delete.",
        GetColor(165, 175, 195)
    );

    if (!loginMessage.empty())
    {
        DrawAppText(
            50, 640,
            loginMessage
        );
    }

    if (Button(220, 700, 180, 45, "SAVE"))
    {
        SaveSchoolForm();
        return;
    }

    if (Button(430, 700, 180, 45, "CANCEL"))
    {
        CancelSchoolForm();
        return;
    }
}

// ============================================================================
// ?? (?) - Personal One (????): Main Program Loop (WinMain)
// ?????????: Member 1
// ============================================================================

int WINAPI WinMain(
    HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPSTR lpCmdLine,
    int nCmdShow
)
{
    ChangeWindowMode(TRUE);
    SetWindowIconID(101); // OpenCampus application icon

    SetGraphMode(SW, SH, 32);

    SetMainWindowText(
        "Kansai IT School Management System"
    );

    if (DxLib_Init() == -1)
        return -1;

    SetDrawScreen(DX_SCREEN_BACK);

    SetMouseDispFlag(TRUE);

    LoadSchools();

    LoadSchoolImages();

    while (
        ProcessMessage() == 0 &&
        CheckHitKey(KEY_INPUT_ESCAPE) == 0
        )
    {
        UpdateMouse();

        ClearDrawScreen();

        switch (currentPage)
        {
        case LOGIN_PAGE:

            DrawLoginPage();

            HandleLoginInput();

            break;

        case ADMIN_PAGE:

            DrawAdminPage();

            break;

        case STUDENT_PAGE:

            DrawSchoolList();

            break;

        case SCHOOL_FORM_PAGE:

            DrawSchoolForm();

            HandleFormInput();

            break;

        case SCHOOL_DETAIL_PAGE:

            DrawSchoolDetailPage();

            break;
        }

        ScreenFlip();
    }

    ReleaseFormImage();

    ReleaseSchoolImages();

    DxLib_End();

    return 0;
}
