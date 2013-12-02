#include "Solitaire.h"
#include "ApiHelpers.h"

#include <stdio.h>
#include <tchar.h>

// static = *(solitaire.exe + BAFA8)
// static + 0x80 -> pointer
// pointer + 0x98 -> pile list
#define STATIC_POINTER_OFFSET 0xBAFA8
#define OFFSET 0x80
#define PILE_LIST_OFFSET 0x98

#define ADD_OFFSET(lpc, qwOffset) ((LPCVOID) ((QWORD) lpc + qwOffset))

#define TRY(call, errorVal, tszError) {if (!call) {printf(tszError ": %d\n", GetLastError()); return errorVal; }}

LPCVOID lpReadPileListPointer(HANDLE hProc, LPCVOID lpModuleBase);
PILE_LIST *lpReadPileList(HANDLE hProc, LPCVOID lpPileListAddress);
PILE *lpReadPile(HANDLE hProc, LPCVOID lpPileAddress);
CARD *lpReadCard(HANDLE hProc, LPCVOID lpCardAddress);

PILE_LIST *ReadPileList(HANDLE hProc, LPCVOID lpModuleBase) {
	LPCVOID lpPileListPointer = lpReadPileListPointer(hProc, lpModuleBase);

	if (lpPileListPointer == (LPCVOID)-1)
		return NULL;

	printf("Pile List: %llx\n", (QWORD)lpPileListPointer);

	return lpReadPileList(hProc, lpPileListPointer);
}

LPCVOID lpReadPileListPointer(HANDLE hProc, LPCVOID lpModuleBase) {
	QWORD qwStaticPointerValue;

	TRY(bReadQword(hProc, ADD_OFFSET(lpModuleBase, STATIC_POINTER_OFFSET), &qwStaticPointerValue),
		(LPCVOID)-1, "Error reading static pointer");

	QWORD qwNextPointer;

	TRY(bReadQword(hProc, ADD_OFFSET(qwStaticPointerValue, OFFSET), &qwNextPointer),
		(LPCVOID)-1, "Error reading intermediate pointer");

	QWORD qwPileListPointer;

	TRY(bReadQword(hProc, ADD_OFFSET(qwNextPointer, PILE_LIST_OFFSET), &qwPileListPointer),
		(LPCVOID)-1, "Error reading pile list pointer");

	return (LPCVOID)qwPileListPointer;
}

PILE_LIST *lpReadPileList(HANDLE hProc, LPCVOID lpPileListAddress) {
	PILE_LIST *lpPileList = (PILE_LIST *) malloc(sizeof(PILE_LIST));

	if (!bReadObject(hProc, lpPileListAddress, sizeof(PILE_LIST), lpPileList)) {
		free(lpPileList);
		printf("Error reading pile list: %d\n", GetLastError());
		return NULL;
	}

	for (int i = 0; i < NUM_PILES; i++) {
		lpPileList->lpPiles[i] = lpReadPile(hProc, lpPileList->lpPiles[i]);

		if (lpPileList->lpPiles[i] == NULL) {
			// error, abort
			for (int j = 0; j < i; j++)
				FreePile(lpPileList->lpPiles[j]);
			free(lpPileList);

			printf("Error reading pile %d: %d\n", i, GetLastError());
			return NULL;
		}
	}

	return lpPileList;
}

PILE *lpReadPile(HANDLE hProc, LPCVOID lpPileAddress) {
	PILE *lpPile = (PILE *) malloc(sizeof(PILE));

	if (!bReadObject(hProc, lpPileAddress, sizeof(PILE), lpPile)) {
		printf("Error reading pile object\n");
		free(lpPile);
		return NULL;
	}

	if (lpPile->lpCardList == NULL) {
		// no cards
		return lpPile;
	}

	CARD_LIST *lpCardList = (CARD_LIST *) malloc(sizeof(CARD_LIST));

	if (!bReadObject(hProc, lpPile->lpCardList, sizeof(CARD_LIST), lpCardList)) {
		printf("Error reading card list\n");
		free(lpPile);
		free(lpCardList);
		return NULL;
	}

	lpPile->lpCardList = lpCardList;

	for (unsigned i = 0; i < lpPile->NumCards; i++) {
		lpCardList->lpCards[i] = lpReadCard(hProc, lpCardList->lpCards[i]);

		if (lpCardList->lpCards[i] == NULL) {
			// error, abort
			for (unsigned j = 0; j < i; j++)
				FreeCard(lpCardList->lpCards[j]);
			free(lpCardList);
			free(lpPile);

			printf("Error reading card %d\n", i);
			return NULL;
		}
	}

	return lpPile;
}

CARD *lpReadCard(HANDLE hProc, LPCVOID lpCardAddress) {
	CARD *lpCard = (CARD *) malloc(sizeof(CARD));

	if (!bReadObject(hProc, lpCardAddress, sizeof(CARD), lpCard)) {
		printf("Error reading card\n");
		free(lpCard);
		return NULL;
	}

	STR *lpStr = (STR *) malloc(sizeof(STR));

	if (!bReadObject(hProc, lpCard->lpCardName, sizeof(STR), lpStr)) {
		printf("Error reading card name\n");
		free(lpStr);
		free(lpCard);
		return NULL;
	}

	lpCard->lpCardName = lpStr;
	return lpCard;
}