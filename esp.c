#include <windows.h>
#include <stdio.h>

#define WINDOWNAME "Counter-Strike: Global Offensive"
#define PLAYERLIST 0x2D62C710
#define HPOFFSET 0x214
#define PLAYERSZ 0x18
/*#define PLAYERSNUMADDR 0x5DE6CCB8*/

const DWORD plr_num_addr = 0x5DE6CCB8;
HANDLE get_process_handle();
BOOL read_four_bytes(PCVOID addr, void* buf);
HANDLE hProcess;

int main(int argc, char** argv)
{
    printf("main fired\n");
	HANDLE h = get_process_handle();
    hProcess = h;
	int buf = 0;
	BOOL r = read_four_bytes((PCVOID)plr_num_addr, &buf);
	CloseHandle(h);
	printf("read value = %d\n", buf);
	printf("main finished\n");
    return 0;
}

HANDLE get_process_handle()
{
	printf("get_process_handle fired\n");
	HANDLE h = 0;
	HWND hWnd = FindWindow(NULL, WINDOWNAME);
	if (hWnd == NULL) {
		printf("FindWindow failed, %08X\n", GetLastError());
		return h;
	}
	printf("hWnd = %08X\n", hWnd);
	DWORD pid;
	GetWindowThreadProcessId(hWnd, &pid);
	h = OpenProcess(PROCESS_VM_READ, 0, pid);
	if (h == NULL) {
		printf("OpenProcess failed, %08X\n", GetLastError());
		return h;
	}
	printf("process handle = %08X\n", h);
	printf("get_process_handle finished\n");
	return h;
}

BOOL read_four_bytes(PCVOID addr, void* buf)
{
	printf("read_four_bytes fired\n");
	printf("addr = %08X\n", addr);
	SIZE_T sz = 0;
    BOOL r = ReadProcessMemory(hProcess, addr, buf, 4, &sz);
	if (r == FALSE || sz == 0) {
        printf("RPM error, %08X\n", GetLastError());
		return FALSE;
	}
	return TRUE;
	printf("read_four bytes finished\n");
}
