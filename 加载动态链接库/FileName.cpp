#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>
#include <stdio.h>
#include <tchar.h>

int ison = 0;

BOOL LoadMyDll(const char* dllName) {
	// 尝试加载 DLL
	HMODULE hDll = LoadLibraryA(dllName);
	if (hDll == NULL) {
		printf("无法加载 DLL: %s. 错误代码: %lu\n", dllName, GetLastError());
		return FALSE;
	}

	printf("成功加载 DLL: %s\n", dllName);
	ison = 1;

	// 可以在这里调用 DLL 中的函数
	return TRUE;
}

int main() {
	// 检查并加载 DLL
	if (ison != 1) {
		if (!LoadMyDll("Dll3.dll")) {
			return 1; // 加载失败
		}
	}

	// 从控制台获取 PID 输入
	UINT pid;
	printf("请输入 PID: ");
	scanf("%u", &pid); // 获取输入的 PID

	// 加载 DLL 并调用 load1 函数
	HMODULE hModule = LoadLibraryA("Dll3.dll");
	if (hModule) {
		typedef BOOL(*LoadFunc)(UINT);
		LoadFunc load = (LoadFunc)GetProcAddress(hModule, "LOADENTRY");
		if (load) {
			BOOL result = load(pid); // 调用导出函数
			if (result) {
				printf("成功调用 load 函数\n");
			}
			else {
				printf("调用 load 函数失败\n");
			}
		}
		else {
			printf("无法获取 load 函数的地址\n");
		}
		FreeLibrary(hModule); // 释放 DLL
	}
	else {
		printf("无法加载 Dll3.dll\n");
	}

	return 0;
}
