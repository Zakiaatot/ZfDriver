#ifndef _KM_H_
#define _KM_H_

#include <ntifs.h>

namespace KM
{
	// Define
	typedef struct _KEYBOARD_INPUT_DATA {
		USHORT UnitId;
		USHORT MakeCode;
		USHORT Flags;
		USHORT Reserved;
		ULONG ExtraInformation;
	} KEYBOARD_INPUT_DATA, * PKEYBOARD_INPUT_DATA;

	typedef struct _MOUSE_INPUT_DATA {
		USHORT UnitId;
		USHORT Flags;
		union {
			ULONG Buttons;
			struct {
				USHORT  ButtonFlags;
				USHORT  ButtonData;
			};
		};
		ULONG RawButtons;
		LONG LastX;
		LONG LastY;
		ULONG ExtraInformation;
	} MOUSE_INPUT_DATA, * PMOUSE_INPUT_DATA;

	typedef VOID(*KEYBOARDCALLBACK) (PDEVICE_OBJECT  deviceObject,
		PKEYBOARD_INPUT_DATA  inputDataStart,
		PKEYBOARD_INPUT_DATA  inputDataEnd,
		PULONG  inputDataConsumed);

	typedef VOID(*MOUSECALLBACK) (PDEVICE_OBJECT  deviceObject,
		PMOUSE_INPUT_DATA  inputDataStart,
		PMOUSE_INPUT_DATA  inputDataEnd,
		PULONG  inputDataConsumed);

	typedef struct _DEVICE_EXTENSION {

		PDEVICE_OBJECT       kbdDeviceObject;        //键盘类设备对象
		PDEVICE_OBJECT       mouDeviceObject;        //鼠标类设备对象
		KEYBOARDCALLBACK  kbdCallback;         //KeyboardClassServiceCallback函数 
		MOUSECALLBACK     mouCallback;         //MouseClassServiceCallback函数
	}DEVICE_EXTENSION, * PDEVICE_EXTENSION;

	typedef struct _KOM_CALLBACK
	{
		PDEVICE_OBJECT KdbDeviceObject;
		KEYBOARDCALLBACK KeyboardClassServiceCallback;
		PDEVICE_OBJECT MouDeviceObject;
		MOUSECALLBACK MouseClassServiceCallback;
	}KOM_CALLBACK, * PKOM_CALLBACK;

	// Function
	NTSTATUS  searchMouServiceCallBack(IN PDRIVER_OBJECT driverObject);
	NTSTATUS searchServiceFromMouExt(PDRIVER_OBJECT mouDriverObject, PDEVICE_OBJECT pPortDev);
	NTSTATUS  searchKdbServiceCallBack(IN PDRIVER_OBJECT driverObject);
	NTSTATUS searchServiceFromKdbExt(PDRIVER_OBJECT kbdDriverObject, PDEVICE_OBJECT pPortDev);

	// Export
	extern "C"
	{
		extern POBJECT_TYPE* IoDriverObjectType;
		NTSTATUS ObReferenceObjectByName
		(
			IN PUNICODE_STRING ObjectName,
			IN ULONG Attributes,
			IN PACCESS_STATE PassedAccessState OPTIONAL,
			IN ACCESS_MASK DesiredAccess OPTIONAL,
			IN POBJECT_TYPE ObjectType,
			IN KPROCESSOR_MODE AccessMode,
			IN OUT PVOID ParseContext OPTIONAL,
			OUT PVOID* Object
		);
	}
	extern KOM_CALLBACK gKoMCallBack;
}

#endif // !_KM_H_
