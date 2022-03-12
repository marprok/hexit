#include <ncurses.h>
#include <cstdint>
#include <string>
#include <iostream>
#include <fstream>
#include <filesystem>
#include <unordered_set>
#include <algorithm>
#include <iterator>

namespace fs = std::filesystem;
typedef std::unordered_set<std::uint32_t> IntCache;

char LEFT_PADDING_FORMAT[sizeof("%%0%dX  ")];

constexpr std::uint32_t LEFT_PADDING_CHARS =  sizeof(std::uint32_t)*2;
constexpr std::uint32_t BYTES_PER_LINE = 16;
constexpr std::uint32_t FIRST_HEX = LEFT_PADDING_CHARS + 2 + 1;
constexpr std::uint32_t FIRST_ASCII = FIRST_HEX + BYTES_PER_LINE*3 + 1;

constexpr int CTRL_Q = 'q' & 0x1F;
constexpr int CTRL_S = 's' & 0x1F;
constexpr int CTRL_X = 'x' & 0x1F;
constexpr int CTRL_A = 'a' & 0x1F;

struct Data
{
    std::uint32_t m_first_line;
    std::uint32_t m_last_line;
    std::uint32_t m_total_lines;
    fs::path      m_name;
    std::ifstream m_file;
    std::vector<std::uint8_t> m_buff;

    Data()
        :m_first_line(0),
         m_last_line(0),
         m_total_lines(0)
    { }

    bool read_from_file(const std::string& fname)
    {
        if (!fs::exists(fname))
        {
            std::cerr << "File " << fname << " does not exist" << std::endl;
            return false;
        }

        m_name = fs::canonical(fname);
        m_file.open(m_name.string(), std::ios::in | std::ios::binary);

        if (!m_file)
        {
            std::cerr << "Failed to open " << m_name.string() << std::endl;
            return false;
        }

        m_buff.reserve(fs::file_size(m_name));
        m_file.unsetf(std::ios_base::skipws);
        m_buff.insert(m_buff.begin(),
                      std::istream_iterator<std::uint8_t>(m_file),
                      std::istream_iterator<std::uint8_t>());

        m_total_lines = m_buff.size() / BYTES_PER_LINE;
        if (m_buff.size() % BYTES_PER_LINE)
            m_total_lines++;

        return true;
    }

    bool read_from_stdin()
    {
        std::cin.unsetf(std::ios_base::skipws);
        std::istream_iterator<std::uint8_t> instream(std::cin);
        m_name = "stdin";

        m_buff.insert(m_buff.begin(),
                      instream,
                      std::istream_iterator<std::uint8_t>());

        m_total_lines = m_buff.size() / BYTES_PER_LINE;
        if (m_buff.size() % BYTES_PER_LINE)
            m_total_lines++;

        // reopen the tty device to allow ncurses to read from stdin
        return freopen("/dev/tty", "rw", stdin) != nullptr;
    }

