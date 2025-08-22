# diskforge

**diskforge** is a command-line utility for creating disk images, backing up, and restoring. It supports sparse copying, compression, and other features.

## Features

*   Copying disks and partitions
*   Creating disk images (full and sparse)
*   Restoring disk images
*   Compressing images (tar.xz, tar.zst)
*   Verifying copies
*   Large file support
*   Sparse copying (skip zero blocks)

## Building

To build diskforge, you need a C compiler (e.g., GCC or Clang) and the zstd, xz, and archive libraries.

1.  Clone the repository:

    ```bash
    git clone <repository_url>
    cd diskforge
    ```

2.  Build the project:

    ```bash
    ./build.sh
    ```

    This will create the `diskforge` executable in the `build/dist` directory.

## Usage

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

### Options

*   `-i input_file`:  Path to the input file (disk, partition, or image).
*   `-o output_file`: Path to the output file (image, disk, or partition).
*   `-b block_size`: Block size for copying (in bytes). Default: 4194304 (4MB).
*   `-v`: Show version information and exit.
*   `-s`: Create a disk image (sparse).
*   `-z`: Restore a disk image.
*   `-x`: Create a `tar.xz` archive.
*   `-t`: Create a `tar.zst` archive.
*   `-f`: Enable sparse copying (skip zero blocks).
*   `-h`: Show help and exit.

### Examples

*   Create a full disk image:

    ```bash
    diskforge -i /dev/sda -o disk.img
    ```

*   Create a sparse disk image:

    ```bash
    diskforge -i /dev/sda -o disk.img -f
    ```

*   Restore a disk image:

    ```bash
    diskforge -i disk.img -o /dev/sdb
    ```

*   Create a `tar.xz` archive:

    ```bash
    diskforge -i /dev/sda -o disk.tar.xz -s
    ```

*   Create a `tar.zst` archive:

    ```bash
    diskforge -i /dev/sda -o disk.tar.zst -s
    ```

## License

diskforge is released under the MIT license. See the [LICENSE](LICENSE) file for more information.

## Author

AnmiTaliDev

## FOSP NurOS

diskforge is part of the FOSP NurOS project.

## Version

1.0.2
