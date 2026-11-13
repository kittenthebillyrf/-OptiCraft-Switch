#include <switch.h>
#include <algorithm>
#include <array>
#include "opticraft_platform.h"
#include "world_save_catalog.h"
#include "world_save_preview.h"
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <sys/stat.h>

namespace {
constexpr int SW=1280, SH=720, MAP_W=16, MAP_H=16, TILE_W=48, TILE_H=24;
enum Tile:uint8_t { Grass,Water,Sand,Stone,Wood,Brick };
struct Cell { Tile type; uint8_t height; bool player_placed; };
struct Point { int x,y; };
std::array<uint32_t,SW*SH> frame_pixels{};
OptiInputSnapshot input{};
Cell world[MAP_H][MAP_W]{};
int px=8,py=8,fx=1,fy=0;
Tile selected=Wood;
unsigned frame_no=0;
constexpr size_t MAX_WORLD_SAVES=8;
OpticraftWorldCatalogEntry world_saves[MAX_WORLD_SAVES]{};
int world_save_count=-1;
int selected_world_index=0;
OpticraftWorldPreview world_preview{};
bool world_preview_loaded=false;
bool sd_mounted=false;

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
bool init_platform(){return opticraft_platform_init()==0;}
void shutdown_platform(){opticraft_platform_shutdown();}
bool poll_pad(){opticraft_platform_poll_events(&input);return true;}
void move_player(int dx,int dy){
    if(dx==0&&dy==0)return;fx=dx;fy=dy;
    int nx=std::clamp(px+dx,0,MAP_W-1),ny=std::clamp(py+dy,0,MAP_H-1);
    if(world[ny][nx].type!=Water){px=nx;py=ny;}
}
void update_game(){
    int dx=0,dy=0;const uint64_t buttons=input.buttons;
    if(world_save_count>0){
        if(input.pressed&OPTI_BUTTON_L){
            selected_world_index=(selected_world_index+world_save_count-1)%world_save_count;
            load_selected_world_preview();
        }else if(input.pressed&OPTI_BUTTON_R){
            selected_world_index=(selected_world_index+1)%world_save_count;
            load_selected_world_preview();
        }
    }
    if((frame_no%8)==0){
        if(buttons&OPTI_BUTTON_DPAD_UP)dy=-1;else if(buttons&OPTI_BUTTON_DPAD_DOWN)dy=1;
        else if(buttons&OPTI_BUTTON_DPAD_LEFT)dx=-1;else if(buttons&OPTI_BUTTON_DPAD_RIGHT)dx=1;
        else if(input.left_x>10000)dx=1;else if(input.left_x< -10000)dx=-1;
        else if(input.left_y>10000)dy=-1;else if(input.left_y< -10000)dy=1;
        move_player(dx,dy);
        int tx=std::clamp(px+fx,0,MAP_W-1),ty=std::clamp(py+fy,0,MAP_H-1);
        if(buttons&OPTI_BUTTON_A)world[ty][tx]={selected,2,true};
        if((buttons&OPTI_BUTTON_B)&&world[ty][tx].player_placed)world[ty][tx]={Grass,1,false};
    }
    if((buttons&OPTI_BUTTON_X)&&(frame_no%12)==0)selected=static_cast<Tile>((static_cast<int>(selected-Wood)+1)%2+Wood);
    if((buttons&OPTI_BUTTON_Y)&&(frame_no%12)==0){px=8;py=8;generate_world();}
}
void load_selected_world_preview(){
    world_preview={};world_preview_loaded=false;
    if(world_save_count<=0||selected_world_index<0||selected_world_index>=world_save_count)return;
    char directory[512];
    int n=std::snprintf(directory,sizeof(directory),"sdmc:/switch/opticraft/saves/%s",
                        world_saves[selected_world_index].folder);
    if(n<0||static_cast<size_t>(n)>=sizeof(directory))return;
    world_preview_loaded=opticraft_world_save_read_preview(directory,&world_preview)==0;
}
void draw_world_catalog(uint8_t* base,uint32_t stride){
    const int x=SW-360,y=18,w=338,h=210;
    rect(base,stride,x,y,w,h,color(24,31,42));
    draw_text(base,stride,x+14,y+12,"JAVA SAVES",color(255,255,255),2);
    if(world_save_count<0){
        draw_text(base,stride,x+14,y+42,"SD CARD NOT READY",color(255,190,120),1);
        draw_text(base,stride,x+14,y+62,"CHECK SDMC PATH",color(210,220,232),1);
        return;
    }
    char count_line[48];
    std::snprintf(count_line,sizeof(count_line),"FOUND: %d",world_save_count);
    draw_text(base,stride,x+14,y+42,count_line,color(177,220,255),1);
    if(world_save_count==0){
        draw_text(base,stride,x+14,y+66,"NO SAVES FOUND",color(210,220,232),1);
        draw_text(base,stride,x+14,y+88,"ADD JAVA WORLD FOLDER",color(210,220,232),1);
        return;
    }
    if(world_preview_loaded&&world_preview.has_level_name){
        char level_line[64];
        std::snprintf(level_line,sizeof(level_line),"LEVEL: %.40s",world_preview.level_name);
        draw_text(base,stride,x+14,y+62,level_line,color(177,220,255),1);
    }else{
        draw_text(base,stride,x+14,y+62,"LEVEL DATA UNREADABLE",color(255,190,120),1);
    }
    const int shown=std::min(world_save_count,5);
    for(int i=0;i<shown;++i){
        char label[OPTICRAFT_WORLD_FOLDER_MAX];
        size_t j=0;
        for(;j<sizeof(label)-1 && world_saves[i].folder[j];++j){
            char ch=world_saves[i].folder[j];
            if(ch>='a'&&ch<='z')ch=static_cast<char>(ch-'a'+'A');
            if(!((ch>='A'&&ch<='Z')||(ch>='0'&&ch<='9')||ch==' '||ch=='-'||ch=='_'))ch='-';
            label[j]=ch;
        }
        label[j]='\0';
        const uint32_t name_color=i==selected_world_index?color(255,236,80):
            (world_saves[i].has_level_dat?color(224,232,242):color(255,190,120));
        draw_text(base,stride,x+14,y+86+i*20,label,name_color,1);
    }
    if(world_save_count>shown)draw_text(base,stride,x+14,y+190,"MORE WORLDS",color(177,220,255),1);
}
void render(){
    const uint32_t stride=SW*static_cast<uint32_t>(sizeof(uint32_t));
    auto* base=reinterpret_cast<uint8_t*>(frame_pixels.data());
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
    draw_text(base,stride,42,SH-62,"DPAD MOVE  A PLACE  B BREAK  X BLOCK  Y NEW  L/R SAVE",color(224,232,242),2);
    draw_world_catalog(base,stride);
    draw_text(base,stride,SW-180,34,"PLUS EXIT",color(255,255,255),2);
    (void)opticraft_platform_present_rgba8888(frame_pixels.data(),SW,SH,SW);
}
} // namespace

int main(int argc,char** argv){
    (void)argc;(void)argv;if(!init_platform())return 1;
    sd_mounted = fsdevMountSdmc() == 0;
    if(sd_mounted){
        (void)mkdir("sdmc:/switch",0777);
        (void)mkdir("sdmc:/switch/opticraft",0777);
        (void)mkdir("sdmc:/switch/opticraft/saves",0777);
        world_save_count=opticraft_world_catalog_scan(
            "sdmc:/switch/opticraft/saves",world_saves,MAX_WORLD_SAVES);
    }else{
        world_save_count=-1;
    }
    load_selected_world_preview();
    generate_world();
    while(appletMainLoop()){
        if(!poll_pad()){++frame_no;continue;}
        if(input.should_exit)break;
        update_game();render();++frame_no;
    }
    if(sd_mounted)fsdevUnmountDevice("sdmc");
    shutdown_platform();return 0;
}