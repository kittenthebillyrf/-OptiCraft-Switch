#include <switch.h>
#include <algorithm>
#include <array>
#include "opticraft_platform.h"
#include "world_save_catalog.h"
#include "world_save_preview.h"
#include "world_anvil_chunk.h"
#include "opticraft_texture_atlas.h"
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <sys/stat.h>

namespace {
constexpr int SW=1280, SH=720, MAP_W=48, MAP_H=48, TILE_W=48, TILE_H=24;
enum Tile:uint8_t { Grass,Water,Sand,Stone,Wood,Brick,Lava,Wool };
struct Cell {
    Tile type; uint8_t height; bool player_placed;
    uint8_t metadata=0; uint16_t block_id=0;
    uint16_t side_block_id[8]{};
    uint8_t side_block_y[8]{};
    uint8_t side_metadata[8]{};
    uint8_t side_count=0;
};
struct Point { int x,y; };
std::array<uint32_t,SW*SH> frame_pixels{};
OptiInputSnapshot input{};
Cell world[MAP_H][MAP_W]{};
int px=24,py=24,fx=1,fy=0;
int32_t player_world_x=0,player_world_z=0;
Tile selected=Wood;
unsigned frame_no=0;
constexpr size_t MAX_WORLD_SAVES=8;
OpticraftWorldCatalogEntry world_saves[MAX_WORLD_SAVES]{};
int world_save_count=-1;
int selected_world_index=0;
OpticraftWorldPreview world_preview{};
bool world_preview_loaded=false;
OpticraftAnvilChunkPreview world_chunk{};
bool world_chunk_loaded=false;
int world_chunks_loaded=0;
bool sd_mounted=false;
OpticraftTextureAtlas terrain_atlas{};
bool terrain_atlas_loaded=false;

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
void draw_textured_top(uint8_t* base,uint32_t stride,int sx,int sy,const Cell& cell,int drop){
    if(!terrain_atlas_loaded||terrain_atlas.width<256||terrain_atlas.height<256||!terrain_atlas.rgba||cell.type==Wool)return;
    int tx=1,ty=0;
    if(cell.block_id==0){
        switch(cell.type){
            case Grass:tx=0;ty=0;break;
            case Water:tx=0;ty=8;break;
            case Sand:tx=2;ty=1;break;
            case Stone:tx=1;ty=0;break;
            case Wood:tx=4;ty=0;break;
            case Brick:tx=7;ty=0;break;
            case Lava:tx=14;ty=10;break;
            case Wool:tx=0;ty=4;break;
        }
    }else switch(cell.block_id){
        case 2:tx=0;ty=0;break;
        case 3:tx=2;ty=0;break;
        case 12:tx=2;ty=1;break;
        case 13:tx=3;ty=1;break;
        case 5:tx=4;ty=0;break;
        case 17:tx=4;ty=1;break;
        case 45:tx=7;ty=0;break;
        case 8:case 9:tx=0;ty=8;break;
        case 10:case 11:tx=14;ty=10;break;
        case 35:tx=0;ty=4;break;
        default:break;
    }
    const int hw=TILE_W/2,hh=TILE_H/2,cy=sy-drop;
    for(int y=std::max(0,cy-hh);y<=std::min(SH-1,cy+hh);++y){
        uint32_t* row=row_at(base,stride,y);
        for(int x=std::max(0,sx-hw);x<=std::min(SW-1,sx+hw);++x){
            const int dx=std::abs(x-sx),dy=std::abs(y-cy);
            if(dx*hh+dy*hw>hw*hh)continue;
            const int u=std::clamp((x-(sx-hw))*16/(2*hw),0,15);
            const int v=std::clamp((y-(cy-hh))*16/(2*hh),0,15);
            const size_t offset=(static_cast<size_t>(ty*16+v)*terrain_atlas.width+tx*16+u)*4;
            const uint8_t* src=terrain_atlas.rgba+offset;
            if(src[3]==0)continue;
            if(src[3]==255)row[x]=color(src[0],src[1],src[2]);
            else{
                const uint32_t dst=row[x];const uint32_t a=src[3],ia=255-a;
                const uint8_t dr=static_cast<uint8_t>(dst&255),dg=static_cast<uint8_t>((dst>>8)&255),db=static_cast<uint8_t>((dst>>16)&255);
                row[x]=color(static_cast<uint8_t>((src[0]*a+dr*ia)/255),
                             static_cast<uint8_t>((src[1]*a+dg*ia)/255),
                             static_cast<uint8_t>((src[2]*a+db*ia)/255));
            }
        }
    }
}
void side_texture_tile(uint16_t id,Tile fallback,int& tx,int& ty){
    tx=1;ty=0;
    switch(id){
        case 2:tx=3;ty=0;break;
        case 3:tx=2;ty=0;break;
        case 12:tx=2;ty=1;break;
        case 13:tx=3;ty=1;break;
        case 5:tx=4;ty=0;break;
        case 17:tx=4;ty=1;break;
        case 45:tx=7;ty=0;break;
        case 8:case 9:tx=0;ty=8;break;
        case 10:case 11:tx=14;ty=10;break;
        default:
            if(fallback==Grass){tx=3;ty=0;}
            else if(fallback==Sand){tx=2;ty=1;}
            else if(fallback==Wood){tx=4;ty=1;}
            else if(fallback==Water){tx=0;ty=8;}
            else if(fallback==Lava){tx=14;ty=10;}
            else if(fallback==Brick){tx=7;ty=0;}
            break;
    }
}
void draw_textured_side(uint8_t* base,uint32_t stride,int sx,int sy,const Cell& cell,int drop,bool left_face){
    if(!terrain_atlas_loaded||!terrain_atlas.rgba||cell.type==Wool)return;
    const int hw=TILE_W/2,hh=TILE_H/2,vlen=2*drop+7;
    const int p0x=left_face?sx-hw:sx;
    const int p0y=left_face?sy-drop:sy+hh-drop;
    const int uy=left_face?hh:-hh;
    for(int y=std::max(0,p0y-hh);y<=std::min(SH-1,p0y+std::abs(uy)+vlen);++y){
        uint32_t* row=row_at(base,stride,y);
        for(int x=std::max(0,p0x);x<=std::min(SW-1,p0x+hw);++x){
            const int u_num=x-p0x;
            const int v_num=(y-p0y)*hw-u_num*uy;
            const int denominator=vlen*hw;
            if(u_num<0||u_num>hw||v_num<0||v_num>denominator)continue;
            const int u=std::clamp(u_num*16/hw,0,15);
            const int v=std::clamp(v_num*16/denominator,0,15);
            const int count=cell.side_count?cell.side_count:1;
            const int layer=cell.side_count?std::min(count-1,v*count/16):0;
            const uint16_t id=cell.side_count?cell.side_block_id[layer]:cell.block_id;
            if(id==35)continue;
            int tx=1,ty=0;side_texture_tile(id,cell.type,tx,ty);
            const size_t offset=(static_cast<size_t>(ty*16+v)*terrain_atlas.width+tx*16+u)*4;
            const uint8_t* src=terrain_atlas.rgba+offset;
            if(src[3]==0)continue;
            if(src[3]==255)row[x]=color(src[0],src[1],src[2]);
            else{
                const uint32_t dst=row[x];const uint32_t a=src[3],ia=255-a;
                const uint8_t dr=static_cast<uint8_t>(dst&255),dg=static_cast<uint8_t>((dst>>8)&255),db=static_cast<uint8_t>((dst>>16)&255);
                row[x]=color(static_cast<uint8_t>((src[0]*a+dr*ia)/255),
                             static_cast<uint8_t>((src[1]*a+dg*ia)/255),
                             static_cast<uint8_t>((src[2]*a+db*ia)/255));
            }
        }
    }
}
void draw_tile(uint8_t* base,uint32_t stride,int sx,int sy,const Cell& cell,bool selected_tile){
    int hw=TILE_W/2,hh=TILE_H/2,drop=terrain_drop(cell.height);
    uint32_t top=color(74,158,66),left=color(43,105,45),right=color(56,126,53);
    switch(cell.type){
        case Water:top=color(45,125,205);left=color(25,75,145);right=color(32,95,175);break;
        case Sand:top=color(211,190,119);left=color(155,130,75);right=color(180,151,87);break;
        case Stone:top=color(135,139,145);left=color(83,87,94);right=color(104,108,115);break;
        case Wood:top=color(154,102,55);left=color(95,58,29);right=color(119,75,38);break;
        case Brick:top=color(165,76,63);left=color(102,42,37);right=color(133,57,48);break;
        case Lava:top=color(255,111,20);left=color(180,54,8);right=color(220,78,10);break;
        case Wool:
            switch(cell.metadata&15){
                case 0:top=color(235,235,235);left=color(150,150,150);right=color(190,190,190);break;
                case 1:top=color(232,120,35);left=color(150,70,20);right=color(190,92,25);break;
                case 2:top=color(190,70,190);left=color(110,35,110);right=color(150,50,150);break;
                case 3:top=color(90,145,220);left=color(45,75,140);right=color(65,110,180);break;
                case 4:top=color(230,205,45);left=color(145,125,20);right=color(185,160,30);break;
                case 5:top=color(105,175,45);left=color(55,100,20);right=color(80,135,30);break;
                case 6:top=color(220,135,155);left=color(140,75,95);right=color(180,105,125);break;
                case 7:top=color(80,80,80);left=color(35,35,35);right=color(55,55,55);break;
                case 8:top=color(165,165,165);left=color(95,95,95);right=color(130,130,130);break;
                case 9:top=color(70,175,175);left=color(35,95,95);right=color(50,135,135);break;
                case 10:top=color(125,65,175);left=color(65,30,95);right=color(95,45,135);break;
                case 11:top=color(55,65,155);left=color(25,30,85);right=color(40,45,120);break;
                case 12:top=color(115,75,45);left=color(60,35,20);right=color(85,50,30);break;
                case 13:top=color(85,115,40);left=color(40,60,15);right=color(60,85,25);break;
                case 14:top=color(175,45,40);left=color(90,20,20);right=color(130,30,30);break;
                default:top=color(35,35,35);left=color(10,10,10);right=color(20,20,20);break;
            }
            break;
        default:break;
    }
    Point t[4]={{sx,sy-hh-drop},{sx+hw,sy-drop},{sx,sy+hh-drop},{sx-hw,sy-drop}};
    Point l[4]={{sx-hw,sy-drop},{sx,sy+hh-drop},{sx,sy+hh+drop+7},{sx-hw,sy+drop+7}};
    Point r[4]={{sx,sy+hh-drop},{sx+hw,sy-drop},{sx+hw,sy+drop+7},{sx,sy+hh+drop+7}};
    polygon(base,stride,l,4,left);polygon(base,stride,r,4,right);polygon(base,stride,t,4,top);
    draw_textured_side(base,stride,sx,sy,cell,drop,true);
    draw_textured_side(base,stride,sx,sy,cell,drop,false);
    draw_textured_top(base,stride,sx,sy,cell,drop);
    if(selected_tile){uint32_t edge=color(255,236,80);
        for(int i=0;i<4;++i){Point a=t[i],b=t[(i+1)%4];int steps=std::max(std::abs(b.x-a.x),std::abs(b.y-a.y));
            for(int k=0;k<=steps;++k)pixel(base,stride,a.x+(b.x-a.x)*k/std::max(1,steps),a.y+(b.y-a.y)*k/std::max(1,steps),edge);
        }
    }
}
bool init_platform(){return opticraft_platform_init()==0;}
void shutdown_platform(){opticraft_platform_shutdown();}
bool poll_pad(){opticraft_platform_poll_events(&input);return true;}
void load_selected_world_chunk();
void move_player(int dx,int dy){
    if(dx==0&&dy==0)return;
    fx=dx;fy=dy;
    const int nx=std::clamp(px+dx,0,MAP_W-1);
    const int ny=std::clamp(py+dy,0,MAP_H-1);
    if(world[ny][nx].type==Water)return;
    px=nx;py=ny;
    if(world_chunk_loaded){
        player_world_x+=dx;
        player_world_z+=dy;
        if(px<16||px>=32||py<16||py>=32)load_selected_world_chunk();
    }
}
void load_selected_world_preview();
void load_selected_world_chunk();
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
    if((buttons&OPTI_BUTTON_Y)&&(frame_no%12)==0){px=24;py=24;player_world_x=0;player_world_z=0;world_chunk_loaded=false;world_chunks_loaded=0;generate_world();}
}
void load_selected_world_preview(){
    world_preview={};world_preview_loaded=false;world_chunk_loaded=false;world_chunks_loaded=0;
    if(world_save_count<=0||selected_world_index<0||selected_world_index>=world_save_count){
        generate_world();return;
    }
    char directory[512];
    int n=std::snprintf(directory,sizeof(directory),"sdmc:/switch/opticraft/saves/%s",
                        world_saves[selected_world_index].folder);
    if(n<0||static_cast<size_t>(n)>=sizeof(directory)){generate_world();return;}
    world_preview_loaded=opticraft_world_save_read_preview(directory,&world_preview)==0;
    player_world_x=world_preview_loaded&&world_preview.has_spawn?world_preview.spawn_x:0;
    player_world_z=world_preview_loaded&&world_preview.has_spawn?world_preview.spawn_z:0;
    load_selected_world_chunk();
}
void load_selected_world_chunk(){
    world_chunk={};world_chunk_loaded=false;world_chunks_loaded=0;
    if(!world_preview_loaded){generate_world();return;}
    const int32_t center_x=player_world_x;
    const int32_t center_z=player_world_z;
    char directory[512];
    int n=std::snprintf(directory,sizeof(directory),"sdmc:/switch/opticraft/saves/%s",
                        world_saves[selected_world_index].folder);
    if(n<0||static_cast<size_t>(n)>=sizeof(directory)){generate_world();return;}
    for(int z=0;z<MAP_H;++z)for(int x=0;x<MAP_W;++x)world[z][x]={Grass,0,false};
    auto map_block=[](uint16_t id)->Tile{
        switch(id){
            case 8:case 9:return Water;
            case 10:case 11:return Lava;
            case 12:case 13:case 24:return Sand;
            case 1:case 4:case 14:case 15:case 16:case 21:case 22:case 23:
            case 48:case 49:case 56:case 57:case 73:case 74:case 79:case 82:
            case 87:case 88:case 89:case 98:case 112:case 155:return Stone;
            case 5:case 17:case 18:case 25:case 47:case 53:case 54:case 58:
            case 61:case 62:case 64:case 85:case 86:case 91:case 96:case 107:
            case 126:case 134:case 146:case 162:case 163:case 164:case 170:return Wood;
            case 35:return Wool;
            case 45:case 159:case 172:return Brick;
            default:return Grass;
        }
    };
    bool center_chunk_loaded=false;
    for(int dz=-1;dz<=1;++dz)for(int dx=-1;dx<=1;++dx){
        OpticraftAnvilChunkPreview chunk{};
        const int32_t sample_x=center_x+dx*16;
        const int32_t sample_z=center_z+dz*16;
        if(opticraft_world_read_spawn_chunk(directory,sample_x,sample_z,&chunk)!=0)continue;
        ++world_chunks_loaded;
        if(dx==0&&dz==0){world_chunk=chunk;center_chunk_loaded=true;}
        const int base_x=(dx+1)*16;
        const int base_z=(dz+1)*16;
        for(int z=0;z<16;++z)for(int x=0;x<16;++x){
            const int index=z*16+x;
            Cell loaded{
                map_block(chunk.block_id[index]),chunk.height[index],false,
                chunk.metadata[index],chunk.block_id[index]
            };
            loaded.side_count=chunk.side_count[index];
            for(int layer=0;layer<loaded.side_count;++layer){
                loaded.side_block_id[layer]=chunk.side_block_id[index][layer];
                loaded.side_block_y[layer]=chunk.side_block_y[index][layer];
                loaded.side_metadata[layer]=chunk.side_metadata[index][layer];
            }
            world[base_z+z][base_x+x]=loaded;
        }
    }
    if(!center_chunk_loaded){world_chunks_loaded=0;generate_world();return;}
    px=16+((center_x%16)+16)%16;
    py=16+((center_z%16)+16)%16;
    world_chunk_loaded=true;
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
        const int save_index=(selected_world_index+i)%world_save_count;
        char label[OPTICRAFT_WORLD_FOLDER_MAX];
        size_t j=0;
        for(;j<sizeof(label)-1 && world_saves[save_index].folder[j];++j){
            char ch=world_saves[save_index].folder[j];
            if(ch>='a'&&ch<='z')ch=static_cast<char>(ch-'a'+'A');
            if(!((ch>='A'&&ch<='Z')||(ch>='0'&&ch<='9')||ch==' '||ch=='-'||ch=='_'))ch='-';
            label[j]=ch;
        }
        label[j]='\0';
        const uint32_t name_color=i==0?color(255,236,80):
            (world_saves[save_index].has_level_dat?color(224,232,242):color(255,190,120));
        draw_text(base,stride,x+14,y+86+i*20,label,name_color,1);
    }
    char chunk_line[48];
    std::snprintf(chunk_line,sizeof(chunk_line),"CHUNKS: %d/9 TEX:%s",world_chunks_loaded,terrain_atlas_loaded?"ON":"OFF");
    draw_text(base,stride,x+14,y+190,chunk_line,color(177,220,255),1);
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
    int psx=SW/2,psy=SH/2-30-terrain_drop(world[py][px].height);Point marker[4]={{psx,psy-13},{psx+10,psy},{psx,psy+13},{psx-10,psy}};
    polygon(base,stride,marker,4,color(255,245,220));Point inner[4]={{psx,psy-7},{psx+5,psy},{psx,psy+7},{psx-5,psy}};
    polygon(base,stride,inner,4,color(45,48,55));
    rect(base,stride,22,18,520,104,color(24,31,42));
    draw_text(base,stride,42,34,"OPTICRAFT",color(255,255,255),3);
    draw_text(base,stride,42,76,world_chunk_loaded?"JAVA SAVE AREA LOADED":"WORLD PROTOTYPE  0.1",color(177,220,255),2);
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
    if(sd_mounted)terrain_atlas_loaded=opticraft_texture_load_terrain_from_jar(
        "sdmc:/switch/opticraft/minecraft.jar",&terrain_atlas)==0;
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
    if(!world_chunk_loaded)generate_world();
    while(appletMainLoop()){
        if(!poll_pad()){++frame_no;continue;}
        if(input.should_exit)break;
        update_game();render();++frame_no;
    }
    opticraft_texture_atlas_free(&terrain_atlas);
    if(sd_mounted)fsdevUnmountDevice("sdmc");
    shutdown_platform();return 0;
}