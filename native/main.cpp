#ifndef UNICODE
#define UNICODE
#endif
#ifndef _UNICODE
#define _UNICODE
#endif
#define NOMINMAX
#include <windows.h>
#include <windowsx.h>
#include <gdiplus.h>
#include <shellapi.h>
#include <shlwapi.h>
#include <mmsystem.h>
#include <string>
#include <vector>
#include <memory>
#include <random>
#include <cmath>
#include <algorithm>
#include "core.hpp"
using namespace Gdiplus;
static HWND wnd;
static HINSTANCE instance;
static lucky::Game game;
static std::mt19937 randomEngine(std::random_device{}());
static std::unique_ptr<Bitmap> icons[5];
static std::vector<IStream*> imageStreams;
static const wchar_t* names[]={L"Codex",L"Claude",L"DeepSeek",L"Gemini"};
static std::array<int,3> shown{0,1,2};
static lucky::Result pending;
static bool spinning=false,wonModal=false,helpModal=false,soundOn=false;
static ULONGLONG started=0;
static float scale=1,offsetX=0,offsetY=0;
static std::wstring channel=L"@Miss_CyberGirl",status=L"Твоя следующая комбинация уже ждёт.";
static const float W=1040,H=800;
static RectF spinRect(356,645,328,70),soundRect(861,39,58,40),helpRect(933,39,58,40);
static RectF openRect(290,475,460,56),continueRect(290,545,460,44);
static Color bg(255,10,12,19),ink(255,241,243,250),muted(255,140,149,172),accent(255,255,90,123);

