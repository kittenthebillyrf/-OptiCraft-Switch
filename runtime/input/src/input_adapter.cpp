#include "input_adapter.hpp"
namespace opti {
namespace {
constexpr std::uint64_t BA=1ull<<0,BB=1ull<<1,BX=1ull<<2,BY=1ull<<3,BL=1ull<<6,BR=1ull<<7;
constexpr std::uint64_t BLEFT=1ull<<12,BUP=1ull<<13,BRIGHT=1ull<<14,BDOWN=1ull<<15;
constexpr int DZ=10000;
enum { W,S,A,D,ESC,E,Q,UP,DOWN,LEFT,RIGHT,COUNT };
constexpr int CODES[COUNT]={KEY_W,KEY_S,KEY_A,KEY_D,KEY_ESCAPE,KEY_E,KEY_Q,KEY_UP,KEY_DOWN,KEY_LEFT,KEY_RIGHT};
void add(EventBatch& b,InputEvent::Type t,int c){if(b.count<16)b.events[b.count++]={t,c};}
}
EventBatch InputAdapter::update(const ControllerFrame& f){
 std::uint32_t k=0,m=0;
 if(f.left_y>DZ)k|=1u<<W;
 if(f.left_y< -DZ)k|=1u<<S;
 if(f.left_x< -DZ)k|=1u<<A;
 if(f.left_x>DZ)k|=1u<<D;
 if(f.buttons&BB)k|=1u<<ESC;
 if(f.buttons&BX)k|=1u<<E;
 if(f.buttons&BY)k|=1u<<Q;
 if(f.buttons&BUP)k|=1u<<UP;
 if(f.buttons&BDOWN)k|=1u<<DOWN;
 if(f.buttons&BLEFT)k|=1u<<LEFT;
 if(f.buttons&BRIGHT)k|=1u<<RIGHT;
 if(f.buttons&(BA|BR))m|=1u;
 if(f.buttons&BL)m|=1u<<1;
 EventBatch b{};
 for(int i=0;i<COUNT;++i){const auto bit=1u<<i;
  if((k&bit)&&!(previous_keys_&bit))add(b,InputEvent::KeyDown,CODES[i]);
  if(!(k&bit)&&(previous_keys_&bit))add(b,InputEvent::KeyUp,CODES[i]);}
 for(int i=0;i<2;++i){const auto bit=1u<<i;
  if((m&bit)&&!(previous_mouse_&bit))add(b,InputEvent::MouseDown,i);
  if(!(m&bit)&&(previous_mouse_&bit))add(b,InputEvent::MouseUp,i);}
 previous_keys_=k;previous_mouse_=m;return b;
}
void InputAdapter::reset(){previous_keys_=previous_mouse_=0;}
}
