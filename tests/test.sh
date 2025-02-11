#!/bin/bash

# Выходим при ошибке
set -e

# Каталоги
BUILD_DIR="build"
DIST_DIR="$BUILD_DIR/dist"
EXECUTABLE="$DIST_DIR/diskforge"
TEST_DIR="tests"

# Файлы
INPUT_FILE="$TEST_DIR/input.txt"
OUTPUT_FILE="$TEST_DIR/output.img"
REF_FILE="$TEST_DIR/ref.txt"

# Создаем тестовые данные
echo "Создание тестовых данных..."
head -c 10M </dev/urandom > "$INPUT_FILE"
echo "Reference data" > "$REF_FILE"

# Функция для сравнения файлов
compare_files() {
  file1="$1"
  file2="$2"
  if cmp -s "$file1" "$file2"; then
    echo "  [OK] Files $file1 and $file2 are identical."
    return 0
  else
    echo "  [FAIL] Files $file1 and $file2 differ."
    return 1
  fi
}

# Функция для запуска теста
run_test() {
  test_name="$1"
  command="$2"
  expected_result="$3"

  echo "Running test: $test_name"
  result=$($command)
  actual_result=$?

  echo "  Command: $command"
  echo "  Expected result: $expected_result"
  echo "  Actual result: $actual_result"

  if [ "$actual_result" -eq "$expected_result" ]; then
    echo "  [OK] Test passed."
  else
    echo "  [FAIL] Test failed."
    echo "  Output: $result"
    return 1
  fi

  return 0
}

# Тесты

# Тест 1: Копирование файла
run_test "Copy file" "$EXECUTABLE -i $INPUT_FILE -o $OUTPUT_FILE" 0
compare_files "$INPUT_FILE" "$OUTPUT_FILE"

# Тест 2: Копирование файла с проверкой
run_test "Copy file with verification" "$EXECUTABLE -i $INPUT_FILE -o $OUTPUT_FILE -V" 0  # Используем другую опцию для верификации, например -V
compare_files "$INPUT_FILE" "$OUTPUT_FILE"

# Тест 3: Отображение версии
run_test "Show version" "$EXECUTABLE --version" 0

# Тест 4: Отображение help
run_test "Show help" "$EXECUTABLE --help" 0

# Тест 5: Неверные аргументы
run_test "Invalid arguments" "$EXECUTABLE -i" 1

# Тест 6: Разреженное копирование (создаем файл с нулями)
dd if=/dev/zero of="$TEST_DIR/sparse_input.img" bs=1M count=10
dd if="$INPUT_FILE" of="$TEST_DIR/sparse_input.img" bs=1M seek=5 count=5 conv=notrunc
run_test "Sparse copy" "$EXECUTABLE -i $TEST_DIR/sparse_input.img -o $OUTPUT_FILE -f" 0

# Тест 7: Создание tar.xz архива
run_test "Create tar.xz archive" "$EXECUTABLE -i $INPUT_FILE -o $TEST_DIR/archive.tar.xz -s" 0

# Тест 8: Создание tar.zst архива
run_test "Create tar.zst archive" "$EXECUTABLE -i $INPUT_FILE -o $TEST_DIR/archive.tar.zst -s" 0

# Очистка
echo "Очистка..."
rm -f "$INPUT_FILE" "$OUTPUT_FILE" "$TEST_DIR/sparse_input.img" "$TEST_DIR/archive.tar.xz" "$TEST_DIR/archive.tar.zst"

echo "Все тесты завершены."