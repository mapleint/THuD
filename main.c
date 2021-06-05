#include <Windows.h>
#include <stdio.h>
#include <time.h>
#include <Psapi.h>
#include "map.h"
#include <stdlib.h>

int SHORUN = 1;

map_int_t m;


void cleanup() {
	const char* key;
	map_iter_t iter = map_iter(&m);
	puts("\n\n\n-------------------------------\n\n\n");

	FILE* fp;
	fopen_s(&fp, "log.txt", "w");
	while ((key = map_next(&m, &iter))) {
		int value = *map_get(&m, key);
		if (fp) {
			fprintf(fp, "%s|%i\n", key, value);
		}
		char* Z = key + strlen(key);
		while (*(Z - 1) != '\\')
			Z--;
		printf("You have used %s for %i seconds\n", Z, value);
	}
	if (fp)
		fclose(fp);
	map_deinit(&m);
}

BOOL WINAPI ctrl(DWORD fdwCtrlType)
{

	switch (fdwCtrlType) {
		case CTRL_C_EVENT:
		case CTRL_CLOSE_EVENT:
		case CTRL_BREAK_EVENT:
			SHORUN = 0;
			puts("[INFO] : CTRL+[KEY] received, exiting");
			cleanup();
			return TRUE;
		case CTRL_LOGOFF_EVENT:
		case CTRL_SHUTDOWN_EVENT:
			SHORUN = 0;
			puts("exiting");
			cleanup();
			return FALSE;
	}
}
int main() 
{

	if (SetConsoleCtrlHandler(ctrl, TRUE)){
		puts("[OK] : successfuly set CTRL handler\n[INFO] : use CTRL+PAUSEBREAK to exit and save");
		map_init(&m);

		puts("[INFO] : opening log...\n");
		char nk[300];
		FILE* f;
		fopen_s(&f, "log.txt", "r");
		if (f) {
			puts("[OK] : successfully opened log\n");
			while (fgets(nk, 300, f) != NULL) {
				char key[260];
				int x = 0;
				while (nk[x] != '|')
					x++;
				strncpy_s(key, sizeof(key), &nk[0], x);
				x++;
				int val = strtol(&nk[x], NULL, 10);
				map_set(&m, key, val);
				printf("[INFO] LOADED : %s : %i\n", key, val);
			}
			fclose(f);
		}
		else {
			puts("[ERROR] : failed to open log\n");
		}
		DWORD ppid = 0;
		DWORD s_focus = 0;
		while (SHORUN) {
			DWORD pid = 0;
			char exename[MAX_PATH];
			Sleep(1000);
			HWND hwnd = GetForegroundWindow();
			if (hwnd == NULL) {
				continue;
			}
			GetWindowThreadProcessId(hwnd, &pid);
			HANDLE handle = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);
			GetModuleFileNameExA(handle, 0, exename, MAX_PATH);
			CloseHandle(handle);

			time_t current_time;
			struct tm time_info;
			char timeString[9];

			time(&current_time);
			localtime_s(&time_info, &current_time);

			strftime(timeString, sizeof(timeString), "%H:%M:%S", (const struct tm*)&time_info);

			if (ppid != pid)
				printf("[%s] PID: %i, %s\n", timeString, pid, exename);
			int* val = map_get(&m, exename);
			if (val)
				map_set(&m, exename, *val + 1);
			else
				map_set(&m, exename, 1);
			ppid = pid;
		}

		const char* key;
		map_iter_t iter = map_iter(&m);
		puts("\n\n\n-------------------------------\n\n\n");

		cleanup();
	} else {
		puts("[ERROR] : could not set ctrl handler");
	}
}