#pragma once
#include "input_adapter.hpp"
#include <cstdint>
#include <vector>
namespace opti {
struct PackedInputEvent { std::int32_t type,code,x,y; };
class SwitchInputBridge {
public:
 void update(const ControllerFrame&,int,int,int,int,bool);
 std::vector<PackedInputEvent> drain();
 bool key_down(int) const; bool mouse_down(int) const;
 int mouse_x()const{return mouse_x_;} int mouse_y()const{return mouse_y_;}
 void reset();
private:
 InputAdapter adapter_;std::vector<PackedInputEvent> pending_;
 std::uint32_t keys_=0,mouse_=0;int mouse_x_=0,mouse_y_=0;bool touch_was_down_=false;
};
}
