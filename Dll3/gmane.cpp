#include "gmane.h"
#include "LoadDriver.h"
#include "api.h"



// 发送 IOCTL 的函数
BOOL SendIoctl1(HANDLE hDevice, DWORD ioctlCode, ULONG64 data) {
	DWORD returned;

	HANDLE wd = (HANDLE)data;
	BOOL success = DeviceIoControl(
		hDevice,
		ioctlCode,
		&wd,             // 输入缓冲区
		sizeof(wd),      // 输入缓冲区大小
		nullptr,         // 输出缓冲区
		0,               // 输出缓冲区大小
		&returned,       // 返回的字节数
		nullptr          // 重叠结构
	);
	if (success) {
		
		return TRUE; // 返回 TRUE 表示成功
	}
	else {
		
		return FALSE; // 返回 FALSE 表示失败
	}
}




 HANDLE hDevice = NULL;
 __declspec(dllexport) BOOL LOADN()
 {

	 if (SH_DriverLoad())
	 {

		 // 打开设备
		 hDevice = CreateFile(
			 L"\\\\.\\MMMMMu",  // 设备名
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
		 // 驱动加载失败的处理
		 return FALSE;
	 }



	 return TRUE; // 驱动加载成功
 }


// 启动函数
 EXTERN_C BOOLEAN WINAPI  LOADENTRY(UINT pid) {


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


	if(!LOADN()) return FALSE ;


	// 发送控制代码 ioctl_disk_customize_serial
	BOOL ison = SendIoctl1(hDevice, ioctl_disk_customize_serial, pid);

	CloseHandle(hDevice); // 确保在结束前关闭设备句柄

	return ison;
}
