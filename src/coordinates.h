#ifndef COORDINATES_H
#define COORDINATES_H

class Coordinates{
public:
	int x;
	int y;

	Coordinates(const int x = 0, const int y = 0);

	Coordinates cpy(void) const;

    Coordinates operator+(const Coordinates &coordinates) const;
    Coordinates operator-(const Coordinates &coordinates) const;
    Coordinates operator%(const Coordinates &coordinates) const;

    void operator+=(const Coordinates &coordinates);
    void operator-=(const Coordinates &coordinates);
	void operator=(const Coordinates &coordinates);

    bool operator== (const Coordinates &coordinates) const;
    bool operator!= (const Coordinates &coordinates) const;
};

inline Coordinates Coordinates::cpy() const{
    return Coordinates(this->x, this->y);
}

inline Coordinates Coordinates::operator+(const Coordinates &coordinates) const{
    return Coordinates(this->x + coordinates.x, this->y + coordinates.y);
}

inline Coordinates Coordinates::operator-(const Coordinates &coordinates) const{
    return Coordinates(this->x - coordinates.x, this->y - coordinates.y);
}

inline Coordinates Coordinates::operator%(const Coordinates &coordinates) const{
    return Coordinates(this->x % coordinates.x, this->y % coordinates.y);
}

#endif
