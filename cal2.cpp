// modern_ccal.cpp - MSYS2 compatible single-file ccal clone
#include <iostream>
#include <iomanip>
#include <ctime>
#include <vector>
#include <string>
#include <sstream>
#include <fstream>
#include <array>
#include <cstdlib>
#include <algorithm>
#include <map>
#include <filesystem>

#ifdef _WIN32
#include <windows.h>
#ifdef max
#undef max
#endif
#ifdef min
#undef min
#endif
#include <direct.h>
#define LOCALTIME(t, result) *(result) = *localtime((t))
#define HOME_ENV "USERPROFILE"
#else
#include <unistd.h>
#include <pwd.h>
#define LOCALTIME(t, result) localtime_r((t), (result))
#define HOME_ENV "HOME"
#endif

// Color codes for different platforms
#ifdef _WIN32
constexpr char RESET[] = "\033[0m";
constexpr char RED[]   = "\033[31m";
constexpr char BLUE[]  = "\033[34m";
constexpr char GREEN[] = "\033[32m";
constexpr char YELLOW[] = "\033[33m";
constexpr char MAGENTA[] = "\033[35m";
constexpr char CYAN[] = "\033[36m";
constexpr char REV[]   = "\033[7m";
constexpr char BLACK[] = "\033[30m";
constexpr char WHITE[] = "\033[37m";
constexpr char BRIGHT_RED[] = "\033[91m";
constexpr char BRIGHT_GREEN[] = "\033[92m";
constexpr char BRIGHT_YELLOW[] = "\033[93m";
constexpr char BRIGHT_BLUE[] = "\033[94m";
constexpr char BRIGHT_MAGENTA[] = "\033[95m";
constexpr char BRIGHT_CYAN[] = "\033[96m";
constexpr char BRIGHT_WHITE[] = "\033[97m";
#else
constexpr char RESET[] = "\x1b[0m";
constexpr char RED[]   = "\x1b[31m";
constexpr char BLUE[]  = "\x1b[34m";
constexpr char GREEN[] = "\x1b[32m";
constexpr char YELLOW[] = "\x1b[33m";
constexpr char MAGENTA[] = "\x1b[35m";
constexpr char CYAN[] = "\x1b[36m";
constexpr char REV[]   = "\x1b[7m";
constexpr char BLACK[] = "\x1b[30m";
constexpr char WHITE[] = "\x1b[37m";
constexpr char BRIGHT_RED[] = "\x1b[91m";
constexpr char BRIGHT_GREEN[] = "\x1b[92m";
constexpr char BRIGHT_YELLOW[] = "\x1b[93m";
constexpr char BRIGHT_BLUE[] = "\x1b[94m";
constexpr char BRIGHT_MAGENTA[] = "\x1b[95m";
constexpr char BRIGHT_CYAN[] = "\x1b[96m";
constexpr char BRIGHT_WHITE[] = "\x1b[97m";
#endif

// Color configuration structure
struct ColorConfig {
#ifdef _WIN32
    std::string sunday_title = BRIGHT_RED;
    std::string saturday_title = BRIGHT_BLUE;
    std::string workday_title = "";
    std::string sunday_date = BRIGHT_RED;
    std::string saturday_date = BRIGHT_BLUE;
    std::string workday_date = "";
    std::string holiday = BRIGHT_RED;
    std::string birthday = BRIGHT_MAGENTA;
    std::string reminder = BRIGHT_CYAN;
    // Month colors - colorful defaults
    std::string january = BRIGHT_CYAN;
    std::string february = BRIGHT_MAGENTA;
    std::string march = BRIGHT_GREEN;
    std::string april = BRIGHT_YELLOW;
    std::string may = BRIGHT_RED;
    std::string june = BRIGHT_BLUE;
    std::string july = BRIGHT_YELLOW;
    std::string august = BRIGHT_GREEN;
    std::string september = BRIGHT_MAGENTA;
    std::string october = BRIGHT_RED;
    std::string november = BRIGHT_CYAN;
    std::string december = BRIGHT_BLUE;
#else
    std::string sunday_title = RED;
    std::string saturday_title = BLUE;
    std::string workday_title = "";
    std::string sunday_date = RED;
    std::string saturday_date = BLUE;
    std::string workday_date = "";
    std::string holiday = RED;
    std::string birthday = MAGENTA;
    std::string reminder = CYAN;
    // Month colors - colorful defaults
    std::string january = CYAN;
    std::string february = MAGENTA;
    std::string march = GREEN;
    std::string april = YELLOW;
    std::string may = RED;
    std::string june = BLUE;
    std::string july = YELLOW;
    std::string august = GREEN;
    std::string september = MAGENTA;
    std::string october = RED;
    std::string november = CYAN;
    std::string december = BLUE;
#endif
};