    void save()
    {
        std::ofstream out(m_name.string(), std::ios::out | std::ios::binary);
        if (!out)
        {
            std::cerr << "Cannot open " << m_name << std::endl;
            std::exit(1);
        }

        out.write(reinterpret_cast<char*>(m_buff.data()), m_buff.size());
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
    enum class Mode
    {
        HEX,
        ASCII,
    };

    WINDOW*       m_screen;
    Data*         m_data;
    bool          m_update;
    Mode          m_mode;
    IntCache      m_dirty_cache;
    std::uint32_t m_current_byte, m_current_byte_offset;
    int           m_cy, m_cx, m_visible_lines, m_cols;

public:
    TScreen(WINDOW* win, Data* data, std::uint32_t starting_byte_offset = 0)
        :m_screen(win),
         m_data(data),
         m_update(true),
         m_mode(Mode::HEX),
         m_current_byte(0), m_current_byte_offset(0),
         m_cy(1), m_cx(FIRST_HEX),
         m_visible_lines(std::min(static_cast<std::uint32_t>(LINES-2), m_data->m_total_lines)),
         m_cols(COLS-2)
    {
        if (starting_byte_offset < m_data->m_buff.size())
        {
            std::uint32_t starting_line = starting_byte_offset/BYTES_PER_LINE;
            if (starting_line > static_cast<std::uint32_t>(m_visible_lines) &&
                m_data->m_total_lines < starting_line + m_visible_lines)
            {
                m_data->m_first_line = starting_line - m_visible_lines + 1;
                m_data->m_last_line = starting_line + 1;
            }else
            {
                m_data->m_first_line = starting_line/m_visible_lines*m_visible_lines;
                m_data->m_last_line = m_data->m_first_line + m_visible_lines;
            }

            m_current_byte = starting_byte_offset;
            m_cy += starting_line - m_data->m_first_line;
            m_cx += starting_byte_offset%BYTES_PER_LINE*3;
        }else
            m_data->m_last_line = m_visible_lines;

        keypad(m_screen, true);
    }

    void draw_line(int line)
    {
        int col = FIRST_HEX;
        auto group = m_data->m_first_line + line;
        std::uint32_t bytes_to_draw = BYTES_PER_LINE;
        std::uint32_t byte_index = group*BYTES_PER_LINE;

        if (m_data->m_total_lines-1 == group && (m_data->m_buff.size() % BYTES_PER_LINE))
            bytes_to_draw = m_data->m_buff.size() % BYTES_PER_LINE;

        mvwprintw(m_screen, line + 1, 1, LEFT_PADDING_FORMAT, byte_index);
        for (std::uint32_t i = 0; i < BYTES_PER_LINE; ++i, col += 3, byte_index++)
        {
            if (i < bytes_to_draw)
            {
                bool is_dirty = m_dirty_cache.find(byte_index) != m_dirty_cache.end();
                if (m_mode == Mode::ASCII && byte_index == m_current_byte)
                    wattron(m_screen, A_REVERSE);
                else if (m_mode == Mode::ASCII && is_dirty)
                    wattron(m_screen, COLOR_PAIR(1) | A_REVERSE);

                mvwprintw(m_screen, line + 1, col, "%02X", m_data->m_buff[byte_index]);
                if (m_mode == Mode::ASCII && byte_index == m_current_byte)
                    wattroff(m_screen, A_REVERSE);
                else if (m_mode == Mode::ASCII && is_dirty)
                    wattroff(m_screen, COLOR_PAIR(1) | A_REVERSE);
            } else
                mvwprintw(m_screen, line + 1, col, "  ");

            mvwprintw(m_screen, line + 1, col+2, " ");
        }

        mvwprintw(m_screen, line + 1, col++, " ");
        byte_index = group*BYTES_PER_LINE;

        for (std::uint32_t i = 0; i < bytes_to_draw; ++i, col++, byte_index++)
        {
            char c = m_data->m_buff[byte_index];
            bool is_dirty = m_dirty_cache.find(byte_index) != m_dirty_cache.end();
            if (m_mode == Mode::HEX && byte_index == m_current_byte)
                wattron(m_screen, A_REVERSE);
            else if (m_mode == Mode::HEX && is_dirty)
                wattron(m_screen, COLOR_PAIR(1) | A_REVERSE);

            mvwprintw(m_screen, line + 1, col, "%c", std::isprint(c) ? c : '.');
            if (m_mode == Mode::HEX &&  byte_index == m_current_byte)
                wattroff(m_screen, A_REVERSE);
            else if (m_mode == Mode::HEX && is_dirty)
                wattroff(m_screen, COLOR_PAIR(1) | A_REVERSE);
        }
    }

    void update_screen()
    {
        if (m_update)
        {
            erase();
            for (int line = 0; line < m_visible_lines; line++)
                draw_line(line);

            std::string filename = m_data->m_name.filename();
            if (m_dirty_cache.empty())
                mvwprintw(m_screen, 0, (COLS - filename.size()) / 2 - 1, "%s", filename.c_str());
            else
                mvwprintw(m_screen, 0, (COLS - filename.size()) / 2 - 1, "*%s", filename.c_str());

            int percentage  = static_cast<float>(m_data->m_last_line)/m_data->m_total_lines*100;
            char mode = m_mode == Mode::ASCII ? 'A' : 'X';
            mvwprintw(m_screen, LINES-1, COLS - 7, "%c/%d%%", mode, percentage);
            m_update = false;
        }else
        {
            if (m_cy-2 >= 0)
                draw_line(m_cy-2);

            draw_line(m_cy-1);

            if (m_cy < m_visible_lines)
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
        m_cx = m_mode == Mode::HEX ? FIRST_HEX : FIRST_ASCII;
        m_visible_lines = std::min(static_cast<std::uint32_t>(LINES-2), m_data->m_total_lines);
        m_cols = COLS-2;

        std::uint32_t current_line = m_current_byte/BYTES_PER_LINE;
        if (m_data->m_total_lines < current_line + m_visible_lines)
        {
            m_data->m_first_line = current_line - m_visible_lines + 1;
            m_data->m_last_line = current_line + 1;
        }else
        {
            m_data->m_first_line = current_line/m_visible_lines*m_visible_lines;
            m_data->m_last_line = m_data->m_first_line + m_visible_lines;
        }

        m_cy += current_line - m_data->m_first_line;
        m_cx += m_current_byte%BYTES_PER_LINE*(m_mode == Mode::HEX ? 3 : 1);
        m_current_byte_offset = 0;

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
            m_current_byte -= BYTES_PER_LINE;
        }
        else if (m_data->m_first_line > 0)
        {
            m_data->m_first_line--;
            m_data->m_last_line--;
            m_current_byte -= BYTES_PER_LINE;
            m_update = true;
        }
    }

    void page_up()
    {
        if (m_data->m_first_line >= static_cast<std::uint32_t>(m_visible_lines-1))
        {
            m_data->m_first_line -= m_visible_lines-1;
            m_data->m_last_line -= m_visible_lines-1;
            m_current_byte -= (m_visible_lines-1)*BYTES_PER_LINE;
            m_update = true;
        }
    }

    void move_down()
    {
        if (m_cy-1 < m_visible_lines - 1)
        {
            m_cy++;
            m_current_byte += BYTES_PER_LINE;
        }else if (m_data->m_last_line < m_data->m_total_lines)
        {
            m_data->m_first_line++;
            m_data->m_last_line++;
            m_current_byte += BYTES_PER_LINE;
            m_update = true;
        }

        if (m_current_byte >= m_data->m_buff.size())
        {
            m_current_byte = (m_data->m_total_lines-1)*BYTES_PER_LINE;
            m_cx = (m_mode == Mode::HEX) ? FIRST_HEX : FIRST_ASCII;
            m_current_byte_offset = 0;
            m_update = true;
        }
    }

    void page_down()
    {
        if (m_data->m_last_line + m_visible_lines-1 < m_data->m_total_lines)
        {
            m_data->m_first_line += m_visible_lines-1;
            m_data->m_last_line += m_visible_lines-1;
            m_current_byte += (m_visible_lines-1)*BYTES_PER_LINE;
            m_update = true;
        }

        if (m_current_byte >= m_data->m_buff.size())
        {
            m_current_byte = (m_data->m_total_lines-1)*BYTES_PER_LINE;
            m_cx = (m_mode == Mode::HEX) ? FIRST_HEX : FIRST_ASCII;
            m_current_byte_offset = 0;
            m_update = true;
        }
    }

    void move_left()
    {
        if (m_mode == Mode::HEX)
        {
            if (m_current_byte_offset == 0)
            {
                if (m_cx < m_cols && m_current_byte%BYTES_PER_LINE > 0)
                {
                    m_current_byte--;
                    m_current_byte_offset = 1;
                    m_cx -= 2;
                }
            }else
            {
                m_current_byte_offset--;
                m_cx--;
            }
        }else
        {
            if (m_cx < m_cols && m_current_byte%BYTES_PER_LINE > 0)
            {
                m_cx--;
                m_current_byte--;
            }
        }
    }

    void move_right()
    {
        auto group_id = m_current_byte/BYTES_PER_LINE;
        auto row_size = BYTES_PER_LINE;

        if (group_id == m_data->m_total_lines-1 && (m_data->m_buff.size() % BYTES_PER_LINE))
            row_size = m_data->m_buff.size() % BYTES_PER_LINE;

        if (m_mode == Mode::HEX)
        {
            if (m_current_byte_offset > 0)
            {
                if (m_cx < m_cols &&
                    m_current_byte%BYTES_PER_LINE < row_size-1)
                {
                    m_current_byte++;
                    m_current_byte_offset = 0;
                    m_cx += 2;
                }
            }else if (m_cx < m_cols)
            {
                m_current_byte_offset++;
                m_cx++;
            }
        }else
        {
            if (m_cx < m_cols && m_current_byte%BYTES_PER_LINE < row_size-1)
            {
                m_cx++;
                m_current_byte++;
            }
        }
    }

    void edit_byte(int c)
    {
        if (m_mode == Mode::ASCII && std::isprint(c))
        {
            m_data->m_buff[m_current_byte] = c;
        } else
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

            m_data->m_buff[m_current_byte] &= 0xF0 >> (1-m_current_byte_offset)*4;
            m_data->m_buff[m_current_byte] |= hex_digit << (1-m_current_byte_offset)*4;
        }

        if (m_dirty_cache.empty())
            m_update = true;

        m_dirty_cache.insert(m_current_byte);
    }

