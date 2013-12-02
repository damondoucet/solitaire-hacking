#pragma once

// It's very important that the following code be compiled and targeted
// to a 64-bit machine so that the pointers are all 8 bytes long just like
// in solitaire.

#include <Windows.h>
#include <stdlib.h>

#define NUM_PILES 13
#define MAX_CARDS 52

typedef struct _STR {
	// unicode string
	wchar_t data[1024];
} STR;

typedef struct _CARD {
	BYTE unused[0x38];
	STR *lpCardName;
} CARD;

typedef struct _CARD_LIST {
	CARD *lpCards[MAX_CARDS];
} CARD_LIST;

typedef struct _PILE {
	BYTE unused[0x130];
	DWORD NumCards;
	BYTE unused2[0x0c];
	CARD_LIST *lpCardList;
} PILE;

typedef struct _PILE_LIST {
	PILE *lpPiles[NUM_PILES];
} PILE_LIST;

PILE_LIST *ReadPileList(HANDLE hProc, LPCVOID lpModuleBase);

// Free functions

static inline void FreeCard(CARD *lpCard) {
	free(lpCard->lpCardName);
	free(lpCard);
}

static inline void FreePile(PILE *lpPile) {
	for (unsigned i = 0; i < lpPile->NumCards; i++)
		FreeCard(lpPile->lpCardList->lpCards[i]);

	free(lpPile->lpCardList);
	free(lpPile);
}

static inline void FreePileList(PILE_LIST *lpPileList) {
	for (int i = 0; i < NUM_PILES; i++)
		FreePile(lpPileList->lpPiles[i]);

	free(lpPileList);
}