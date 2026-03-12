# Fractional Indexing

Fractional indexing solves the problem of maintaining an ordered list where items can be inserted, moved, or reordered without renumbering existing items. Instead of using sequential integers (1, 2, 3, ...) that require renumbering on insert, each item gets a string key that sorts lexicographically in the desired order.

When you insert an item between two existing items, the algorithm generates a new key that sorts between the two neighbors:

```
Before:  "a0"  "a1"  "a2"
Insert between a0 and a1:
After:   "a0"  "a0V"  "a1"  "a2"
```

Keys can always be generated between any two adjacent keys, no matter how many insertions have occurred. The keys grow in length as needed, but stay short in practice.

## When to Use It

Fractional indexing is useful when:

- **Reorderable lists in databases** -- Store the key in an indexed column and `ORDER BY` it. Reordering an item only updates one row instead of renumbering the entire list.
- **Collaborative/real-time editing** -- Multiple users can insert items concurrently without conflicts, since each insertion produces a unique key that doesn't affect other keys.
- **CRDTs** -- Fractional indices are a natural fit for conflict-free replicated data types that need ordered sequences.
- **Drag-and-drop UIs** -- When a user drags an item to a new position, compute a key between its new neighbors and update a single record.

Fractional indexing is *not* needed when items are always appended to the end, when the list is rebuilt from scratch on every change, or when renumbering the entire list is acceptable.

## How It Works

Each key consists of two parts:

1. **Integer part** -- A variable-length encoded integer. The first character (the "head") is a letter from `A-Z` or `a-z` that determines how many digits follow. Lowercase heads (`a-z`) represent increasing positive lengths; uppercase heads (`A-Z`) represent decreasing lengths approaching zero.
2. **Fractional part** -- Optional additional digits that provide arbitrary precision between two integer values.

For example, in base62:
- `a0` is the default starting key (integer zero)
- `a1`, `a2`, ..., `az` increment the integer part
- `a0V` is the midpoint between `a0` and `a1` (since `V` is the middle character of the base62 alphabet)
- `b00` follows `az` (the integer part rolls over to a longer encoding)

Keys are compared using standard lexicographic (string) comparison, which means they work with `ORDER BY` in SQL databases, `strcmp()` in C, and sort functions in any language.

## Building

The library is two files: `fractional_indexing.c` and `fractional_indexing.h`. Add them to your project, or compile from the command line:

```bash
cc -O2 -o fractional_indexing fractional_indexing/fractional_indexing.c fractional_indexing/main.c
```

There are no external dependencies beyond the C standard library.

## Testing

The test suite is in `main.c` and covers:

- Midpoint computation across all bases
- Integer increment/decrement with carry and overflow
- Key validation (trailing zeros, smallest integer, bad heads, short keys)
- Single key generation (both-NULL, a-NULL, b-NULL, both-non-NULL)
- Batch key generation (sequential, reversed, divide-and-conquer)
- Custom base API
- Error reporting and propagation
- Ordering invariants (100,000-key sequences verified as strictly ordered)
- Default API (base62 wrapper) equivalence

Run the tests:

```bash
cc -O2 -o test_fi fractional_indexing/fractional_indexing.c fractional_indexing/main.c && ./test_fi
```

The suite currently has **335 tests** with a 100% pass rate.

## API Reference

The default API uses base62 encoding (`0-9`, `A-Z`, `a-z`), which produces compact keys using only alphanumeric characters. This is the recommended choice for most applications, especially SQLite databases, since keys sort correctly with the default `BINARY` collation and need no escaping in SQL, JSON, or URLs.

All returned strings are heap-allocated. The caller must `free()` them.

### generate_key_between

```c
char *generate_key_between(const char *a, const char *b);
```

Returns a new key that sorts between `a` and `b`.

| `a` | `b` | Behavior |
|---|---|---|
| `NULL` | `NULL` | Returns the default starting key (`"a0"`) |
| `NULL` | non-NULL | Returns a key that sorts before `b` |
| non-NULL | `NULL` | Returns a key that sorts after `a` |
| non-NULL | non-NULL | Returns a key that sorts between `a` and `b` |