    void save()
    {
        m_data->save();
        m_dirty_cache.clear();
        m_update = true;
    }

    void toggle_ascii_mode()
    {
        if (m_mode == Mode::ASCII)
            return;

        m_mode = Mode::ASCII;
        m_update = true;
        m_cx = FIRST_ASCII + m_current_byte % BYTES_PER_LINE;
        m_current_byte_offset = 0;
    }

    void toggle_hex_mode()
    {
        if (m_mode == Mode::HEX)
            return;

        m_mode = Mode::HEX;
        m_update = true;
        m_cx = FIRST_HEX + (m_current_byte % BYTES_PER_LINE)*3;
    }
};

static void init_ncurses()
{
    // Global ncurses initialization and setup
    initscr();
    raw();
    noecho();
    start_color();
    init_pair(1, COLOR_GREEN, COLOR_BLACK);

    std::sprintf(LEFT_PADDING_FORMAT, "%%0%dX  ", LEFT_PADDING_CHARS);
}

static void print_help_and_exit(char *bin)
{
    std::cerr << "USAGE: " << bin << " [-f file] [-o offset]" << std::endl;
    std::exit(EXIT_SUCCESS);
}

char* get_arg(int argc, char **argv, const std::string &arg)
{
    auto res = std::find(argv, argv + argc, arg);
    if (res != argv + argc && ++res != argv + argc)
        return *res;

    return nullptr;
}