// Global color configuration
ColorConfig colors;

// Event types
enum class EventType {
    HOLIDAY,
    BIRTHDAY,
    REMINDER
};

// Event structure
struct Event {
    int month;
    int day;
    std::string description;
    EventType type;
};

// Global events map: key = "MM-DD", value = vector of events
std::map<std::string, std::vector<Event>> events;

// Function to convert color name to ANSI code
std::string get_color_code(const std::string& color_name) {
    std::string lower_name = color_name;
    std::transform(lower_name.begin(), lower_name.end(), lower_name.begin(), ::tolower);

    // Basic colors
    if (lower_name == "red") return RED;
    if (lower_name == "blue") return BLUE;
    if (lower_name == "green") return GREEN;
    if (lower_name == "yellow") return YELLOW;
    if (lower_name == "magenta") return MAGENTA;
    if (lower_name == "cyan") return CYAN;
    if (lower_name == "black") return BLACK;
    if (lower_name == "white") return WHITE;
    
    // Bright colors
    if (lower_name == "bright_red") return BRIGHT_RED;
    if (lower_name == "bright_green") return BRIGHT_GREEN;
    if (lower_name == "bright_yellow") return BRIGHT_YELLOW;
    if (lower_name == "bright_blue") return BRIGHT_BLUE;
    if (lower_name == "bright_magenta") return BRIGHT_MAGENTA;
    if (lower_name == "bright_cyan") return BRIGHT_CYAN;
    if (lower_name == "bright_white") return BRIGHT_WHITE;
    
    // 256-color support (common colors)
    if (lower_name == "orange") return "\033[38;5;208m";
    if (lower_name == "purple") return "\033[38;5;129m";
    if (lower_name == "pink") return "\033[38;5;205m";
    if (lower_name == "brown") return "\033[38;5;130m";
    if (lower_name == "gray" || lower_name == "grey") return "\033[38;5;244m";
    if (lower_name == "dark_red") return "\033[38;5;88m";
    if (lower_name == "dark_green") return "\033[38;5;22m";
    if (lower_name == "dark_blue") return "\033[38;5;18m";
    if (lower_name == "light_red") return "\033[38;5;203m";
    if (lower_name == "light_green") return "\033[38;5;119m";
    if (lower_name == "light_blue") return "\033[38;5;117m";
    if (lower_name == "gold") return "\033[38;5;220m";
    if (lower_name == "silver") return "\033[38;5;250m";
    if (lower_name == "lime") return "\033[38;5;154m";
    if (lower_name == "navy") return "\033[38;5;17m";
    if (lower_name == "maroon") return "\033[38;5;52m";
    if (lower_name == "olive") return "\033[38;5;58m";
    if (lower_name == "teal") return "\033[38;5;30m";
    if (lower_name == "aqua") return "\033[38;5;51m";
    if (lower_name == "fuchsia") return "\033[38;5;201m";
    
    // Colors optimized for light backgrounds
    if (lower_name == "dark_gray" || lower_name == "dark_grey") return "\033[38;5;236m";
    if (lower_name == "charcoal") return "\033[38;5;238m";
    if (lower_name == "slate") return "\033[38;5;240m";
    if (lower_name == "steel") return "\033[38;5;67m";
    if (lower_name == "indigo") return "\033[38;5;54m";
    if (lower_name == "violet") return "\033[38;5;93m";
    if (lower_name == "crimson") return "\033[38;5;160m";
    if (lower_name == "forest") return "\033[38;5;28m";
    if (lower_name == "emerald") return "\033[38;5;34m";
    if (lower_name == "sapphire") return "\033[38;5;19m";
    if (lower_name == "amber") return "\033[38;5;214m";
    if (lower_name == "coral") return "\033[38;5;209m";
    if (lower_name == "rust") return "\033[38;5;166m";
    if (lower_name == "bronze") return "\033[38;5;136m";
    if (lower_name == "copper") return "\033[38;5;173m";
    if (lower_name == "chocolate") return "\033[38;5;94m";
    if (lower_name == "coffee") return "\033[38;5;52m";
    if (lower_name == "wine") return "\033[38;5;89m";
    if (lower_name == "plum") return "\033[38;5;96m";
    if (lower_name == "midnight") return "\033[38;5;17m";
    if (lower_name == "deep_blue") return "\033[38;5;20m";
    if (lower_name == "deep_green") return "\033[38;5;22m";
    if (lower_name == "deep_red") return "\033[38;5;88m";
    if (lower_name == "deep_purple") return "\033[38;5;55m";
    if (lower_name == "deep_orange") return "\033[38;5;130m";
    if (lower_name == "royal_blue") return "\033[38;5;21m";
    if (lower_name == "royal_purple") return "\033[38;5;57m";
    if (lower_name == "sea_green") return "\033[38;5;29m";
    if (lower_name == "sky_blue") return "\033[38;5;75m";
    if (lower_name == "rose") return "\033[38;5;168m";
    if (lower_name == "salmon") return "\033[38;5;174m";
    if (lower_name == "peach") return "\033[38;5;216m";
    if (lower_name == "mint") return "\033[38;5;121m";
    if (lower_name == "lavender") return "\033[38;5;183m";
    if (lower_name == "turquoise") return "\033[38;5;80m";
    
    // Default/none
    if (lower_name == "none" || lower_name == "default" || lower_name == "") return "";

    // If color name not recognized, return empty string (default color)
    return "";
}

