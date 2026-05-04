// main.cpp — Finance Manager
// Phase 2 entry point: blank ImGui + GLFW + OpenGL3 window.
// All app/UI/data-structure work is wired in from later phases.

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <GLFW/glfw3.h>

#include <cstdio>
#include <cstdlib>
#include <iostream>

#include "BudgetManager.h"
#include "Expense.h"
#include "Bill.h"
#include "Date.h"

// Phase 4 Checkpoint C: CLI smoke test. Runs before any GLFW work so the
// output reaches the terminal even if the window pops up immediately after.
static void run_cli_smoke_test() {
    std::cout << "---------- BudgetManager smoke test ----------" << std::endl;

    BudgetManager manager;

    // Budget setup
    manager.setBudgetLimit("Food", 500.0);
    manager.setBudgetLimit("Transport", 200.0);

    // Expenses
    Expense e1; e1.category = "Food"; e1.amount = 400.0; e1.date = Date(1, 4, 2025); e1.description = "Groceries";
    manager.addExpense(e1);
    manager.checkBudget("Food");        // expect: WARNING (80%)

    Expense e2; e2.category = "Food"; e2.amount = 150.0; e2.date = Date(3, 4, 2025); e2.description = "Restaurant";
    manager.addExpense(e2);
    manager.checkBudget("Food");        // expect: EXCEEDED (110%)

    Expense e3; e3.category = "Transport"; e3.amount = 50.0; e3.date = Date(5, 4, 2025); e3.description = "Bus pass";
    manager.addExpense(e3);
    manager.checkBudget("Transport");   // expect: OK (25%)

    // Bills
    Bill rent; rent.name = "Rent"; rent.amountDue = 1200.0; rent.dueDate = Date(1, 5, 2025);
    Bill internet; internet.name = "Internet"; internet.amountDue = 60.0; internet.dueDate = Date(15, 4, 2025);
    manager.addBill(rent);
    manager.addBill(internet);
    std::cout << "Next bill: " << manager.getNextBill().name << std::endl; // expect: Internet

    // Range query
    auto expenses = manager.getExpensesByRange(Date(1, 4, 2025), Date(4, 4, 2025));
    std::cout << "Expenses Apr 1-4: " << expenses.size() << std::endl;     // expect: 2

    // Report
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

    // ImGui setup
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    ImVec4 clear_color = ImVec4(0.10f, 0.10f, 0.12f, 1.00f);

    // Main loop
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Placeholder until Wave 2 panels are wired in.
        ImGui::ShowDemoWindow();

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
