
NTKERNELAPI PCHAR PsGetProcessImageFileName(PEPROCESS Process);

char* blocked_process[] = { "calc.exe", "notepad.exe" };


VOID MyCreateProcessNotifyEx
(
__inout   PEPROCESS Process,
__in      HANDLE ProcessId,
__in_opt  PPS_CREATE_NOTIFY_INFO CreateInfo
)
{
	int i;

	if (CreateInfo != NULL)	
	{
		DbgPrint("[block_execute_64][Parent: %ld]: %wZ",
			CreateInfo->ParentProcessId,
			CreateInfo->ImageFileName);

		for (i = 0; i < (sizeof(blocked_process) / sizeof(char*)); i++)
		{
			if (!_stricmp(blocked_process[i], PsGetProcessImageFileName(Process)))
			{
				// blocked
				CreateInfo->CreationStatus = STATUS_UNSUCCESSFUL;
			}
		}
	}
}