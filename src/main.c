#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// --- Lispオブジェクトの型 ---
typedef enum {
	LISP_SYMBOL,
	LISP_LIST
} lisp_type;

// --- Lispオブジェクト ---
typedef struct lisp_obj {
	lisp_type type;
	union {
		char* symbol;
		struct {
			struct lisp_obj* car;
			struct lisp_obj* cdr;
		} list;
	} val;
} lisp_obj;

// シンボルのコンストラクタ
lisp_obj* create_symbol(char* name) {
	lisp_obj* obj = malloc(sizeof(lisp_obj));
	obj->type = LISP_SYMBOL;
	obj->val.symbol = strdup(name);
	return obj;
}

// --- PRINT ---
void print_obj(lisp_obj* obj) {
	// NIL
	if (obj == NULL) {
		printf("nil");
		return;
	}

	// シンボル
	if (obj->type == LISP_SYMBOL) {
		printf("%s", obj->val.symbol);
	}
}

int main(void) {
	char input[256];
	printf("Welcome to Pure Lisp (Phase 1) \n");

	while (1) {
		printf("> ");
		if (fgets(input, sizeof(input), stdin) == NULL) break;

		// 改行を消す
		input[strcspn(input, "\n")] = 0;

		if (strcmp(input, "(exit)") == 0 ||
			strcmp(input, "(bye)") == 0) break;
		if (strlen(input) == 0) continue;

		lisp_obj* tmp_obj = create_symbol(input);

		print_obj(tmp_obj);
		putchar('\n');
	}

	puts("Bye bye!");

	return 0;
}
