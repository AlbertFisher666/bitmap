# ROADMAP: bitmap

План разработки битовой карты (bitmap) на C++.

**Статус:** завершён. Все 8 фаз пройдены. Проект закрыт.

**Что это за файл:** не только план, но и справочник по C++
(Часть A), теория bitmap (Часть B) и запись пройденного пути
(Часть C). Части A и B полезны вне проекта - их можно читать
отдельно.

**Структура:**
- Часть A - справочник C++ (битовые операции, __builtin, типы).
- Часть B - теория bitmap (индексация, set/clear/test, popcount).
- Часть C - пошаговый план (8 фаз, все закрыты).

## Как пользоваться этим файлом

- Работайте **сверху вниз**, шаг за шагом.
- После каждого шага - **компилируйте** и **проверяйте**.
- Не переходите к следующему шагу, пока текущий не работает.
- Когда дойдёте до чекпоинта - покажите наставнику.
- Если застряли - отметьте `[?]` и продолжайте, разберётесь позже.

## Легенда

- `[ ]` - не начато
- `[~]` - в работе
- `[x]` - сделано
- `[!]` - блокер, нужна помощь
- `[?]` - отложено

---

## Оглавление

- [Часть A. Справочник C++](#часть-a-справочник-c)
  - [A.1. Битовые операции](#a1-битовые-операции)
  - [A.2. __builtin-функции GCC](#a2-__builtin-функции-gcc)
  - [A.3. Типы фиксированного размера](#a3-типы-фиксированного-размера)
  - [A.4. Указатели и массивы](#a4-указатели-и-массивы)
  - [A.5. Функции: параметры и возврат](#a5-функции-параметры-и-возврат)
  - [A.6. struct и layout](#a6-struct-и-layout)
  - [A.7. enum class](#a7-enum-class)
  - [A.8. malloc / free](#a8-malloc--free)
  - [A.9. Анонимный namespace](#a9-анонимный-namespace)
  - [A.10. Guard clauses](#a10-guard-clauses)

- [Часть B. Теория bitmap](#часть-b-теория-bitmap)
  - [B.1. Что такое bitmap](#b1-что-такое-bitmap)
  - [B.2. Индексация битов](#b2-индексация-битов)
  - [B.3. Операции: set, clear, test](#b3-операции-set-clear-test)
  - [B.4. Popcount](#b4-popcount)
  - [B.5. Поиск первого установленного](#b5-поиск-первого-установленного)
  - [B.6. Поиск первого свободного](#b6-поиск-первого-свободного)
  - [B.7. Логические операции](#b7-логические-операции)

- [Часть C. Пошаговый план](#часть-c-пошаговый-план)
  - [Фаза 0. Подготовка](#фаза-0-подготовка)
  - [Фаза 1. Структура и init/free](#фаза-1-структура-и-initfree)
  - [Фаза 2. Базовые операции](#фаза-2-базовые-операции)
  - [Фаза 3. Массовые операции](#фаза-3-массовые-операции)
  - [Фаза 4. Popcount и find](#фаза-4-popcount-и-find)
  - [Фаза 5. Логические операции](#фаза-5-логические-операции)
  - [Фаза 6. Диагностика](#фаза-6-диагностика)
  - [Фаза 7. Стресс-тесты](#фаза-7-стресс-тесты)
  - [Фаза 8. Финализация](#фаза-8-финализация)

- [Если застряли](#если-застряли)
- [Когда возвращаться к наставнику](#когда-возвращаться-к-наставнику)
- [Итог](#итог)

---

# ЧАСТЬ A: СПРАВОЧНИК C++

Справочник **короткий** - bitmap использует **мало** конструкций C++. Основное - битовые операции.

## A.1. Битовые операции

### Базовые операции

```cpp
u64 x = 0b1010;       // 10

x | (1ULL << 3);      // установить бит 3 -> 0b1010 | 0b1000 = 0b1010
x & ~(1ULL << 1);     // сбросить бит 1 -> 0b1010 & ~0b0010 = 0b1000
x ^ (1ULL << 2);      // переключить бит 2 -> 0b1010 ^ 0b0100 = 0b1110
x & (1ULL << 2);      // проверить бит 2 -> 0b1010 & 0b0100 = 0b0000
```

**Правило:** используйте `1ULL` (unsigned long long), а не `1`. Иначе на 64-битных числах сдвиг на 63 может дать UB.

### Сдвиги

```cpp
1ULL << 0;    // 1
1ULL << 1;    // 2
1ULL << 63;   // 9223372036854775808
```

**Сдвиг на 64 и больше - UB.** Всегда проверяйте `shift < 64`.

### Маски

```cpp
u64 mask = 0xFFFFFFFFFFFFFFFFULL;   // все 64 бита
u64 low8 = 0xFFULL;                  // младшие 8 бит
u64 high8 = 0xFF00000000000000ULL;   // старшие 8 бит
```

## A.2. `__builtin`-функции GCC

GCC и Clang дают **встроенные** функции для работы с битами. Быстрые, компилируются в **одну** инструкцию процессора (`popcnt`, `tzcnt`).

### Popcount - сколько битов установлено

```cpp
int n = __builtin_popcountll(x);   // для u64
int m = __builtin_popcount(x);     // для u32
```

Пример:

```cpp
__builtin_popcountll(0b1011);   // -> 3
__builtin_popcountll(0);        // -> 0
__builtin_popcountll(~0ULL);    // -> 64
```

### Count trailing zeros - индекс первого установленного бита

```cpp
int i = __builtin_ctzll(x);   // для u64
int j = __builtin_ctz(x);     // для u32
```

Возвращает **число нулей справа** = индекс **младшего** установленного бита.

Пример:

```cpp
__builtin_ctzll(0b1000);   // -> 3
__builtin_ctzll(0b1010);   // -> 1
```

**Осторожно:** `__builtin_ctzll(0)` - **UB**. Проверяйте `x != 0`.

### Count leading zeros - сколько нулей слева

```cpp
int i = __builtin_clzll(x);   // для u64
```

Возвращает **число нулей слева** до первого установленного бита.

Пример:

```cpp
__builtin_clzll(0b1000);   // -> 60 (для 64-битного)
```

**Осторожно:** `__builtin_clzll(0)` - **UB**.

### Использование в bitmap

```cpp
// Найти первый установленный бит в слове word.
if (word != 0) {
    int i = __builtin_ctzll(word);   // индекс от 0 до 63
    // глобальный индекс = word_index * 64 + i
}
```

## A.3. Типы фиксированного размера

```cpp
using u8  = std::uint8_t;    // 1 байт
using u16 = std::uint16_t;   // 2 байта
using u32 = std::uint32_t;   // 4 байта
using u64 = std::uint64_t;   // 8 байт
```

В bitmap работаем с **`u64`** - самое быстрое. Внутри - массив `u64`.

## A.4. Указатели и массивы

```cpp
u64* words = static_cast<u64*>(std::malloc(n_words * sizeof(u64)));
```

- `words[i]` - доступ к i-му слову.
- `words[i] >> j` - сдвиг внутри слова.
- `&words[i]` - указатель на i-е слово.

**Правило:** проверяйте `words != nullptr` перед использованием.

## A.5. Функции: параметры и возврат

```cpp
// По указателю - может быть nullptr.
void bm_set(Bitmap* bm, std::size_t index);

// По ссылке - всегда валидна.
void bm_set(Bitmap& bm, std::size_t index);
```

В стиле `vfs` - используем **ссылки** (`Bitmap&`), потому что bitmap **всегда существует**.

**Возврат:**

```cpp
bool bm_test(const Bitmap& bm, std::size_t index);   // возвращает true/false
BmError bm_init(Bitmap& bm, std::size_t n);          // код ошибки
std::size_t bm_count(const Bitmap& bm);              // число
```

## A.6. `struct` и layout

```cpp
struct Bitmap {
    u64*        words      = nullptr;   // массив слов
    std::size_t n_bits     = 0;         // сколько битов
    std::size_t n_words    = 0;         // сколько слов
    std::size_t count      = 0;         // сколько установлено
};
```

**Правило:** инициализируйте поля **в определении**. Тогда `Bitmap bm;` создаст "пустой" объект.

## A.7. `enum class`

```cpp
enum class BmError : u32 {
    Ok              = 0,
    NoMemory        = 1,
    OutOfRange      = 2,
    SizeMismatch    = 3,
    NotInitialized  = 4,
};
```

**Правило:** `enum class` + явный подлежащий тип (`: u32`).

## A.8. `malloc` / `free`

```cpp
#include <cstdlib>

u64* words = static_cast<u64*>(std::malloc(bytes));
if (words == nullptr) return BmError::NoMemory;

// ... использование ...

std::free(words);
```

**Правило:** всегда проверяйте `nullptr`. `free(nullptr)` - безопасно.

## A.9. Анонимный namespace

```cpp
namespace {

// Видны только в этом .cpp.
u64* word_ptr(Bitmap& bm, std::size_t word_idx) {
    return &bm.words[word_idx];
}

} // anonymous namespace
```

Внутренние функции - в анонимный namespace.

## A.10. Guard clauses

```cpp
BmError bm_set(Bitmap& bm, std::size_t index) {
    if (bm.words == nullptr) return BmError::NotInitialized;
    if (index >= bm.n_bits)  return BmError::OutOfRange;

    // Основная логика.
    ...
    return BmError::Ok;
}
```

**Правило:** все проверки - в начале. Ранний `return` при ошибке. Логика - в конце.

---

# ЧАСТЬ B: ТЕОРИЯ BITMAP

## B.1. Что такое bitmap

**Bitmap** - массив **битов**, каждый из которых может быть **0** или **1**.

Физически биты хранятся в **словах** (`u64`). Одно слово = **64 бита**.

```
Слово 0:  [63][62]...[2][1][0]
Слово 1:  [63][62]...[2][1][0]
Слово 2:  [63][62]...[2][1][0]
...
```

**Пример:** bitmap на 100 битов = 2 слова (128 бит, 28 "лишних").

## B.2. Индексация битов

Для глобального индекса `i`:

- **Номер слова:** `word_idx = i / 64`.
- **Позиция в слове:** `bit_idx = i % 64`.
- **Маска:** `mask = 1ULL << bit_idx`.

**Пример:** `i = 100`.

- `word_idx = 100 / 64 = 1`.
- `bit_idx  = 100 % 64 = 36`.
- `mask = 1ULL << 36`.

**Проверка:** `bm.words[1] & (1ULL << 36)`.

## B.3. Операции: set, clear, test

**Set** (установить бит в 1):

```cpp
words[word_idx] |= mask;
```

**Clear** (сбросить в 0):

```cpp
words[word_idx] &= ~mask;
```

**Test** (проверить):

```cpp
bool set = (words[word_idx] & mask) != 0;
```

**Toggle** (переключить):

```cpp
words[word_idx] ^= mask;
```

## B.4. Popcount

**Сколько битов установлено** в bitmap.

**Наивно:** пройти по всем словам, посчитать.

```cpp
std::size_t count = 0;
for (std::size_t i = 0; i < n_words; ++i) {
    count += __builtin_popcountll(words[i]);
}
```

**С `count` в структуре:** поддерживать **инкрементально**.

- `bm_set` увеличивает `count`.
- `bm_clear` уменьшает.
- `bm_toggle` - если был 0 -> +1, если 1 -> -1.

**Плюс:** `bm_count` - за **O(1)**.

**Минус:** надо **аккуратно** поддерживать. Если `set` уже установленного бита - `count` **не растёт**. Если `clear` сброшенного - **не падает**.

## B.5. Поиск первого установленного

**Задача:** найти **минимальный индекс** `i`, где бит = 1.

**Алгоритм:**

```cpp
for (std::size_t w = 0; w < n_words; ++w) {
    if (words[w] == 0) continue;                    // пропустить пустые слова
    int bit = __builtin_ctzll(words[w]);            // индекс в слове
    return w * 64 + bit;                             // глобальный индекс
}
return BM_NOT_FOUND;
```

**Плюс:** пропускаем **64 бита** за раз, если слово пустое.

## B.6. Поиск первого свободного

**Задача:** найти **минимальный индекс** `i`, где бит = 0.

**Алгоритм:**

```cpp
for (std::size_t w = 0; w < n_words; ++w) {
    u64 inverted = ~words[w];               // инвертируем
    if (inverted == 0) continue;            // всё установлено
    int bit = __builtin_ctzll(inverted);    // первый свободный
    std::size_t global = w * 64 + bit;
    if (global >= n_bits) return BM_NOT_FOUND;  // за пределами
    return global;
}
return BM_NOT_FOUND;
```

**Важно:** последнее слово может содержать "лишние" биты (за `n_bits`). Их надо **игнорировать** - считать **занятыми**.

## B.7. Логические операции

**AND** (пересечение): `c[i] = a[i] & b[i]`.

**OR** (объединение): `c[i] = a[i] | b[i]`.

**XOR** (симметрическая разность): `c[i] = a[i] ^ b[i]`.

**NOT** (дополнение): `c[i] = ~a[i]` - но **только** в пределах `n_bits`.

**Требование:** `a.n_bits == b.n_bits`. Иначе `SizeMismatch`.

---

# ЧАСТЬ C: ПОШАГОВЫЙ ПЛАН

## Фаза 0. Подготовка

### 0.1. Создать структуру

```bash
cd ~/Documents/TestCsharpAndCpp
mkdir -p Test31/include/bitmap
mkdir -p Test31/src
cd Test31
```

- [x] Созданы `include/bitmap/` и `src/`.

### 0.2. `CMakeLists.txt`

```cmake
cmake_minimum_required(VERSION 3.20)
project(bitmap VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "" FORCE)
endif()

add_library(bitmap_lib STATIC
    src/bitmap.cpp
)
target_include_directories(bitmap_lib PUBLIC include)
target_compile_options(bitmap_lib PRIVATE
    -Wall -Wextra -Wpedantic -Wshadow
)

add_executable(bitmap_test src/bitmap_test.cpp)
target_link_libraries(bitmap_test PRIVATE bitmap_lib)
target_compile_options(bitmap_test PRIVATE
    -Wall -Wextra -Wpedantic -Wshadow
)
```

- [x] `CMakeLists.txt` создан.

### 0.3. Пустые файлы

```bash
touch include/bitmap/bitmap.hpp
touch src/bitmap.cpp
touch src/bitmap_test.cpp
```

- [x] Файлы созданы.

**Чекпоинт 0.**

---

## Фаза 1. Структура и init/free

### 1.1. `bitmap.hpp`

```cpp
// ============================================================
// bitmap.hpp
// A low-level bitset with fixed size.
//
// Style: mirrors vfs and MemorySystem.
//   - Global Bitmap object.
//   - Free functions bm_*.
//   - Error codes via enum class BmError.
//   - Fixed size, no growth.
// ============================================================

#pragma once

#include <cstdint>
#include <cstddef>

namespace bitmap {

using u8  = std::uint8_t;
using u16 = std::uint16_t;
using u32 = std::uint32_t;
using u64 = std::uint64_t;

// Sentinel: "not found".
constexpr std::size_t BM_NOT_FOUND = static_cast<std::size_t>(-1);

// Error codes.
enum class BmError : u32 {
    Ok              = 0,
    NoMemory        = 1,
    OutOfRange      = 2,
    SizeMismatch    = 3,
    NotInitialized  = 4,
};

const char* bm_error_string(BmError e);

// Bitmap structure.
struct Bitmap {
    u64*        words   = nullptr;   // array of 64-bit words
    std::size_t n_bits  = 0;         // requested size in bits
    std::size_t n_words = 0;         // number of words
    std::size_t count   = 0;         // number of set bits
};

// Initialise with the given number of bits.
// Allocates n_words = ceil(n_bits / 64) words.
// All bits start at 0.
BmError bm_init(Bitmap& bm, std::size_t n_bits);

// Free.
void bm_free(Bitmap& bm);

// True if the bitmap is usable.
bool bm_valid(const Bitmap& bm);

} // namespace bitmap
```

- [x] Заголовок написан.

### 1.2. `bitmap.cpp` - скелет

```cpp
#include "bitmap/bitmap.hpp"
#include <cstdlib>
#include <cstring>

namespace bitmap {

const char* bm_error_string(BmError e) {
    switch (e) {
        case BmError::Ok:             return "ok";
        case BmError::NoMemory:       return "no memory";
        case BmError::OutOfRange:     return "out of range";
        case BmError::SizeMismatch:   return "size mismatch";
        case BmError::NotInitialized: return "not initialized";
    }
    return "unknown";
}

BmError bm_init(Bitmap& bm, std::size_t n_bits) {
    // TODO: реализуйте
    (void)bm;
    (void)n_bits;
    return BmError::Ok;
}

void bm_free(Bitmap& bm) {
    // TODO: реализуйте
    (void)bm;
}

bool bm_valid(const Bitmap& bm) {
    return bm.words != nullptr && bm.n_bits > 0;
}

} // namespace bitmap
```

- [x] Скелет написан.

### 1.3. Реализовать `bm_init`

**Шаги:**

1. Если `n_bits == 0` - вернуть `BmError::Ok` (пустой bitmap).
2. `n_words = (n_bits + 63) / 64`.
3. `words = malloc(n_words * sizeof(u64))`.
4. Если `nullptr` - вернуть `BmError::NoMemory`.
5. `memset(words, 0, ...)` - все биты в 0.
6. Заполнить поля `n_bits`, `n_words`, `count = 0`.
7. Вернуть `BmError::Ok`.

- [x] `bm_init` реализована.

### 1.4. Реализовать `bm_free`

**Шаги:**

1. Если `words != nullptr` - `free(words)`.
2. Обнулить все поля.

- [x] `bm_free` реализована.

### 1.5. Тест

В `bitmap_test.cpp`:

```cpp
#include "bitmap/bitmap.hpp"
#include <cstdio>

using namespace bitmap;

static int g_failures = 0;
static int g_checks   = 0;

void check(bool cond, const char* what) {
    ++g_checks;
    if (!cond) {
        std::fprintf(stderr, "FAIL: %s\n", what);
        ++g_failures;
    }
}

int main() {
    std::printf("=== bitmap tests ===\n");

    Bitmap bm;
    check(bm.words == nullptr, "empty bitmap has nullptr words");

    BmError err = bm_init(bm, 100);
    check(err == BmError::Ok, "init ok");
    check(bm.n_bits == 100, "n_bits = 100");
    check(bm.n_words == 2, "n_words = 2");
    check(bm.count == 0, "count = 0");
    check(bm_valid(bm), "valid");

    bm_free(bm);
    check(bm.words == nullptr, "freed");

    std::printf("\n%d checks, %d failures\n", g_checks, g_failures);
    return g_failures == 0 ? 0 : 1;
}
```

- [x] Тест написан.

### 1.6. Сборка и запуск

```bash
mkdir -p build && cd build
cmake ..
cmake --build .
./bitmap_test
```

**Ожидается:** `3 checks, 0 failures`.

**Чекпоинт 1.**

---

## Фаза 2. Базовые операции

### 2.1. Объявления в `bitmap.hpp`

```cpp
// Set bit at index.
BmError bm_set(Bitmap& bm, std::size_t index);

// Clear bit at index.
BmError bm_clear(Bitmap& bm, std::size_t index);

// Test bit at index.
bool bm_test(const Bitmap& bm, std::size_t index);

// Toggle bit at index.
BmError bm_toggle(Bitmap& bm, std::size_t index);

// Total number of bits.
std::size_t bm_size(const Bitmap& bm);

// Number of set bits.
std::size_t bm_count(const Bitmap& bm);
```

- [x] Объявления добавлены.

### 2.2. Реализация

**`bm_set`:**

```cpp
BmError bm_set(Bitmap& bm, std::size_t index) {
    if (!bm_valid(bm)) return BmError::NotInitialized;
    if (index >= bm.n_bits) return BmError::OutOfRange;

    const std::size_t word_idx = index / 64;
    const std::size_t bit_idx  = index % 64;
    const u64 mask = 1ULL << bit_idx;

    if ((bm.words[word_idx] & mask) == 0) {
        bm.words[word_idx] |= mask;
        ++bm.count;
    }
    return BmError::Ok;
}
```

**`bm_clear`:**

```cpp
BmError bm_clear(Bitmap& bm, std::size_t index) {
    if (!bm_valid(bm)) return BmError::NotInitialized;
    if (index >= bm.n_bits) return BmError::OutOfRange;

    const std::size_t word_idx = index / 64;
    const std::size_t bit_idx  = index % 64;
    const u64 mask = 1ULL << bit_idx;

    if ((bm.words[word_idx] & mask) != 0) {
        bm.words[word_idx] &= ~mask;
        --bm.count;
    }
    return BmError::Ok;
}
```

**`bm_test`:**

```cpp
bool bm_test(const Bitmap& bm, std::size_t index) {
    if (!bm_valid(bm)) return false;
    if (index >= bm.n_bits) return false;

    const std::size_t word_idx = index / 64;
    const std::size_t bit_idx  = index % 64;
    return (bm.words[word_idx] & (1ULL << bit_idx)) != 0;
}
```

**`bm_toggle`:**

```cpp
BmError bm_toggle(Bitmap& bm, std::size_t index) {
    if (!bm_valid(bm)) return BmError::NotInitialized;
    if (index >= bm.n_bits) return BmError::OutOfRange;

    const std::size_t word_idx = index / 64;
    const std::size_t bit_idx  = index % 64;
    const u64 mask = 1ULL << bit_idx;

    if (bm.words[word_idx] & mask) {
        bm.words[word_idx] &= ~mask;
        --bm.count;
    } else {
        bm.words[word_idx] |= mask;
        ++bm.count;
    }
    return BmError::Ok;
}
```

**`bm_size`:**

```cpp
std::size_t bm_size(const Bitmap& bm) {
    return bm.n_bits;
}
```

**`bm_count`:**

```cpp
std::size_t bm_count(const Bitmap& bm) {
    return bm.count;
}
```

- [x] Все функции реализованы.

### 2.3. Тесты

Добавьте в `bitmap_test.cpp`:

```cpp
void test_basic() {
    std::printf("-- test_basic\n");

    Bitmap bm;
    bm_init(bm, 100);

    check(!bm_test(bm, 0), "bit 0 initially 0");

    bm_set(bm, 0);
    check(bm_test(bm, 0), "bit 0 set");
    check(bm_count(bm) == 1, "count = 1");

    bm_set(bm, 0);  // set already set
    check(bm_count(bm) == 1, "count stays 1");

    bm_clear(bm, 0);
    check(!bm_test(bm, 0), "bit 0 cleared");
    check(bm_count(bm) == 0, "count = 0");

    bm_toggle(bm, 5);
    check(bm_test(bm, 5), "toggle set");
    bm_toggle(bm, 5);
    check(!bm_test(bm, 5), "toggle clear");

    // Out of range.
    check(bm_set(bm, 100) == BmError::OutOfRange, "set oob");
    check(bm_clear(bm, 1000) == BmError::OutOfRange, "clear oob");

    // Boundary: last bit.
    bm_set(bm, 99);
    check(bm_test(bm, 99), "last bit set");

    bm_free(bm);
}
```

Вызовите в `main`:

```cpp
test_basic();
```

- [x] Тесты добавлены.

**Чекпоинт 2.**

---

## Фаза 3. Массовые операции

### 3.1. Объявления

```cpp
// Set all bits to 1.
BmError bm_set_all(Bitmap& bm);

// Clear all bits.
BmError bm_clear_all(Bitmap& bm);

// Print to stdout as hex.
void bm_print(const Bitmap& bm);
```

- [x] Объявления добавлены.

### 3.2. Реализация

**`bm_set_all`:**

```cpp
BmError bm_set_all(Bitmap& bm) {
    if (!bm_valid(bm)) return BmError::NotInitialized;

    for (std::size_t i = 0; i < bm.n_words; ++i) {
        bm.words[i] = ~0ULL;
    }

    // Clear trailing bits beyond n_bits in the last word.
    const std::size_t extra = bm.n_words * 64 - bm.n_bits;
    if (extra > 0) {
        bm.words[bm.n_words - 1] &= ~0ULL >> extra;
    }

    bm.count = bm.n_bits;
    return BmError::Ok;
}
```

**`bm_clear_all`:**

```cpp
BmError bm_clear_all(Bitmap& bm) {
    if (!bm_valid(bm)) return BmError::NotInitialized;
    for (std::size_t i = 0; i < bm.n_words; ++i) {
        bm.words[i] = 0;
    }
    bm.count = 0;
    return BmError::Ok;
}
```

**`bm_print`:**

```cpp
void bm_print(const Bitmap& bm) {
    if (!bm_valid(bm)) {
        std::printf("(invalid bitmap)\n");
        return;
    }
    std::printf("bitmap: %zu bits, %zu words, %zu set\n",
                bm.n_bits, bm.n_words, bm.count);
    for (std::size_t i = 0; i < bm.n_words; ++i) {
        std::printf("  word %2zu: 0x%016llX\n", i,
                    static_cast<unsigned long long>(bm.words[i]));
    }
}
```

- [x] Реализованы.

### 3.3. Тесты

```cpp
void test_mass() {
    std::printf("-- test_mass\n");

    Bitmap bm;
    bm_init(bm, 100);

    bm_set_all(bm);
    check(bm_count(bm) == 100, "set_all count");

    for (std::size_t i = 0; i < 100; ++i) {
        if (!bm_test(bm, i)) {
            std::printf("FAIL: bit %zu not set\n", i);
            ++g_failures;
        }
    }
    ++g_checks;

    bm_clear_all(bm);
    check(bm_count(bm) == 0, "clear_all count");
    check(!bm_test(bm, 0), "bit 0 clear");
    check(!bm_test(bm, 99), "bit 99 clear");

    bm_free(bm);
}
```

- [x] Тесты добавлены.

**Чекпойнт 3.**

---

## Фаза 4. Popcount и find

### 4.1. Объявления

```cpp
// Find first set bit. Returns BM_NOT_FOUND if none.
std::size_t bm_find_first_set(const Bitmap& bm);

// Find first clear bit. Returns BM_NOT_FOUND if none.
std::size_t bm_find_first_clear(const Bitmap& bm);

// Recompute count from scratch.
std::size_t bm_recount(const Bitmap& bm);
```

- [x] Объявления добавлены.

### 4.2. Реализация

**`bm_find_first_set`:**

```cpp
std::size_t bm_find_first_set(const Bitmap& bm) {
    if (!bm_valid(bm)) return BM_NOT_FOUND;

    for (std::size_t w = 0; w < bm.n_words; ++w) {
        if (bm.words[w] == 0) continue;
        const int bit = __builtin_ctzll(bm.words[w]);
        const std::size_t global = w * 64 + static_cast<std::size_t>(bit);
        if (global >= bm.n_bits) return BM_NOT_FOUND;
        return global;
    }
    return BM_NOT_FOUND;
}
```

**`bm_find_first_clear`:**

```cpp
std::size_t bm_find_first_clear(const Bitmap& bm) {
    if (!bm_valid(bm)) return BM_NOT_FOUND;

    for (std::size_t w = 0; w < bm.n_words; ++w) {
        u64 inverted = ~bm.words[w];
        if (inverted == 0) continue;
        const int bit = __builtin_ctzll(inverted);
        const std::size_t global = w * 64 + static_cast<std::size_t>(bit);
        if (global >= bm.n_bits) return BM_NOT_FOUND;
        return global;
    }
    return BM_NOT_FOUND;
}
```

**`bm_recount`:**

```cpp
std::size_t bm_recount(const Bitmap& bm) {
    if (!bm_valid(bm)) return 0;
    std::size_t count = 0;
    for (std::size_t w = 0; w < bm.n_words; ++w) {
        count += static_cast<std::size_t>(
            __builtin_popcountll(bm.words[w]));
    }
    return count;
}
```

- [x] Реализованы.

### 4.3. Тесты

```cpp
void test_find() {
    std::printf("-- test_find\n");

    Bitmap bm;
    bm_init(bm, 200);

    check(bm_find_first_set(bm) == BM_NOT_FOUND, "no set bits");
    check(bm_find_first_clear(bm) == 0, "first clear is 0");

    bm_set(bm, 0);
    bm_set(bm, 5);
    bm_set(bm, 63);
    bm_set(bm, 64);
    bm_set(bm, 130);

    check(bm_find_first_set(bm) == 0, "first set is 0");
    check(bm_find_first_clear(bm) == 1, "first clear is 1");
    check(bm_recount(bm) == 5, "recount = 5");
    check(bm_count(bm) == 5, "count = 5");

    // Clear bit 0, find again.
    bm_clear(bm, 0);
    check(bm_find_first_set(bm) == 5, "first set is 5");

    // Fill all, no clear bits.
    bm_set_all(bm);
    check(bm_find_first_clear(bm) == BM_NOT_FOUND, "no clear bits");
    check(bm_recount(bm) == 200, "recount = 200");

    bm_free(bm);
}
```

- [x] Тесты добавлены.

**Чекпоинт 4.**

---

## Фаза 5. Логические операции

### 5.1. Объявления

```cpp
// c = a & b
BmError bm_and(const Bitmap& a, const Bitmap& b, Bitmap& c);

// c = a | b
BmError bm_or(const Bitmap& a, const Bitmap& b, Bitmap& c);

// c = a ^ b
BmError bm_xor(const Bitmap& a, const Bitmap& b, Bitmap& c);

// c = ~a
BmError bm_not(const Bitmap& a, Bitmap& c);

// c = a
BmError bm_copy(const Bitmap& a, Bitmap& c);
```

- [x] Объявления добавлены.

### 5.2. Реализация

**`bm_and`:**

```cpp
BmError bm_and(const Bitmap& a, const Bitmap& b, Bitmap& c) {
    if (!bm_valid(a) || !bm_valid(b)) return BmError::NotInitialized;
    if (a.n_bits != b.n_bits) return BmError::SizeMismatch;

    BmError err = bm_init(c, a.n_bits);
    if (err != BmError::Ok) return err;

    c.count = 0;
    for (std::size_t i = 0; i < a.n_words; ++i) {
        c.words[i] = a.words[i] & b.words[i];
        c.count += static_cast<std::size_t>(
            __builtin_popcountll(c.words[i]));
    }
    return BmError::Ok;
}
```

**`bm_or`:**

```cpp
BmError bm_or(const Bitmap& a, const Bitmap& b, Bitmap& c) {
    if (!bm_valid(a) || !bm_valid(b)) return BmError::NotInitialized;
    if (a.n_bits != b.n_bits) return BmError::SizeMismatch;

    BmError err = bm_init(c, a.n_bits);
    if (err != BmError::Ok) return err;

    c.count = 0;
    for (std::size_t i = 0; i < a.n_words; ++i) {
        c.words[i] = a.words[i] | b.words[i];
        c.count += static_cast<std::size_t>(
            __builtin_popcountll(c.words[i]));
    }
    return BmError::Ok;
}
```

**`bm_xor`:**

```cpp
BmError bm_xor(const Bitmap& a, const Bitmap& b, Bitmap& c) {
    if (!bm_valid(a) || !bm_valid(b)) return BmError::NotInitialized;
    if (a.n_bits != b.n_bits) return BmError::SizeMismatch;

    BmError err = bm_init(c, a.n_bits);
    if (err != BmError::Ok) return err;

    c.count = 0;
    for (std::size_t i = 0; i < a.n_words; ++i) {
        c.words[i] = a.words[i] ^ b.words[i];
        c.count += static_cast<std::size_t>(
            __builtin_popcountll(c.words[i]));
    }
    return BmError::Ok;
}
```

**`bm_not`:**

```cpp
BmError bm_not(const Bitmap& a, Bitmap& c) {
    if (!bm_valid(a)) return BmError::NotInitialized;

    BmError err = bm_init(c, a.n_bits);
    if (err != BmError::Ok) return err;

    for (std::size_t i = 0; i < a.n_words; ++i) {
        c.words[i] = ~a.words[i];
    }

    // Clear trailing bits.
    const std::size_t extra = a.n_words * 64 - a.n_bits;
    if (extra > 0) {
        c.words[a.n_words - 1] &= ~0ULL >> extra;
    }

    c.count = a.n_bits - a.count;
    return BmError::Ok;
}
```

**`bm_copy`:**

```cpp
BmError bm_copy(const Bitmap& a, Bitmap& c) {
    if (!bm_valid(a)) return BmError::NotInitialized;

    BmError err = bm_init(c, a.n_bits);
    if (err != BmError::Ok) return err;

    for (std::size_t i = 0; i < a.n_words; ++i) {
        c.words[i] = a.words[i];
    }
    c.count = a.count;
    return BmError::Ok;
}
```

- [x] Реализованы.

### 5.3. Тесты

```cpp
void test_logic() {
    std::printf("-- test_logic\n");

    Bitmap a, b, c;
    bm_init(a, 100);
    bm_init(b, 100);

    // a: bits 0-49
    for (std::size_t i = 0; i < 50; ++i) bm_set(a, i);
    // b: bits 25-74
    for (std::size_t i = 25; i < 75; ++i) bm_set(b, i);

    // AND -> bits 25-49 (25 bits)
    bm_and(a, b, c);
    check(bm_count(c) == 25, "and count");
    check(bm_test(c, 25), "and has 25");
    check(!bm_test(c, 0), "and no 0");
    check(!bm_test(c, 50), "and no 50");
    bm_free(c);

    // OR -> bits 0-74 (75 bits)
    bm_or(a, b, c);
    check(bm_count(c) == 75, "or count");
    check(bm_test(c, 0), "or has 0");
    check(bm_test(c, 74), "or has 74");
    check(!bm_test(c, 75), "or no 75");
    bm_free(c);

    // XOR -> bits 0-24 and 50-74 (50 bits)
    bm_xor(a, b, c);
    check(bm_count(c) == 50, "xor count");
    check(bm_test(c, 0), "xor has 0");
    check(bm_test(c, 74), "xor has 74");
    check(!bm_test(c, 25), "xor no 25");
    bm_free(c);

    // NOT of a -> bits 50-99 (50 bits)
    bm_not(a, c);
    check(bm_count(c) == 50, "not count");
    check(!bm_test(c, 0), "not no 0");
    check(bm_test(c, 50), "not has 50");
    check(bm_test(c, 99), "not has 99");
    bm_free(c);

    bm_free(a);
    bm_free(b);
}
```

- [x] Тесты добавлены.

**Чекпоинт 5.**

---

## Фаза 6. Диагностика

### 6.1. Объявление

```cpp
// Verify internal consistency. Returns number of problems.
u32 bm_check(const Bitmap& bm);
```

- [x] Объявление добавлено.

### 6.2. Реализация

```cpp
u32 bm_check(const Bitmap& bm) {
    u32 problems = 0;

    if (bm.words == nullptr) {
        if (bm.n_bits != 0 || bm.n_words != 0 || bm.count != 0) {
            std::fprintf(stderr, "check: null words but non-zero fields\n");
            ++problems;
        }
        return problems;
    }

    // Check n_words.
    const std::size_t expected_words = (bm.n_bits + 63) / 64;
    if (bm.n_words != expected_words) {
        std::fprintf(stderr, "check: n_words %zu != %zu\n",
                     bm.n_words, expected_words);
        ++problems;
    }

    // Check count.
    const std::size_t real_count = bm_recount(bm);
    if (bm.count != real_count) {
        std::fprintf(stderr, "check: count %zu != recount %zu\n",
                     bm.count, real_count);
        ++problems;
    }

    // Check trailing bits are 0.
    const std::size_t extra = bm.n_words * 64 - bm.n_bits;
    if (extra > 0) {
        const u64 trailing = bm.words[bm.n_words - 1] >> (64 - extra);
        if (trailing != 0) {
            std::fprintf(stderr, "check: trailing bits not clear\n");
            ++problems;
        }
    }

    return problems;
}
```

- [x] Реализована.

### 6.3. Тест

```cpp
void test_check() {
    std::printf("-- test_check\n");

    Bitmap bm;
    bm_init(bm, 100);

    check(bm_check(bm) == 0, "check empty");

    bm_set(bm, 0);
    bm_set(bm, 50);
    bm_set(bm, 99);
    check(bm_check(bm) == 0, "check after set");

    bm_set_all(bm);
    check(bm_check(bm) == 0, "check after set_all");

    bm_clear_all(bm);
    check(bm_check(bm) == 0, "check after clear_all");

    bm_free(bm);
}
```

- [x] Тест добавлен.

**Чекпоинт 6.**

---

## Фаза 7. Стресс-тесты

### 7.1. Тест на 1 миллион битов

```cpp
void test_stress_large() {
    std::printf("-- test_stress_large\n");

    const std::size_t N = 1000000;
    Bitmap bm;
    bm_init(bm, N);

    // Set every 7th bit.
    for (std::size_t i = 0; i < N; i += 7) {
        bm_set(bm, i);
    }

    const std::size_t expected = (N + 6) / 7;
    check(bm_count(bm) == expected, "large count");

    // Verify.
    for (std::size_t i = 0; i < N; ++i) {
        bool want = (i % 7 == 0);
        bool got = bm_test(bm, i);
        if (want != got) {
            std::printf("FAIL: bit %zu want %d got %d\n",
                        i, want, got);
            ++g_failures;
            break;
        }
    }
    ++g_checks;

    check(bm_check(bm) == 0, "large check");
    bm_free(bm);
}
```

### 7.2. Тест toggle

```cpp
void test_toggle_stress() {
    std::printf("-- test_toggle_stress\n");

    Bitmap bm;
    bm_init(bm, 1000);

    // Toggle each bit twice.
    for (std::size_t i = 0; i < 1000; ++i) {
        bm_toggle(bm, i);
    }
    check(bm_count(bm) == 1000, "all toggled on");

    for (std::size_t i = 0; i < 1000; ++i) {
        bm_toggle(bm, i);
    }
    check(bm_count(bm) == 0, "all toggled off");

    check(bm_check(bm) == 0, "toggle check");
    bm_free(bm);
}
```

- [x] Тесты добавлены.

**Чекпоинт 7.**

---

## Фаза 8. Финализация

### 8.1. Все тесты в `main`

```cpp
int main() {
    std::printf("=== bitmap tests ===\n");

    test_basic();
    test_mass();
    test_find();
    test_logic();
    test_check();
    test_stress_large();
    test_toggle_stress();

    std::printf("\n%d checks, %d failures\n", g_checks, g_failures);
    return g_failures == 0 ? 0 : 1;
}
```

- [x] Все тесты вызываются.

### 8.2. Valgrind / AddressSanitizer

Valgrind заменён на AddressSanitizer (встроен в GCC, не требует
установки пакетов):

    g++ -std=c++17 -g -O1 -fsanitize=address,undefined -fno-omit-frame-pointer \
        -Iinclude src/bitmap.cpp src/bitmap_test.cpp -o /tmp/bitmap_test_asan
    /tmp/bitmap_test_asan

    g++ -std=c++17 -g -O1 -fsanitize=address,undefined -fno-omit-frame-pointer \
        -Iinclude src/bitmap.cpp src/bitmap_demo.cpp -o /tmp/bitmap_demo_asan
    /tmp/bitmap_demo_asan

**Ожидается:** нет вывода от ASan (значит, чисто).

- [x] Нет утечек и UB.

### 8.3. README

Написать краткий `README.md`:

- Что делает bitmap.
- Примеры API.
- Как собрать и запустить.

- [x] README написан.

### 8.4. Git

```bash
git init
git add .
git commit -m "bitmap: initial implementation"
```

- [x] Закоммичено.
- [x] Запушено в origin/main.

**Финальный чекпоинт.**

---

## Если застряли

1. **Ошибка компиляции** - читайте **первую** ошибку.
2. **Тест падает** - добавьте `bm_print` в месте проверки.
3. **Segfault** - `gdb ./bitmap_test`, `bt`.
4. **Утечки** - `valgrind`.
5. **Непонятно** - уменьшите задачу. Один бит, одна операция.

## Когда возвращаться к наставнику

- После каждого чекпоинта.
- Когда `[!]` - застряли.
- Когда сомневаетесь в решении.
- Когда тесты проходят, но не понимаете почему.

## Что НЕ делать

- Не копировать готовые решения из интернета.
- Не использовать `std::bitset` - цель **своя** реализация.
- Не пропускать фазы.
- Не отчаиваться. Bitmap - **проще** `hashtable`. Справитесь.

---

## Итог

Проект завершён. Все 8 фаз пройдены:

| Фаза | Что | Статус |
|---|---|---|
| 0 | Подготовка (структура, CMake, файлы) | [x] |
| 1 | Структура и init/free | [x] |
| 2 | Базовые операции (set/clear/test/toggle) | [x] |
| 3 | Массовые операции (set_all/clear_all/print) | [x] |
| 4 | Popcount и find | [x] |
| 5 | Логические операции (and/or/xor/not/copy) | [x] |
| 6 | Диагностика (bm_check) | [x] |
| 7 | Стресс-тесты | [x] |
| 8 | Финализация (README, ASan, git) | [x] |

**Артефакты:**
- `include/bitmap/bitmap.hpp` - интерфейс.
- `src/bitmap.cpp` - реализация.
- `src/bitmap_test.cpp` - 13 секций тестов.
- `src/bitmap_demo.cpp` - CP+ disk block allocator demo.
- `tests/sieve/sieve.cpp` - решето Эратосфена на Bitmap (демонстрация).
- `README.md` - описание, API, сборка, тесты.
- `LICENSE` - MIT.

**Проверено:**
- Все тесты проходят.
- ASan/UBSan чисто (нет утечек, нет UB).
- Демо работает (аллокатор CP+ на 128 секторов).
- Git-история чистая, запушена в origin/main.

**Файл сохранён как есть** (Части A, B, C не удалены) - потому что:
- Часть A - справочник по C++, полезен вне проекта.
- Часть B - теория bitmap, полезна для будущих реализаций.
- Часть C - запись пройденного пути, важна как история решений.
