#include <ntifs.h>
#include <wdm.h>
#include <ntimage.h>
#include <windef.h>
#include <ioctls.h>
#include <process_block.h>


#define DEVICE_NAME L"\\Device\\simpledev"
#define DOS_DEVICE_NAME L"\\DosDevices\\simpledev"

DRIVER_INITIALIZE DriverEntry;
DRIVER_UNLOAD DrvUnload;
__drv_dispatchType(IRP_MJ_CREATE)
__drv_dispatchType(IRP_MJ_CLOSE)
__drv_dispatchType(IRP_MJ_DEVICE_CONTROL)
DRIVER_DISPATCH DrvDispatch;
  
#pragma alloc_text(INIT, DriverEntry)
#pragma alloc_text(PAGE, DrvUnload)
#pragma alloc_text(PAGE, DrvDispatch)

void DebugInfo(char *str) {
    DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "simpledev: %s\n", str);
}
 
NTSTATUS DrvDispatch(PDEVICE_OBJECT DeviceObject, PIRP Irp) {
    PIO_STACK_LOCATION iostack;
    NTSTATUS status = STATUS_NOT_SUPPORTED;
    PVOID buf;
    ULONG len;
    PEPROCESS currentProcess;
    PAGED_CODE();
    
    currentProcess = PsGetCurrentProcess();
    iostack = IoGetCurrentIrpStackLocation(Irp);
 
    switch(iostack->MajorFunction) {
        case IRP_MJ_CREATE:
            status = STATUS_SUCCESS;
            break;
        case IRP_MJ_CLOSE:
            status = STATUS_SUCCESS;
            break;
        case IRP_MJ_DEVICE_CONTROL:{
            buf = Irp->AssociatedIrp.SystemBuffer;
            len = iostack->Parameters.DeviceIoControl.InputBufferLength;
            // check io ctl
            status = STATUS_SUCCESS;
            break;
        }
        default:
            status = STATUS_INVALID_DEVICE_REQUEST;
    }
 
    Irp->IoStatus.Status = status;
    Irp->IoStatus.Information = 0;
 
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
 
    return status;
}
 
VOID DrvUnload(PDRIVER_OBJECT DriverObject)
{
    UNICODE_STRING dosdev;
     
    PAGED_CODE();
 
    DebugInfo("driver unloading");
 
    RtlInitUnicodeString(&dosdev, DOS_DEVICE_NAME);
    IoDeleteSymbolicLink(&dosdev);
 
	PsSetCreateProcessNotifyRoutineEx((PCREATE_PROCESS_NOTIFY_ROUTINE_EX)MyCreateProcessNotifyEx, TRUE);

    IoDeleteDevice(DriverObject->DeviceObject);
}
 
NTSTATUS DriverEntry(
    __in PDRIVER_OBJECT DriverObject,
    __in PUNICODE_STRING RegistryPath
    )
{
    UNICODE_STRING devname, dosname;
    PDEVICE_OBJECT devobj;
    NTSTATUS status;
    
    DebugInfo("driver initializing");
  
    DriverObject->MajorFunction[IRP_MJ_CREATE] = DrvDispatch;
    DriverObject->MajorFunction[IRP_MJ_CLOSE] = DrvDispatch;
    DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = DrvDispatch;
    DriverObject->DriverUnload = DrvUnload;
 
    RtlInitUnicodeString(&devname, DEVICE_NAME);
    RtlInitUnicodeString(&dosname, DOS_DEVICE_NAME);
 
    status = IoCreateDevice(DriverObject, 0, &devname, FILE_DEVICE_UNKNOWN, 
        FILE_DEVICE_SECURE_OPEN, FALSE, &devobj);
 
    if (!NT_SUCCESS(status)) {
        DebugInfo("error creating device");
        return status;
    }
 
    status = IoCreateSymbolicLink(&dosname, &devname);
     
    if (!NT_SUCCESS(status)) {
        DebugInfo("error creating symbolic link");
        return status;
    }

	status = PsSetCreateProcessNotifyRoutineEx((PCREATE_PROCESS_NOTIFY_ROUTINE_EX)MyCreateProcessNotifyEx, FALSE);
	DbgPrint("PsSetCreateProcessNotifyRoutineEx: %x", status);
         
    return STATUS_SUCCESS;
}