// main.cpp -- Finance Manager
// Full ImGui UI with dynamic panel layout (scales with window size)
// and F11 fullscreen toggle.

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <GLFW/glfw3.h>

#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <string>

#include "BudgetManager.h"
#include "Expense.h"
#include "Bill.h"
#include "Date.h"
#include "CategoryInfo.h"

// CLI smoke test preserved from Phase 4.
static void run_cli_smoke_test() {
    std::cout << "---------- BudgetManager smoke test ----------" << std::endl;

    BudgetManager manager;

    manager.setBudgetLimit("Food", 500.0);
    manager.setBudgetLimit("Transport", 200.0);

    Expense e1; e1.category = "Food"; e1.amount = 400.0; e1.date = Date(1, 4, 2025); e1.description = "Groceries";
    manager.addExpense(e1);
    manager.checkBudget("Food");

    Expense e2; e2.category = "Food"; e2.amount = 150.0; e2.date = Date(3, 4, 2025); e2.description = "Restaurant";
    manager.addExpense(e2);
    manager.checkBudget("Food");

    Expense e3; e3.category = "Transport"; e3.amount = 50.0; e3.date = Date(5, 4, 2025); e3.description = "Bus pass";
    manager.addExpense(e3);
    manager.checkBudget("Transport");

    Bill rent; rent.name = "Rent"; rent.amountDue = 1200.0; rent.dueDate = Date(1, 5, 2025);
    Bill internet; internet.name = "Internet"; internet.amountDue = 60.0; internet.dueDate = Date(15, 4, 2025);
    manager.addBill(rent);
    manager.addBill(internet);
    std::cout << "Next bill: " << manager.getNextBill().name << std::endl;

    auto expenses = manager.getExpensesByRange(Date(1, 4, 2025), Date(4, 4, 2025));
    std::cout << "Expenses Apr 1-4: " << expenses.size() << std::endl;

    manager.generateReport();
    std::cout << "---------- end smoke test ----------" << std::endl;
}

static void glfw_error_callback(int error, const char* description) {
    std::fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

// Fullscreen toggle state — stored at file scope so the key callback can reach it.
static bool  s_isFullscreen = false;
static int   s_savedX = 100, s_savedY = 100;
static int   s_savedW = 1280, s_savedH = 720;

static void key_callback(GLFWwindow* window, int key, int /*scancode*/, int action, int /*mods*/) {
    if (key == GLFW_KEY_F11 && action == GLFW_PRESS) {
        if (!s_isFullscreen) {
            glfwGetWindowPos(window,  &s_savedX, &s_savedY);
            glfwGetWindowSize(window, &s_savedW, &s_savedH);
            GLFWmonitor*       monitor = glfwGetPrimaryMonitor();
            const GLFWvidmode* mode    = glfwGetVideoMode(monitor);
            glfwSetWindowMonitor(window, monitor, 0, 0,
                                 mode->width, mode->height, mode->refreshRate);
            s_isFullscreen = true;
        } else {
            glfwSetWindowMonitor(window, nullptr,
                                 s_savedX, s_savedY, s_savedW, s_savedH, 0);
            s_isFullscreen = false;
        }
    }
    // ESC exits fullscreen (but does not close the window).
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS && s_isFullscreen) {
        glfwSetWindowMonitor(window, nullptr,
                             s_savedX, s_savedY, s_savedW, s_savedH, 0);
        s_isFullscreen = false;
    }
}

