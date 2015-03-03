#include <windows.h>
#include <stdio.h>
#include <psapi.h>
#include <string.h>
#include <time.h>

#define WINDOWNAME "Counter-Strike: Global Offensive"
#define PLRSZ 0x18
#define SERVERDLL "server.dll"

const DWORD plr_num_addr = 0x5DE6CCB8;
const DWORD plr_num_offset = 0x93CCB8;
const DWORD plr_list_offset = 0x0089944C;
const DWORD hp_offset = 0x214;
const DWORD coords_offset = 0x1D0;
DWORD server_dll_base;
HANDLE get_process_handle();
BOOL read_bytes(PCVOID addr, int num, void* buf);
HANDLE hProcess;

int main(int argc, char** argv)
{
	HANDLE h = get_process_handle();
    hProcess = h;
	DWORD addr = server_dll_base + plr_list_offset;
	printf("player list offset: %08X\n", addr);
	int players_on_map, i, hp;
	float coords[3];
	DWORD plr_addr;
	read_bytes((PCVOID)addr, 4, &plr_addr);
	printf("players list addr: %08X\n", plr_addr);
	for (;;Sleep(500)) {
		system("cls");
		printf("tick\n");
		read_bytes((PCVOID)(server_dll_base + plr_num_offset), 4, 
				&players_on_map);
		printf("players on the map: %d\n", players_on_map);
		for (i = 0; i < players_on_map; i++) {
			read_bytes((PCVOID)(addr+i*PLRSZ), 4, &plr_addr);
			read_bytes((PCVOID)(plr_addr + hp_offset), 4, &hp);
			read_bytes((PCVOID)(plr_addr + coords_offset), 12, &coords);
			printf("player %d health: %d (%g:%g:%g)\n", i, hp,
					coords[0], coords[1], coords[2]);
		}
	}
	CloseHandle(h);
    return 0;
}

HANDLE get_process_handle()
{
	HANDLE h = 0;
	DWORD pid = 0;
	HWND hWnd = FindWindow(NULL, WINDOWNAME);
	if (hWnd == NULL) {
		printf("FindWindow failed, %08X\n", GetLastError());
		return h;
	}
	printf("hWnd = %08X\n", hWnd);
	GetWindowThreadProcessId(hWnd, &pid);
	h = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, 0, pid);
	if (h == NULL) {
		printf("OpenProcess failed, %08X\n", GetLastError());
		return h;
	}
	printf("process handle = %08X\n", h);
	HMODULE hMods[1024];
	int i;
	if (EnumProcessModules(h, hMods, sizeof(hMods), &pid) == FALSE) {
		printf("enumprocessmodules failed, %08X\n", GetLastError());
	} else {
	    for (i = 0; i < (pid / sizeof(HMODULE)); i++) {
		    TCHAR szModName[MAX_PATH];
		    if (GetModuleFileNameEx(h, hMods[i], szModName,
					sizeof(szModName)/sizeof(TCHAR))) { 
				if (strstr(szModName, SERVERDLL) != NULL) {
					printf("server.dll base: %08X\n", hMods[i]);
					server_dll_base = (DWORD)hMods[i];
				}
			}
		}
	}	
	return h;
}

BOOL read_bytes(PCVOID addr, int num, void* buf)
{
	SIZE_T sz = 0;
    BOOL r = ReadProcessMemory(hProcess, addr, buf, num, &sz);
	if (r == FALSE || sz == 0) {
        printf("RPM error, %08X\n", GetLastError());
		return FALSE;
	}
	return TRUE;
}