static std::unique_ptr<GraphicsPath> rounded(RectF r,float radius){
    auto p=std::make_unique<GraphicsPath>();float d=radius*2;
    p->AddArc(r.X,r.Y,d,d,180,90);p->AddArc(r.GetRight()-d,r.Y,d,d,270,90);
    p->AddArc(r.GetRight()-d,r.GetBottom()-d,d,d,0,90);p->AddArc(r.X,r.GetBottom()-d,d,d,90,90);
    p->CloseFigure();return p;
}
static void box(Graphics& g,RectF r,Color fill,float radius=16,Color border=Color(0,0,0,0)){
    auto p=rounded(r,radius);SolidBrush b(fill);g.FillPath(&b,p.get());
    if(border.GetA()){Pen pen(border,1);g.DrawPath(&pen,p.get());}
}
static void txt(Graphics& g,const std::wstring& s,RectF r,float size,Color c=ink,bool bold=false,StringAlignment align=StringAlignmentNear){
    Font f(L"Segoe UI",size,bold?FontStyleBold:FontStyleRegular,UnitPixel);
    SolidBrush b(c);StringFormat format;format.SetAlignment(align);format.SetLineAlignment(StringAlignmentCenter);
    format.SetTrimming(StringTrimmingEllipsisCharacter);format.SetFormatFlags(StringFormatFlagsNoWrap);
    g.DrawString(s.c_str(),-1,&f,r,&format,&b);
}
static void centered(Graphics&g,const std::wstring&s,float x,float y,float w,float h,float size,Color c=ink,bool bold=false){txt(g,s,RectF(x,y,w,h),size,c,bold,StringAlignmentCenter);}
static std::wstring number(int n){auto s=std::to_wstring(n);for(int i=int(s.size())-3;i>0;i-=3)s.insert(i,L" ");return s;}
static void image(Graphics&g,int i,RectF r,float alpha=1){
    if(!icons[i])return;
    auto* im=icons[i].get();float ratio=float(im->GetWidth())/im->GetHeight();
    if(ratio>1){float hh=r.Width/ratio;r.Y+=(r.Height-hh)/2;r.Height=hh;}
    else {float ww=r.Height*ratio;r.X+=(r.Width-ww)/2;r.Width=ww;}
    if(alpha>=.99f)g.DrawImage(im,r,0,0,im->GetWidth(),im->GetHeight(),UnitPixel);
    else {ImageAttributes attr;ColorMatrix m={{{1,0,0,0,0},{0,1,0,0,0},{0,0,1,0,0},{0,0,0,alpha,0},{0,0,0,0,1}}};attr.SetColorMatrix(&m);g.DrawImage(im,r,0,0,im->GetWidth(),im->GetHeight(),UnitPixel,&attr);}
}
static bool inside(RectF r,float x,float y){return r.Contains(x,y);}
static bool validChannel(const std::wstring&s){
    if(s.size()<6||s.size()>33||s[0]!=L'@')return false;
    for(size_t i=1;i<s.size();++i)if(!((s[i]>=L'a'&&s[i]<=L'z')||(s[i]>=L'A'&&s[i]<=L'Z')||(s[i]>=L'0'&&s[i]<=L'9')||s[i]==L'_'))return false;
    return true;
}
static void loadChannel(){
    wchar_t path[MAX_PATH];GetModuleFileNameW(nullptr,path,MAX_PATH);PathRemoveFileSpecW(path);PathAppendW(path,L"settings.ini");
    wchar_t value[128];GetPrivateProfileStringW(L"game",L"telegram",L"@Miss_CyberGirl",value,128,path);
    if(validChannel(value))channel=value;
}
static void tone(bool win){if(soundOn)PlaySoundW(MAKEINTRESOURCEW(win?202:201),instance,SND_RESOURCE|SND_ASYNC|SND_NODEFAULT);}
static void startSpin(){
    if(spinning||wonModal||helpModal)return;
    pending=game.spin([](){return int(randomEngine()%4);});
    spinning=true;started=GetTickCount64();status=L"Барабаны крутятся…";tone(false);SetTimer(wnd,1,16,nullptr);InvalidateRect(wnd,nullptr,FALSE);
}
static void openChannel(){
    if(channel==L"@YOUR_CHANNEL")return;
    std::wstring url=L"https://t.me/"+channel.substr(1);ShellExecuteW(wnd,L"open",url.c_str(),nullptr,nullptr,SW_SHOWNORMAL);
}
static void draw(Graphics&g){
    g.Clear(bg);g.SetSmoothingMode(SmoothingModeAntiAlias);g.SetInterpolationMode(InterpolationModeHighQualityBicubic);g.SetTextRenderingHint(TextRenderingHintClearTypeGridFit);
    g.TranslateTransform(offsetX,offsetY);g.ScaleTransform(scale,scale);
    SolidBrush glow(Color(18,255,84,126));g.FillEllipse(&glow,560,-220,700,510);
    image(g,4,RectF(40,36,48,48));
    txt(g,L"Lucky Break",RectF(102,33,400,30),24,ink,true);
    txt(g,L"Слоты",RectF(103,64,300,21),12,muted);
    box(g,soundRect,Color(255,24,28,41),12);centered(g,soundOn?L"♪":L"♫×",861,39,58,40,22,muted);
    box(g,helpRect,Color(255,24,28,41),12);centered(g,L"?",933,39,58,40,21,muted,true);
    centered(g,L"ОДИН СПИН. ТРИ СОВПАДЕНИЯ.",40,110,960,46,32,ink,true);
    centered(g,status,40,164,960,30,15,spinning?accent:muted);
    box(g,RectF(40,214,960,395),Color(255,17,21,32),26,Color(255,40,47,66));
    txt(g,L"ВИРТУАЛЬНЫЙ БАЛАНС",RectF(68,232,260,22),10,muted,true);
    txt(g,number(game.balance-(spinning&&pending.win?lucky::Game::reward:0))+L"  ◈",RectF(68,254,260,34),27,ink,true);
    txt(g,L"СТАВКА",RectF(718,232,250,22),10,muted,true,StringAlignmentFar);
    txt(g,L"50  ◈",RectF(718,254,250,34),27,ink,true,StringAlignmentFar);
    float elapsed=spinning?float(GetTickCount64()-started):10000;
    for(int i=0;i<3;i++){
        float x=80+i*296;
        Color border=wonModal?Color(255,86,222,174):Color(255,45,53,76);
        box(g,RectF(x,309,288,236),Color(255,11,14,24),20,border);
        auto clip=g.Save();g.SetClip(RectF(x+2,311,284,232));
        float duration=1650+i*380;
        if(spinning&&elapsed<duration){
            float t=std::min(1.f,elapsed/duration);float ease=1-std::pow(1-t,3.f);
            float pos=ease*(16+i*4);int step=int(pos);float frac=pos-step;
            for(int k=-1;k<=2;++k){int symbol=(shown[i]+step+k+8)%4;
                if(step>=15+i*4)symbol=pending.symbols[i];
                float yy=342+(k-frac)*176;image(g,symbol,RectF(x+64,yy,160,160),.95f);}
        }else image(g,spinning?pending.symbols[i]:shown[i],RectF(x+58,335,172,172));
        g.Restore(clip);
        int label=spinning&&elapsed<duration?-1:(spinning?pending.symbols[i]:shown[i]);
        centered(g,label<0?L"• • •":names[label],x,513,288,25,12,muted);
    }
    SolidBrush lineDot(accent);g.FillEllipse(&lineDot,60,422,8,8);g.FillEllipse(&lineDot,972,422,8,8);
    centered(g,L"ТРИ ОДИНАКОВЫХ  =  500 КРЕДИТОВ",68,560,904,26,12,Color(255,176,184,207),true);
    bool disabled=spinning;auto p=rounded(spinRect,20);
    LinearGradientBrush grad(PointF(356,645),PointF(684,715),disabled?Color(255,76,49,70):Color(255,255,112,105),disabled?Color(255,69,45,70):Color(255,233,62,124));
    g.FillPath(&grad,p.get());centered(g,spinning?L"КРУТИТСЯ…":L"КРУТИТЬ  →",356,645,328,70,23,disabled?muted:Color(255,255,255,255),true);
    txt(g,L"СПИНЫ",RectF(44,647,230,24),10,muted,true);txt(g,number(game.spins),RectF(44,671,230,35),24,ink,true);
    txt(g,L"ПРОБЕЛ — КРУТИТЬ",RectF(746,648,250,24),11,muted,false,StringAlignmentFar);
    txt(g,L"3 совпадения — победа",RectF(746,676,250,24),12,muted,false,StringAlignmentFar);
    if(wonModal||helpModal){
        SolidBrush cover(Color(210,3,5,12));g.FillRectangle(&cover,0.f,0.f,W,H);
        box(g,RectF(254,204,532,408),Color(255,23,28,43),26,Color(255,66,78,105));
        if(wonModal){
            image(g,pending.symbols[0],RectF(475,224,90,90));
            centered(g,L"ЕСТЬ СОВПАДЕНИЕ!",282,324,476,40,29,ink,true);
            centered(g,L"+500 кредитов",282,369,476,32,22,Color(255,95,229,175),true);
            centered(g,L"Продолжение — в моём Telegram",282,419,476,30,16,muted);
            box(g,openRect,Color(255,46,139,202),14);centered(g,channel,290,475,460,56,22,ink,true);
            centered(g,L"Продолжить игру",290,545,460,44,15,muted);
        }else{
            centered(g,L"КАК ИГРАТЬ",282,242,476,45,28,ink,true);
            centered(g,L"Одно вращение стоит 50 кредитов.",282,313,476,32,17,muted);
            centered(g,L"Собери три одинаковых значка ИИ.",282,353,476,32,17,muted);
            centered(g,L"Выигрыш — 500 виртуальных кредитов.",282,393,476,32,17,muted);
            centered(g,L"Пробел — крутить. Esc — закрыть окно.",282,439,476,32,14,muted);
            box(g,continueRect,accent,12);centered(g,L"ПОНЯТНО",290,545,460,44,16,ink,true);
        }
    }
}
static LRESULT CALLBACK procedure(HWND h,UINT msg,WPARAM wp,LPARAM lp){
    switch(msg){
    case WM_ERASEBKGND:return 1;
    case WM_SIZE:InvalidateRect(h,nullptr,FALSE);return 0;
    case WM_GETMINMAXINFO:{auto p=reinterpret_cast<MINMAXINFO*>(lp);p->ptMinTrackSize={760,640};return 0;}
    case WM_PAINT:{
        PAINTSTRUCT ps;HDC dc=BeginPaint(h,&ps);RECT r;GetClientRect(h,&r);
        if(r.right>0&&r.bottom>0){
            Bitmap back(r.right,r.bottom,PixelFormat32bppPARGB);Graphics canvas(&back);
            scale=std::min(r.right/W,r.bottom/H);offsetX=(r.right-W*scale)/2;offsetY=(r.bottom-H*scale)/2;
            draw(canvas);Graphics screen(dc);screen.DrawImage(&back,0,0);
        }
        EndPaint(h,&ps);return 0;}
    case WM_TIMER:if(spinning){
        if(GetTickCount64()-started>=2500){spinning=false;shown=pending.symbols;KillTimer(h,1);
            if(pending.win){wonModal=true;status=L"Отличная комбинация. Попробуем ещё?";tone(true);}
            else status=L"Почти! Следующее вращение может стать твоим.";
        }InvalidateRect(h,nullptr,FALSE);
    }return 0;
    case WM_LBUTTONUP:{
        float x=(GET_X_LPARAM(lp)-offsetX)/scale,y=(GET_Y_LPARAM(lp)-offsetY)/scale;
        if(wonModal){if(inside(continueRect,x,y))wonModal=false;else if(inside(openRect,x,y))openChannel();}
        else if(helpModal){if(inside(continueRect,x,y))helpModal=false;}
        else if(inside(soundRect,x,y)){soundOn=!soundOn;}
        else if(inside(helpRect,x,y))helpModal=true;
        else if(inside(spinRect,x,y))startSpin();
        InvalidateRect(h,nullptr,FALSE);return 0;}
    case WM_KEYDOWN:
        if(wp==VK_ESCAPE){wonModal=false;helpModal=false;InvalidateRect(h,nullptr,FALSE);}
        else if(wp==VK_SPACE&&!(lp&(1LL<<30)))startSpin();return 0;
    case WM_SETCURSOR:if(LOWORD(lp)==HTCLIENT){POINT p;GetCursorPos(&p);ScreenToClient(h,&p);float x=(p.x-offsetX)/scale,y=(p.y-offsetY)/scale;bool hand=wonModal?(inside(openRect,x,y)||inside(continueRect,x,y)):helpModal?inside(continueRect,x,y):(inside(spinRect,x,y)||inside(helpRect,x,y)||inside(soundRect,x,y));SetCursor(LoadCursor(nullptr,hand?IDC_HAND:IDC_ARROW));return TRUE;}break;
    case WM_DESTROY:KillTimer(h,1);PostQuitMessage(0);return 0;
    }return DefWindowProc(h,msg,wp,lp);
}
int WINAPI wWinMain(HINSTANCE hi,HINSTANCE,LPWSTR,int show){
    instance=hi;SetProcessDPIAware();GdiplusStartupInput input;ULONG_PTR token;
    if(GdiplusStartup(&token,&input,nullptr)!=Ok)return 1;
    for(int i=0;i<5;++i){
        HRSRC resource=FindResourceW(hi,MAKEINTRESOURCEW(101+i),RT_RCDATA);
        if(!resource){MessageBoxW(nullptr,L"Не удалось загрузить значки игры.",L"Lucky Break",MB_ICONERROR);return 2;}
        HGLOBAL data=LoadResource(hi,resource);DWORD size=SizeofResource(hi,resource);IStream* stream=SHCreateMemStream(static_cast<const BYTE*>(LockResource(data)),size);
        imageStreams.push_back(stream);icons[i].reset(Bitmap::FromStream(stream));
    }
    loadChannel();WNDCLASSW wc{};wc.lpfnWndProc=procedure;wc.hInstance=hi;wc.lpszClassName=L"LuckyBreakAiSlots";wc.hCursor=LoadCursor(nullptr,IDC_ARROW);wc.hIcon=LoadIconW(hi,MAKEINTRESOURCEW(1));RegisterClassW(&wc);
    RECT rect{0,0,int(W),int(H)};AdjustWindowRect(&rect,WS_OVERLAPPEDWINDOW,FALSE);
    RECT desktop;SystemParametersInfoW(SPI_GETWORKAREA,0,&desktop,0);
    int width=std::min(rect.right-rect.left,desktop.right-desktop.left-32),height=std::min(rect.bottom-rect.top,desktop.bottom-desktop.top-32);
    int x=desktop.left+(desktop.right-desktop.left-width)/2,y=desktop.top+(desktop.bottom-desktop.top-height)/2;
    wnd=CreateWindowW(wc.lpszClassName,L"Lucky Break",WS_OVERLAPPEDWINDOW,x,y,width,height,nullptr,nullptr,hi,nullptr);
    if(!wnd)return 3;
    ShowWindow(wnd,show);UpdateWindow(wnd);MSG msg{};while(GetMessageW(&msg,nullptr,0,0)>0){TranslateMessage(&msg);DispatchMessageW(&msg);}
    for(auto&i:icons)i.reset();for(auto*s:imageStreams)if(s)s->Release();GdiplusShutdown(token);return 0;
}
