#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

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

// 関数プロトタイプ宣言
lisp_obj* lisp_eval(lisp_obj* obj, lisp_obj** env);
lisp_obj* evlis(lisp_obj* args, lisp_obj** env);

// シンボルのコンストラクタ
lisp_obj* create_symbol(char* name) {
	lisp_obj* obj = malloc(sizeof(lisp_obj));
	obj->type = LISP_SYMBOL;
	obj->val.symbol = strdup(name);
	return obj;
}

// --- helper functions ---
int issym(lisp_obj* obj) {
	return obj == NULL || obj->type == LISP_SYMBOL;
}

int islist(lisp_obj* obj) {
	return obj != NULL && obj->type == LISP_LIST;
}

lisp_obj* lcar(lisp_obj* obj) {
	if (!islist(obj)) return NULL;
	return obj->val.list.car;
}

lisp_obj* lcdr(lisp_obj* obj) {
	if (!islist(obj)) return NULL;
	return obj->val.list.cdr;
}

char* lsym(lisp_obj* obj) {
	if (obj == NULL) return NULL;
	if (!issym(obj)) return NULL;
	return obj->val.symbol;
}

lisp_obj* lcons(lisp_obj* first, lisp_obj* second) {
	lisp_obj* new_cell = malloc(sizeof(lisp_obj));
	new_cell->type = LISP_LIST;
	new_cell->val.list.car = first;
	new_cell->val.list.cdr = second;
	return new_cell;
}

// --- READ ---

void skipsp(char** input) {
	while (**input == ' ') { (*input)++; }
}

lisp_obj* read_symbol(char** input) {
	char buf[256];
	int i = 0;
	while (**input != '\0' && isalnum(**input)) {
		buf[i++] = **input;
		(*input)++;
		if (i >= 255) break;
	}
	buf[i] = '\0';

	return create_symbol(buf);
}

lisp_obj* read_expr(char** input) {
	// スペースを読み飛ばす
	skipsp(input);

	// リストの読み取り
	if (**input == '(') {
		// ポインタを進める
		(*input)++; 

		// リストの先頭
		lisp_obj* head = NULL;
		// リストの末尾
		lisp_obj* tail = NULL;

		while (1) {
			// スペースを飛ばす
			skipsp(input);

			// 閉じカッコが見つかったら終了
			if (**input == ')') {
				(*input)++;

				// 先頭（なにも読んでいなければNULL（空リスト））を返す
				return head;
			}

			// 文字列終端に達したら、閉じカッコ忘れ
			if (**input == '\0') {
				return NULL;
			}

			// 次の要素を再帰的に読み取る
			lisp_obj* obj = read_expr(input);

			// 新しいコンスセルを作って繋ぐ
			lisp_obj* new_cell = lcons(obj, NULL);

			if (head == NULL) {
				head = tail = new_cell;
			} else {
				tail = tail->val.list.cdr = new_cell;
			}
		}
	}

	// シンボルの読み取り
	if (isalnum(**input)) {
		return read_symbol(input);
	}

	return NULL;
}

// --- EVAL ---
// env_lookup: 環境からからシンボルの値を検索する
// return: (KEY . VALUE) のドット対
lisp_obj* env_lookup(lisp_obj* symbol, lisp_obj* env) {
	if (symbol == NULL) return NULL;
	if (!issym(symbol)) return NULL;
	if (!islist(env)) return NULL;

	// (KEY . VALUE) のペア
	lisp_obj* pair = lcar(env);
	// KEY
	lisp_obj* key = lcar(pair);

	// 検索キーと等しければ、 (KEY . VALUE) のペアを返す
	if (strcmp(lsym(key), lsym(symbol)) == 0) {
		return pair;
	}

	// 次のペアへ
	return env_lookup(symbol, lcdr(env));
}

// --- env_bind: 環境に1個の束縛を追加する ---
lisp_obj* env_bind(lisp_obj* var, lisp_obj* val, lisp_obj* env) {
	if (var == NULL || !issym(var)) return env;
	return lcons(lcons(var, val), env);
}

// --- evlis: 引数の事前評価リストを作成する ---
lisp_obj* evlis(lisp_obj* args, lisp_obj** env) {
	if (!islist(args)) return NULL;

	// 先頭の引数を評価する
	lisp_obj* evaluated_car = lisp_eval(lcar(args), env);

	// 残りの引数を再帰的に評価する
	lisp_obj* evaluated_cdr = evlis(lcdr(args), env);

	// 評価された結果同士を cons で新しくつなぐ
	return lcons(evaluated_car, evaluated_cdr);
}

