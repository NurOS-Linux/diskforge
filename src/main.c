#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <zstd.h>
#include <archive.h>
#include <archive_entry.h>
#include <sys/ioctl.h> // For terminal size
#include <math.h>       // For math functions
#include <getopt.h>

#define DEFAULT_BLOCK_SIZE (4 * 1024 * 1024) // 4MB - разумный компромисс
#define MAX_FILENAME_LENGTH 256
#define VERSION "1.0.2"

// ANSI color codes
#define RED "\x1B[31m"
#define GREEN "\x1B[32m"
#define YELLOW "\x1B[33m"
#define BLUE "\x1B[34m"
#define MAGENTA "\x1B[35m"
#define CYAN "\x1B[36m"
#define RESET "\x1B[0m"

// Структура для хранения параметров командной строки
typedef struct {
  char input_file[MAX_FILENAME_LENGTH];
  char output_file[MAX_FILENAME_LENGTH];
  size_t block_size;
  bool show_version;
  bool verify;
  bool compress_xz;
  bool compress_zst;
  bool to_image;
  bool from_image;
  bool sparse;
  bool show_help;
} Options;

// Функция для разбора аргументов командной строки
bool parse_arguments(int argc, char *argv[], Options *options);

// Функция для копирования данных с использованием указанного размера блока
int copy_data(const char *input_file,
              const char *output_file,
              size_t block_size,
              bool verify,
              bool sparse);

// Функция для проверки идентичности двух файлов
bool verify_copy(const char *original_file,
                 const char *copied_file,
                 size_t block_size);

// Функция для печати сообщения об использовании
void print_usage(const char *program_name);

// Функция для создания ISO образа
int create_iso(const char *input_file, const char *output_file);

// Функция для создания TAR архива (xz или zst)
int create_tar_archive(const char *input_file,
                       const char *output_file,
                       bool compress_xz,
                       bool compress_zst);

// Функция для восстановления данных из ISO/IMG/TAR архива
int restore_from_image(const char *input_file, const char *output_file);

// Функция для определения типа файла
const char *get_file_extension(const char *filename);

// Функция для получения размера файла
off_t get_file_size(const char *filename);

// Функция для отображения прогресса
void show_progress(off_t current, off_t total);

int main(int argc, char *argv[]) {
  Options options = {0};
  options.block_size = DEFAULT_BLOCK_SIZE; // Установка размера блока по умолчанию

  if (!parse_arguments(argc, argv, &options)) {
    return EXIT_FAILURE;
  }

  if (options.show_version) {
    printf("AnmiTali FOSP NurOS diskforge v%s\n", VERSION);
    printf("The MIT Licence\n");
    return EXIT_SUCCESS;
  }

  if (options.show_help) {
    print_usage(argv[0]);
    return EXIT_SUCCESS;
  }

  printf(CYAN "Input file: " RESET "%s\n", options.input_file);
  printf(CYAN "Output file: " RESET "%s\n", options.output_file);
  printf(CYAN "Block size: " RESET "%zu bytes\n", options.block_size);
  printf(CYAN "Verify: " RESET "%s\n", options.verify ? "yes" : "no");
  printf(CYAN "Compress XZ: " RESET "%s\n", options.compress_xz ? "yes" : "no");
  printf(CYAN "Compress ZST: " RESET "%s\n", options.compress_zst ? "yes" : "no");
  printf(CYAN "To Image: " RESET "%s\n", options.to_image ? "yes" : "no");
  printf(CYAN "From Image: " RESET "%s\n", options.from_image ? "yes" : "no");
  printf(CYAN "Sparse Copy: " RESET "%s\n", options.sparse ? "yes" : "no");

  int result = 0;

  if (options.to_image) {
    const char *output_ext = get_file_extension(options.output_file);
    if (strcmp(output_ext, "iso") == 0) {
      result = create_iso(options.input_file, options.output_file);
    } else if (strcmp(output_ext, "img") == 0) {
      result = copy_data(options.input_file, options.output_file,
                         options.block_size, options.verify, options.sparse);
    } else if (strcmp(output_ext, "tar.xz") == 0 ||
               strcmp(output_ext, "txz") == 0) {
      result = create_tar_archive(options.input_file, options.output_file,
                                  true, false);
    } else if (strcmp(output_ext, "tar.zst") == 0 ||
               strcmp(output_ext, "tzst") == 0) {
      result = create_tar_archive(options.input_file, options.output_file,
                                  false, true);
    } else {
      fprintf(stderr, RED "Unsupported image format: %s" RESET "\n",
              output_ext);
      result = EXIT_FAILURE;
    }
  } else if (options.from_image) {
    result = restore_from_image(options.input_file, options.output_file);
  } else {
    result = copy_data(options.input_file, options.output_file,
                       options.block_size, options.verify, options.sparse);

    if (result == 0 && options.verify) {
      if (verify_copy(options.input_file, options.output_file,
                      options.block_size)) {
        printf(GREEN "Copy and verification successful!" RESET "\n");
      } else {
        fprintf(stderr, RED "Verification failed!" RESET "\n");
        return EXIT_FAILURE;
      }
    }
  }

  if (result == 0) {
    printf(GREEN "Operation successful!" RESET "\n");
    return EXIT_SUCCESS;
  } else {
    fprintf(stderr, RED "Operation failed with error code %d" RESET "\n",
            result);
    return EXIT_FAILURE;
  }
}

