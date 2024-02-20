# ZfDriver

Windows 平台驱动级内存读写库，方便无痕游戏辅助开发
**Warning**: Progressing Project...

[API](https://github.com/Zakiaatot/ZfDriver/blob/main/ZfDriver/ZfDriver.h):

```c
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
static BOOL KeyDown(IN USHORT keyCode);
static BOOL keyUp(IN USHORT keyCode);
static BOOL MouseLeftButtonDown();
static BOOL MouseLeftButtonUp();
static BOOL MouseRightButtonDown();
static BOOL MouseRightButtonUp();
static BOOL MouseMiddleButtonDown();
static BOOL MouseMiddleButtonUp();
static BOOL MouseMoveRelative(LONG dx, LONG dy);
static BOOL MouseMoveAbsolute(LONG dx, LONG dy);
// Utils
static BOOL ForceDeleteFile(IN PCWSTR filePath); // 强制删除文件  filePath 为宽字符路径  例如 L"C:\\123.exe"
static DWORD64 GetModuleBase(IN DWORD pid, IN PCWSTR moduleName); // 取进程模块基址
static BOOL ProcessHide(IN DWORD pid); // 隐藏进程
static BOOL WindowHide(IN HWND hwnd); // 隐藏窗口 反截图
```