// --- lisp_eval: 評価ロジック本体 ---
lisp_obj* lisp_eval(lisp_obj* obj, lisp_obj** env) {
	if (obj == NULL) return NULL;

	// シンボルの評価
	if (issym(obj)) {
		lisp_obj* val = lcdr(env_lookup(obj, *env));
		if (val != NULL) return val;
		// TODO: 環境になければ、 NIL を返す
		return obj;
	}

	// リストの評価
	if (islist(obj)) {
		lisp_obj* first = lcar(obj);

		// --- パターンA: 先頭がシンボル ---
		if (issym(first)) {
			char* op = lsym(first);

			// 【特殊形式】quote, cond, define, lambda...

			// --- quote: 評価しない ---
			if (strcmp(op, "quote") == 0) {
				return lcar(lcdr(obj));
			}

			// --- cond: 条件分岐 ---
			if (strcmp(op, "cond") == 0) {
				// 条件と式のペアのリスト、 ((条件1 式1) (条件2 式2) ...) を取り出す
				lisp_obj* clauses = lcdr(obj);

				while (islist(clauses)) {
					// 現在見ているペア
					lisp_obj* clause = lcar(clauses);

					if (islist(clause)) {
						// 条件式
						lisp_obj* cond_expr = lcar(clause);

						// 条件式の評価
						lisp_obj* cond_res = lisp_eval(cond_expr, env);

						if (cond_res != NULL &&
								!(issym(cond_res) && strcmp(lsym(cond_res), "NIL") == 0)) {
							return lisp_eval(lcar(lcdr(clause)), env);
						}
					}

					// 次のペアへ
					clauses = lcdr(clauses);
				}

				// どこにもヒットしなければ NIL
				return NULL;
			}

			// --- define: 環境の永続化 ---
			// 例: (define X APPLE) -> (X . APPLE) を取り出し、環境に追加する
			if (strcmp(op, "define") == 0) {
				lisp_obj* arg_list = lcdr(obj);
				if (!islist(arg_list)) return NULL;

				// 変数名を取り出す
				lisp_obj* var = lcar(arg_list);
				if (var == NULL || !issym(var)) return NULL;

				// 値を取り出して、評価する
				lisp_obj* val = lisp_eval(lcar(lcdr(arg_list)), env);

				// (変数名 . 値) のドット対を環境に追加する
				*env = env_bind(var, val, *env);

				// 登録した変数名を返す
				return var;
			}

			// 【通常関数】

			// 評価済み引数リスト
			lisp_obj* args = evlis(lcdr(obj), env);

			// --- car: コンスセルの先頭を取り出す ---
			if (strcmp(op, "car") == 0) {
				return lcar(lcar(args));
			}

			// --- cdr: コンスセルの先頭を除いた残りのリストを取り出す ---
			if (strcmp(op, "cdr") == 0) {
				return lcdr(lcar(args));
			}

			// --- cons: ペアを作る ---
			if (strcmp(op, "cons") == 0) {
				// 第1引数
				lisp_obj* first_arg = lcar(args);

				// 第2引数
				if (lcdr(args) == NULL) return NULL;
				lisp_obj* second_arg = lcar(lcdr(args));

				return lcons(first_arg, second_arg);
			}

			// --- atom: 引数がシンボルなら T、リストなら NIL を返す ---
			if (strcmp(op, "atom") == 0) {
				if (!islist(args)) return NULL;

				if (issym(lcar(args)))
					return create_symbol("T");

				return NULL;
			}
		}

		return obj;
	}

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
	if (issym(obj)) {
		printf("%s", lsym(obj));
	}

	// リスト
	if (islist(obj)) {
		putchar('(');

		lisp_obj* curr = obj;
		while (curr != NULL) {
			// リスト要素を再帰的に出力
			print_obj(lcar(curr));

			// 次のセルへ
			lisp_obj* next = lcdr(curr);
			if (next == NULL) break;

			// ドット対
			if (issym(next)) {
				printf(" . ");
				print_obj(next);
				break;
			}

			putchar(' ');
			curr = next;
		}
		putchar(')');
	}
}

int main(void) {
	char input[256];
	printf("Welcome to Pure Lisp (Phase 1) \n");

	lisp_obj* global_env = NULL;

	while (1) {
		printf("> ");
		if (fgets(input, sizeof(input), stdin) == NULL) break;

		// 改行を消す
		input[strcspn(input, "\n")] = 0;

		if (strcmp(input, "(exit)") == 0 ||
			strcmp(input, "(bye)") == 0) break;
		if (strlen(input) == 0) continue;

		char* p = input;
		lisp_obj* parsed = read_expr(&p);

		lisp_obj* evaluated = lisp_eval(parsed, &global_env);

		print_obj(evaluated);
		putchar('\n');
	}

	puts("Bye bye!");

	return 0;
}
