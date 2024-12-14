# `DAW Integer` - Safer C++ Integers

[![License: Boost](https://img.shields.io/badge/License-Boost%201.0-blue.svg)](https://www.boost.org/LICENSE_1_0.txt)

`daw_integer` is a lightweight library that provides safer integer operations for C++ developers. It helps prevent common issues such as integer overflows, underflows, and undefined behavior, ensuring safer arithmetic operations and improving code reliability.

## Table of Contents

- [Features](#features)
- [Usage](#usage)
- [Example](#example)
- [License](#license)
- [Contributing](#contributing)

---

## Features

- **Safer Integer Operations**: Helps prevent overflow and underflow errors.
- **Customizable Behavior**: Options to handle errors gracefully or enforce strict checks.
- **Lightweight**: Minimal overhead for safety enhancements.
- **C++17 and Later**: Leverages modern C++ standards for clean and efficient code.

---

## Usage 
To use daw_integer, simply include the header in your project:

```cpp
#include <daw/integers/daw_signed.h>
```
You can then replace standard integer types with safer versions provided by daw_integer such as `daw::i8`, `daw::i16`, `daw::i32`, and `daw::i64`.

 
