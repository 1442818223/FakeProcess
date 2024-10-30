#include "gmane.h"
#include "LoadDriver.h"
#include "api.h"



// ���� IOCTL �ĺ���
BOOL SendIoctl1(HANDLE hDevice, DWORD ioctlCode, ULONG64 data) {
	DWORD returned;

	HANDLE wd = (HANDLE)data;
	BOOL success = DeviceIoControl(
		hDevice,
		ioctlCode,
		&wd,             // ���뻺����
		sizeof(wd),      // ���뻺������С
		nullptr,         // ���������
		0,               // �����������С
		&returned,       // ���ص��ֽ���
		nullptr          // �ص��ṹ
	);
	if (success) {
		
		return TRUE; // ���� TRUE ��ʾ�ɹ�
	}
	else {
		
		return FALSE; // ���� FALSE ��ʾʧ��
	}
}




 HANDLE hDevice = NULL;
 __declspec(dllexport) BOOL LOADN()
 {

	 if (SH_DriverLoad())
	 {

		 // ���豸
		 hDevice = CreateFile(
			 L"\\\\.\\MMMMMu",  // �豸��
			 GENERIC_WRITE | GENERIC_READ,
			 0,
			 nullptr,
			 OPEN_EXISTING,
			 FILE_ATTRIBUTE_NORMAL,
			 nullptr
		 );

		 if (hDevice == INVALID_HANDLE_VALUE) {

			 return FALSE;
		 }

		 return TRUE;
	 }
	 else {
		 // ��������ʧ�ܵĴ���
		 return FALSE;
	 }



	 return TRUE; // �������سɹ�
 }


// ��������
 EXTERN_C BOOLEAN WINAPI  LOADENTRY(UINT pid) {


	// ֹͣ���������   MMMMMu���������Ƿ�����
	 const char* stopCmd = "sc stop MMMMMu";  // �޸�Ϊ MyDriver
	 // ɾ�����������
	 const char* deleteCmd = "sc delete MMMMMu"; // �޸�Ϊ MyDriver

	 // ִ��ֹͣ��������
	 int resultStop = system(stopCmd);
	 if (resultStop != 0) {
		 printf("ֹͣ����ʧ��\n");
	 }

	 // ִ��ɾ����������
	 int resultDelete = system(deleteCmd);
	 if (resultDelete != 0) {
		 printf("ɾ������ʧ��\n");
	 }


	if(!LOADN()) return FALSE ;


	// ���Ϳ��ƴ��� ioctl_disk_customize_serial
	BOOL ison = SendIoctl1(hDevice, ioctl_disk_customize_serial, pid);

	CloseHandle(hDevice); // ȷ���ڽ���ǰ�ر��豸���

	return ison;
}
