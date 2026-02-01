#pragma once
#ifndef POSITION_H
#define POSITION_H
#include <iostream>
#include <cmath>
const double pos_epsilon = 1e-6;
template<class type_t>
class position{
public:
    type_t x;
    type_t y;
public:
    position(type_t x = type_t(), type_t y = type_t()) :
        x(x), y(y)
    {}
    position(const position& pos2) :
        x(pos2.x), y(pos2.y)
    {}
    position operator+(const position& pos2) const {
        return position(x + pos2.x, y + pos2.y);
    }
    position operator-(const position& pos2) const{
        return position(x - pos2.x, y - pos2.y);
    }
    type_t mod() const{
        return sqrt(x * x + y * y);
    }
    position unify() const {
        type_t m = mod();
        return position(x / m, y / m);
    }
    bool operator==(const position& pos2) const {
        return x == pos2.x && y == pos2.y;
    }
    bool operator!=(const position& pos2) const{
        return !(*this == pos2);
    }
    bool operator<(const position& pos2) const{
        return x < pos2.x && y < pos2.y;
    }
    bool operator>(const position& pos2) const{
        return x > pos2.x && y > pos2.y;
    }
    position operator*(double factor) const {
        return {x * factor, y* factor};
    }
    void delta(type_t dx, type_t dy){
        x += dx, y += dy;
    }
    void delta(const position& pos2){
        x += pos2.x, y += pos2.y;
    }
    position offset(const position& off) const{
        return { x + off.x, y + off.y};
    }
    // 浮点坐标只允许使用这个判断相等
    bool is_same(const position& pos2) const{
        return fabs(x - pos2.x) <= pos_epsilon &&
               fabs(y - pos2.y) <= pos_epsilon;
    }
    double dot_product(const position& p2) const {
        return x * p2.x + y * p2.y;
    }
};
template<class type_t>
std::ostream& operator<<(std::ostream& os, const position<type_t>& pos){
    os << "(" << pos.x << ", " << pos.y << ")";
    return os;
}
// 整数坐标
typedef position<int> int_pos;
// 浮点坐标
typedef position<double> float_pos;

#endif // POSITION_H
