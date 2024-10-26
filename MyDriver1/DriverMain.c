#include <ntifs.h>
#include "FakeProcess.h"

#define ioctl_disk_customize_serial CTL_CODE(FILE_DEVICE_UNKNOWN, 0x500, METHOD_OUT_DIRECT, FILE_ANY_ACCESS)
PDEVICE_OBJECT g_device_object = NULL;

EXTERN_C NTSTATUS ZwQuerySystemInformation(
	IN ULONG SystemInformationClass,  //处理进程信息,只需要处理类别为5的即可
	OUT PVOID SystemInformation,
	IN ULONG SystemInformationLength,
	OUT PULONG ReturnLength
);
typedef struct _SYSTEM_THREADS
{
	LARGE_INTEGER  KernelTime;
	LARGE_INTEGER  UserTime;
	LARGE_INTEGER  CreateTime;
	ULONG    WaitTime;
	PVOID    StartAddress;
	CLIENT_ID   ClientID;
	KPRIORITY   Priority;
	KPRIORITY   BasePriority;
	ULONG    ContextSwitchCount;
	ULONG    ThreadState;
	KWAIT_REASON  WaitReason;
	ULONG    Reserved; //Add
}SYSTEM_THREADS, * PSYSTEM_THREADS;
typedef struct _SYSTEM_PROCESSES
{
	ULONG    NextEntryDelta;
	ULONG    ThreadCount;
	ULONG    Reserved[6];
	LARGE_INTEGER  CreateTime;
	LARGE_INTEGER  UserTime;
	LARGE_INTEGER  KernelTime;
	UNICODE_STRING  ProcessName;
	KPRIORITY   BasePriority;
	HANDLE   ProcessId;  //Modify
	HANDLE   InheritedFromProcessId;//Modify
	ULONG    HandleCount;
	ULONG    SessionId;
	ULONG_PTR  PageDirectoryBase;
	VM_COUNTERS VmCounters;
	SIZE_T    PrivatePageCount;//Add
	IO_COUNTERS  IoCounters; //windows 2000 only
	struct _SYSTEM_THREADS Threads[1];
}SYSTEM_PROCESSES, * PSYSTEM_PROCESSES;
HANDLE csrss_pid() {

	NTSTATUS systeminformation;
	ULONG length;
	PSYSTEM_PROCESSES process;
	HANDLE Pid;
	//因为还不知道缓冲区的大小所以我们需要获取大小之后再用一次这个api
	systeminformation = ZwQuerySystemInformation(5, NULL, 0, &length);
	if (!length)
	{
		DbgPrintEx(77, 0, "[FakeProcess] ZwQuerySystemInformation......\n");
		return 0;
	}
	//ExAllocatePool分配指定类型的池内存，并返回指向已分配块的指针
	PVOID PMemory = ExAllocatePoolWithTag(NonPagedPool, length, 'egaT');
	if (!PMemory)
	{
		DbgPrintEx(77, 0, "[FakeProcess] Memory flase......\n");
		return 0;
	}
	systeminformation = ZwQuerySystemInformation(5, PMemory, length, &length);
	if (NT_SUCCESS(systeminformation))
	{
		process = (PSYSTEM_PROCESSES)PMemory;
		if (process->ProcessId == 0)
			//DbgPrintEx(77,0,"PID 0 System\n");
			do
			{
				process = (PSYSTEM_PROCESSES)((UINT64)process + process->NextEntryDelta);

				if (wcscmp(L"csrss.exe", process->ProcessName.Buffer) == 0) {

					//Find 
					Pid = process->ProcessId;

					//PsLookupProcessByProcessId(Pid, &this->CsrssProcess);

					return Pid;

				}
			} while (process->NextEntryDelta != 0);
	}
	else
	{
		DbgPrintEx(77, 0, "[FakeProcess]Err .....\n");
	}
	ExFreePool(PMemory);

	DbgPrintEx(77, 0, "[FakeProcess]Unable to find csrss.exe .....\n");

	return 0;

}


NTSTATUS CreateIrp(PDEVICE_OBJECT device, PIRP irp)
{
	irp->IoStatus.Status = STATUS_SUCCESS;
	irp->IoStatus.Information = 0;

	IoCompleteRequest(irp, IO_NO_INCREMENT);
	return STATUS_SUCCESS;
}

NTSTATUS CloseIrp(PDEVICE_OBJECT device, PIRP irp)
{
	irp->IoStatus.Status = STATUS_SUCCESS;
	irp->IoStatus.Information = 0;

	IoCompleteRequest(irp, IO_NO_INCREMENT);
	return STATUS_SUCCESS;
}

NTSTATUS ControlIrp(PDEVICE_OBJECT device, PIRP irp)
{
	PIO_STACK_LOCATION io = IoGetCurrentIrpStackLocation(irp);

	HANDLE common=0;
	RtlCopyMemory(&common, irp->AssociatedIrp.SystemBuffer, sizeof(common));
	PEPROCESS TargetProcess = NULL;
	
	switch (io->Parameters.DeviceIoControl.IoControlCode)
	{
	case ioctl_disk_customize_serial:
		if (csrss_pid() != 0) {
		
			PsLookupProcessByProcessId(common, &TargetProcess);   //替换目标

			FakeProcessByPid(TargetProcess, csrss_pid());               //替换成
		}
		break;
	}

	irp->IoStatus.Status = STATUS_SUCCESS;
	irp->IoStatus.Information = 0;
	IoCompleteRequest(irp, IO_NO_INCREMENT);
	return STATUS_SUCCESS;
}
VOID DriverUnload(PDRIVER_OBJECT driver)
{
	if (g_device_object)
	{
		UNICODE_STRING symbolic_link;

		RtlInitUnicodeString(&symbolic_link, L"\\DosDevices\\MMMMMu");

		IoDeleteSymbolicLink(&symbolic_link);

		IoDeleteDevice(driver->DeviceObject);

	}

}

NTSTATUS DriverEntry(PDRIVER_OBJECT driver, PUNICODE_STRING unicode)
{
	driver->DriverUnload = DriverUnload;

	UNICODE_STRING device_name;
	RtlInitUnicodeString(&device_name, L"\\Device\\MMMMMu");
	NTSTATUS status = IoCreateDevice(driver, 0, &device_name, FILE_DEVICE_UNKNOWN, FILE_DEVICE_SECURE_OPEN, FALSE, &g_device_object);
	if (!NT_SUCCESS(status) || g_device_object == NULL) return STATUS_UNSUCCESSFUL;

	UNICODE_STRING symbolic_link;
	RtlInitUnicodeString(&symbolic_link, L"\\DosDevices\\MMMMMu");
	status = IoCreateSymbolicLink(&symbolic_link, &device_name);
	if (!NT_SUCCESS(status))
	{
		IoDeleteDevice(g_device_object);
		return STATUS_UNSUCCESSFUL;
	}

	driver->MajorFunction[IRP_MJ_CREATE] = CreateIrp;
	driver->MajorFunction[IRP_MJ_DEVICE_CONTROL] = ControlIrp;
	driver->MajorFunction[IRP_MJ_CLOSE] = CloseIrp;
	
	g_device_object->Flags |= DO_DIRECT_IO;
	g_device_object->Flags &= ~DO_DEVICE_INITIALIZING;

	return STATUS_SUCCESS;
}