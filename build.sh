#!/bin/bash

# Скрипт для сборки diskforge

# Выходим при ошибке
set -e

# Каталоги
SRC_DIR="src"
BUILD_DIR="build"
DIST_DIR="$BUILD_DIR/dist"

# Имя исполняемого файла
EXECUTABLE="diskforge"

# Компилятор
COMPILER="clang"

# Флаги компиляции
CFLAGS="-Wall -Wextra -O3 -march=native -flto"  # Оптимизация и LTO
LDFLAGS="-lzstd -larchive -lm"  # Подключаем библиотеки zstd, archive и math

# Создаем каталоги
mkdir -p "$DIST_DIR"

# Компиляция
echo "Компиляция..."
$COMPILER $CFLAGS "$SRC_DIR/main.c" -o "$DIST_DIR/$EXECUTABLE" $LDFLAGS

echo "Сборка завершена. Исполняемый файл: $DIST_DIR/$EXECUTABLE"