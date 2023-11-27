#pragma once
#include<cmath>

struct Position {
	int x, y;

	Position() : x(0), y(0) {};
	Position(int x, int y) : x(x), y(y) {};
	Position(const Position& pos) : x(pos.x), y(pos.y) {};

	Position abs() const { return Position(x < 0 ? -x : x, y < 0 ? -y : y); }
	Position unit() const {
		if (x == 0 && y == 0) return Position(0, 0);
		else if (x == 0) return Position(0, y / std::abs(y));
		else if (y == 0) return Position(x / std::abs(x), 0);
	}

	bool operator==(const Position& pos) const { return (x == pos.x) && (y == pos.y); }
	Position operator+(const Position& pos) const { return Position(x + pos.x, y + pos.y); }
	Position operator-(const Position& pos) const { return Position(x - pos.x, y - pos.y); }
	Position& operator=(const Position& pos) { x = pos.x; y = pos.y; return *this; }
};