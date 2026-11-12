#include "switch_input_bridge.hpp"
#include <algorithm>
namespace opti {
namespace {
std::uint32_t mask(int k){switch(k){
 case KEY_W:return 1u<<0;case KEY_S:return 1u<<1;case KEY_A:return 1u<<2;case KEY_D:return 1u<<3;
 case KEY_ESCAPE:return 1u<<4;case KEY_E:return 1u<<5;case KEY_Q:return 1u<<6;
 case KEY_UP:return 1u<<7;case KEY_DOWN:return 1u<<8;case KEY_LEFT:return 1u<<9;case KEY_RIGHT:return 1u<<10;default:return 0;}}
}
void SwitchInputBridge::update(const ControllerFrame& f,int sw,int sh,int tx,int ty,bool touch){
 auto b=adapter_.update(f);
 for(unsigned i=0;i<b.count;++i){const auto& e=b.events[i];int t=e.type==InputEvent::KeyDown?1:e.type==InputEvent::KeyUp?2:e.type==InputEvent::MouseDown?3:4;
  pending_.push_back({t,e.code,mouse_x_,mouse_y_});
  if(e.type==InputEvent::KeyDown)keys_|=mask(e.code);
  else if(e.type==InputEvent::KeyUp)keys_&=~mask(e.code);
  else if(e.type==InputEvent::MouseDown)mouse_|=1u<<(e.code&31);
  else mouse_&=~(1u<<(e.code&31));
 }
 int dx=(f.right_x>8500||f.right_x< -8500)?f.right_x*7/32767:0;
 int dy=(f.right_y>8500||f.right_y< -8500)?f.right_y*7/32767:0;
 if(dx||dy){int ox=mouse_x_,oy=mouse_y_;mouse_x_=std::clamp(mouse_x_+dx,0,std::max(0,sw-1));mouse_y_=std::clamp(mouse_y_-dy,0,std::max(0,sh-1));if(ox!=mouse_x_||oy!=mouse_y_)pending_.push_back({5,0,mouse_x_,mouse_y_});}
 if(touch){tx=std::clamp(tx,0,std::max(0,sw-1));ty=std::clamp(ty,0,std::max(0,sh-1));
  if(tx!=mouse_x_||ty!=mouse_y_){mouse_x_=tx;mouse_y_=ty;pending_.push_back({5,0,mouse_x_,mouse_y_});}
  if(!touch_was_down_){mouse_|=1u;pending_.push_back({3,0,mouse_x_,mouse_y_});}}
 else if(touch_was_down_){mouse_&=~1u;pending_.push_back({4,0,mouse_x_,mouse_y_});}
 touch_was_down_=touch;
}
std::vector<PackedInputEvent> SwitchInputBridge::drain(){std::vector<PackedInputEvent> r;r.swap(pending_);return r;}
bool SwitchInputBridge::key_down(int k)const{auto m=mask(k);return m&&(keys_&m);}
bool SwitchInputBridge::mouse_down(int b)const{return (mouse_&(1u<<(b&31)))!=0;}
void SwitchInputBridge::reset(){adapter_.reset();pending_.clear();keys_=mouse_=0;mouse_x_=mouse_y_=0;touch_was_down_=false;}
}