bool get_flag(int argc, char **argv, const std::string &flag)
{
    auto res = std::find(argv, argv + argc, flag);
    return res != (argv + argc);
}

std::uint32_t get_starting_offset(const char* offset)
{
    if (!offset)
        return 0;

    std::string starting_offset(offset);
    if (starting_offset.size() > 2 && starting_offset[0] == '0' &&
        (starting_offset[1] == 'x' || starting_offset[1] == 'X'))
        return std::stoll(starting_offset, nullptr, 16);

    return std::stoll(starting_offset, nullptr);
}

int main(int argc, char **argv)
{
    auto input_file = get_arg(argc - 1, argv + 1, "-f");
    auto starting_offset = get_arg(argc - 1, argv + 1, "-o");
    auto help = get_flag(argc - 1, argv + 1, "-h");

    if (help)
        print_help_and_exit(*argv);

    Data data;
    if (!input_file && !data.read_from_stdin())
    {
        std::cerr << "Could not read from standard input!\n";
        std::exit(EXIT_FAILURE);
    }else if (input_file && !data.read_from_file(input_file))
    {
        std::cerr << "Could not read from <" << input_file << ">\n";
        std::exit(EXIT_FAILURE);
    }

    init_ncurses();
    TScreen win(stdscr, &data, get_starting_offset(starting_offset));

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
            case KEY_PPAGE:
                win.page_up();
                break;
            case KEY_NPAGE:
                win.page_down();
                break;
            case KEY_RESIZE:
                win.resize();
                break;
            case CTRL_S:
                win.save();
                break;
            case CTRL_Q:
                end = true;
                break;
            case CTRL_X:
                win.toggle_hex_mode();
                break;
            case CTRL_A:
                win.toggle_ascii_mode();
                break;
            default:
                win.edit_byte(c);
                break;
        }
    }

    endwin();
    return 0;
}
