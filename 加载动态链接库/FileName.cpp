#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>
#include <stdio.h>
#include <tchar.h>

int ison = 0;

BOOL LoadMyDll(const char* dllName) {
	// ���Լ��� DLL
	HMODULE hDll = LoadLibraryA(dllName);
	if (hDll == NULL) {
		printf("�޷����� DLL: %s. �������: %lu\n", dllName, GetLastError());
		return FALSE;
	}

	printf("�ɹ����� DLL: %s\n", dllName);
	ison = 1;

	// ������������� DLL �еĺ���
	return TRUE;
}

int main() {
	// ��鲢���� DLL
	if (ison != 1) {
		if (!LoadMyDll("Dll3.dll")) {
			return 1; // ����ʧ��
		}
	}

	// �ӿ���̨��ȡ PID ����
	UINT pid;
	printf("������ PID: ");
	scanf("%u", &pid); // ��ȡ����� PID

	// ���� DLL ������ load1 ����
	HMODULE hModule = LoadLibraryA("Dll3.dll");
	if (hModule) {
		typedef BOOL(*LoadFunc)(UINT);
		LoadFunc load = (LoadFunc)GetProcAddress(hModule, "LOADENTRY");
		if (load) {
			BOOL result = load(pid); // ���õ�������
			if (result) {
				printf("�ɹ����� load ����\n");
			}
			else {
				printf("���� load ����ʧ��\n");
			}
		}
		else {
			printf("�޷���ȡ load �����ĵ�ַ\n");
		}
		FreeLibrary(hModule); // �ͷ� DLL
	}
	else {
		printf("�޷����� Dll3.dll\n");
	}

	return 0;
}
