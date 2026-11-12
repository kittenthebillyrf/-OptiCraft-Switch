#include <switch.h>
#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <cstdlib>

namespace {
constexpr int SW=1280, SH=720, MAP_W=16, MAP_H=16, TILE_W=48, TILE_H=24;
enum Tile:uint8_t { Grass,Water,Sand,Stone,Wood,Brick };
struct Cell { Tile type; uint8_t height; bool player_placed; };
struct Point { int x,y; };
Framebuffer fb{}; bool fb_ready=false;
HidNpadFullKeyState pad{};
Cell world[MAP_H][MAP_W]{};
int px=8,py=8,fx=1,fy=0;
Tile selected=Wood;
unsigned frame_no=0;

uint32_t color(uint8_t r,uint8_t g,uint8_t b){return RGBA8(r,g,b,255);}
uint32_t* row_at(uint8_t* base,uint32_t stride,int y){return reinterpret_cast<uint32_t*>(base+static_cast<size_t>(y)*stride);}
void pixel(uint8_t* base,uint32_t stride,int x,int y,uint32_t c){
    if(x<0||y<0||x>=SW||y>=SH)return; row_at(base,stride,y)[x]=c;
}
void rect(uint8_t* base,uint32_t stride,int x,int y,int w,int h,uint32_t c){
    int x0=std::max(0,x),y0=std::max(0,y),x1=std::min(SW,x+w),y1=std::min(SH,y+h);
    for(int yy=y0;yy<y1;++yy){uint32_t* row=row_at(base,stride,yy);std::fill(row+x0,row+x1,c);}
}
void polygon(uint8_t* base,uint32_t stride,const Point* p,int n,uint32_t c){
    int min_y=SH,max_y=0;
    for(int i=0;i<n;++i){min_y=std::min(min_y,p[i].y);max_y=std::max(max_y,p[i].y);}
    min_y=std::max(0,min_y);max_y=std::min(SH-1,max_y);
    for(int y=min_y;y<=max_y;++y){
        int xs[8],count=0;
        for(int i=0;i<n;++i){Point a=p[i],b=p[(i+1)%n];
            if((a.y<=y&&b.y>y)||(b.y<=y&&a.y>y))
                if(count<8)xs[count++]=a.x+(y-a.y)*(b.x-a.x)/(b.y-a.y);
        }
        if(count<2)continue;std::sort(xs,xs+count);
        int x0=std::max(0,xs[0]),x1=std::min(SW-1,xs[count-1]);
        uint32_t* row=row_at(base,stride,y);for(int x=x0;x<=x1;++x)row[x]=c;
    }
}
const uint8_t* glyph(char ch){
    static const uint8_t blank[7]{};
    static const uint8_t A[7]={14,17,17,31,17,17,17},B[7]={30,17,17,30,17,17,30},C[7]={14,17,16,16,16,17,14};
    static const uint8_t D[7]={30,17,17,17,17,17,30},E[7]={31,16,16,30,16,16,31},F[7]={31,16,16,30,16,16,16};
    static const uint8_t G[7]={14,17,16,23,17,17,15},H[7]={17,17,17,31,17,17,17},I[7]={14,4,4,4,4,4,14};
    static const uint8_t J[7]={7,2,2,2,2,18,12},K[7]={17,18,20,24,20,18,17},L[7]={16,16,16,16,16,16,31};
    static const uint8_t M[7]={17,27,21,21,17,17,17},N[7]={17,25,21,19,17,17,17},O[7]={14,17,17,17,17,17,14};
    static const uint8_t P[7]={30,17,17,30,16,16,16},Q[7]={14,17,17,17,21,18,13},R[7]={30,17,17,30,20,18,17};
    static const uint8_t S[7]={15,16,16,14,1,1,30},T[7]={31,4,4,4,4,4,4},U[7]={17,17,17,17,17,17,14};
    static const uint8_t V[7]={17,17,17,17,17,10,4},W[7]={17,17,17,21,21,21,10},X[7]={17,17,10,4,10,17,17};
    static const uint8_t Y[7]={17,17,10,4,4,4,4},Z[7]={31,1,2,4,8,16,31};
    static const uint8_t N0[7]={14,17,19,21,25,17,14},N1[7]={4,12,4,4,4,4,14},N2[7]={14,17,1,2,4,8,31};
    static const uint8_t N3[7]={30,1,1,14,1,1,30},N4[7]={2,6,10,18,31,2,2},N5[7]={31,16,16,30,1,1,30};
    static const uint8_t N6[7]={14,16,16,30,17,17,14},N7[7]={31,1,2,4,8,8,8},N8[7]={14,17,17,14,17,17,14},N9[7]={14,17,17,15,1,1,14};
    static const uint8_t dash[7]={0,0,0,31,0,0,0},colon[7]={0,4,4,0,4,4,0},slash[7]={1,2,2,4,8,8,16};
    switch(ch){
#define GLYPH(C) case #C[0]:return C
        GLYPH(A);GLYPH(B);GLYPH(C);GLYPH(D);GLYPH(E);GLYPH(F);GLYPH(G);GLYPH(H);GLYPH(I);GLYPH(J);GLYPH(K);GLYPH(L);GLYPH(M);GLYPH(N);GLYPH(O);GLYPH(P);GLYPH(Q);GLYPH(R);GLYPH(S);GLYPH(T);GLYPH(U);GLYPH(V);GLYPH(W);GLYPH(X);GLYPH(Y);GLYPH(Z);
#undef GLYPH
        case '0':return N0;case '1':return N1;case '2':return N2;case '3':return N3;case '4':return N4;case '5':return N5;case '6':return N6;case '7':return N7;case '8':return N8;case '9':return N9;
        case '-':return dash;case ':':return colon;case '/':return slash;default:return blank;
    }
}
void draw_text(uint8_t* base,uint32_t stride,int x,int y,const char* s,uint32_t c,int scale=2){
    for(;*s;++s,x+=6*scale){const uint8_t* rows=glyph(*s);
        for(int yy=0;yy<7;++yy)for(int xx=0;xx<5;++xx)
            if(rows[yy]&(1u<<(4-xx)))rect(base,stride,x+xx*scale,y+yy*scale,scale,scale,c);
    }
}
unsigned hash2(int x,int y){
    uint32_t v=static_cast<uint32_t>(x)*0x45d9f3bu^static_cast<uint32_t>(y)*0x119de1f3u^0x0C71C4A5u;
    v^=v>>16;v*=0x45d9f3bu;v^=v>>16;return v;
}
void generate_world(){
    for(int y=0;y<MAP_H;++y)for(int x=0;x<MAP_W;++x){
        unsigned h=hash2(x,y);int elevation=static_cast<int>((h>>5)%4);Tile type=Grass;
        if((h%19)==0||(x>10&&y<5&&(h%3)==0))type=Water;
        else if((h%13)==0)type=Stone;else if((h%11)==0)type=Sand;
        world[y][x]={type,static_cast<uint8_t>(elevation),false};
    }
    world[py][px]={Grass,1,false};
}
void draw_tile(uint8_t* base,uint32_t stride,int sx,int sy,const Cell& cell,bool selected_tile){
    int hw=TILE_W/2,hh=TILE_H/2,drop=static_cast<int>(cell.height)*5;
    uint32_t top=color(74,158,66),left=color(43,105,45),right=color(56,126,53);
    switch(cell.type){
        case Water:top=color(45,125,205);left=color(25,75,145);right=color(32,95,175);break;
        case Sand:top=color(211,190,119);left=color(155,130,75);right=color(180,151,87);break;
        case Stone:top=color(135,139,145);left=color(83,87,94);right=color(104,108,115);break;
        case Wood:top=color(154,102,55);left=color(95,58,29);right=color(119,75,38);break;
        case Brick:top=color(165,76,63);left=color(102,42,37);right=color(133,57,48);break;
        default:break;
    }
    Point t[4]={{sx,sy-hh-drop},{sx+hw,sy-drop},{sx,sy+hh-drop},{sx-hw,sy-drop}};
    Point l[4]={{sx-hw,sy-drop},{sx,sy+hh-drop},{sx,sy+hh+drop+7},{sx-hw,sy+drop+7}};
    Point r[4]={{sx,sy+hh-drop},{sx+hw,sy-drop},{sx+hw,sy+drop+7},{sx,sy+hh+drop+7}};
    polygon(base,stride,l,4,left);polygon(base,stride,r,4,right);polygon(base,stride,t,4,top);
    if(selected_tile){uint32_t edge=color(255,236,80);
        for(int i=0;i<4;++i){Point a=t[i],b=t[(i+1)%4];int steps=std::max(std::abs(b.x-a.x),std::abs(b.y-a.y));
            for(int k=0;k<=steps;++k)pixel(base,stride,a.x+(b.x-a.x)*k/std::max(1,steps),a.y+(b.y-a.y)*k/std::max(1,steps),edge);
        }
    }
}
bool init_platform(){
    Result rc=hidInitialize();if(R_FAILED(rc))return false;hidInitializeNpad();
    rc=framebufferCreate(&fb,nwindowGetDefault(),SW,SH,PIXEL_FORMAT_RGBA_8888,2);
    if(R_FAILED(rc)){hidExit();return false;}
    rc=framebufferMakeLinear(&fb);if(R_FAILED(rc)){framebufferClose(&fb);hidExit();return false;}
    fb_ready=true;return true;
}
void shutdown_platform(){if(fb_ready){framebufferClose(&fb);hidExit();fb_ready=false;}}
bool poll_pad(){
    size_t count=hidGetNpadStatesHandheld(HidNpadIdType_Handheld,&pad,1);
    if(count==0)count=hidGetNpadStatesFullKey(HidNpadIdType_No1,&pad,1);
    return count!=0;
}
void move_player(int dx,int dy){
    if(dx==0&&dy==0)return;fx=dx;fy=dy;
    int nx=std::clamp(px+dx,0,MAP_W-1),ny=std::clamp(py+dy,0,MAP_H-1);
    if(world[ny][nx].type!=Water){px=nx;py=ny;}
}
void update_game(){
    int dx=0,dy=0;u64 buttons=pad.buttons;
    if((frame_no%8)==0){
        if(buttons&HidNpadButton_Up)dy=-1;else if(buttons&HidNpadButton_Down)dy=1;
        else if(buttons&HidNpadButton_Left)dx=-1;else if(buttons&HidNpadButton_Right)dx=1;
        else if(pad.analog_stick_l.x>10000)dx=1;else if(pad.analog_stick_l.x< -10000)dx=-1;
        else if(pad.analog_stick_l.y>10000)dy=-1;else if(pad.analog_stick_l.y< -10000)dy=1;
        move_player(dx,dy);
        int tx=std::clamp(px+fx,0,MAP_W-1),ty=std::clamp(py+fy,0,MAP_H-1);
        if(buttons&HidNpadButton_A)world[ty][tx]={selected,2,true};
        if((buttons&HidNpadButton_B)&&world[ty][tx].player_placed)world[ty][tx]={Grass,1,false};
    }
    if((buttons&HidNpadButton_X)&&(frame_no%12)==0)selected=static_cast<Tile>((static_cast<int>(selected-Wood)+1)%2+Wood);
    if((buttons&HidNpadButton_Y)&&(frame_no%12)==0){px=8;py=8;generate_world();}
}
void render(){
    uint32_t stride=0;auto* base=static_cast<uint8_t*>(framebufferBegin(&fb,&stride));if(!base)return;
    for(int y=0;y<SH;++y){uint32_t* row=row_at(base,stride,y);for(int x=0;x<SW;++x)row[x]=color(105,174,220);}
    rect(base,stride,0,0,SW,95,color(91,158,211));
    for(int sum=0;sum<=2*(MAP_W-1);++sum)for(int x=0;x<MAP_W;++x){
        int y=sum-x;if(y<0||y>=MAP_H)continue;int dx=x-px,dy=y-py;
        int sx=SW/2+(dx-dy)*(TILE_W/2),sy=SH/2-30+(dx+dy)*(TILE_H/2);
        if(sx< -TILE_W||sx>SW+TILE_W||sy< -TILE_H*2||sy>SH+TILE_H*2)continue;
        bool target=x==std::clamp(px+fx,0,MAP_W-1)&&y==std::clamp(py+fy,0,MAP_H-1);
        draw_tile(base,stride,sx,sy,world[y][x],target);
    }
    int psx=SW/2,psy=SH/2-30;Point marker[4]={{psx,psy-13},{psx+10,psy},{psx,psy+13},{psx-10,psy}};
    polygon(base,stride,marker,4,color(255,245,220));Point inner[4]={{psx,psy-7},{psx+5,psy},{psx,psy+7},{psx-5,psy}};
    polygon(base,stride,inner,4,color(45,48,55));
    rect(base,stride,22,18,520,104,color(24,31,42));
    draw_text(base,stride,42,34,"OPTICRAFT",color(255,255,255),3);
    draw_text(base,stride,42,76,"WORLD PROTOTYPE  0.1",color(177,220,255),2);
    rect(base,stride,22,SH-112,SW-44,88,color(24,31,42));
    char line[96];std::snprintf(line,sizeof(line),"POS %02d/%02d  BLOCK %s",px,py,selected==Wood?"WOOD":"BRICK");
    draw_text(base,stride,42,SH-94,line,color(255,255,255),2);
    draw_text(base,stride,42,SH-62,"DPAD MOVE  A PLACE  B BREAK  X BLOCK  Y NEW WORLD",color(224,232,242),2);
    draw_text(base,stride,SW-180,34,"PLUS EXIT",color(255,255,255),2);
    framebufferEnd(&fb);
}
} // namespace

int main(int argc,char** argv){
    (void)argc;(void)argv;if(!init_platform())return 1;generate_world();
    while(appletMainLoop()){
        if(!poll_pad()){++frame_no;continue;}
        if(pad.buttons&HidNpadButton_Plus)break;
        update_game();render();++frame_no;
    }
    shutdown_platform();return 0;
}