std::string get_home_dir() {
    const char* home = std::getenv(HOME_ENV);
    if (home) return std::string(home);

#ifdef _WIN32
    return std::string();
#else
    struct passwd* pw = getpwuid(getuid());
    return pw ? std::string(pw->pw_dir) : std::string();
#endif
}

std::string get_config_path() {
    std::string home = get_home_dir();
    if (home.empty()) return "";
    
#ifdef _WIN32
    return home + "\\.cal2\\cal2.ini";
#else
    return home + "/.cal2/cal2.ini";
#endif
}

void parse_date(const std::string& date_str, int& month, int& day) {
    month = 0;
    day = 0;
    
    size_t pos = date_str.find('/');
    if (pos != std::string::npos) {
        try {
            month = std::stoi(date_str.substr(0, pos));
            day = std::stoi(date_str.substr(pos + 1));
        } catch (const std::exception& e) {
            std::cerr << "Error parsing date " << date_str << ": " << e.what() << std::endl;
        }
    } else {
        // Try MM-DD format
        pos = date_str.find('-');
        if (pos != std::string::npos) {
            try {
                month = std::stoi(date_str.substr(0, pos));
                day = std::stoi(date_str.substr(pos + 1));
            } catch (const std::exception& e) {
                std::cerr << "Error parsing date " << date_str << ": " << e.what() << std::endl;
            }
        }
    }
}

