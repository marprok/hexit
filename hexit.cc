#include <ncurses.h>
#include <cstdint>
#include <string>
#include <iostream>
#include <fstream>
#include <filesystem>
#include <unordered_set>

namespace fs = std::filesystem;

constexpr int                   OFFSET_LEN =  8;
constexpr std::uint32_t     BYTES_PER_LINE = 16;
constexpr int           FIRST_BYTE_PER_ROW = OFFSET_LEN + 2 + 1;

struct FileData
{
    std::uint32_t         m_size;
    std::uint32_t   m_first_line;
    std::uint32_t    m_last_line;
    std::uint32_t  m_total_lines;
    fs::path              m_name;
    std::ifstream         m_file;
    std::uint8_t         *m_buff;

    FileData()
        :m_size(0),
         m_first_line(0),
         m_last_line(0),
         m_total_lines(0),
         m_buff(nullptr)
    { }

    ~FileData()
    {
        delete[] m_buff;
    }

    bool read(const std::string& fname)
    {
        if (!fs::exists(fname))
        {
            std::cerr << "File " << fname << " does not exist" << std::endl;
            return false;
        }

        m_name = fs::canonical(fname);
        m_size = fs::file_size(m_name);

        m_file.open(m_name.string(), std::ios::in | std::ios::binary);
        if (!m_file)
        {
            std::cerr << "Failed to open " << m_name.string() << std::endl;
            return false;
        }

        m_buff = new std::uint8_t[m_size];
        if (!m_buff)
        {
            std::cerr << "Failed to allocate " << m_size << " bytes of data" << std::endl;
            return false;
        }

        m_file.read(reinterpret_cast<char*>(m_buff), m_size);
        if (m_file.fail())
        {
            std::cerr << "Failed to read " << m_name.string() << std::endl;
            return false;
        }

        m_total_lines = m_size / BYTES_PER_LINE;
        if (m_size % BYTES_PER_LINE)
            m_total_lines++;

        return true;
    }

    void save()
    {
        std::ofstream out(m_name.string(), std::ios::out | std::ios::binary);
        if (!out)
        {
            std::cerr << "Cannot open " << m_name << std::endl;
            std::exit(1);
        }

        out.write(reinterpret_cast<char*>(m_buff), m_size);
        if (out.fail())
        {
            std::cerr << "Cannot save " << m_name << std::endl;
            std::exit(1);
        }
    }
};

class TScreen
{
private:
    WINDOW*                                m_screen;
    FileData*                               m_fdata;
    int                 m_cy, m_cx, m_lines, m_cols;
    std::uint32_t             m_byte, m_byte_offset;
    bool                                   m_update;
    std::unordered_set<std::uint32_t> m_dirty_cache;

public:
    TScreen(WINDOW* win, FileData* fdata)
        :m_screen(win),
         m_fdata(fdata),
         m_cy(1),
         m_cx(FIRST_BYTE_PER_ROW),
         m_lines(std::min(LINES-2, static_cast<int>(m_fdata->m_total_lines))),
         m_cols(COLS-2),
         m_byte(0),
         m_byte_offset(0),
         m_update(true)
    {
        m_fdata->m_last_line = m_lines;
        keypad(m_screen, true);
    }

    void draw_line(int line)
    {
        auto group = m_fdata->m_first_line + line;
        std::uint32_t bytes_to_draw = BYTES_PER_LINE;

        if (m_fdata->m_total_lines-1 == group &&
            (m_fdata->m_size % BYTES_PER_LINE))
        {
            bytes_to_draw = m_fdata->m_size % BYTES_PER_LINE;
        }

        mvwprintw(m_screen, line + 1, 1, "%08X  ", group*BYTES_PER_LINE);

        std::uint32_t byte_index = group*BYTES_PER_LINE;
        int col = FIRST_BYTE_PER_ROW;

        for (std::uint32_t i = 0; i < BYTES_PER_LINE; ++i, col += 3)
        {
            if (i < bytes_to_draw)
                mvwprintw(m_screen, line + 1, col, "%02X ", m_fdata->m_buff[byte_index++]);
            else
                mvwprintw(m_screen, line + 1, col, "   ");
        }

        mvwprintw(m_screen, line + 1, col++, " ");
        byte_index = group*BYTES_PER_LINE;

        for (std::uint32_t i = 0; i < bytes_to_draw; ++i, col++, byte_index++)
        {
            char c = m_fdata->m_buff[byte_index];
            bool is_dirty = m_dirty_cache.find(byte_index) != m_dirty_cache.end();

            if (byte_index == m_byte)
                wattron(m_screen, A_REVERSE);
            else if (is_dirty)
                wattron(m_screen, COLOR_PAIR(1) | A_REVERSE);

            mvwprintw(m_screen, line + 1, col, "%c", std::isprint(c) ? c : '.');

            if (byte_index == m_byte)
                wattroff(m_screen, A_REVERSE);
            else if (is_dirty)
                wattroff(m_screen, COLOR_PAIR(1) | A_REVERSE);
        }
    }

