/**
 * BlackLibraryGUI.cc
 */

// BlackLibrary application for GLFW + OpenGL 3, using programmable pipeline
// (GLFW is a cross-platform general purpose library for handling windows, inputs, OpenGL/Vulkan/Metal graphics context creation, etc.)
// If you are new to Dear ImGui, read documentation from the docs/ folder + read the top of imgui.cpp.
// Read online: https://github.com/ocornut/imgui/tree/master/docs

#include <stdio.h>
#include <stdlib.h>

#include <algorithm>
#include <iostream>
#include <sstream>

#include <FileOperations.h>
#include <TimeOperations.h>

#include <BlackLibraryGUI.h>

#if defined(IMGUI_IMPL_OPENGL_ES2)
#include <GLES2/gl2.h>
// About Desktop OpenGL function loaders:
//  Modern desktop OpenGL doesn't have a standard portable header file to load OpenGL function pointers.
//  Helper libraries are often used for this purpose! Here we are supporting a few common ones (gl3w, glew, glad).
//  You may use another loader/header of your choice (glext, glLoadGen, etc.), or chose to manually implement your own.
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GL3W)
#include <GL/gl3w.h>            // Initialize with gl3wInit()
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLEW)
#include <GL/glew.h>            // Initialize with glewInit()
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLAD)
#include <glad/glad.h>          // Initialize with gladLoadGL()
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLAD2)
#include <glad/gl.h>            // Initialize with gladLoadGL(...) or gladLoaderLoadGL()
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLBINDING2)
#define GLFW_INCLUDE_NONE       // GLFW including OpenGL headers causes ambiguity or multiple definition errors.
#include <glbinding/Binding.h>  // Initialize with glbinding::Binding::initialize()
#include <glbinding/gl/gl.h>
using namespace gl;
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLBINDING3)
#define GLFW_INCLUDE_NONE       // GLFW including OpenGL headers causes ambiguity or multiple definition errors.
#include <glbinding/glbinding.h>// Initialize with glbinding::initialize()
#include <glbinding/gl/gl.h>
using namespace gl;
#else
#include IMGUI_IMPL_OPENGL_LOADER_CUSTOM
#endif

// Include glfw3.h after our OpenGL definitions
#include <GLFW/glfw3.h>

// [Win32] Our example includes a copy of glfw3.lib pre-compiled with VS2010 to maximize ease of testing and compatibility with old VS compilers.
// To link with VS2010-era libraries, VS2015+ requires linking with legacy_stdio_definitions.lib, which we do using this pragma.
// Your own project should not be affected, as you are likely to link with a newer binary of GLFW that is adequate for your version of Visual Studio.
#if defined(_MSC_VER) && (_MSC_VER >= 1900) && !defined(IMGUI_DISABLE_WIN32_FUNCTIONS)
#pragma comment(lib, "legacy_stdio_definitions")
#endif

// Enforce cdecl calling convention for functions called by the standard library, in case compilation settings changed the default to e.g. __vectorcall
#ifndef IMGUI_CDECL
#ifdef _MSC_VER
#define IMGUI_CDECL __cdecl
#else
#define IMGUI_CDECL
#endif
#endif

namespace black_library {

namespace BlackLibraryCommon = black_library::core::common;
namespace BlackLibraryDB = black_library::core::db;

void glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}

void HelpMarker(const char* desc)
{
    ImGui::TextDisabled("(?)");
    if (ImGui::IsItemHovered())
    {
        ImGui::BeginTooltip();
        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
        ImGui::TextUnformatted(desc);
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
}

void PushStyleCompact()
{
    ImGuiStyle& style = ImGui::GetStyle();
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(style.FramePadding.x, (float)(int)(style.FramePadding.y * 0.60f)));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(style.ItemSpacing.x, (float)(int)(style.ItemSpacing.y * 0.60f)));
}

void PopStyleCompact()
{
    ImGui::PopStyleVar(2);
}

