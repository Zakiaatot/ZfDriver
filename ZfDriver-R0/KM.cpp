#include "KM.h"

KM::KOM_CALLBACK KM::gKoMCallBack = { 0 };

NTSTATUS  KM::searchMouServiceCallBack(IN PDRIVER_OBJECT driverObject)
{
	//�����õ���һ��ȫ�ֱ�������Щ����������ǹ���˼���  
	NTSTATUS status = STATUS_SUCCESS;
	UNICODE_STRING uniNtNameString;
	PDEVICE_OBJECT pTargetDeviceObject = NULL;
	PDRIVER_OBJECT kbdDriverObject = NULL;
	PDRIVER_OBJECT kbdhidDriverObject = NULL;
	PDRIVER_OBJECT kbd8042DriverObject = NULL;
	PDRIVER_OBJECT usingDriverObject = NULL;
	PDEVICE_OBJECT usingDeviceObject = NULL;

	PVOID UsingDeviceExt = NULL;

	//����Ĵ���������USB���̶˿���������������  
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
	//��PS/2���̵���������  
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
	//��������豸��û���ҵ�  
	if (!kbd8042DriverObject && !kbdhidDriverObject)
	{
		return STATUS_SUCCESS;
	}
	//���USB���̺�PS/2����ͬʱ���ڣ�ʹ��USB���
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
		//���û�гɹ���ֱ�ӷ��ؼ���  
		DbgPrint("[ZfDriver] Coundn't get the Mouse driver Object\n");
		return STATUS_UNSUCCESSFUL;
	}
	else
	{
		ObDereferenceObject(kbdDriverObject);
	}
	//����KbdDriverObject�µ��豸���� 
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
		//�����������ҵ��Ķ˿��������豸��չ��ÿһ��ָ��  
		for (i = 0; i < 4096; i++, deviceExt++)
		{
			PVOID tmp;
			if (!MmIsAddressValid(deviceExt))
			{
				break;
			}
			//�ҵ������д�����ȫ�ֱ����У��������Ƿ��Ѿ������  
			//����Ѿ�����˾Ͳ��ü������ˣ�����ֱ���˳�  
			if (gKoMCallBack.MouDeviceObject && gKoMCallBack.MouseClassServiceCallback)
			{
				status = STATUS_SUCCESS;
				break;
			}
			//�ڶ˿��������豸��չ��ҵ����������豸��������������豸��������  
			tmp = *(PVOID*)deviceExt;
			if (tmp == pTargetDeviceObject)
			{
				gKoMCallBack.MouDeviceObject = pTargetDeviceObject;
				continue;
			}

			//������豸��չ���ҵ�һ����ַλ��KbdClass��������У��Ϳ�����Ϊ�����������Ҫ�ҵĻص�����  
			if ((tmp > kbdDriverStart) && (tmp < (UCHAR*)kbdDriverStart + kbdDriverSize) &&
				(MmIsAddressValid(tmp)))
			{
				//������ص�������¼����  
				gKoMCallBack.MouseClassServiceCallback = (MOUSECALLBACK)tmp;
				//g_KoMCallBack.MouSerCallAddr = (PVOID *)DeviceExt;
				status = STATUS_SUCCESS;
			}
		}
		if (status == STATUS_SUCCESS)
		{
			break;
		}
		//������һ���豸����������  
		pTargetDeviceObject = pTargetDeviceObject->NextDevice;
	}
	return status;
}

NTSTATUS  KM::searchKdbServiceCallBack(IN PDRIVER_OBJECT driverObject)
{
	//�����õ���һ��ȫ�ֱ�������Щ����������ǹ���˼���  
	NTSTATUS status = STATUS_SUCCESS;
	UNICODE_STRING uniNtNameString;
	PDEVICE_OBJECT pTargetDeviceObject = NULL;
	PDRIVER_OBJECT kbdDriverObject = NULL;
	PDRIVER_OBJECT kbdhidDriverObject = NULL;
	PDRIVER_OBJECT kbd8042DriverObject = NULL;
	PDRIVER_OBJECT usingDriverObject = NULL;
	PDEVICE_OBJECT usingDeviceObject = NULL;

	PVOID UsingDeviceExt = NULL;

	//����Ĵ���������USB���̶˿���������������  
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
	//��PS/2���̵���������  
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
	//��δ��뿼����һ�����������õ���������USB���̺�PS/2����ͬʱ���ڣ���PS/2����
	//��������豸��û���ҵ�  
	if (!kbd8042DriverObject && !kbdhidDriverObject)
	{
		return STATUS_SUCCESS;
	}
	//�ҵ����ʵ��������󣬲�����USB����PS/2������һ��Ҫ�ҵ�һ��   
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
		//���û�гɹ���ֱ�ӷ��ؼ���  
		DbgPrint("[ZfDriver] Coundn't get the kbd driver Object\n");
		return STATUS_UNSUCCESSFUL;
	}
	else
	{
		ObDereferenceObject(kbdDriverObject);
	}

	//����KbdDriverObject�µ��豸���� 
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

	//����ɹ��ҵ��ˣ��Ͱ���������滻�������Լ��Ļص�����  
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
		//�����������ҵ��Ķ˿��������豸��չ��ÿһ��ָ��  
		for (i = 0; i < 4096; i++, deviceExt++)
		{
			PVOID tmp;
			if (!MmIsAddressValid(deviceExt))
			{
				break;
			}
			//�ҵ������д�����ȫ�ֱ����У��������Ƿ��Ѿ������  
			//����Ѿ�����˾Ͳ��ü������ˣ�����ֱ���˳�  
			if (gKoMCallBack.KdbDeviceObject && gKoMCallBack.KeyboardClassServiceCallback)
			{
				status = STATUS_SUCCESS;
				break;
			}
			//�ڶ˿��������豸��չ��ҵ����������豸��������������豸��������  
			tmp = *(PVOID*)deviceExt;
			if (tmp == pTargetDeviceObject)
			{
				gKoMCallBack.KdbDeviceObject = pTargetDeviceObject;
				continue;
			}

			//������豸��չ���ҵ�һ����ַλ��KbdClass��������У��Ϳ�����Ϊ�����������Ҫ�ҵĻص�����  
			if ((tmp > kbdDriverStart) && (tmp < (UCHAR*)kbdDriverStart + kbdDriverSize) &&
				(MmIsAddressValid(tmp)))
			{
				//������ص�������¼����  
				gKoMCallBack.KeyboardClassServiceCallback = (KEYBOARDCALLBACK)tmp;
			}
		}
		if (status == STATUS_SUCCESS)
		{
			break;
		}
		//������һ���豸����������  
		pTargetDeviceObject = pTargetDeviceObject->NextDevice;
	}
	return status;
}