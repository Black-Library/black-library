/**
 * BlackLibraryGUI.h
 */

#ifndef __BLACK_LIBRARY_BLACKLIBRARY_GUI_H__
#define __BLACK_LIBRARY_BLACKLIBRARY_GUI_H__

#include <atomic>
#include <vector>

#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>

#include <BlackLibraryBinder.h>
#include <BlackLibraryDB.h>

namespace black_library {

namespace BlackLibraryBinder = black_library::core::binder;
namespace BlackLibraryDB = black_library::core::db;

// Usage:
//  static ApplicationLog my_log;
//  my_log.AddLog("Hello %d world\n", 123);
//  my_log.Draw();
struct ApplicationLog
{
    ImGuiTextBuffer     Buf;
    ImGuiTextFilter     Filter;
    ImVector<int>       LineOffsets; // Index to lines offset. We maintain this with AddLog() calls.
    bool                AutoScroll;  // Keep scrolling if already at the bottom.

    ApplicationLog()
    {
        AutoScroll = true;
        Clear();
    }

    void    Clear()
    {
        Buf.clear();
        LineOffsets.clear();
        LineOffsets.push_back(0);
    }

    void    AddLog(const char* fmt, ...) IM_FMTARGS(2)
    {
        int old_size = Buf.size();
        va_list args;
        va_start(args, fmt);
        Buf.appendfv(fmt, args);
        va_end(args);
        for (int new_size = Buf.size(); old_size < new_size; ++old_size)
            if (Buf[old_size] == '\n')
                LineOffsets.push_back(old_size + 1);
    }

    void    Draw(bool* p_open = NULL)
    {
        bool clear = ImGui::Button("Clear");
        ImGui::SameLine();
        bool copy = ImGui::Button("Copy");
        ImGui::SameLine();
        Filter.Draw("Filter", -100.0f);

        ImGui::Separator();
        ImGui::BeginChild("scrolling", ImVec2(0, ImGui::GetFontSize() * 20.0f), false, ImGuiWindowFlags_HorizontalScrollbar);

        if (clear)
            Clear();
        if (copy)
            ImGui::LogToClipboard();

        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
        const char* buf = Buf.begin();
        const char* buf_end = Buf.end();
        if (Filter.IsActive())
        {
            // In this example we don't use the clipper when Filter is enabled.
            // This is because we don't have a random access on the result on our filter.
            // A real application processing logs with ten of thousands of entries may want to store the result of
            // search/filter.. especially if the filtering function is not trivial (e.g. reg-exp).
            for (int line_no = 0; line_no < LineOffsets.Size; ++line_no)
            {
                const char* line_start = buf + LineOffsets[line_no];
                const char* line_end = (line_no + 1 < LineOffsets.Size) ? (buf + LineOffsets[line_no + 1] - 1) : buf_end;
                if (Filter.PassFilter(line_start, line_end))
                    ImGui::TextUnformatted(line_start, line_end);
            }
        }
        else
        {
            // The simplest and easy way to display the entire buffer:
            //   ImGui::TextUnformatted(buf_begin, buf_end);
            // And it'll just work. TextUnformatted() has specialization for large blob of text and will fast-forward
            // to skip non-visible lines. Here we instead demonstrate using the clipper to only process lines that are
            // within the visible area.
            // If you have tens of thousands of items and their processing cost is non-negligible, coarse clipping them
            // on your side is recommended. Using ImGuiListClipper requires
            // - A) random access into your data
            // - B) items all being the  same height,
            // both of which we can handle since we an array pointing to the beginning of each line of text.
            // When using the filter (in the block of code above) we don't have random access into the data to display
            // anymore, which is why we don't use the clipper. Storing or skimming through the search result would make
            // it possible (and would be recommended if you want to search through tens of thousands of entries).
            ImGuiListClipper clipper;
            clipper.Begin(LineOffsets.Size);
            while (clipper.Step())
            {
                for (int line_no = clipper.DisplayStart; line_no < clipper.DisplayEnd; ++line_no)
                {
                    const char* line_start = buf + LineOffsets[line_no];
                    const char* line_end = (line_no + 1 < LineOffsets.Size) ? (buf + LineOffsets[line_no + 1] - 1) : buf_end;
                    ImGui::TextUnformatted(line_start, line_end);
                }
            }
            clipper.End();
        }
        ImGui::PopStyleVar();

        if (AutoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
            ImGui::SetScrollHereY(1.0f);

        ImGui::EndChild();
    }
};

struct EntrySort
{
    ImGuiTableSortSpecs* s_current_sort_specs;