void load_events() {
    std::string config_path = get_config_path();
    if (config_path.empty()) {
        std::cerr << "Warning: Could not determine home directory\n";
        return;
    }
    
    // Debug: print the config path being used
    // std::cerr << "Looking for config file at: " << config_path << std::endl;
    
    std::ifstream file(config_path);
    if (!file.is_open()) {
        std::cerr << "Warning: Could not open config file: " << config_path << std::endl;
        
        // Try alternative locations for MSYS2
        std::vector<std::string> alt_paths = {
            "./cal2.ini",  // Current directory
            "cal2.ini",    // Current directory without ./
            std::string(std::getenv("HOME") ? std::getenv("HOME") : "") + "/.cal2/cal2.ini",
            std::string(std::getenv("USERPROFILE") ? std::getenv("USERPROFILE") : "") + "/.cal2/cal2.ini"
        };
        
        for (const auto& alt_path : alt_paths) {
            if (alt_path.empty()) continue;
            // std::cerr << "Trying: " << alt_path << std::endl;
            std::ifstream alt_file(alt_path);
            if (alt_file.is_open()) {
	        // std::cerr << "Found config at: " << alt_path << std::endl;
                file = std::move(alt_file);
                break;
            }
        }
        
        if (!file.is_open()) {
            std::cerr << "No config file found. Create cal2.ini in current directory or ~/.cal2/cal2.ini\n";
            return;
        }
    } else {
      // std::cerr << "Successfully opened config file: " << config_path << std::endl;
    }
    
    std::string line;
    int events_loaded = 0;
    std::string current_section = "";

    while (std::getline(file, line)) {
        // Skip empty lines and comments
        if (line.empty() || line[0] == '#' || line[0] == ';') continue;

        // Trim whitespace
        line.erase(0, line.find_first_not_of(" \t"));
        line.erase(line.find_last_not_of(" \t") + 1);

        // Check for section headers [section]
        if (line[0] == '[' && line.back() == ']') {
            current_section = line.substr(1, line.length() - 2);
            std::transform(current_section.begin(), current_section.end(), current_section.begin(), ::tolower);
            continue;
        }

        // Handle color configuration
        if (current_section == "colors") {
            size_t eq_pos = line.find('=');
            if (eq_pos != std::string::npos) {
                std::string key = line.substr(0, eq_pos);
                std::string value = line.substr(eq_pos + 1);

                // Trim key and value
                key.erase(0, key.find_first_not_of(" \t"));
                key.erase(key.find_last_not_of(" \t") + 1);
                value.erase(0, value.find_first_not_of(" \t"));
                value.erase(value.find_last_not_of(" \t") + 1);

                std::transform(key.begin(), key.end(), key.begin(), ::tolower);

                if (key == "sunday_title") colors.sunday_title = get_color_code(value);
                else if (key == "saturday_title") colors.saturday_title = get_color_code(value);
                else if (key == "workday_title") colors.workday_title = get_color_code(value);
                else if (key == "sunday_date") colors.sunday_date = get_color_code(value);
                else if (key == "saturday_date") colors.saturday_date = get_color_code(value);
                else if (key == "workday_date") colors.workday_date = get_color_code(value);
                else if (key == "holiday") colors.holiday = get_color_code(value);
                else if (key == "birthday") colors.birthday = get_color_code(value);
                else if (key == "reminder") colors.reminder = get_color_code(value);
                // Month colors
                else if (key == "january" || key == "jan" || key == "1") colors.january = get_color_code(value);
                else if (key == "february" || key == "feb" || key == "2") colors.february = get_color_code(value);
                else if (key == "march" || key == "mar" || key == "3") colors.march = get_color_code(value);
                else if (key == "april" || key == "apr" || key == "4") colors.april = get_color_code(value);
                else if (key == "may" || key == "5") colors.may = get_color_code(value);
                else if (key == "june" || key == "jun" || key == "6") colors.june = get_color_code(value);
                else if (key == "july" || key == "jul" || key == "7") colors.july = get_color_code(value);
                else if (key == "august" || key == "aug" || key == "8") colors.august = get_color_code(value);
                else if (key == "september" || key == "sep" || key == "9") colors.september = get_color_code(value);
                else if (key == "october" || key == "oct" || key == "10") colors.october = get_color_code(value);
                else if (key == "november" || key == "nov" || key == "11") colors.november = get_color_code(value);
                else if (key == "december" || key == "dec" || key == "12") colors.december = get_color_code(value);
            }
            continue;
        }

        // Handle events (default section or any other section)
        std::istringstream iss(line);
        std::string date_str, description;

        if (iss >> date_str) {
            // Get the rest of the line as description
            std::getline(iss, description);
            if (!description.empty() && description[0] == ' ') {
                description = description.substr(1); // Remove leading space
            }

            int month = 0, day = 0;
            parse_date(date_str, month, day);

            if (month >= 1 && month <= 12 && day >= 1 && day <= 31) {
                Event event;
                event.month = month;
                event.day = day;
                event.description = description;

                // Determine event type based on section or description
                if (current_section == "holidays" || current_section == "holiday") {
                    event.type = EventType::HOLIDAY;
                } else if (current_section == "birthdays" || current_section == "birthday") {
                    event.type = EventType::BIRTHDAY;
                } else if (current_section == "reminders" || current_section == "reminder") {
                    event.type = EventType::REMINDER;
                } else {
                    // Auto-detect based on description
                    if (description.find("birthday") != std::string::npos ||
                        description.find("Birthday") != std::string::npos ||
                        description.find("BIRTHDAY") != std::string::npos) {
                        event.type = EventType::BIRTHDAY;
                    } else if (description.find("holiday") != std::string::npos ||
                              description.find("Holiday") != std::string::npos ||
                              description.find("HOLIDAY") != std::string::npos) {
                        event.type = EventType::HOLIDAY;
                    } else {
                        event.type = EventType::REMINDER;
                    }
                }

                std::string key = std::to_string(month) + "-" + std::to_string(day);
                events[key].push_back(event);
                events_loaded++;
            } else {
                std::cerr << "Invalid date: " << month << "/" << day << std::endl;
            }
        }
    }
    
    // std::cerr << "Total events loaded: " << events_loaded << std::endl;
}

