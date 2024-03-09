# ZfDriver

Windows X64 平台驱动级内存读写库，方便无痕游戏辅助开发

目前支持：Win10 ~ Latest

**Warning**: Progressing Project...

[API](https://github.com/Zakiaatot/ZfDriver/blob/main/ZfDriver/ZfDriver.h):

```c
static BOOL Install(); // 驱动安装
static VOID Uninstall(); // 驱动卸载
static DWORD Test(IN DWORD num); // 驱动测试: 如果正常 返回 num+1
// Read
static BOOL ReadBytes(IN DWORD pid, IN DWORD64 address, IN DWORD size, OUT BYTE* data); // 读字节集: data需自己申请空间且确保空间大于size
static BOOL ReadByte(IN DWORD pid, IN DWORD64 address, OUT BYTE* data); // 读字节
static BOOL ReadShort(IN DWORD pid, IN DWORD64 address, OUT SHORT* data); // 读短整数
static BOOL ReadInt(IN DWORD pid, IN DWORD64 address, OUT INT* data); // 读整数
static BOOL ReadLong(IN DWORD pid, IN DWORD64 address, OUT LONGLONG* data); // 读长整数
static BOOL ReadFloat(IN DWORD pid, IN DWORD64 address, OUT FLOAT* data); // 读小数
static BOOL ReadDouble(IN DWORD pid, IN DWORD64 address, OUT DOUBLE* data); // 读双精度小数
// Write
static BOOL WriteBytes(IN DWORD pid, IN DWORD64 address, IN DWORD size, IN BYTE* data); // 写字节集: data为写入数据 确保一次写入小于1000字节
static BOOL WriteByte(IN DWORD pid, IN DWORD64 address, IN BYTE data); // 写字节
static BOOL WriteShort(IN DWORD pid, IN DWORD64 address, IN SHORT data); // 写短整数
static BOOL WriteInt(IN DWORD pid, IN DWORD64 address, IN INT data); // 写整数
static BOOL WriteLong(IN DWORD pid, IN DWORD64 address, IN LONGLONG data); // 写长整数
static BOOL WriteFloat(IN DWORD pid, IN DWORD64 address, IN FLOAT data); // 写小数
static BOOL WriteDouble(IN DWORD pid, IN DWORD64 address, IN DOUBLE data); // 写双精度小数
// Keyboard and Mouse
static BOOL KeyDown(IN USHORT keyCode); // 键盘按下
static BOOL keyUp(IN USHORT keyCode); // 键盘弹起
static BOOL MouseLeftButtonDown(); // 鼠标左键按下
static BOOL MouseLeftButtonUp(); // 鼠标左键弹起
static BOOL MouseRightButtonDown(); // 鼠标右键按下
static BOOL MouseRightButtonUp(); // 鼠标右键弹起
static BOOL MouseMiddleButtonDown(); // 鼠标滚轮按下
static BOOL MouseMiddleButtonUp(); // 鼠标滚轮弹起
static BOOL MouseMoveRelative(IN LONG dx, IN LONG dy); // 鼠标相对移动
static BOOL MouseMoveAbsolute(IN LONG dx, IN LONG dy); // 鼠标绝对移动
// GDI Draw
static BOOL GDIDrawInit(IN DRAW_LOOP drawLoop, IN INT fontSize = 16); // 初始化绘制
static BOOL GDIDrawDestroy(); // 结束绘制
static BOOL GDIDrawFps(); // 绘制FPS
static BOOL GDIDrawText(IN LONG x, IN LONG y, IN LPCWSTR str, IN COLORREF color, IN INT fontSize = 16); // 绘制文本
static BOOL GDIDrawLine(IN LONG x1, IN LONG y1, IN  LONG x2, IN  LONG y2, IN LONG lineWidth, IN  COLORREF color); // 绘制线条
static BOOL GDIDrawRect(IN LONG x, IN LONG y, IN LONG width, IN  LONG height, IN  LONG lineWidth, IN COLORREF color); // 绘制矩形
static BOOL GDIDrawRectFill(IN LONG x, IN LONG y, IN LONG width, IN LONG height, IN COLORREF color); // 填充矩形
static BOOL GDIDrawCircle(IN LONG x, IN LONG y, IN LONG r, IN COLORREF color, IN LONG lineCount, IN LONG lineWidth); // 绘制圆圈
// D3D Draw
static BOOL D3DDrawInit(IN DRAW_LOOP drawLoop, IN INT fontSize = 16); // 初始化绘制
static BOOL D3DDrawDestroy(); // 结束绘制
static BOOL D3DDrawFps(IN INT fontSzie); // 绘制FPS
static BOOL D3DDrawText(IN LONG x, IN LONG y, IN LPCWSTR str, IN D3DCOLOR color, IN INT fontSize = 16); // 绘制文本
static BOOL D3DDrawLine(IN FLOAT x1, IN FLOAT y1, IN  FLOAT x2, IN  FLOAT y2, IN FLOAT lineWidth, IN  D3DCOLOR color); // 绘制线条
static BOOL D3DDrawRect(IN FLOAT x, IN FLOAT y, IN FLOAT width, IN  FLOAT height, IN  FLOAT lineWidth, IN D3DCOLOR color); // 绘制矩形
static BOOL D3DDrawRectFill(IN FLOAT x, IN FLOAT y, IN FLOAT width, IN FLOAT height, IN D3DCOLOR color); // 填充矩形
static BOOL D3DDrawCircle(IN FLOAT x, IN FLOAT y, IN FLOAT r, IN D3DCOLOR color, IN LONG lineCount, IN FLOAT lineWidth); // 绘制圆圈
// Utils
static BOOL ForceDeleteFile(IN PCWSTR filePath); // 强制删除文件  filePath 为宽字符路径  例如 L"C:\\123.exe"
static DWORD64 GetModuleBase(IN DWORD pid, IN PCWSTR moduleName); // 取进程模块基址
static BOOL ProcessHide(IN DWORD pid, IN BOOL hide = TRUE); // 隐藏进程  hide==0时回复隐藏  Warning: 如果要用绘制，请先初始化绘制模块再隐藏进程
static BOOL WindowHide(IN HWND hwnd); // 隐藏窗口 反截图
static DWORD GetProcessId(IN PCWSTR processName);// 根据进程名称获取ID
static BOOL InjectDll(IN DWORD pid, IN PCWSTR dllPath);// DLL注入
```

ToDo:

- [x] MDL Read MDL 读内存
- [x] MDL Write MDL 写内存
- [x] Keyboard and Mouse 驱动键鼠
- [x] GetModuleBase 取进程模块基址
- [x] ForceDeleteFile 强删文件
- [x] ProcessHide 进程隐藏
- [x] WindowHide 窗口隐藏
- [x] GetProcessId 获取进程 ID
- [x] InjectDll DLL 注入
- [x] GDI 绘制 API
- [x] D3D 绘制 API
- [x] 打包为 Lib 和 Dll
- [ ] 转易语言模块
- [ ] 驱动隐藏