Returns `NULL` if either key is invalid, or if `a >= b`.

```c
char *key = generate_key_between(NULL, NULL);   // "a0"
char *k2  = generate_key_between(key, NULL);    // "a1"
char *mid = generate_key_between(key, k2);      // "a0V"
free(key); free(k2); free(mid);
```

### generate_n_keys_between

```c
char **generate_n_keys_between(const char *a, const char *b, int n);
```

Returns an array of `n` keys that sort between `a` and `b`, evenly distributed. The returned keys are in ascending order and are guaranteed to sort correctly between `a` and `b` (and between each other).

Returns `NULL` if `n == 0`, if either key is invalid, or if `a >= b`.

The caller must free the result with `free_keys()`.

```c
char **keys = generate_n_keys_between(NULL, NULL, 5);
// keys[0]="a0", keys[1]="a1", keys[2]="a2", keys[3]="a3", keys[4]="a4"
free_keys(keys, 5);

char **batch = generate_n_keys_between("a0", "a2", 3);
// Produces 3 keys strictly between "a0" and "a2", in order
free_keys(batch, 3);
```

### validate_order_key

```c
bool validate_order_key(const char *key);
```

Returns `true` if `key` is a well-formed order key. Checks:

- Not `NULL` or empty
- Head character is a valid letter (`A-Z` or `a-z`)
- Integer part has the correct length for the head
- Not the smallest representable integer
- Fractional part (if any) does not end with the zero digit

```c
validate_order_key("a0");    // true
validate_order_key("a0V");   // true
validate_order_key("a00");   // false (trailing zero in fractional part)
validate_order_key(NULL);    // false
validate_order_key("b");     // false (too short for head 'b')
```

### midpoint

```c
char *midpoint(const char *a, const char *b, char **error);
```

Low-level function. Computes the midpoint between two fractional parts `a` and `b`. Both are digit strings (no integer prefix). `a` can be empty (meaning the lowest value), and `b` can be `NULL` (meaning no upper bound).

The `error` parameter is optional. If non-NULL and the operation fails, it is set to a static error string (do **not** free it).

Returns `NULL` on error (e.g., `a >= b`).

```c
char *err = NULL;
char *m = midpoint("", NULL, &err);   // "V"
free(m);

m = midpoint("1", "2", &err);         // "1V"
free(m);

m = midpoint("2", "1", &err);         // NULL, err = "Error: a >= b"
```

### increment_integer / decrement_integer

```c
char *increment_integer(const char *x);
char *decrement_integer(const char *x);
```

Increment or decrement the integer part of a key. The input `x` must be a valid integer part (head character + digits).

Returns a new integer string, or `NULL` if the value would overflow (e.g., incrementing `"zzzzzzzzzzzzzzzzzzzzzzzzzzz"` or decrementing `"A00000000000000000000000000"`).

```c
char *r = increment_integer("a0");   // "a1"
free(r);

r = increment_integer("az");         // "b00" (carries into longer encoding)
free(r);

r = decrement_integer("a0");         // "Zz" (borrows into shorter encoding)
free(r);
```

### free_keys

```c
void free_keys(char **keys, int n);
```

Frees an array of keys returned by `generate_n_keys_between()`. Frees each individual key string and then the array itself. Safe to call with `NULL`.

```c
char **keys = generate_n_keys_between(NULL, NULL, 10);
// ... use keys ...
free_keys(keys, 10);
```

## Why Base62 Is the Default

The library defaults to base62 (`0-9`, `A-Z`, `a-z`) because it hits the sweet spot for most applications:

- **Correct `ORDER BY` out of the box** -- The 62 digits are in ascending ASCII byte order (`0` = 0x30, `9` = 0x39, `A` = 0x41, `Z` = 0x5A, `a` = 0x61, `z` = 0x7A), so SQLite's default `BINARY` collation sorts them correctly. No custom collation is needed. The same applies to any system that compares strings by raw byte values (`strcmp`, `memcmp`, JavaScript's `<` operator, etc.).
- **Compact keys** -- 62 symbols per position keeps keys much shorter than base10. After thousands of insertions, base62 keys remain practical in length.
- **Safe everywhere** -- Alphanumeric characters only. No escaping needed in SQL literals, JSON strings, URLs, HTML attributes, CSV fields, or log output.