std::string get_event_color(const Event& event) {
    switch (event.type) {
        case EventType::HOLIDAY: return colors.holiday;
        case EventType::BIRTHDAY: return colors.birthday;
        case EventType::REMINDER: return colors.reminder;
        default: return GREEN;
    }
}

bool has_events(int month, int day) {
    std::string key = std::to_string(month) + "-" + std::to_string(day);
    return events.find(key) != events.end();
}

std::vector<Event> get_events(int month, int day) {
    std::string key = std::to_string(month) + "-" + std::to_string(day);
    auto it = events.find(key);
    return (it != events.end()) ? it->second : std::vector<Event>();
}

std::string get_month_color(int month) {
    switch (month) {
        case 1: return colors.january;
        case 2: return colors.february;
        case 3: return colors.march;
        case 4: return colors.april;
        case 5: return colors.may;
        case 6: return colors.june;
        case 7: return colors.july;
        case 8: return colors.august;
        case 9: return colors.september;
        case 10: return colors.october;
        case 11: return colors.november;
        case 12: return colors.december;
        default: return "";
    }
}

int weekday(int y, int m, int d, bool monday_first = false) {
    if (m <= 2) { m += 12; --y; }
    int K = y % 100;
    int J = y / 100;
    int h = (d + 13*(m + 1)/5 + K + K/4 + J/4 + 5*J) % 7;
    int result = (h + 6) % 7;  // 0 = Sunday
    if (monday_first) {
        result = (result + 6) % 7;  // 0 = Monday
    }
    return result;
}

int days_in_month(int y, int m) {
    static const int md[] = {0,31,28,31,30,31,30,31,31,30,31,30,31};
    if (m == 2 && (y % 4 == 0 && (y % 100 != 0 || y % 400 == 0))) return 29;
    return md[m];
}

// Structure to hold month data for horizontal printing
struct MonthData {
    std::string header;
    std::vector<std::string> weeks;
    int year;
    int month;
};