bool parse_arguments(int argc, char *argv[], Options *options) {
  int opt;

  static struct option long_options[] = {
      {"version", no_argument, 0, 'v'}, // --version -> -v
      {"help", no_argument, 0, 'h'},    // --help -> -h
      {0, 0, 0, 0}};
  int option_index = 0;

  while ((opt = getopt_long(argc, argv, "i:o:b:vxtszfh", long_options,
                             &option_index)) != -1) {
    switch (opt) {
    case 'i':
      if (strlen(optarg) >= MAX_FILENAME_LENGTH) {
        fprintf(stderr, RED "Input filename too long." RESET "\n");
        print_usage(argv[0]);
        return false;
      }
      strncpy(options->input_file, optarg, MAX_FILENAME_LENGTH - 1);
      options->input_file[MAX_FILENAME_LENGTH - 1] = '\0';
      break;
    case 'o':
      if (strlen(optarg) >= MAX_FILENAME_LENGTH) {
        fprintf(stderr, RED "Output filename too long." RESET "\n");
        print_usage(argv[0]);
        return false;
      }
      strncpy(options->output_file, optarg, MAX_FILENAME_LENGTH - 1);
      options->output_file[MAX_FILENAME_LENGTH - 1] = '\0';
      break;
    case 'b': {
      char *endptr;
      unsigned long long block_size = strtoull(optarg, &endptr, 10);
      if (*endptr != '\0' || block_size == 0) {
        fprintf(stderr, RED "Invalid block size: %s" RESET "\n", optarg);
        print_usage(argv[0]);
        return false;
      }
      options->block_size = (size_t)block_size;
      break;
    }
    case 'v': // --version или -v
      options->show_version = true;
      break;
    case 'x':
      options->compress_xz = true;
      options->to_image = true;
      break;
    case 't':
      options->compress_zst = true;
      options->to_image = true;
      break;
    case 's':
      options->to_image = true;
      break;
    case 'z':
      options->from_image = true;
      break;
    case 'f':
      options->sparse = true;
      break;
    case 'h': // --help или -h
      options->show_help = true;
      break;
    case '?':
    default:
      print_usage(argv[0]);
      return false;
    }
  }

  if (strlen(options->input_file) == 0 && !options->show_version &&
      !options->show_help) {
    fprintf(stderr, RED "Input file must be specified unless --version or "
                        "--help is used." RESET "\n");
    print_usage(argv[0]);
    return false;
  }
  if (strlen(options->output_file) == 0 && !options->show_version &&
      !options->show_help) {
    fprintf(stderr, RED "Output file must be specified unless --version or "
                        "--help is used." RESET "\n");
    print_usage(argv[0]);
    return false;
  }

  return true;
}

