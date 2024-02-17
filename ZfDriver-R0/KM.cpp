#include "KM.h"

KM::KOM_CALLBACK KM::gKoMCallBack = { 0 };

NTSTATUS  KM::searchMouServiceCallBack(IN PDRIVER_OBJECT driverObject)
{
	//定义用到的一组全局变量，这些变量大多数是顾名思义的  
	NTSTATUS status = STATUS_SUCCESS;
	UNICODE_STRING uniNtNameString;
	PDEVICE_OBJECT pTargetDeviceObject = NULL;
	PDRIVER_OBJECT kbdDriverObject = NULL;
	PDRIVER_OBJECT kbdhidDriverObject = NULL;
	PDRIVER_OBJECT kbd8042DriverObject = NULL;
	PDRIVER_OBJECT usingDriverObject = NULL;
	PDEVICE_OBJECT usingDeviceObject = NULL;

	PVOID UsingDeviceExt = NULL;

	//这里的代码用来打开USB键盘端口驱动的驱动对象  
	RtlInitUnicodeString(&uniNtNameString, L"\\Driver\\mouhid");
	status = ObReferenceObjectByName(&uniNtNameString,
		OBJ_CASE_INSENSITIVE, NULL, 0,
		*IoDriverObjectType,
		KernelMode,
		NULL,
		(PVOID*)&kbdhidDriverObject);
	if (!NT_SUCCESS(status))
	{
		DbgPrint("[ZfDriver] Couldn't get the USB Mouse Object\n");
	}
	else
	{
		ObDereferenceObject(kbdhidDriverObject);
		DbgPrint("[ZfDriver] Get the USB Mouse Object\n");
	}
	//打开PS/2键盘的驱动对象  
	RtlInitUnicodeString(&uniNtNameString, L"\\Driver\\i8042prt");
	status = ObReferenceObjectByName(&uniNtNameString,
		OBJ_CASE_INSENSITIVE,
		NULL, 0,
		*IoDriverObjectType,
		KernelMode,
		NULL,
		(PVOID*)&kbd8042DriverObject);
	if (!NT_SUCCESS(status))
	{
		DbgPrint("[ZfDriver] Couldn't get the PS/2 Mouse Object %08x\n", status);
	}
	else
	{
		ObDereferenceObject(kbd8042DriverObject);
		DbgPrint("get the PS/2 Mouse Object\n");
	}
	//如果两个设备都没有找到  
	if (!kbd8042DriverObject && !kbdhidDriverObject)
	{
		return STATUS_SUCCESS;
	}
	//如果USB键盘和PS/2键盘同时存在，使用USB鼠标
	if (kbdhidDriverObject)
	{
		usingDriverObject = kbdhidDriverObject;
	}
	else
	{
		usingDriverObject = kbd8042DriverObject;
	}
	RtlInitUnicodeString(&uniNtNameString, L"\\Driver\\mouclass");
	status = ObReferenceObjectByName(&uniNtNameString,
		OBJ_CASE_INSENSITIVE, NULL,
		0,
		*IoDriverObjectType,
		KernelMode,
		NULL,
		(PVOID*)&kbdDriverObject);
	if (!NT_SUCCESS(status))
	{
		//如果没有成功，直接返回即可  
		DbgPrint("[ZfDriver] Coundn't get the Mouse driver Object\n");
		return STATUS_UNSUCCESSFUL;
	}
	else
	{
		ObDereferenceObject(kbdDriverObject);
	}
	//遍历KbdDriverObject下的设备对象 
	usingDeviceObject = usingDriverObject->DeviceObject;
	while (usingDeviceObject)
	{
		status = searchServiceFromMouExt(kbdDriverObject, usingDeviceObject);
		if (status == STATUS_SUCCESS)
		{
			break;
		}
		usingDeviceObject = usingDeviceObject->NextDevice;
	}
	if (gKoMCallBack.MouDeviceObject && gKoMCallBack.MouseClassServiceCallback)
	{
		DbgPrint("[ZfDriver] Find MouseClassServiceCallback\n");
	}
	return status;
}

