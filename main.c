#pragma once

#pragma comment(lib, "Shlwapi.lib")

#include <Windows.h>
#include <shlwapi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DS_STREAM_RENAME L":wtfbbq"
#define DS_DEBUG_LOG(msg) wprintf(L"[LOG] - %s\n", msg)

static
HANDLE
ds_open_handle(
	PWCHAR pwPath
)
{
	return CreateFileW(pwPath, DELETE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
}

static
BOOL
ds_rename_handle(
	HANDLE hHandle
)
{
	FILE_RENAME_INFO* pfRename = (FILE_RENAME_INFO*)malloc(1000);
	if (pfRename == NULL) {
		return FALSE;
	}
	RtlSecureZeroMemory(pfRename, sizeof(*pfRename));

	// set our FileNameLength and FileName to DS_STREAM_RENAME
	LPWSTR lpwStream = DS_STREAM_RENAME;
	pfRename->FileNameLength = 7 * sizeof(WCHAR);
	RtlCopyMemory(pfRename->FileName, lpwStream, 7 * sizeof(WCHAR));
	return SetFileInformationByHandle(hHandle, FileRenameInfo, pfRename, sizeof(*pfRename) + pfRename->FileNameLength);
}

static
BOOL
ds_deposite_handle(
	HANDLE hHandle
)
{
	// set FILE_DISPOSITION_INFO::DeleteFile to TRUE
	FILE_DISPOSITION_INFO fDelete;
	RtlSecureZeroMemory(&fDelete, sizeof(fDelete));

	fDelete.DeleteFile = TRUE;

	return SetFileInformationByHandle(hHandle, FileDispositionInfo, &fDelete, sizeof(fDelete));
}

int
main(
	int argc,
	char** argv
)
{
	WCHAR wcPath[MAX_PATH + 1];
	RtlSecureZeroMemory(wcPath, sizeof(wcPath));

	// get the path to the current running process ctx
	if (GetModuleFileNameW(NULL, wcPath, MAX_PATH) == 0)
	{
		DS_DEBUG_LOG(L"failed to get the current module handle");
		return 0;
	}

	HANDLE hCurrent = ds_open_handle(wcPath);
	if (hCurrent == INVALID_HANDLE_VALUE)
	{
		DS_DEBUG_LOG(L"failed to acquire handle to current running process");
		return 0;
	}

	// rename the associated HANDLE's file name
	DS_DEBUG_LOG(L"attempting to rename file name");
	if (!ds_rename_handle(hCurrent))
	{
		DS_DEBUG_LOG(L"failed to rename to stream");
		return 0;
	}

	DS_DEBUG_LOG(L"successfully renamed file primary :$DATA ADS to specified stream, closing initial handle");
	CloseHandle(hCurrent);

	// open another handle, trigger deletion on close
	hCurrent = ds_open_handle(wcPath);
	if (hCurrent == INVALID_HANDLE_VALUE)
	{
		DS_DEBUG_LOG(L"failed to reopen current module");
		return 0;
	}

	if (!ds_deposite_handle(hCurrent))
	{
		DS_DEBUG_LOG(L"failed to set delete deposition");
		return 0;
	}

	// trigger the deletion deposition on hCurrent
	DS_DEBUG_LOG(L"closing handle to trigger deletion deposition");
	CloseHandle(hCurrent);

	// verify we've been deleted
	if (PathFileExistsW(wcPath))
	{
		DS_DEBUG_LOG(L"failed to delete copy, file still exists");
		return 0;
	}

	DS_DEBUG_LOG(L"successfully deleted self from disk");
}