int copy_data(const char *input_file,
              const char *output_file,
              size_t block_size,
              bool verify,
              bool sparse) {
  int input_fd = -1, output_fd = -1;
  void *buffer = NULL;
  ssize_t bytes_read, bytes_written;
  off_t total_bytes_written = 0;
  struct stat input_stat;
  (void)verify; // Suppress unused parameter warning

  input_fd = open(input_file, O_RDONLY);
  if (input_fd == -1) {
    perror(RED "Error opening input file" RESET);
    return errno;
  }

  if (fstat(input_fd, &input_stat) == -1) {
    perror(RED "Error getting input file size" RESET);
    close(input_fd);
    return errno;
  }

  output_fd = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, 0666);
  if (output_fd == -1) {
    perror(RED "Error opening output file" RESET);
    close(input_fd);
    return errno;
  }

  buffer = malloc(block_size);
  if (buffer == NULL) {
    fprintf(stderr, RED "Failed to allocate memory for buffer." RESET "\n");
    close(input_fd);
    close(output_fd);
    return ENOMEM;
  }

  off_t total_size = input_stat.st_size;
  off_t current_position = 0;

  while ((bytes_read = read(input_fd, buffer, block_size)) > 0) {
    if (sparse) {
      // Проверка на нулевой блок
      bool is_zero_block = true;
      for (ssize_t i = 0; i < bytes_read; ++i) {
        if (((unsigned char *)buffer)[i] != 0) {
          is_zero_block = false;
          break;
        }
      }

      if (is_zero_block) {
        // Пропустить запись, переместить указатель
        off_t seek_result = lseek(output_fd, bytes_read, SEEK_CUR);
        if (seek_result == -1) {
          perror(RED "Error seeking output file" RESET);
          free(buffer);
          close(input_fd);
          close(output_fd);
          return errno;
        }
        total_bytes_written += bytes_read;
        current_position += bytes_read;
        show_progress(current_position, total_size);
        continue;
      }
    }

    bytes_written = write(output_fd, buffer, bytes_read);
    if (bytes_written == -1) {
      perror(RED "Error writing to output file" RESET);
      free(buffer);
      close(input_fd);
      close(output_fd);
      return errno;
    }
    if (bytes_written != bytes_read) {
      fprintf(stderr, RED "Short write detected!" RESET "\n");
      free(buffer);
      close(input_fd);
      close(output_fd);
      return EIO;
    }
    total_bytes_written += bytes_written;
    current_position += bytes_written;
    show_progress(current_position, total_size);
  }

  if (bytes_read == -1) {
    perror(RED "Error reading from input file" RESET);
    free(buffer);
    close(input_fd);
    close(output_fd);
    return errno;
  }

  free(buffer);
  close(input_fd);

  // Truncate the file *before* closing the output file descriptor
  if (ftruncate(output_fd, total_bytes_written) == -1) {
    perror(RED "Error truncating output file" RESET);
    close(output_fd); // Ensure output_fd is closed even if truncate fails
    return errno;
  }

  close(output_fd);

  printf("\n"); // Newline after progress bar completes

  return 0;
}

bool verify_copy(const char *original_file,
                 const char *copied_file,
                 size_t block_size) {
  int original_fd = -1, copied_fd = -1;
  void *original_buffer = NULL, *copied_buffer = NULL;
  ssize_t original_bytes_read, copied_bytes_read;
  bool result = true;

  original_fd = open(original_file, O_RDONLY);
  if (original_fd == -1) {
    perror(RED "Error opening original file for verification" RESET);
    return false;
  }

  copied_fd = open(copied_file, O_RDONLY);
  if (copied_fd == -1) {
    perror(RED "Error opening copied file for verification" RESET);
    close(original_fd);
    return false;
  }

  original_buffer = malloc(block_size);
  copied_buffer = malloc(block_size);

  if (!original_buffer || !copied_buffer) {
    fprintf(stderr, RED "Failed to allocate memory for verification buffers." RESET
                       "\n");
    if (original_buffer)
      free(original_buffer);
    if (copied_buffer)
      free(copied_buffer);
    close(original_fd);
    close(copied_fd);
    return false;
  }

  do {
    original_bytes_read = read(original_fd, original_buffer, block_size);
    copied_bytes_read = read(copied_fd, copied_buffer, block_size);

    if (original_bytes_read == -1) {
      perror(RED "Error reading original file for verification" RESET);
      result = false;
      break;
    }

    if (copied_bytes_read == -1) {
      perror(RED "Error reading copied file for verification" RESET);
      result = false;
      break;
    }

    if (original_bytes_read != copied_bytes_read) {
      fprintf(stderr, RED "Files have different sizes." RESET "\n");
      result = false;
      break;
    }

    if (memcmp(original_buffer, copied_buffer, original_bytes_read) != 0) {
      fprintf(stderr, RED "Files content differs." RESET "\n");
      result = false;
      break;
    }
  } while (original_bytes_read > 0);

  free(original_buffer);
  free(copied_buffer);
  close(original_fd);
  close(copied_fd);
  return result;
}