MonthData get_month_data(int y, int m, int today_y, int today_m, int today_d, bool monday_first = false) {
    static const char* names[] = {
        "Jan","Feb","Mar","Apr","May","Jun",
        "Jul","Aug","Sep","Oct","Nov","Dec"
    };
    
    MonthData data;
    data.year = y;
    data.month = m;
    
    // Compact header format: "Jan 2024" (8 chars max)
    std::string year_str = std::to_string(y);
    std::string month_color = get_month_color(m);
    std::string month_name = std::string(names[m - 1]) + " " + year_str;
    
    // Calculate padding for plain text (without color codes)
    int header_len = month_name.length();
    if (header_len > 20) {
        month_name = month_name.substr(0, 20);
        header_len = 20;
    }
    int padding = (20 - header_len) / 2;
    
    // Build header with color codes
    data.header = std::string(padding, ' ') + month_color + month_name + RESET + std::string(20 - header_len - padding, ' ');
    
    int start = weekday(y, m, 1, monday_first);
    int dim = days_in_month(y, m);
    
    // Calculate total weeks needed
    int total_weeks = (start + dim - 1) / 7 + 1;
    
    for (int week = 0; week < total_weeks; ++week) {
        std::string week_str = "";
        
        for (int wday = 0; wday < 7; ++wday) {
            int day_num = week * 7 + wday - start + 1;
            
            if (day_num < 1 || day_num > dim) {
                // Empty space - 2 chars + space
                week_str += "  ";
                if (wday < 6) week_str += " ";
            } else {
                // Format the day
                bool is_today = (y == today_y && m == today_m && day_num == today_d);
                bool has_event = has_events(m, day_num);
                
                std::string day_str = "";
                
                // Apply colors in priority order: today > events > weekend
                if (is_today) {
                    day_str += REV;
                } else if (has_event) {
                    auto day_events = get_events(m, day_num);
                    if (!day_events.empty()) {
                        day_str += get_event_color(day_events[0]);
                    }
                } else if ((monday_first && wday == 6) || (!monday_first && wday == 0)) {
                    day_str += colors.sunday_date;  // Sunday
                } else if ((monday_first && wday == 5) || (!monday_first && wday == 6)) {
                    day_str += colors.saturday_date; // Saturday
                } else {
                    day_str += colors.workday_date; // Workday
                }
                
                // Format day with exactly 3 chars per day including space
                if (day_num < 10) {
                    day_str += " " + std::to_string(day_num);
                } else {
                    day_str += std::to_string(day_num);
                }
                
                day_str += RESET;
                week_str += day_str;
                
                // Add space after day except for last day of week
                if (wday < 6) {
                    week_str += " ";
                }
            }
        }
        
        data.weeks.push_back(week_str);
    }
    
    return data;
}

void print_month_horizontal(const std::vector<MonthData>& months, bool monday_first = false) {
    if (months.empty()) return;

    // Print headers - each month is 20 chars, 1 space between (20+1+20+1+20 = 62)
    for (size_t i = 0; i < months.size(); ++i) {
        std::cout << months[i].header;
        if (i < months.size() - 1) std::cout << " ";
    }
    std::cout << "\n";

    // Print day headers - Su Mo Tu We Th Fr Sa = 20 chars exactly
    for (size_t i = 0; i < months.size(); ++i) {
        if (monday_first) {
            std::cout << colors.workday_title << "Mo Tu We Th Fr " << RESET
                      << colors.saturday_title << "Sa" << RESET << " "
                      << colors.sunday_title << "Su" << RESET;
        } else {
            std::cout << colors.sunday_title << "Su" << RESET << " "
                      << colors.workday_title << "Mo Tu We Th Fr " << RESET
                      << colors.saturday_title << "Sa" << RESET;
        }
        if (i < months.size() - 1) std::cout << " ";
    }
    std::cout << "\n";

    // Find the maximum number of weeks
    size_t max_weeks = 0;
    for (const auto& month : months) {
        max_weeks = std::max(max_weeks, month.weeks.size());
    }

    // Print weeks - each week is exactly 20 chars
    for (size_t week = 0; week < max_weeks; ++week) {
        for (size_t i = 0; i < months.size(); ++i) {
            if (week < months[i].weeks.size()) {
                std::cout << months[i].weeks[week];
            } else {
                std::cout << "                    "; // 20 spaces for empty weeks
            }
            if (i < months.size() - 1) std::cout << " ";
        }
        std::cout << "\n";
    }
}