BlackLibraryGUI::BlackLibraryGUI(const std::string &db_path, const std::string &storage_path) :
    blacklibrary_db_(db_path, false),
    blacklibrary_binder_(storage_path),
    black_entries_(),
    staging_entries_(),
    filter_(),
    initialized_(false),
    is_refreshing_(false),
    force_sort_black_(false),
    force_sort_staging_(false)
{
    std::cout << "\nRunning Black Library GUI" << std::endl;

    if (!blacklibrary_db_.IsReady())
    {
        std::cout << "Error: Black Library GUI stalled, database not initalized/ready" << std::endl;
        return;
    }

    if (!blacklibrary_binder_.SetBindDir("/mnt/black-library/output/"))
    {
        std::cout << "Error: could not set bind directory" << std::endl;
    }

    RefreshDBEntries();

    initialized_ = true;
}

int BlackLibraryGUI::Run()
{
    if (!initialized_)
    {
        std::cout << "Error: Black Library GUI not initialized" << std::endl;
        return -1;
    }

    // Setup window
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
        return 1;

    // Decide GL+GLSL versions
#if defined(IMGUI_IMPL_OPENGL_ES2)
    // GL ES 2.0 + GLSL 100
    const char* glsl_version = "#version 100";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
#elif defined(__APPLE__)
    // GL 3.2 + GLSL 150
    const char* glsl_version = "#version 150";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // Required on Mac
#else
    // GL 3.0 + GLSL 130
    const char* glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    //glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // 3.0+ only
#endif

    // Create window with graphics context
    GLFWwindow* window = glfwCreateWindow(1920, 1080, "The Black Library", NULL, NULL);
    if (window == NULL)
        return 1;
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync

    // Initialize OpenGL loader
#if defined(IMGUI_IMPL_OPENGL_LOADER_GL3W)
    bool err = gl3wInit() != 0;
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLEW)
    bool err = glewInit() != GLEW_OK;
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLAD)
    bool err = gladLoadGL() == 0;
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLAD2)
    bool err = gladLoadGL(glfwGetProcAddress) == 0; // glad2 recommend using the windowing library loader instead of the (optionally) bundled one.
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLBINDING2)
    bool err = false;
    glbinding::Binding::initialize();
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLBINDING3)
    bool err = false;
    glbinding::initialize([](const char* name) { return (glbinding::ProcAddress)glfwGetProcAddress(name); });
#else
    bool err = false; // If you use IMGUI_IMPL_OPENGL_LOADER_CUSTOM, your loader is likely to requires some form of initialization.