    void update_screen()
    {
        if (m_update)
        {
            erase();
            for (int line = 0; line < m_lines; line++)
                draw_line(line);

            std::string filename = m_fdata->m_name.filename();
            mvwprintw(m_screen, 0, (COLS - filename.size()) / 2 - 1,
                      "%s", filename.c_str());

            int percentage  = static_cast<float>(m_fdata->m_last_line)/m_fdata->m_total_lines*100;
            mvwprintw(m_screen, LINES-1, COLS - 5, "%d%%", percentage);
            m_update = false;
        }else
        {
            if (m_cy-2 >= 0)
                draw_line(m_cy-2);

            draw_line(m_cy-1);

            if (m_cy < m_lines)
                draw_line(m_cy);
        }
    }

    void erase()
    {
        werase(m_screen);
        box(m_screen, 0, 0);
    }

    void refresh()
    {
        wrefresh(m_screen);
    }

    void resize()
    {
        m_cy = 1;
        m_cx = FIRST_BYTE_PER_ROW;
        m_byte = 0;
        m_byte_offset = 0;
        m_lines = std::min(LINES-2, static_cast<int>(m_fdata->m_total_lines));
        m_cols = COLS-2;
        m_fdata->m_first_line = 0;
        m_fdata->m_last_line = m_lines;
        m_update = true;
    }

    void reset_cursor()
    {
        wmove(m_screen, m_cy, m_cx);
    }

    int get_char()
    {
        return wgetch(m_screen);
    }

    void move_up()
    {
        if (m_cy-1 > 0)
        {
            m_cy--;
            m_byte -= BYTES_PER_LINE;
        }
        else if (m_fdata->m_first_line > 0)
        {
            m_fdata->m_first_line--;
            m_fdata->m_last_line--;
            m_byte -= BYTES_PER_LINE;
            m_update = true;
        }
    }

    void move_down()
    {
        if (m_cy-1 < m_lines - 1)
        {
            m_cy++;
            m_byte += BYTES_PER_LINE;
        }else if (m_fdata->m_last_line < m_fdata->m_total_lines)
        {
            m_fdata->m_first_line++;
            m_fdata->m_last_line++;
            m_byte += BYTES_PER_LINE;
            m_update = true;
        }

        if (m_byte >= m_fdata->m_size)
        {
            m_byte = (m_fdata->m_total_lines-1)*BYTES_PER_LINE;
            m_cx = FIRST_BYTE_PER_ROW;
            m_byte_offset = 0;
            m_update = true;
        }

    }

    void move_left()
    {
        if (m_byte_offset == 0)
        {
            if (m_byte%BYTES_PER_LINE > 0)
            {
                m_byte--;
                m_byte_offset = 1;
                m_cx -= 2;
            }
        }else
        {
            m_byte_offset--;
            m_cx--;
        }
    }

    void move_right()
    {
        auto group_id = m_byte/BYTES_PER_LINE;
        auto row_size = BYTES_PER_LINE;
        if (group_id == m_fdata->m_total_lines-1 &&
            (m_fdata->m_size % BYTES_PER_LINE))
        {
            row_size = m_fdata->m_size % BYTES_PER_LINE;
        }

        if (m_byte_offset > 0)
        {
            if (m_byte%BYTES_PER_LINE < row_size-1 &&
                m_cx+1 <= m_cols)
            {
                m_byte++;
                m_byte_offset = 0;
                m_cx += 2;
            }
        }else if (m_cx <= m_cols)
        {
            m_byte_offset++;
            m_cx++;
        }
    }

    void edit_byte(int c)
    {
        if (c < '0' || c > 'f')
            return;

        uint8_t hex_digit = 0;
        if (c - '0' <= 9)
            hex_digit = c - '0';
        else if (c >= 'A' && c - 'A' <= 5)
            hex_digit = 10 + c - 'A';
        else if (c >= 'a' && c - 'a' <= 5)
            hex_digit = 10 + c - 'a';
        else
            return;

        m_fdata->m_buff[m_byte] &= 0xF0 >> (1-m_byte_offset)*4;
        m_fdata->m_buff[m_byte] |= hex_digit << (1-m_byte_offset)*4;

        m_dirty_cache.insert(m_byte);
    }

    void save()
    {
        m_fdata->save();
        m_dirty_cache.clear();
        m_update = true;
    }
};

static void init_curses()
{
    /* Global curses initialization and setup */
    initscr();
    cbreak();
    noecho();
    start_color();
    init_pair(1, COLOR_GREEN, COLOR_BLACK);
}

static void print_usage_and_exit(char **argv)
{
    std::cerr << "USAGE: " << argv[0] << " file" << std::endl;
    std::exit(1);
}

int main(int argc, char **argv)
{
    if (argc != 2)
        print_usage_and_exit(argv);

    FileData fData;
    if (!fData.read(argv[1]))
        std::exit(1);

    init_curses();
    TScreen win(stdscr, &fData);
    bool end = false;

    while (!end)
    {
        win.update_screen();
        win.reset_cursor();
        win.refresh();
        auto c = win.get_char();
        switch (c)
        {
            case KEY_UP:
                win.move_up();
                break;
            case KEY_DOWN:
                win.move_down();
                break;
            case KEY_RIGHT:
                win.move_right();
                break;
            case KEY_LEFT:
                win.move_left();
                break;
            case KEY_RESIZE:
                win.resize();
                break;
            case ERR:
                std::cerr << "Error: Invalid input!" << std::endl;
                end = true;
                break;
            case 'S':
            case 's':
                win.save();
                break;
            default:
                win.edit_byte(c);
                break;
        }
    }

    endwin();
    return 0;
}