void print_month_vertical(int y, int m, int today_y, int today_m, int today_d, bool monday_first = false) {
    static const char* names[] = {
        "January","February","March","April","May","June",
        "July","August","September","October","November","December"
    };
    
    std::string month_color = get_month_color(m);
    if (monday_first) {
        std::cout << "     " << month_color << names[m - 1] << " " << y << RESET << "\n"
                  << colors.workday_title << "Mo Tu We Th Fr " << RESET
                  << colors.saturday_title << "Sa " << RESET
                  << colors.sunday_title << "Su" << RESET << "\n";
    } else {
        std::cout << "     " << month_color << names[m - 1] << " " << y << RESET << "\n"
                  << colors.sunday_title << "Su " << RESET
                  << colors.workday_title << "Mo Tu We Th Fr " << RESET
                  << colors.saturday_title << "Sa" << RESET << "\n";
    }
    
    int start = weekday(y, m, 1, monday_first);
    int dim = days_in_month(y, m);
    for (int i = 0; i < start; ++i) std::cout << "   ";
    for (int d = 1; d <= dim; ++d) {
        int wday = weekday(y, m, d, monday_first);
        bool is_today = (y == today_y && m == today_m && d == today_d);
        bool has_event = has_events(m, d);
        
        // Apply colors in priority order: today > events > weekend
        if (is_today) {
            std::cout << REV;
        } else if (has_event) {
            auto day_events = get_events(m, d);
            if (!day_events.empty()) {
                std::cout << get_event_color(day_events[0]);
            }
        } else if ((monday_first && wday == 6) || (!monday_first && wday == 0)) {
            std::cout << colors.sunday_date;  // Sunday
        } else if ((monday_first && wday == 5) || (!monday_first && wday == 6)) {
            std::cout << colors.saturday_date; // Saturday
        } else {
            std::cout << colors.workday_date; // Workday
        }
        
        if (d < 10) {
            std::cout << " " << d << RESET << " ";
        } else {
            std::cout << d << RESET << " ";
        }
        if (wday == 6) std::cout << "\n";
    }
    std::cout << "\n";
}

void print_year(int year, int today_y, int today_m, int today_d, bool monday_first = false) {
    // Center year in 80 columns
    std::string year_str = std::to_string(year);
    int padding = (80 - year_str.length()) / 2;
    std::cout << std::string(padding, ' ') << year << "\n\n";
    
    for (int row = 0; row < 4; ++row) {
        std::vector<MonthData> months;
        for (int col = 0; col < 3; ++col) {
            int month = row * 3 + col + 1;
            if (month <= 12) {
                months.push_back(get_month_data(year, month, today_y, today_m, today_d, monday_first));
            }
        }
        if (!months.empty()) {
            print_month_horizontal(months, monday_first);
            std::cout << "\n";
        }
    }
}

void print_twelve_months(int start_year, int start_month, int today_y, int today_m, int today_d, bool monday_first = false) {
    // Center year in 80 columns
    std::string year_str = std::to_string(start_year);
    int padding = (80 - year_str.length()) / 2;
    std::cout << std::string(padding, ' ') << start_year << "\n\n";
    
    for (int row = 0; row < 4; ++row) {
        std::vector<MonthData> months;
        for (int col = 0; col < 3; ++col) {
            int month_offset = row * 3 + col;
            int current_month = start_month + month_offset;
            int current_year = start_year + (current_month - 1) / 12;
            int adjusted_month = ((current_month - 1) % 12) + 1;
            
            months.push_back(get_month_data(current_year, adjusted_month, today_y, today_m, today_d, monday_first));
        }
        if (!months.empty()) {
            print_month_horizontal(months, monday_first);
            std::cout << "\n";
        }
    }
}