By contrast, base10 produces keys 2-3x longer for the same number of insertions, and base95 saves a few bytes per key but uses characters like spaces, quotes, and backslashes that require escaping in virtually every context.

## Base Variants

In addition to the default base62 API, the library provides `_base10` and `_base95` suffixed variants of every function, plus a custom base API:

| Base | Digits | Key length | Characters |
|---|---|---|---|
| base10 | `0-9` | Longest | Numeric only |
| base62 | `0-9 A-Z a-z` | Medium | Alphanumeric (recommended) |
| base95 | All printable ASCII | Shortest | Includes spaces and special characters |

**base62** is recommended for most use cases. base10 produces unnecessarily long keys. base95 produces shorter keys but includes characters that require escaping in SQL strings, JSON, and URLs.

For custom digit sets, use `generate_key_between_custom()`, `generate_n_keys_between_custom()`, and `validate_order_key_custom()`. The digits string must contain characters in ascending ASCII order.

## Usage with SQLite

Store the order key in a `TEXT` column with an index:

```sql
CREATE TABLE items (
    id INTEGER PRIMARY KEY,
    order_key TEXT NOT NULL,
    content TEXT
);
CREATE INDEX idx_items_order ON items(order_key);

-- Retrieve items in order
SELECT * FROM items ORDER BY order_key;
```

To insert an item between two existing items, read their `order_key` values and call `generate_key_between()`:

```c
// item_a has order_key "a3", item_b has order_key "a4"
char *new_key = generate_key_between("a3", "a4");
// INSERT INTO items (order_key, content) VALUES (new_key, '...');
free(new_key);
```

To move an item to a new position, compute a key between its new neighbors and update a single row.

## References

- [Implementing Fractional Indexing](https://observablehq.com/@dgreensp/implementing-fractional-indexing) -- David Greenspan
- [Realtime Editing of Ordered Sequences](https://www.figma.com/blog/realtime-editing-of-ordered-sequences/) -- Figma Engineering
- [Reordering Fractional Indices](https://www.steveruiz.me/posts/reordering-fractional-indices) -- Steve Ruiz
- [Fractional Indexing](https://vlcn.io/blog/fractional-indexing) -- vlcn.io
- [CRDT Fractional Indexing](https://madebyevan.com/algos/crdt-fractional-indexing/) -- Evan Wallace

## License

MIT License. See [LICENSE](LICENSE) file.

---

## Part of the SQLite AI Ecosystem

This project is part of the **SQLite AI** ecosystem, a collection of extensions that bring modern AI capabilities to the world’s most widely deployed database. The goal is to make SQLite the default data and inference engine for Edge AI applications.

Other projects in the ecosystem include:

- **[SQLite-AI](https://github.com/sqliteai/sqlite-ai)** — On-device inference and embedding generation directly inside SQLite.
- **[SQLite-Memory](https://github.com/sqliteai/sqlite-memory)** — Markdown-based AI agent memory with semantic search.
- **[SQLite-Vector](https://github.com/sqliteai/sqlite-vector)** — Ultra-efficient vector search for embeddings stored as BLOBs in standard SQLite tables.
- **[SQLite-Sync](https://github.com/sqliteai/sqlite-sync)** — Local-first CRDT-based synchronization for seamless, conflict-free data sync and real-time collaboration across devices.
- **[SQLite-Agent](https://github.com/sqliteai/sqlite-agent)** — Run autonomous AI agents directly from within SQLite databases.
- **[SQLite-MCP](https://github.com/sqliteai/sqlite-mcp)** — Connect SQLite databases to MCP servers and invoke their tools.
- **[SQLite-JS](https://github.com/sqliteai/sqlite-js)** — Create custom SQLite functions using JavaScript.
- **[Liteparser](https://github.com/sqliteai/liteparser)** — A highly efficient and fully compliant SQLite SQL parser.

Learn more at **[SQLite AI](https://sqlite.ai)**.