#endif
    if (err)
    {
        fprintf(stderr, "Failed to initialize OpenGL loader!\n");
        return 1;
    }

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsClassic();

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    // Our state
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    // Main loop
    while (!glfwWindowShouldClose(window))
    {
        // Poll and handle events (inputs, window resize, etc.)
        // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
        // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application.
        // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application.
        // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
        glfwPollEvents();

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // We specify a default position/size in case there's no data in the .ini file.
        // We only do it to make the demo applications a little more welcoming, but typically this isn't required.
        const ImGuiViewport* main_viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(main_viewport->WorkPos);
        ImGui::SetNextWindowSize(main_viewport->WorkSize);

        static bool show_copy_location_window = false;
        static bool show_app_metrics = false;
        static bool show_app_about = false;

        if (!ImGui::Begin("Dear ImGui 1"))
        {
            // Early out if the window is collapsed, as an optimization.
            ImGui::End();
            return 0;
        }

        if (show_copy_location_window)
        {
            ShowCopyLocationWindow(&show_copy_location_window);
        }
        if (show_app_metrics)
        {
            ImGui::SetNextWindowSize(ImVec2(550, 680), ImGuiCond_FirstUseEver);
            ImGui::ShowMetricsWindow(&show_app_metrics);
        }
        if (show_app_about)
        {
            ImGui::SetNextWindowSize(ImVec2(550, 680), ImGuiCond_FirstUseEver);
            ImGui::ShowAboutWindow(&show_app_about);
        }

        // e.g. Leave a fixed amount of width for labels (by passing a negative value), the rest goes to widgets.
        ImGui::PushItemWidth(ImGui::GetFontSize() * -12);

        if (ImGui::BeginMainMenuBar())
        {
            if (ImGui::BeginMenu("File"))
            {
                ImGui::MenuItem("Set copy location", "CTRL+L", &show_copy_location_window);
                if (ImGui::MenuItem("Quit", "CTRL+Q"))
                {
                    glfwSetWindowShouldClose(window, 1);
                }
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Tools"))
            {
                ImGui::MenuItem("Metrics/Debugger", "CTRL+M", &show_app_metrics);
                ImGui::MenuItem("About Dear ImGui", NULL, &show_app_about);
                ImGui::EndMenu();
            }
            ImGui::EndMainMenuBar();
        }

        ShowRefreshAndSearch();
        ImGui::Spacing();
        ShowBlackEntryTable();
        ImGui::Spacing();
        ShowStagingEntryTable();
        ImGui::Spacing();
        ShowLog();

        ImGui::PopItemWidth();
        ImGui::End();

        // Rendering
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
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

    return 0;
}

int BlackLibraryGUI::Stop()
{
    return 0;
}

void BlackLibraryGUI::BindEntry(const std::string &uuid)
{
    auto entry = blacklibrary_db_.ReadBlackEntry(uuid);

    blacklibrary_binder_.Bind(uuid, entry.title);
}

void BlackLibraryGUI::ShowCopyLocationWindow(bool* p_open)
{
    ImGui::SetNextWindowSize(ImVec2(500, 440), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("Set Copy Location", p_open, ImGuiWindowFlags_MenuBar))
    {
        if (ImGui::BeginMenuBar())
        {
            if (ImGui::BeginMenu("File"))
            {
                if (ImGui::MenuItem("Close")) *p_open = false;
                ImGui::EndMenu();
            }
            ImGui::EndMenuBar();
        }

        static char buf1[64] = "/mnt/black-library/output/";
        static char buf2[64] = "";
        ImGui::Text("Current Copy Location: %s", buf1);
        ImGui::InputText("Copy Location", buf2, 64);
        if (ImGui::Button("Save"))
        {
            // save copy location
            snprintf(buf1, 64, "%s", buf2);
            if (!blacklibrary_binder_.SetBindDir(std::string(buf2)))
                std::cout << "Error: could not set bind directory" << std::endl;
        }
    }
    ImGui::End();
}

void BlackLibraryGUI::ShowRefreshAndSearch()
{
    if (ImGui::Button("Refresh"))
        is_refreshing_ = true;
    if (is_refreshing_)
    {
        ImGui::SameLine();
        ImGui::Text("Refreshing...");
        RefreshDBEntries();
    }

    ImGui::SameLine();
    filter_.Draw();
}

void BlackLibraryGUI::ShowBlackEntryTable()
{
    const float TEXT_BASE_HEIGHT = ImGui::GetTextLineHeightWithSpacing();

    // Options
    static ImGuiTableFlags flags =
        ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable | ImGuiTableFlags_Sortable | ImGuiTableFlags_SortMulti
        | ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersV | ImGuiTableFlags_NoBordersInBody
        | ImGuiTableFlags_ScrollY;
    PushStyleCompact();
    ImGui::SameLine(); HelpMarker("When sorting is enabled: hold shift when clicking headers to sort on multiple column. ");
    PopStyleCompact();

    ImGui::Text("Black Entries");

    if (ImGui::BeginTable("table_black_entries", 11, flags, ImVec2(0.0f, TEXT_BASE_HEIGHT * 30), 0.0f))
    {
        // Declare columns
        SetupTableColumns(BlackLibraryDB::BLACK_ENTRY);

        // Sort our data if sort specs have been changed!
        if (ImGuiTableSortSpecs* sorts_specs = ImGui::TableGetSortSpecs())
        {
            if (sorts_specs->SpecsDirty || force_sort_black_)
            {
                EntrySort sort_struct;
                sort_struct.s_current_sort_specs = sorts_specs;
                if (black_entries_.size() > 1)
                    std::sort(black_entries_.begin(), black_entries_.end(), sort_struct);
                sorts_specs->SpecsDirty = false;
                force_sort_black_ = false;
            }
        }

        if (filter_.IsActive())
        {
            for (size_t row_n = 0; row_n < black_entries_.size(); ++row_n)
            {
                // Display a filtered database entry
                if (filter_.PassFilter(black_entries_[row_n].title.c_str()) || filter_.PassFilter(black_entries_[row_n].author.c_str()))
                    ShowEntry(black_entries_[row_n], BlackLibraryDB::BLACK_ENTRY);
            }
        }
        else
        {
        // Use clipper for large list
        ImGuiListClipper clipper;
        clipper.Begin(black_entries_.size());
        while (clipper.Step())
            for (int row_n = clipper.DisplayStart; row_n < clipper.DisplayEnd; ++row_n)
            {
                ShowEntry(black_entries_[row_n], BlackLibraryDB::BLACK_ENTRY);
            }
        }
        ImGui::EndTable();
    }
}

void BlackLibraryGUI::ShowStagingEntryTable()
{
    if (ImGui::TreeNode("Staging Entries"))
    {
        const float TEXT_BASE_HEIGHT = ImGui::GetTextLineHeightWithSpacing();

        // Options
        static ImGuiTableFlags flags =
            ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable | ImGuiTableFlags_Sortable | ImGuiTableFlags_SortMulti
            | ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersV | ImGuiTableFlags_NoBordersInBody
            | ImGuiTableFlags_ScrollY;

        if (ImGui::BeginTable("table_staging_entries", 10, flags, ImVec2(0.0f, TEXT_BASE_HEIGHT * 30), 0.0f))
        {
            // Declare columns
            SetupTableColumns(BlackLibraryDB::STAGING_ENTRY);
            
            // Sort our data if sort specs have been changed!
            if (ImGuiTableSortSpecs* sorts_specs = ImGui::TableGetSortSpecs())
                if (sorts_specs->SpecsDirty || force_sort_staging_)
                {
                    EntrySort sort_struct;
                    sort_struct.s_current_sort_specs = sorts_specs;
                    if (staging_entries_.size() > 1)
                        std::sort(staging_entries_.begin(), staging_entries_.end(), sort_struct);
                    sorts_specs->SpecsDirty = false;
                    force_sort_staging_ = false;
                }

            // Use clipper for large list
            ImGuiListClipper clipper;
            clipper.Begin(staging_entries_.size());
            while (clipper.Step())
                for (int row_n = clipper.DisplayStart; row_n < clipper.DisplayEnd; ++row_n)
                {
                    // Display a database entry
                    ShowEntry(staging_entries_[row_n], BlackLibraryDB::STAGING_ENTRY);
                }
            ImGui::EndTable();
        }
        ImGui::TreePop();
    }
}

void BlackLibraryGUI::ShowLog()
{
    if (ImGui::TreeNode("Log"))
    {
        // TODO: hook this up to logging
        // static ApplicationLog log;

        // log.AddLog("[%05d] [%s] Hello, current time is %.1f, here's a word: '%s'\n",
        //     ImGui::GetFrameCount(), "hello", ImGui::GetTime(), "world");

        // log.Draw();

        ImGui::TreePop();
    }
}

void BlackLibraryGUI::SetupTableColumns(BlackLibraryDB::entry_table_rep_t type)
{
    if (type == BlackLibraryDB::BLACK_ENTRY)
        ImGui::TableSetupColumn("UUID",             ImGuiTableColumnFlags_PreferSortDescending | ImGuiTableColumnFlags_WidthFixed,                                     0.0f, static_cast<unsigned int>(BlackLibraryDB::DBEntryColumnID::uuid));
    ImGui::TableSetupColumn("UUID",             ImGuiTableColumnFlags_PreferSortDescending | ImGuiTableColumnFlags_WidthFixed,                                     0.0f, static_cast<unsigned int>(BlackLibraryDB::DBEntryColumnID::uuid));
    ImGui::TableSetupColumn("Title",            ImGuiTableColumnFlags_PreferSortDescending | ImGuiTableColumnFlags_WidthFixed,                                     0.0f, static_cast<unsigned int>(BlackLibraryDB::DBEntryColumnID::title));
    ImGui::TableSetupColumn("Author",           ImGuiTableColumnFlags_PreferSortDescending | ImGuiTableColumnFlags_WidthFixed,                                     0.0f, static_cast<unsigned int>(BlackLibraryDB::DBEntryColumnID::author));
    ImGui::TableSetupColumn("Nickname",         ImGuiTableColumnFlags_PreferSortDescending | ImGuiTableColumnFlags_WidthFixed,                                     0.0f, static_cast<unsigned int>(BlackLibraryDB::DBEntryColumnID::nickname));
    ImGui::TableSetupColumn("Source",           ImGuiTableColumnFlags_PreferSortDescending | ImGuiTableColumnFlags_WidthFixed,                                     0.0f, static_cast<unsigned int>(BlackLibraryDB::DBEntryColumnID::source));
    ImGui::TableSetupColumn("Series Length",    ImGuiTableColumnFlags_PreferSortDescending | ImGuiTableColumnFlags_WidthFixed,                                     0.0f, static_cast<unsigned int>(BlackLibraryDB::DBEntryColumnID::series_length));
    ImGui::TableSetupColumn("Last Update Date", ImGuiTableColumnFlags_PreferSortDescending | ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_DefaultSort, 0.0f, static_cast<unsigned int>(BlackLibraryDB::DBEntryColumnID::update_date));
    ImGui::TableSetupColumn("Last Check Date",  ImGuiTableColumnFlags_PreferSortDescending | ImGuiTableColumnFlags_WidthFixed,                                     0.0f, static_cast<unsigned int>(BlackLibraryDB::DBEntryColumnID::check_date));
    ImGui::TableSetupColumn("Birth Date",       ImGuiTableColumnFlags_PreferSortDescending | ImGuiTableColumnFlags_WidthFixed,                                     0.0f, static_cast<unsigned int>(BlackLibraryDB::DBEntryColumnID::birth_date));
    ImGui::TableSetupColumn("URL",              ImGuiTableColumnFlags_PreferSortDescending | ImGuiTableColumnFlags_WidthFixed,                                     0.0f, static_cast<unsigned int>(BlackLibraryDB::DBEntryColumnID::url));
    ImGui::TableSetupScrollFreeze(0, 1); // Make row always visible
    ImGui::TableHeadersRow();
}

void BlackLibraryGUI::ShowEntry(const BlackLibraryDB::DBEntry &entry, BlackLibraryDB::entry_table_rep_t type)
{
    ImGui::PushID(entry.uuid.c_str());
    ImGui::TableNextRow();
    if (type == BlackLibraryDB::BLACK_ENTRY)
    {
        ImGui::TableNextColumn();
        if (ImGui::SmallButton("Copy"))
            {
                std::cout << "Copy UUID: " << entry.uuid << std::endl;
                BindEntry(entry.uuid);
            }
    }
    ImGui::TableNextColumn();
    ImGui::Text("%s", entry.uuid.c_str());
    ImGui::TableNextColumn();
    ImGui::TextUnformatted(entry.title.c_str());
    ImGui::TableNextColumn();
    ImGui::Text("%s", entry.author.c_str());
    ImGui::TableNextColumn();
    ImGui::Text("%s", entry.nickname.c_str());
    ImGui::TableNextColumn();
    ImGui::Text("%s", entry.source.c_str());
    ImGui::TableNextColumn();
    ImGui::Text("%d", entry.series_length);
    ImGui::TableNextColumn();
    ImGui::Text("%s", BlackLibraryCommon::GetGUITimeString(entry.update_date).c_str());
    ImGui::TableNextColumn();
    ImGui::Text("%s", BlackLibraryCommon::GetGUITimeString(entry.check_date).c_str());
    ImGui::TableNextColumn();
    ImGui::Text("%s", BlackLibraryCommon::GetGUITimeString(entry.birth_date).c_str());
    ImGui::TableNextColumn();
    ImGui::Text("%s", entry.url.c_str());
    ImGui::PopID();
}

void BlackLibraryGUI::RefreshDBEntries()
{
    black_entries_ = blacklibrary_db_.GetBlackEntryList();
    staging_entries_ = blacklibrary_db_.GetStagingEntryList();

    is_refreshing_ = false;
    force_sort_black_ = true;
    force_sort_staging_ = true;
}

} // namespace black_library
