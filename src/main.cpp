// main.cpp -- Finance Manager
// Phase 5 Checkpoint D: full ImGui UI.
// Five fixed panels in a 1280x720 window driven by a single BudgetManager.

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <GLFW/glfw3.h>

#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <string>

#include "BudgetManager.h"
#include "Expense.h"
#include "Bill.h"
#include "Date.h"
#include "CategoryInfo.h"

// Phase 4 Checkpoint C: CLI smoke test. Runs before any GLFW work so the
// output reaches the terminal even if the window pops up immediately after.
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

int main() {
    run_cli_smoke_test();

    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit()) {
        std::fprintf(stderr, "Failed to initialize GLFW\n");
        return EXIT_FAILURE;
    }

    // OpenGL 3.3 core profile. GLFW_OPENGL_FORWARD_COMPAT is required on macOS
    // for any 3.2+ core profile context (Apple drops removed-functionality bits).
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
    glfwSwapInterval(1); // vsync

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    ImVec4 clear_color = ImVec4(0.10f, 0.10f, 0.12f, 1.00f);

    // ----- App state -----
    BudgetManager manager;

    // Log Expense form
    char expCategory[64]     = "";
    char expDescription[128] = "";
    int  expDay = 1, expMonth = 1, expYear = 2025;
    double expAmount = 0.0;
    std::string lastExpStatus;

    // Add Bill form
    char   billName[64] = "";
    double billAmount   = 0.0;
    int    billDay = 1, billMonth = 1, billYear = 2025;

    // Add Category form
    char   catName[64] = "";
    double catLimit    = 0.0;

    const ImGuiWindowFlags fixedFlags =
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoCollapse;

    // ----- Main loop -----
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // ===== Panel 1: Budget Overview (left, full height) =====
        ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(420, 720), ImGuiCond_Always);
        ImGui::Begin("Budget Overview", nullptr, fixedFlags);
        {
            auto snapshot = manager.getBudgetSnapshot();
            if (ImGui::BeginTable("BudgetTbl", 5,
                ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingFixedFit)) {
                ImGui::TableSetupColumn("Category",  ImGuiTableColumnFlags_WidthStretch);
                ImGui::TableSetupColumn("Limit",     ImGuiTableColumnFlags_WidthFixed, 70.f);
                ImGui::TableSetupColumn("Spent",     ImGuiTableColumnFlags_WidthFixed, 70.f);
                ImGui::TableSetupColumn("% Used",    ImGuiTableColumnFlags_WidthFixed, 60.f);
                ImGui::TableSetupColumn("Status",    ImGuiTableColumnFlags_WidthFixed, 75.f);
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

        // ===== Panel 2: Log Expense (top-middle) =====
        ImGui::SetNextWindowPos(ImVec2(420, 0), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(430, 360), ImGuiCond_Always);
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

                // Derive status from the snapshot rather than capturing stdout
                // from checkBudget(). Keeps UI rendering side-effect-free.
                lastExpStatus.clear();
                for (auto& ci : manager.getBudgetSnapshot()) {
                    if (ci.name == e.category) {
                        if (ci.budgetLimit == 0.0) {
                            lastExpStatus = "Logged (no budget set)";
                        } else {
                            double p = ci.totalSpent / ci.budgetLimit * 100.0;
                            if      (p > 100.0) lastExpStatus = "EXCEEDED budget!";
                            else if (p >= 80.0) lastExpStatus = "WARNING: near limit";
                            else                lastExpStatus = "OK";
                        }
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

        // ===== Panel 3: Bills (bottom-middle) =====
        ImGui::SetNextWindowPos(ImVec2(420, 360), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(430, 360), ImGuiCond_Always);
        ImGui::Begin("Bills", nullptr, fixedFlags);
        {
            ImGui::Text("Next Due Bill:");
            ImGui::Separator();
            if (manager.hasPendingBills()) {
                Bill next = manager.getNextBill();
                ImGui::Text("%-10s  $%.2f", next.name.c_str(), next.amountDue);
                ImGui::Text("Due: %s", next.dueDate.toString().c_str());
                ImGui::Spacing();
                if (ImGui::Button("Mark as Paid")) {
                    manager.payNextBill(Date(4, 5, 2025));
                }
            } else {
                ImGui::TextDisabled("No pending bills.");
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

        // ===== Panel 4: Set Budget (top-right) =====
        ImGui::SetNextWindowPos(ImVec2(850, 0), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(430, 360), ImGuiCond_Always);
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

        // ===== Panel 5: Expense History (bottom-right) =====
        ImGui::SetNextWindowPos(ImVec2(850, 360), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(430, 360), ImGuiCond_Always);
        ImGui::Begin("Expense History", nullptr, fixedFlags);
        {
            auto all = manager.getExpensesByRange(Date(1,1,2000), Date(31,12,2099));
            if (ImGui::BeginTable("ExpTbl", 4,
                ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg |
                ImGuiTableFlags_ScrollY | ImGuiTableFlags_SizingFixedFit,
                ImVec2(0, 260))) {
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

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return EXIT_SUCCESS;
}
