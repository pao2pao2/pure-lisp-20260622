# pure-lisp-20260622

C言語で制作している、小さなPure Lisp処理系です。

## About

Pure Lispの処理系を、自分で一から実装する個人プロジェクトです。

REPLを土台にして、

```text
READ → EVAL → PRINT → LOOP
```

という処理系の基本的な流れを、自分の手で実装しています。

「プログラミング言語は、思っているより小さく作り始められるのではないか」という興味から制作を始めました。

## Pure Lisp

このプロジェクトでは、シンボルとリストを中心とした小さなLispを扱っています。

主な要素として、次のような関数・特殊形式を実装していきます。

- `atom`
- `eq`
- `car`
- `cdr`
- `cons`
- `quote`
- `cond`
- `lambda`
- `define`

## Build and Run

GCCが利用できる環境でビルドできます。

```bash
gcc -std=c11 -Wall src/main.c -o pure-lisp
```

ビルド後、生成された実行ファイルを起動します。

```bash
./pure-lisp
```

## REPL

REPL（Read-Eval-Print Loop）では、入力された式を読み取り、評価し、結果を表示します。

例：

```lisp
> (quote a)
a

> (quote (a b c))
(a b c)

> (car (quote (a b c)))
a

> ((lambda (x) (cons x x)) (quote a))
(a . a)
```

## Examples

### Basic functions

#### `atom`

値がシンボルかどうかを判定します。

```lisp
> (atom (quote a))
T

> (atom (quote (a b)))
nil
```

#### `eq`

2つのシンボルが同じかどうかを判定します。

```lisp
> (eq (quote a) (quote a))
T

> (eq (quote a) (quote b))
nil
```

#### `car`

リストの先頭を取り出します。

```lisp
> (car (quote (a b c)))
a
```

#### `cdr`

リストの先頭を除いた残りを取り出します。

```lisp
> (cdr (quote (a b c)))
(b c)
```

#### `cons`

2つの値を組み合わせます。

```lisp
> (cons (quote a) (quote (b c)))
(a b c)

> (cons (quote a) (quote b))
(a . b)
```

### Special forms

#### `quote`

式を評価せず、そのままデータとして扱います。

```lisp
> (quote a)
a

> (quote (a b c))
(a b c)
```

#### `cond`

条件を上から順に評価し、最初に真になった式を評価します。

```lisp
> (cond
...   ((eq (quote a) (quote b)) (quote no))
...   ((quote T) (quote yes)))
yes
```

#### `lambda`

関数を作ります。

```lisp
> ((lambda (x) (cons x x)) (quote a))
(a . a)
```

#### `define`

シンボルに値を束縛します。

```lisp
> (define x (quote a))
x

> x
a
```

関数を束縛することもできます。

```lisp
> (define duplicate
...   (lambda (x) (cons x x)))
duplicate

> (duplicate (quote a))
(a . a)
```

## Purpose

このプロジェクトの目的は、完成された高機能なLisp処理系を作ることだけではありません。

自分で処理系を実装することで、

- コードがどのように読み取られるのか
- 内部でどのようなデータ構造になるのか
- 式がどのように評価されるのか
- REPLがどのように動いているのか

を理解することも目的としています。

## Status

開発中のプロトタイプです。

小さく動くところから始め、少しずつ機能を追加しています。
