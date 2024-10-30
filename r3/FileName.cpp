#include <Windows.h>
#include <iostream>
#include <string>

#define ioctl_disk_customize_serial CTL_CODE(FILE_DEVICE_UNKNOWN, 0x500, METHOD_OUT_DIRECT, FILE_ANY_ACCESS)


void SendIoctl(HANDLE hDevice, DWORD ioctlCode, ULONG64 ����) {
	DWORD returned;

	HANDLE wd = (HANDLE)����;
	BOOL success = DeviceIoControl(
		hDevice,
		ioctlCode,
		&wd,             // ���뻺����
		sizeof(����),      // ���뻺������С
		nullptr,               // ���������
		0,                     // �����������С
		&returned,             // ���ص��ֽ���
		nullptr                // �ص��ṹ
	);
	if (success) {
		std::cout << "Successfully sent IOCTL code: " << ioctlCode << std::endl;
	}
	else {
		std::cout << "Failed to send IOCTL code: " << ioctlCode << ". Error: " << GetLastError() << std::endl;
	}
}

int main() {
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// ��ȡ��ǰ����Ŀ¼
	char currentPath[MAX_PATH];
	if (GetCurrentDirectoryA(MAX_PATH, currentPath) == 0) {
		printf("��ȡ��ǰĿ¼ʧ��\n");
		return 1;
	}

	// �����������������·��
	char driverPath[MAX_PATH];
	snprintf(driverPath, sizeof(driverPath), "%s\\MMMMMu.sys", currentPath); // �������������ļ���Ϊ 12356.sys
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
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
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// ������������     MMMMMu���������Ƿ�����
	char createCmd[512];
	snprintf(createCmd, sizeof(createCmd), "sc create MMMMMu binPath= \"%s\" type= kernel start= demand", driverPath);
	int resultCreate = system(createCmd);
	if (resultCreate != 0) {
		printf("��������ʧ��\n");
		return resultCreate;
	}

	// ������������
	const char* startCmd = "sc start MMMMMu";
	int resultStart = system(startCmd);
	if (resultStart != 0) {
		printf("��������ʧ��\n");
		return resultStart;
	}

	printf("���������ѳɹ����ز�������\n");

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	HANDLE hDevice = CreateFile(
		L"\\\\.\\MMMMMu",  // �豸��
		GENERIC_WRITE | GENERIC_READ,
		0,
		nullptr,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		nullptr
	);

	if (hDevice == INVALID_HANDLE_VALUE) {
		std::cerr << "Failed to open device. Error: " << GetLastError() << std::endl;
		return 1;
	}

	std::cout << "Device opened successfully." << std::endl;

	// �û����� PID
	int pid;
	std::cout << "������ PID: ";
	std::cin >> pid;
	// ���Ϳ��ƴ��� ioctl_disk_customize_serial
	SendIoctl(hDevice, ioctl_disk_customize_serial, pid);


	CloseHandle(hDevice);


	system("pause");





}