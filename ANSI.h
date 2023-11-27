#pragma once

#include<algorithm>
#include<string>
#include<vector>

#define ANSIEND "\033[0m"

#define FG_BLACK 30
#define FG_RED 31
#define FG_GREEN 32
#define FG_YELLOW 33
#define FG_BLUE 34
#define FG_MAGENTA 35
#define FG_CYAN 36
#define FG_WHITE 37

#define BG_BLACK 40
#define BG_RED 41
#define BG_GREEN 42
#define BG_YELLOW 43
#define BG_BLUE 44
#define BG_MAGENTA 45
#define BG_CYAN 46
#define BG_WHITE 47

#define FG_GRAY 90
#define FG_BRIGHT_RED 91
#define FG_BRIGHT_GREEN 92
#define FG_BRIGHT_YELLOW 93
#define FG_BRIGHT_BLUE 94
#define FG_BRIGHT_MAGENTA 95
#define FG_BRIGHT_CYAN 96
#define FG_BRIGHT_WHITE 97

#define BG_GRAY 100 // Bright black
#define BG_BRIGHT_RED 91
#define BG_BRIGHT_GREEN 92
#define BG_BRIGHT_YELLOW 93
#define BG_BRIGHT_BLUE 94
#define BG_BRIGHT_MAGENTA 95
#define BG_BRIGHT_CYAN 96
#define BG_BRIGHT_WHITE 107

using namespace std;

namespace ANSI {

	constexpr char ansiHeader[]{ 0x1B,'[','\0' };
	constexpr char ansiReset[]{ 0x1B,'[','0','m','\0' };

	string getColor(int fg, int bg, bool bold) {
		const char* formatAttribute = bold ? "2" : "0";
		return string(ansiHeader) + formatAttribute + ";" + to_string(fg) + ";" + to_string(bg) + "m";
	}

	string getColor(int fg, int bg) { return getColor(fg, bg, false); }

	string render(int fg, int bg, bool bold, string str) { return getColor(fg, bg, bold) + str + ANSIEND; }

	string render(int fg, int bg, string str) { return render(fg, bg, false, str); }

	string getEnd() { return string(ansiReset); }

	enum Mode { standard, full, extended_std };

	vector<pair<string, size_t>> fgColors = { {"FG_BLACK", 30}, {"FG_RED", 31}, {"FG_GREEN", 32}, {"FG_YELLOW", 33},
							{"FG_BLUE", 34}, {"FG_MAGENTA", 35}, {"FG_CYAN", 36}, {"FG_WHITE", 37},
							{"FG_GRAY", 90}, {"FG_BRIGHT_RED", 91}, {"FG_BRIGHT_GREEN", 92}, {"FG_BRIGHT_YELLOW", 93},
							{"FG_BRIGHT_BLUE", 94}, {"FG_BRIGHT_MAGENTA", 95}, {"FG_BRIGHT_CYAN", 96}, {"FG_BRIGHT_WHITE", 97} };

	vector<pair<string, size_t>> bgColors = { {"BG_BLACK", 40}, {"BG_RED", 41}, {"BG_GREEN", 42}, {"BG_YELLOW", 43},
								{"BG_BLUE", 44}, {"BG_MAGENTA", 45}, {"BG_CYAN", 46}, {"BG_WHITE", 47},
								{"BG_GRAY", 100}, {"BG_BRIGHT_RED", 91}, {"BG_BRIGHT_GREEN", 92}, {"BG_BRIGHT_YELLOW", 93},
								{"BG_BRIGHT_BLUE", 94}, {"BG_BRIGHT_MAGENTA", 95}, {"BG_BRIGHT_CYAN", 96}, {"BG_BRIGHT_WHITE", 107} };

	string getColorList(Mode mode) {

		string colorlist = "";

		for (size_t i = 0; i < (mode==standard ? 1 : bgColors.size()); ++i) {
			for (size_t j = 0; j < fgColors.size(); ++j)
				colorlist += (render((fgColors[j].second), (bgColors[i].second), "A") + " " + fgColors[j].first + " + " + bgColors[i].first + "\n");
		}

		return colorlist;
	}

	string getColorList() { return getColorList(standard); }
};