NTSTATUS KM::searchServiceFromMouExt(PDRIVER_OBJECT mouDriverObject, PDEVICE_OBJECT pPortDev)
{
	PDEVICE_OBJECT pTargetDeviceObject = NULL;
	UCHAR* deviceExt;
	int i = 0;
	NTSTATUS status;
	PVOID kbdDriverStart;
	ULONG kbdDriverSize = 0;
	PDEVICE_OBJECT  pTmpDev;
	UNICODE_STRING  kbdDriName;

	kbdDriverStart = mouDriverObject->DriverStart;
	kbdDriverSize = mouDriverObject->DriverSize;

	status = STATUS_UNSUCCESSFUL;

	RtlInitUnicodeString(&kbdDriName, L"\\Driver\\mouclass");
	pTmpDev = pPortDev;
	while (pTmpDev->AttachedDevice != NULL)
	{
		DbgPrint("[ZfDriver] Att:  0x%x", pTmpDev->AttachedDevice);
		DbgPrint("[ZfDriver] Dri Name : %wZ", &pTmpDev->AttachedDevice->DriverObject->DriverName);
		if (RtlCompareUnicodeString(&pTmpDev->AttachedDevice->DriverObject->DriverName,
			&kbdDriName, TRUE) == 0)
		{
			DbgPrint("[ZfDriver] Find Object Device: ");
			break;
		}
		pTmpDev = pTmpDev->AttachedDevice;
	}
	if (pTmpDev->AttachedDevice == NULL)
	{
		return status;
	}
	pTargetDeviceObject = mouDriverObject->DeviceObject;
	while (pTargetDeviceObject)
	{
		if (pTmpDev->AttachedDevice != pTargetDeviceObject)
		{
			pTargetDeviceObject = pTargetDeviceObject->NextDevice;
			continue;
		}
		deviceExt = (UCHAR*)pTmpDev->DeviceExtension;
		gKoMCallBack.MouDeviceObject = NULL;
		//遍历我们先找到的端口驱动的设备扩展的每一个指针  
		for (i = 0; i < 4096; i++, deviceExt++)
		{
			PVOID tmp;
			if (!MmIsAddressValid(deviceExt))
			{
				break;
			}
			//找到后会填写到这个全局变量中，这里检查是否已经填好了  
			//如果已经填好了就不用继续找了，可以直接退出  
			if (gKoMCallBack.MouDeviceObject && gKoMCallBack.MouseClassServiceCallback)
			{
				status = STATUS_SUCCESS;
				break;
			}
			//在端口驱动的设备扩展里，找到了类驱动设备对象，填好类驱动设备对象后继续  
			tmp = *(PVOID*)deviceExt;
			if (tmp == pTargetDeviceObject)
			{
				gKoMCallBack.MouDeviceObject = pTargetDeviceObject;
				continue;
			}

			//如果在设备扩展中找到一个地址位于KbdClass这个驱动中，就可以认为，这就是我们要找的回调函数  
			if ((tmp > kbdDriverStart) && (tmp < (UCHAR*)kbdDriverStart + kbdDriverSize) &&
				(MmIsAddressValid(tmp)))
			{
				//将这个回调函数记录下来  
				gKoMCallBack.MouseClassServiceCallback = (MOUSECALLBACK)tmp;
				//g_KoMCallBack.MouSerCallAddr = (PVOID *)DeviceExt;
				status = STATUS_SUCCESS;
			}
		}
		if (status == STATUS_SUCCESS)
		{
			break;
		}
		//换成下一个设备，继续遍历  
		pTargetDeviceObject = pTargetDeviceObject->NextDevice;
	}
	return status;
}

NTSTATUS  KM::searchKdbServiceCallBack(IN PDRIVER_OBJECT driverObject)
{
	//定义用到的一组全局变量，这些变量大多数是顾名思义的  
	NTSTATUS status = STATUS_SUCCESS;
	UNICODE_STRING uniNtNameString;
	PDEVICE_OBJECT pTargetDeviceObject = NULL;
	PDRIVER_OBJECT kbdDriverObject = NULL;
	PDRIVER_OBJECT kbdhidDriverObject = NULL;
	PDRIVER_OBJECT kbd8042DriverObject = NULL;
	PDRIVER_OBJECT usingDriverObject = NULL;
	PDEVICE_OBJECT usingDeviceObject = NULL;

	PVOID UsingDeviceExt = NULL;

	//这里的代码用来打开USB键盘端口驱动的驱动对象  
	RtlInitUnicodeString(&uniNtNameString, L"\\Driver\\kbdhid");
	status = ObReferenceObjectByName(&uniNtNameString,
		OBJ_CASE_INSENSITIVE, NULL, 0,
		*IoDriverObjectType,
		KernelMode,
		NULL,
		(PVOID*)&kbdhidDriverObject);
	if (!NT_SUCCESS(status))
	{
		DbgPrint("[ZfDriver] Couldn't get the USB driver Object\n");
	}
	else
	{
		ObDereferenceObject(kbdhidDriverObject);
		DbgPrint("[ZfDriver] Get the USB driver Object\n");
	}
	//打开PS/2键盘的驱动对象  
	RtlInitUnicodeString(&uniNtNameString, L"\\Driver\\i8042prt");
	status = ObReferenceObjectByName(&uniNtNameString,
		OBJ_CASE_INSENSITIVE,
		NULL, 0,
		*IoDriverObjectType,
		KernelMode,
		NULL,
		(PVOID*)&kbd8042DriverObject);
	if (!NT_SUCCESS(status))
	{
		DbgPrint("[ZfDriver] Couldn't get the PS/2 driver Object %08x\n", status);
	}
	else
	{
		ObDereferenceObject(kbd8042DriverObject);
		DbgPrint("[ZfDriver] Get the PS/2 driver Object\n");
	}
	//这段代码考虑有一个键盘起作用的情况。如果USB键盘和PS/2键盘同时存在，用PS/2键盘
	//如果两个设备都没有找到  
	if (!kbd8042DriverObject && !kbdhidDriverObject)
	{
		return STATUS_SUCCESS;
	}
	//找到合适的驱动对象，不管是USB还是PS/2，反正一定要找到一个   
	usingDriverObject = kbd8042DriverObject ? kbd8042DriverObject : kbdhidDriverObject;

	RtlInitUnicodeString(&uniNtNameString, L"\\Driver\\kbdclass");
	status = ObReferenceObjectByName(&uniNtNameString,
		OBJ_CASE_INSENSITIVE, NULL,
		0,
		*IoDriverObjectType,
		KernelMode,
		NULL,
		(PVOID*)&kbdDriverObject);
	if (!NT_SUCCESS(status))
	{
		//如果没有成功，直接返回即可  
		DbgPrint("[ZfDriver] Coundn't get the kbd driver Object\n");
		return STATUS_UNSUCCESSFUL;
	}
	else
	{
		ObDereferenceObject(kbdDriverObject);
	}

	//遍历KbdDriverObject下的设备对象 
	usingDeviceObject = usingDriverObject->DeviceObject;
	while (usingDeviceObject)
	{
		status = searchServiceFromKdbExt(kbdDriverObject, usingDeviceObject);
		if (status == STATUS_SUCCESS)
		{
			break;
		}
		usingDeviceObject = usingDeviceObject->NextDevice;
	}

	//如果成功找到了，就把这个函数替换成我们自己的回调函数  
	if (gKoMCallBack.KdbDeviceObject && gKoMCallBack.KeyboardClassServiceCallback)
	{
		DbgPrint("[ZfDriver] Find keyboradClassServiceCallback\n");
	}
	return status;
}

