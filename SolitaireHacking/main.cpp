#include <stdio.h>
#include <tchar.h>
#include <Windows.h>

#include "ApiHelpers.h"
#include "Solitaire.h"

#define BUF_SIZE 1024
#define PROCESS_NAME "solitaire.exe"

DWORD TryReadProcessId();
HANDLE TryOpenProcess(DWORD dwProcessId);
LPCVOID TryLocateModuleBase(DWORD dwProcessId);
void PrintPileList(PILE_LIST *lpPileList);

void ClearScreen() {
	for (int i = 0; i < 100; i++)
		printf("\n");
}

int main() {
	TCHAR buf[BUF_SIZE];

	DWORD dwProcessId = TryReadProcessId();
	HANDLE hProc = TryOpenProcess(dwProcessId);
	LPCVOID lpModuleBaseAddr = TryLocateModuleBase(dwProcessId);

	do {
		PILE_LIST *lpPileList = ReadPileList(hProc, lpModuleBaseAddr);

		if (lpPileList == NULL) {
			// error will have been printed
			printf("Press enter to exit\n");
			_fgetts(buf, BUF_SIZE, stdin);
			return 1;
		}

		PrintPileList(lpPileList);

		printf("Current board:\n");
		printf("Type done to exit\n");
		_fgetts(buf, BUF_SIZE, stdin);

		FreePileList(lpPileList);
		ClearScreen();
	} while (_tcsnccmp(buf, "done\n", BUF_SIZE) != 0);
	
	return 0;
}

void PrintPileList(PILE_LIST *lpPileList) {
	for (int i = 0; i < NUM_PILES; i++) {
		printf("Pile %d\n", i);
		printf("\tNum Cards: %d\n", lpPileList->lpPiles[i]->NumCards);

		for (unsigned j = 0; j < lpPileList->lpPiles[i]->NumCards; j++)
			wprintf(L"\t Card %d: %s\n", j, 
				lpPileList->lpPiles[i]->lpCardList->lpCards[j]->lpCardName->data);
	}
}

DWORD TryReadProcessId() {
	DWORD dwProcessId = dwFindProcessId(PROCESS_NAME);

	if (dwProcessId == -1) {
		getchar();
		exit(1);
	}

	return dwProcessId;
}

HANDLE TryOpenProcess(DWORD dwProcessId) {
	HANDLE hProc = hProcOpenProcess(dwProcessId);

	if (hProc == INVALID_HANDLE_VALUE) {
		printf(_T("Unable to open process. Error code: %d\n"), GetLastError());
		getchar();
		exit(1);
	}

	return hProc;
}

LPCVOID TryLocateModuleBase(DWORD dwProcessId) {
	LPCVOID lpModuleBaseAddr = lpLocateModuleBase(dwProcessId, PROCESS_NAME);

	if (lpModuleBaseAddr == NULL) {
		getchar();
		exit(1);
	}

	return lpModuleBaseAddr;
}