    bool operator() (const BlackLibraryDB::DBEntry &left, const BlackLibraryDB::DBEntry &right)
    {
        for (int n = 0; n < s_current_sort_specs->SpecsCount; ++n)
        {
            const ImGuiTableColumnSortSpecs* sort_spec = &s_current_sort_specs->Specs[n];
            int delta = 0;
            switch (sort_spec->ColumnUserID)
            {
                case static_cast<unsigned int>(BlackLibraryDB::DBEntryColumnID::uuid):             delta = (left.uuid.compare(right.uuid));               break;
                case static_cast<unsigned int>(BlackLibraryDB::DBEntryColumnID::title):            delta = (left.title.compare(right.title));             break;
                case static_cast<unsigned int>(BlackLibraryDB::DBEntryColumnID::author):           delta = (left.author.compare(right.author));           break;
                case static_cast<unsigned int>(BlackLibraryDB::DBEntryColumnID::nickname):         delta = (left.nickname.compare(right.nickname));       break;
                case static_cast<unsigned int>(BlackLibraryDB::DBEntryColumnID::source):           delta = (left.source.compare(right.source));           break;
                case static_cast<unsigned int>(BlackLibraryDB::DBEntryColumnID::series_length):    delta = (left.series_length - right.series_length);    break;
                case static_cast<unsigned int>(BlackLibraryDB::DBEntryColumnID::update_date):      delta = (left.update_date - right.update_date);        break;
                case static_cast<unsigned int>(BlackLibraryDB::DBEntryColumnID::check_date):       delta = (left.check_date - right.check_date);          break;
                case static_cast<unsigned int>(BlackLibraryDB::DBEntryColumnID::birth_date):       delta = (left.birth_date - right.birth_date);          break;
                case static_cast<unsigned int>(BlackLibraryDB::DBEntryColumnID::url):              delta = (left.url.compare(right.url));                 break;
                default: IM_ASSERT(0); break;
            }
            if (delta > 0)
            {
                if (sort_spec->SortDirection == ImGuiSortDirection_Ascending)
                    return false;
                return true;
            }
            if (delta < 0)
            {
                if (sort_spec->SortDirection == ImGuiSortDirection_Ascending)
                    return true;
                return false;
            }
        }

        return left.update_date < right.update_date;
    }
};

class BlackLibraryGUI {
public:
    explicit BlackLibraryGUI(const std::string &db_path, const std::string &storage_path);
    BlackLibraryGUI &operator = (BlackLibraryGUI &&) = default;

    int Run();
    int Stop();

private:
    void BindEntry(const std::string &uuid);

    void ShowCopyLocationWindow(bool* p_open);
    void ShowRefreshAndSearch();
    void ShowBlackEntryTable();
    void ShowStagingEntryTable();
    void ShowLog();
    void ShowEntry(const BlackLibraryDB::DBEntry &entry, BlackLibraryDB::entry_table_rep_t type);

    void SetupTableColumns(BlackLibraryDB::entry_table_rep_t type);
    void RefreshDBEntries();

    BlackLibraryDB::BlackLibraryDB blacklibrary_db_;
    BlackLibraryBinder::BlackLibraryBinder blacklibrary_binder_;
    std::vector<BlackLibraryDB::DBEntry> black_entries_;
    std::vector<BlackLibraryDB::DBEntry> staging_entries_;
    ImGuiTextFilter filter_;
    bool initialized_;
    bool is_refreshing_;
    bool force_sort_black_;
    bool force_sort_staging_;
};

} // namespace black_library

#endif