NTSTATUS KM::searchServiceFromKdbExt(PDRIVER_OBJECT kbdDriverObject, PDEVICE_OBJECT pPortDev)
{
	PDEVICE_OBJECT pTargetDeviceObject = NULL;
	UCHAR* deviceExt;
	int i = 0;
	NTSTATUS status;
	PVOID kbdDriverStart;
	ULONG kbdDriverSize = 0;
	PDEVICE_OBJECT  pTmpDev;
	UNICODE_STRING  kbdDriName;

	kbdDriverStart = kbdDriverObject->DriverStart;
	kbdDriverSize = kbdDriverObject->DriverSize;

	status = STATUS_UNSUCCESSFUL;

	RtlInitUnicodeString(&kbdDriName, L"\\Driver\\kbdclass");
	pTmpDev = pPortDev;
	while (pTmpDev->AttachedDevice != NULL)
	{
		DbgPrint("[ZfDriver] Att:  0x%x", pTmpDev->AttachedDevice);
		DbgPrint("[ZfDriver] Dri Name : %wZ", &pTmpDev->AttachedDevice->DriverObject->DriverName);
		if (RtlCompareUnicodeString(&pTmpDev->AttachedDevice->DriverObject->DriverName,
			&kbdDriName, TRUE) == 0)
		{
			break;
		}
		pTmpDev = pTmpDev->AttachedDevice;
	}
	if (pTmpDev->AttachedDevice == NULL)
	{
		return status;
	}

	pTargetDeviceObject = kbdDriverObject->DeviceObject;
	while (pTargetDeviceObject)
	{
		if (pTmpDev->AttachedDevice != pTargetDeviceObject)
		{
			pTargetDeviceObject = pTargetDeviceObject->NextDevice;
			continue;
		}
		deviceExt = (UCHAR*)pTmpDev->DeviceExtension;
		gKoMCallBack.KdbDeviceObject = NULL;
		//遍历我们先找到的端口驱动的设备扩展的每一个指针  
		for (i = 0; i < 4096; i++, deviceExt++)
		{
			PVOID tmp;
			if (!MmIsAddressValid(deviceExt))
			{
				break;
			}
			//找到后会填写到这个全局变量中，这里检查是否已经填好了  
			//如果已经填好了就不用继续找了，可以直接退出  
			if (gKoMCallBack.KdbDeviceObject && gKoMCallBack.KeyboardClassServiceCallback)
			{
				status = STATUS_SUCCESS;
				break;
			}
			//在端口驱动的设备扩展里，找到了类驱动设备对象，填好类驱动设备对象后继续  
			tmp = *(PVOID*)deviceExt;
			if (tmp == pTargetDeviceObject)
			{
				gKoMCallBack.KdbDeviceObject = pTargetDeviceObject;
				continue;
			}

			//如果在设备扩展中找到一个地址位于KbdClass这个驱动中，就可以认为，这就是我们要找的回调函数  
			if ((tmp > kbdDriverStart) && (tmp < (UCHAR*)kbdDriverStart + kbdDriverSize) &&
				(MmIsAddressValid(tmp)))
			{
				//将这个回调函数记录下来  
				gKoMCallBack.KeyboardClassServiceCallback = (KEYBOARDCALLBACK)tmp;
			}
		}
		if (status == STATUS_SUCCESS)
		{
			break;
		}
		//换成下一个设备，继续遍历  
		pTargetDeviceObject = pTargetDeviceObject->NextDevice;
	}
	return status;
}