void print_events_legend() {
    std::cout << "\nLegend:\n";
#ifdef _WIN32
    // Use asterisk for better compatibility in Visual Studio console
    std::cout << colors.holiday << "*" << RESET << " Holiday  ";
    std::cout << colors.birthday << "*" << RESET << " Birthday  ";
    std::cout << colors.reminder << "*" << RESET << " Reminder\n";
#else
    std::cout << colors.holiday << "●" << RESET << " Holiday  ";
    std::cout << colors.birthday << "●" << RESET << " Birthday  ";
    std::cout << colors.reminder << "●" << RESET << " Reminder\n";
#endif
}

void print_help() {
    std::cout << "Usage: cal2 [options] [[[day] month] year]\n";
    std::cout << "mycal [options] <monthname> [year]\n";
    std::cout << "Options:\n";
    std::cout << "  -3, --three           Display prev/current/next month\n";
    std::cout << "  -m, --monday          Monday as first day of week\n";
    std::cout << "  -y, --year            Display a calendar for the current year\n";
    std::cout << "  -Y, --twelve          Display the next twelve months\n";
    std::cout << "  -h, --help            Display this help\n\n";
    std::cout << "Events are loaded from ~/.cal2/cal2.ini\n";
    std::cout << "Format: MM/DD Description (e.g., 12/25 Christmas)\n";
}

int main(int argc, char* argv[]) {
    int y, m;
    time_t now = time(nullptr);
    struct tm lt;
    LOCALTIME(&now, &lt);
    y = lt.tm_year + 1900;
    m = lt.tm_mon + 1;
    int d = lt.tm_mday;

    bool show3 = false;
    bool show_year = false;
    bool show_twelve = false;
    bool show_help = false;
    bool monday_first = false;
    
    // Parse command line arguments
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "-3" || arg == "--three") {
            show3 = true;
        } else if (arg == "-y" || arg == "--year") {
            show_year = true;
        } else if (arg == "-Y" || arg == "--twelve") {
            show_twelve = true;
        } else if (arg == "-m" || arg == "--monday") {
            monday_first = true;
        } else if (arg == "-h" || arg == "--help") {
            show_help = true;
        } else if (arg[0] != '-') {
            // Try to parse as month/year
            try {
                if (i == argc - 2) {
                    // Two arguments: month year
                    m = std::stoi(argv[i]);
                    y = std::stoi(argv[i + 1]);
                    break;
                } else if (i == argc - 1) {
                    // Single argument: could be month or year
                    int val = std::stoi(arg);
                    if (val >= 1 && val <= 12) {
                        m = val;
                    } else if (val >= 1900 && val <= 2100) {
                        y = val;
                    }
                }
            } catch (const std::exception&) {
                // Ignore parsing errors
            }
        }
    }

    if (show_help) {
        print_help();
        return 0;
    }

#ifdef _WIN32
    // Enable ANSI color support on Windows
    HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD mode = 0;
    if (GetConsoleMode(h, &mode)) {
        SetConsoleMode(h, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
    }
#endif

    // Load events from config file
    load_events();

    if (show_twelve) {
        print_twelve_months(y, m, y, m, d, monday_first);
    } else if (show_year) {
        print_year(y, y, m, d, monday_first);
    } else if (show3) {
        int pm = m == 1 ? 12 : m - 1;
        int py = m == 1 ? y - 1 : y;
        int nm = m == 12 ? 1 : m + 1;
        int ny = m == 12 ? y + 1 : y;
        
        std::vector<MonthData> months;
        months.push_back(get_month_data(py, pm, y, m, d, monday_first));
        months.push_back(get_month_data(y, m, y, m, d, monday_first));
        months.push_back(get_month_data(ny, nm, y, m, d, monday_first));
        
        print_month_horizontal(months, monday_first);
    } else {
        print_month_vertical(y, m, y, m, d, monday_first);
    }
    
    // Show legend if there are events
    if (!events.empty()) {
        print_events_legend();
    }
    
    return 0;
}
