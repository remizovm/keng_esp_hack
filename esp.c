#include <windows.h>
#include <stdio.h>
#include <psapi.h>
#include <string.h>
#include <time.h>
#include <math.h>

#define WINDOWNAME "Counter-Strike: Global Offensive"
#define PLRSZ 0x18
#define SERVERDLL "server.dll"
#define CLIENTDLL "client.dll"

const DWORD plr_num_offset = 0x93DCF8;
const DWORD plr_list_offset = 0x89A48C;
const DWORD hp_offset = 0x214;
const DWORD coords_offset = 0x1D0;
const DWORD v_matrix_offset = 0x4A087D4;
DWORD server_dll_base;
DWORD client_dll_base;
HANDLE hProcess;
float view_matrix[4][4];
int world_to_screen(float* from, float* to);
RECT rect;
HWND hWnd;
float my_coords[3];
HDC hDC;

void get_process_handle();
int read_bytes(PCVOID addr, int num, void* buf);
void esp();
void read_my_coords();
void get_view_matrix();
void draw_health(float x, float y, int health);

int main(int argc, char** argv)
{
	get_process_handle();
	esp();
	CloseHandle(hProcess);
    return 0;
}

void read_my_coords(DWORD addr)
{
	DWORD plr_addr;
	read_bytes((PCVOID)(addr), 4, &plr_addr);
	read_bytes((PCVOID)(plr_addr + coords_offset), 12, &my_coords);
}

void esp()
{
	int players_on_map, i, hp;
	float coords[3];
	DWORD plr_addr;
	DWORD addr = server_dll_base + plr_list_offset;
	read_bytes((PCVOID)addr, 4, &plr_addr);
	SetTextAlign(hDC, TA_CENTER|TA_NOUPDATECP);
	SetBkColor(hDC, RGB(0,0,0));
	SetBkMode(hDC, TRANSPARENT);
	SetTextColor(hDC, RGB(0, 255, 0));
	for (;;Sleep(1)) {
		system("cls");
		GetWindowRect(hWnd, &rect);
		read_bytes((PCVOID)(server_dll_base + plr_num_offset), 4,
				&players_on_map);
		printf("players on the map: %d\n", players_on_map);
		if (players_on_map == 0) continue;
		printf("players on the screeen:\n");
		read_my_coords(addr);
		for (i = 1; i < players_on_map; i++) {
			read_bytes((PCVOID)(addr + i*PLRSZ), 4, &plr_addr);
			read_bytes((PCVOID)(plr_addr + hp_offset), 4, &hp);
			if (hp == 0) continue;
			read_bytes((PCVOID)(plr_addr + coords_offset), 12, &coords);
			get_view_matrix();
			float tempCoords[3];
			if (world_to_screen(coords, tempCoords) == 1) {
				printf("player %d health: %d (%g:%g:%g)\n", i, hp,
					coords[0], coords[1], coords[2]);
				draw_health(tempCoords[0] - rect.left,
						    tempCoords[1] - rect.top-20, hp);
			}
		}
	}
}

void draw_health(float x, float y, int health)
{
	char buf[sizeof(int)*3+2];
	snprintf(buf, sizeof buf, "%d", health);
	TextOutA(hDC, x, y, buf, strlen(buf));
}

void get_view_matrix()
{
	read_bytes((PCVOID)(client_dll_base+v_matrix_offset), 64, 
		&view_matrix);
}

int world_to_screen(float* from, float* to)
{
	float w = 0.0f;
	to[0] = view_matrix[0][0] * from[0] + view_matrix[0][1] * from[1] +	view_matrix[0][2] * from[2] + view_matrix[0][3];
	to[1] = view_matrix[1][0] * from[0] + view_matrix[1][1] * from[1] + view_matrix[1][2] * from[2] + view_matrix[1][3];
	w     = view_matrix[3][0] * from[0] + view_matrix[3][1] * from[1] + view_matrix[3][2] * from[2] + view_matrix[3][3];
	if (w < 0.01f) 
		return 0;
	float invw = 1.0f / w;
	to[0] *= invw;
	to[1] *= invw;
	int width = (int)(rect.right - rect.left);
	int height = (int)(rect.bottom - rect.top);
	float x = width/2;
	float y = height/2;
	x += 0.5 * to[0] * width + 0.5;
    y -= 0.5 * to[1] * height + 0.5;
	to[0] = x + rect.left;
	to[1] = y + rect.top;
	return 1;
}

void get_process_handle()
{
	DWORD pid = 0;
	hWnd = FindWindow(0, WINDOWNAME);
	if (hWnd == 0) {
		printf("FindWindow failed, %08X\n", GetLastError());
		return;
	}
	GetWindowThreadProcessId(hWnd, &pid);
	hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, 0, pid);
	if (hProcess == 0) {
		printf("OpenProcess failed, %08X\n", GetLastError());
		return;
	}
	hDC = GetDC(hWnd);
	HMODULE hMods[1024];
	int i;
	if (EnumProcessModules(hProcess, hMods, sizeof(hMods), &pid) == 0) {
		printf("enumprocessmodules failed, %08X\n", GetLastError());
	} else {
	    for (i = 0; i < (pid / sizeof(HMODULE)); i++) {
		    TCHAR szModName[MAX_PATH];
		    if (GetModuleFileNameEx(hProcess, hMods[i], szModName,
					sizeof(szModName)/sizeof(TCHAR))) { 
				if (strstr(szModName, SERVERDLL) != 0) {
					printf("server.dll base: %08X\n", hMods[i]);
					server_dll_base = (DWORD)hMods[i];
				}
				if (strstr(szModName, CLIENTDLL) != 0) {
					printf("client.dll base: %08X\n", hMods[i]);
					client_dll_base = (DWORD)hMods[i];
				}
			}
		}
	}
}

int read_bytes(PCVOID addr, int num, void* buf)
{
	SIZE_T sz = 0;
    int r = ReadProcessMemory(hProcess, addr, buf, num, &sz);
	if (r == 0 || sz == 0) {
        printf("RPM error, %08X\n", GetLastError());
		return 0;
	}
	return 1;
}