void print_usage(const char *program_name) {
  fprintf(stderr, YELLOW "Usage: %s [options]\n" RESET, program_name);
  fprintf(stderr, "  -i input_file:  The input file to copy from.\n");
  fprintf(stderr, "  -o output_file: The output file to copy to.\n");
  fprintf(stderr,
          "  -b block_size:  The block size to use for copying (default: "
          "%d).\n",
          DEFAULT_BLOCK_SIZE);
  fprintf(stderr, "  -v:             Show version and exit.\n");
  fprintf(stderr, "  -s:             Create an image from a disk (sparse).\n");
  fprintf(stderr, "  -z:             Restore an image to a disk.\n");
  fprintf(stderr, "  -x:             Create a tar.xz archive from a disk.\n");
  fprintf(stderr, "  -t:             Create a tar.zst archive from a disk.\n");
  fprintf(stderr, "  -f:             Enable sparse copy (skip zero blocks).\n");
  fprintf(stderr, "  -h:             Show help and exit.\n");
  fprintf(stderr, YELLOW "Examples:\n" RESET);
  fprintf(stderr,
          "  %s -i /dev/sda -o disk.img  (Create a full disk image)\n",
          program_name);
  fprintf(stderr,
          "  %s -i /dev/sda -o disk.img -f (Create a sparse disk image)\n",
          program_name);
  fprintf(stderr,
          "  %s -i disk.img -o /dev/sdb  (Restore a disk image)\n",
          program_name);
  fprintf(stderr,
          "  %s -i /dev/sda -o disk.tar.xz -s (Create a tar.xz archive)\n",
          program_name);
  fprintf(stderr,
          "  %s -i /dev/sda -o disk.tar.zst -s (Create a tar.zst archive)\n",
          program_name);
}

const char *get_file_extension(const char *filename) {
  const char *dot = strrchr(filename, '.');
  if (!dot || dot == filename)
    return "";
  return dot + 1;
}

off_t get_file_size(const char *filename) {
  struct stat st;
  if (stat(filename, &st) == 0)
    return st.st_size;
  return -1;
}

int create_iso(const char *input_file, const char *output_file) {
  (void)input_file;
  (void)output_file;
  fprintf(stderr, RED "ISO creation is not yet implemented." RESET "\n");
  return -1;
}

int create_tar_archive(const char *input_file,
                       const char *output_file,
                       bool compress_xz,
                       bool compress_zst) {
  struct archive *archive;
  struct archive_entry *entry;
  int fd;
  char buf[8192];
  ssize_t len;
  int r;
  struct stat st;

  if (stat(input_file, &st) != 0) {
    perror(RED "stat" RESET);
    return -1;
  }

  archive = archive_write_new();
  archive_write_set_format_pax_restricted(archive);

  if (compress_xz) {
    archive_write_add_filter_xz(archive);
  } else if (compress_zst) {
    archive_write_add_filter_zstd(archive);
  }

  archive_write_open_filename(archive, output_file);

  entry = archive_entry_new();
  archive_entry_set_pathname(entry, input_file);
  archive_entry_set_size(entry, st.st_size);
  archive_entry_set_filetype(entry, AE_IFREG);
  archive_entry_set_perm(entry, 0644);
  archive_entry_set_mtime(entry, time(NULL), 0);

  archive_write_header(archive, entry);

  fd = open(input_file, O_RDONLY);
  if (fd < 0) {
    perror(RED "open" RESET);
    archive_entry_free(entry);
    archive_write_free(archive);
    return -1;
  }

  while ((len = read(fd, buf, sizeof(buf))) > 0) {
    r = archive_write_data(archive, buf, len);
    if (r < ARCHIVE_OK) {
      fprintf(stderr, RED "%s" RESET "\n", archive_error_string(archive));
      break;
    }
  }

  if (len < 0) {
    perror(RED "read" RESET);
  }

  close(fd);
  archive_write_close(archive);
  archive_write_free(archive);
  archive_entry_free(entry);

  return 0;
}

int restore_from_image(const char *input_file, const char *output_file) {
  (void)input_file;
  (void)output_file;
  fprintf(stderr, RED "Restoring from image is not yet implemented." RESET "\n");
  return -1;
}

void show_progress(off_t current, off_t total) {
  int bar_width = 50;
  double progress = (double)current / (double)total;
  int pos = bar_width * progress;

  printf("\r" BLUE "[");
  for (int i = 0; i < bar_width; ++i) {
    if (i < pos)
      printf("=");
    else if (i == pos)
      printf(">");
    else
      printf(" ");
  }
  printf("]" RESET " %3d%% (%ld/%ld)", (int)(progress * 100.0), (long)current,
         (long)total);
  fflush(stdout);
}