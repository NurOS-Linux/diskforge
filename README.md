# diskforge

**diskforge** - это утилита командной строки для создания образов дисков, резервного копирования и восстановления. Она поддерживает разреженное копирование, сжатие и другие функции.

## Особенности

*   Копирование дисков и разделов
*   Создание образов дисков (полных и разреженных)
*   Восстановление образов дисков
*   Сжатие образов (tar.xz, tar.zst)
*   Верификация копий
*   Поддержка больших файлов
*   Разреженное копирование (пропуск нулевых блоков)

## Сборка

Для сборки diskforge требуется компилятор C (например, GCC или Clang) и библиотеки zstd, xz и archive.

1.  Клонируйте репозиторий:

    ```bash
    git clone <repository_url>
    cd diskforge
    ```

2.  Соберите проект:

    ```bash
    ./build.sh
    ```

    Это создаст исполняемый файл `diskforge` в директории `build/dist`.

## Использование

```
Usage: diskforge [options]
  -i input_file:  The input file to copy from.
  -o output_file: The output file to copy to.
  -b block_size:  The block size to use for copying (default: 4194304).
  -v:             Show version and exit.
  -s:             Create an image from a disk (sparse).
  -z:             Restore an image to a disk.
  -x:             Create a tar.xz archive from a disk.
  -t:             Create a tar.zst archive from a disk.
  -f:             Enable sparse copy (skip zero blocks).
  -h:             Show help and exit.
Examples:
  diskforge -i /dev/sda -o disk.img  (Create a full disk image)
  diskforge -i /dev/sda -o disk.img -f (Create a sparse disk image)
  diskforge -i disk.img -o /dev/sdb  (Restore a disk image)
  diskforge -i /dev/sda -o disk.tar.xz -s (Create a tar.xz archive)
  diskforge -i /dev/sda -o disk.tar.zst -s (Create a tar.zst archive)
```

### Опции

*   `-i input_file`:  Путь к входному файлу (диску, разделу, образу).
*   `-o output_file`: Путь к выходному файлу (образу, диску, разделу).
*   `-b block_size`: Размер блока для копирования (в байтах).  По умолчанию: 4194304 (4MB).
*   `-v`: Показать информацию о версии и выйти.
*   `-s`: Создать образ диска (разреженный).
*   `-z`: Восстановить образ диска.
*   `-x`: Создать `tar.xz` архив.
*   `-t`: Создать `tar.zst` архив.
*   `-f`: Включить разреженное копирование (пропускать нулевые блоки).
*   `-h`: Показать справку и выйти.

### Примеры

*   Создание полного образа диска:

    ```bash
    diskforge -i /dev/sda -o disk.img
    ```

*   Создание разреженного образа диска:

    ```bash
    diskforge -i /dev/sda -o disk.img -f
    ```

*   Восстановление образа диска:

    ```bash
    diskforge -i disk.img -o /dev/sdb
    ```

*   Создание `tar.xz` архива:

    ```bash
    diskforge -i /dev/sda -o disk.tar.xz -s
    ```

*   Создание `tar.zst` архива:

    ```bash
    diskforge -i /dev/sda -o disk.tar.zst -s
    ```

## Лицензия

diskforge распространяется под лицензией MIT. См. файл [LICENSE](LICENSE) для получения дополнительной информации.

## Автор

AnmiTaliDev

## FOSP NurOS

diskforge является частью проекта FOSP NurOS.

## Версия

1.0.2