int main() {
    run_cli_smoke_test();

    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit()) {
        std::fprintf(stderr, "Failed to initialize GLFW\n");
        return EXIT_FAILURE;
    }

    const char* glsl_version = "#version 150";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    GLFWwindow* window = glfwCreateWindow(1280, 720, "Finance Manager", nullptr, nullptr);
    if (window == nullptr) {
        std::fprintf(stderr, "Failed to create GLFW window\n");
        glfwTerminate();
        return EXIT_FAILURE;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);
    glfwSetKeyCallback(window, key_callback);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    ImGui::StyleColorsDark();

    // Save unscaled style so we can re-apply a fresh scale each frame.
    ImGuiStyle baseStyle = ImGui::GetStyle();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    ImVec4 clear_color = ImVec4(0.10f, 0.10f, 0.12f, 1.00f);

    // ----- App state -----
    BudgetManager manager;

    char   expCategory[64]     = "";
    char   expDescription[128] = "";
    int    expDay = 1, expMonth = 1, expYear = 2025;
    double expAmount = 0.0;
    std::string lastExpStatus;

    char   billName[64] = "";
    double billAmount   = 0.0;
    int    billDay = 1, billMonth = 1, billYear = 2025;

    char   catName[64] = "";
    double catLimit    = 0.0;

    // Default Budget Overview to the current real-clock month.
    time_t now_t = std::time(nullptr);
    std::tm* lt  = std::localtime(&now_t);
    int viewMonth = lt->tm_mon + 1;
    int viewYear  = lt->tm_year + 1900;
    // Navigation bounds: no future months; up to 2 years back.
    const int maxMonth = viewMonth, maxYear = viewYear;
    const int minMonth = viewMonth, minYear = viewYear - 2;

    static const char* kMonthNames[] = {
        "Jan","Feb","Mar","Apr","May","Jun",
        "Jul","Aug","Sep","Oct","Nov","Dec"
    };

    const ImGuiWindowFlags fixedFlags =
        ImGuiWindowFlags_NoMove    |
        ImGuiWindowFlags_NoResize  |
        ImGuiWindowFlags_NoCollapse;

    // ----- Main loop -----
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        // Get current window size each frame so panels scale on resize/fullscreen.
        int winW, winH;
        glfwGetWindowSize(window, &winW, &winH);
        float W = static_cast<float>(winW);
        float H = static_cast<float>(winH);

        // Scale UI relative to base 1280x720. Clamp so it never gets too tiny/huge.
        float scale = (W / 1280.0f < H / 720.0f) ? W / 1280.0f : H / 720.0f;
        if (scale < 0.5f) scale = 0.5f;
        if (scale > 4.0f) scale = 4.0f;
        io.FontGlobalScale = scale;
        ImGui::GetStyle() = baseStyle;
        ImGui::GetStyle().ScaleAllSizes(scale);

        // Layout: 3 columns. Column 1 = 33%, columns 2 & 3 split remaining equally.
        float c1W = W * 0.33f;
        float c2W = (W - c1W) * 0.5f;
        float c3W = W - c1W - c2W;
        float topH  = H * 0.5f;
        float botH  = H - topH;

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // ===== Panel 1: Budget Overview (left, full height) =====
        ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(c1W, H), ImGuiCond_Always);
        ImGui::Begin("Budget Overview", nullptr, fixedFlags);
        {
            // Month navigator — disabled at bounds
            bool atMin = (viewYear < minYear) || (viewYear == minYear && viewMonth <= minMonth);
            bool atMax = (viewYear > maxYear) || (viewYear == maxYear && viewMonth >= maxMonth);

            if (atMin) ImGui::BeginDisabled();
            if (ImGui::ArrowButton("##prev", ImGuiDir_Left)) {
                if (--viewMonth < 1)  { viewMonth = 12; viewYear--; }
            }
            if (atMin) ImGui::EndDisabled();

            ImGui::SameLine();
            ImGui::Text("%s %d", kMonthNames[viewMonth - 1], viewYear);
            ImGui::SameLine();

            if (atMax) ImGui::BeginDisabled();
            if (ImGui::ArrowButton("##next", ImGuiDir_Right)) {
                if (++viewMonth > 12) { viewMonth = 1; viewYear++; }
            }
            if (atMax) ImGui::EndDisabled();
            ImGui::Separator();

            auto snapshot = manager.getBudgetSnapshotForMonth(viewMonth, viewYear);
            if (ImGui::BeginTable("BudgetTbl", 5,
                ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingFixedFit)) {
                ImGui::TableSetupColumn("Category",  ImGuiTableColumnFlags_WidthStretch);
                ImGui::TableSetupColumn("Limit",     ImGuiTableColumnFlags_WidthFixed, 60.f);
                ImGui::TableSetupColumn("Spent",     ImGuiTableColumnFlags_WidthFixed, 60.f);
                ImGui::TableSetupColumn("% Used",    ImGuiTableColumnFlags_WidthFixed, 55.f);
                ImGui::TableSetupColumn("Status",    ImGuiTableColumnFlags_WidthFixed, 70.f);
                ImGui::TableHeadersRow();
                for (auto& info : snapshot) {
                    double pct = (info.budgetLimit > 0.0)
                                 ? (info.totalSpent / info.budgetLimit * 100.0) : 0.0;
                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0); ImGui::Text("%s", info.name.c_str());
                    ImGui::TableSetColumnIndex(1); ImGui::Text("$%.0f", info.budgetLimit);
                    ImGui::TableSetColumnIndex(2); ImGui::Text("$%.0f", info.totalSpent);
                    ImGui::TableSetColumnIndex(3); ImGui::Text("%.0f%%", pct);
                    ImGui::TableSetColumnIndex(4);
                    if      (info.budgetLimit == 0.0) ImGui::TextColored({0.6f,0.6f,0.6f,1.f}, "no limit");
                    else if (pct > 100.0)             ImGui::TextColored({1.f,0.2f,0.2f,1.f},  "EXCEEDED");
                    else if (pct >= 80.0)             ImGui::TextColored({1.f,0.85f,0.f,1.f},  "WARNING");
                    else                              ImGui::TextColored({0.2f,0.9f,0.2f,1.f}, "OK");
                }
                ImGui::EndTable();
            }
            if (snapshot.empty()) ImGui::TextDisabled("No categories yet.");
        }
        ImGui::End();

        // ===== Panel 2: Set Budget (middle, top half) =====
        ImGui::SetNextWindowPos(ImVec2(c1W, 0), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(c2W, topH), ImGuiCond_Always);
        ImGui::Begin("Set Budget", nullptr, fixedFlags);
        {
            ImGui::InputText("Name##c",           catName, sizeof(catName));
            ImGui::InputDouble("Budget Limit##c", &catLimit, 1.0, 100.0, "$%.2f");
            if (ImGui::Button("Set Budget") && catName[0] != '\0' && catLimit > 0.0) {
                manager.setBudgetLimit(catName, catLimit);
                catName[0] = '\0';
                catLimit   = 0.0;
            }
        }
        ImGui::End();

        // ===== Panel 3: Bills (middle, bottom half) =====
        ImGui::SetNextWindowPos(ImVec2(c1W, topH), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(c2W, botH), ImGuiCond_Always);
        ImGui::Begin("Bills", nullptr, fixedFlags);
        {
            ImGui::Text("Pending Bills:");
            ImGui::Separator();
            {
                auto bills = manager.getAllBills();
                std::string toRemove;
                if (bills.empty()) {
                    ImGui::TextDisabled("No pending bills.");
                } else {
                    if (ImGui::BeginTable("BillsTbl", 4,
                        ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg |
                        ImGuiTableFlags_SizingStretchProp)) {
                        ImGui::TableSetupColumn("Name");
                        ImGui::TableSetupColumn("Amount");
                        ImGui::TableSetupColumn("Due");
                        ImGui::TableSetupColumn("##action");
                        ImGui::TableHeadersRow();
                        for (auto& b : bills) {
                            ImGui::TableNextRow();
                            ImGui::TableSetColumnIndex(0); ImGui::Text("%s", b.name.c_str());
                            ImGui::TableSetColumnIndex(1); ImGui::Text("$%.2f", b.amountDue);
                            ImGui::TableSetColumnIndex(2); ImGui::Text("%s", b.dueDate.toString().c_str());
                            ImGui::TableSetColumnIndex(3);
                            std::string lbl = "Mark as Paid##" + b.name;
                            if (ImGui::SmallButton(lbl.c_str())) {
                                toRemove = b.name;
                            }
                        }
                        ImGui::EndTable();
                    }
                }
                if (!toRemove.empty()) {
                    manager.removeBill(toRemove, Date(4, 5, 2025));
                }
            }
            ImGui::Separator();
            ImGui::Text("Add Bill:");
            ImGui::InputText("Name##b",     billName,   sizeof(billName));
            ImGui::InputDouble("Amount##b", &billAmount, 0.01, 10.0, "$%.2f");
            ImGui::Text("Due:"); ImGui::SameLine();
            ImGui::SetNextItemWidth(50); ImGui::InputInt("D##b", &billDay,   0, 0); ImGui::SameLine();
            ImGui::SetNextItemWidth(50); ImGui::InputInt("M##b", &billMonth, 0, 0); ImGui::SameLine();
            ImGui::SetNextItemWidth(70); ImGui::InputInt("Y##b", &billYear,  0, 0);
            if (ImGui::Button("Add Bill") && billName[0] != '\0' && billAmount > 0.0) {
                Bill b;
                b.name      = billName;
                b.amountDue = billAmount;
                b.dueDate   = Date(billDay, billMonth, billYear);
                manager.addBill(b);
                billName[0] = '\0';
                billAmount  = 0.0;
            }
        }
        ImGui::End();

        // ===== Panel 4: Log Expense (right, top half) =====
        ImGui::SetNextWindowPos(ImVec2(c1W + c2W, 0), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(c3W, topH), ImGuiCond_Always);
        ImGui::Begin("Log Expense", nullptr, fixedFlags);
        {
            ImGui::InputText("Category##e",    expCategory,    sizeof(expCategory));
            ImGui::InputText("Description##e", expDescription, sizeof(expDescription));
            ImGui::InputDouble("Amount##e", &expAmount, 0.01, 10.0, "$%.2f");
            ImGui::Text("Date:"); ImGui::SameLine();
            ImGui::SetNextItemWidth(50); ImGui::InputInt("D##e", &expDay,   0, 0); ImGui::SameLine();
            ImGui::SetNextItemWidth(50); ImGui::InputInt("M##e", &expMonth, 0, 0); ImGui::SameLine();
            ImGui::SetNextItemWidth(70); ImGui::InputInt("Y##e", &expYear,  0, 0);

            if (ImGui::Button("Log Expense") && expAmount > 0.0 && expCategory[0] != '\0') {
                Expense e;
                e.category    = expCategory;
                e.description = expDescription;
                e.amount      = expAmount;
                e.date        = Date(expDay, expMonth, expYear);
                manager.addExpense(e);

                lastExpStatus.clear();
                for (auto& ci : manager.getBudgetSnapshotForMonth(e.date.month, e.date.year)) {
                    if (ci.name == e.category) {
                        if (ci.budgetLimit == 0.0) {
                            lastExpStatus = "Logged (no budget set)";
                        } else {
                            double p = ci.totalSpent / ci.budgetLimit * 100.0;
                            if      (p > 100.0) lastExpStatus = "EXCEEDED budget!";
                            else if (p >= 80.0) lastExpStatus = "WARNING: near limit";
                            else                lastExpStatus = "OK";
                        }
                        break;
                    }
                }
                expAmount         = 0.0;
                expCategory[0]    = '\0';
                expDescription[0] = '\0';
            }
            if (!lastExpStatus.empty()) {
                if      (lastExpStatus.find("EXCEEDED") != std::string::npos)
                    ImGui::TextColored({1.f,0.2f,0.2f,1.f}, "%s", lastExpStatus.c_str());
                else if (lastExpStatus.find("WARNING")  != std::string::npos)
                    ImGui::TextColored({1.f,0.85f,0.f,1.f}, "%s", lastExpStatus.c_str());
                else
                    ImGui::TextColored({0.2f,0.9f,0.2f,1.f}, "%s", lastExpStatus.c_str());
            }
        }
        ImGui::End();

        // ===== Panel 5: Expense History (right, bottom half) =====
        ImGui::SetNextWindowPos(ImVec2(c1W + c2W, topH), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(c3W, botH), ImGuiCond_Always);
        ImGui::Begin("Expense History", nullptr, fixedFlags);
        {
            auto all = manager.getExpensesByRange(Date(1,1,2000), Date(31,12,2099));
            float tableH = botH - 40.f;
            if (ImGui::BeginTable("ExpTbl", 4,
                ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg |
                ImGuiTableFlags_ScrollY | ImGuiTableFlags_SizingFixedFit,
                ImVec2(0, tableH))) {
                ImGui::TableSetupScrollFreeze(0, 1);
                ImGui::TableSetupColumn("Date",        ImGuiTableColumnFlags_WidthFixed,   90.f);
                ImGui::TableSetupColumn("Category",    ImGuiTableColumnFlags_WidthStretch);
                ImGui::TableSetupColumn("Amount",      ImGuiTableColumnFlags_WidthFixed,   65.f);
                ImGui::TableSetupColumn("Description", ImGuiTableColumnFlags_WidthStretch);
                ImGui::TableHeadersRow();
                for (auto& e : all) {
                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0); ImGui::Text("%s", e.date.toString().c_str());
                    ImGui::TableSetColumnIndex(1); ImGui::Text("%s", e.category.c_str());
                    ImGui::TableSetColumnIndex(2); ImGui::Text("$%.2f", e.amount);
                    ImGui::TableSetColumnIndex(3); ImGui::Text("%s", e.description.c_str());
                }
                ImGui::EndTable();
            }
            if (all.empty()) ImGui::TextDisabled("No expenses yet.");
        }
        ImGui::End();

        ImGui::Render();

        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();

    return EXIT_SUCCESS;
}
