#include <Windows.h>
#include <iostream>
#include <string>

#define ioctl_disk_customize_serial CTL_CODE(FILE_DEVICE_UNKNOWN, 0x500, METHOD_OUT_DIRECT, FILE_ANY_ACCESS)


void SendIoctl(HANDLE hDevice, DWORD ioctlCode, ULONG64 数据) {
	DWORD returned;

	HANDLE wd = (HANDLE)数据;
	BOOL success = DeviceIoControl(
		hDevice,
		ioctlCode,
		&wd,             // 输入缓冲区
		sizeof(数据),      // 输入缓冲区大小
		nullptr,               // 输出缓冲区
		0,                     // 输出缓冲区大小
		&returned,             // 返回的字节数
		nullptr                // 重叠结构
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
	// 获取当前工作目录
	char currentPath[MAX_PATH];
	if (GetCurrentDirectoryA(MAX_PATH, currentPath) == 0) {
		printf("获取当前目录失败\n");
		return 1;
	}

	// 构造驱动程序的完整路径
	char driverPath[MAX_PATH];
	snprintf(driverPath, sizeof(driverPath), "%s\\MMMMMu.sys", currentPath); // 假设驱动程序文件名为 12356.sys
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// 停止服务的命令   MMMMMu这两个都是服务名
	const char* stopCmd = "sc stop MMMMMu";  // 修改为 MyDriver
	// 删除服务的命令
	const char* deleteCmd = "sc delete MMMMMu"; // 修改为 MyDriver

	// 执行停止服务命令
	int resultStop = system(stopCmd);
	if (resultStop != 0) {
		printf("停止服务失败\n");
	}

	// 执行删除服务命令
	int resultDelete = system(deleteCmd);
	if (resultDelete != 0) {
		printf("删除服务失败\n");
	}
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// 创建服务命令     MMMMMu这两个都是服务名
	char createCmd[512];
	snprintf(createCmd, sizeof(createCmd), "sc create MMMMMu binPath= \"%s\" type= kernel start= demand", driverPath);
	int resultCreate = system(createCmd);
	if (resultCreate != 0) {
		printf("创建服务失败\n");
		return resultCreate;
	}

	// 启动服务命令
	const char* startCmd = "sc start MMMMMu";
	int resultStart = system(startCmd);
	if (resultStart != 0) {
		printf("启动服务失败\n");
		return resultStart;
	}

	printf("驱动程序已成功加载并启动。\n");

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	HANDLE hDevice = CreateFile(
		L"\\\\.\\MMMMMu",  // 设备名
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

	// 用户输入 PID
	int pid;
	std::cout << "请输入 PID: ";
	std::cin >> pid;
	// 发送控制代码 ioctl_disk_customize_serial
	SendIoctl(hDevice, ioctl_disk_customize_serial, pid);


	CloseHandle(hDevice);


	system("pause");





}