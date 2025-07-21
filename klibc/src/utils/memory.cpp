#include "memory.hpp"

namespace internal {
void copy_forward_aligned(uintptr_t dest, uintptr_t src, size_t len) {
  qword_t a, b;

  switch (len % 8) {
    case 0:
      if ((threshold <= 3 * sizeof(qword_t)) && (len == 0)) {
        return;
      }

      a = reinterpret_cast<qword_t *>(src)[0];
      dest -= sizeof(qword_t);

      goto task0;
    case 1:
      b = reinterpret_cast<qword_t *>(src)[0];
      src += sizeof(qword_t);
      len--;

      if ((threshold <= 3 * sizeof(qword_t)) && (len == 0)) {
        goto exit;
      }

      goto task1;
    case 2:
      a = reinterpret_cast<qword_t *>(src)[0];
      src -= 6 * sizeof(qword_t);
      dest -= 7 * sizeof(qword_t);
      len += 6;

      goto task2;
    case 3:
      b = reinterpret_cast<qword_t *>(src)[0];
      src -= 5 * sizeof(qword_t);
      dest -= 6 * sizeof(qword_t);
      len += 5;

      goto task3;
    case 4:
      a = reinterpret_cast<qword_t *>(src)[0];
      src -= 4 * sizeof(qword_t);
      dest -= 5 * sizeof(qword_t);
      len += 4;

      goto task4;
    case 5:
      b = reinterpret_cast<qword_t *>(src)[0];
      src -= 3 * sizeof(qword_t);
      dest -= 4 * sizeof(qword_t);
      len += 3;

      goto task5;
    case 6:
      a = reinterpret_cast<qword_t *>(src)[0];
      src -= 2 * sizeof(qword_t);
      dest -= 3 * sizeof(qword_t);
      len += 2;

      goto task6;
    case 7:
      b = reinterpret_cast<qword_t *>(src)[0];
      src -= sizeof(qword_t);
      dest -= 2 * sizeof(qword_t);
      len++;

      goto task7;
  }

  do {
  task1:
    a = reinterpret_cast<qword_t *>(src)[0];
    reinterpret_cast<qword_t *>(dest)[0] = b;
  task0:
    b = reinterpret_cast<qword_t *>(src)[1];
    reinterpret_cast<qword_t *>(dest)[1] = a;
  task7:
    a = reinterpret_cast<qword_t *>(src)[2];
    reinterpret_cast<qword_t *>(dest)[2] = b;
  task6:
    b = reinterpret_cast<qword_t *>(src)[3];
    reinterpret_cast<qword_t *>(dest)[3] = a;
  task5:
    a = reinterpret_cast<qword_t *>(src)[4];
    reinterpret_cast<qword_t *>(dest)[4] = b;
  task4:
    b = reinterpret_cast<qword_t *>(src)[5];
    reinterpret_cast<qword_t *>(dest)[5] = a;
  task3:
    a = reinterpret_cast<qword_t *>(src)[6];
    reinterpret_cast<qword_t *>(dest)[6] = b;
  task2:
    b = reinterpret_cast<qword_t *>(src)[7];
    reinterpret_cast<qword_t *>(dest)[7] = a;

    src += 8 * sizeof(qword_t);
    dest += 8 * sizeof(qword_t);
    len -= 8;
  } while (len != 0);

exit:
  reinterpret_cast<qword_t *>(dest)[0] = b;
}

void copy_forward_dest_aligned(uintptr_t dest, uintptr_t src, size_t len) {
  qword_t a, b, c, d;

  int shift_1 = 8 * (src % sizeof(qword_t));
  int shift_2 = 8 * sizeof(qword_t) - shift_1;

  src &= -sizeof(qword_t);

  switch (len % 4) {
    case 0:
      if ((threshold <= 3 * sizeof(qword_t)) && (len == 0)) {
        return;
      }

      d = reinterpret_cast<qword_t *>(src)[0];
      a = reinterpret_cast<qword_t *>(src)[1];
      src += sizeof(qword_t);
      dest -= sizeof(qword_t);

      goto task0;
    case 1:
      c = reinterpret_cast<qword_t *>(src)[0];
      d = reinterpret_cast<qword_t *>(src)[1];
      src += 2 * sizeof(qword_t);
      len--;

      if ((threshold <= 3 * sizeof(qword_t)) && (len == 0)) {
        goto exit;
      }

      goto task1;
    case 2:
      b = reinterpret_cast<qword_t *>(src)[0];
      c = reinterpret_cast<qword_t *>(src)[1];
      src -= sizeof(qword_t);
      dest -= 3 * sizeof(qword_t);
      len += 2;

      goto task2;
    case 3:
      a = reinterpret_cast<qword_t *>(src)[0];
      b = reinterpret_cast<qword_t *>(src)[1];
      dest -= 2 * sizeof(qword_t);
      len++;

      goto task3;
  }

  do {
  task1:
    a = reinterpret_cast<qword_t *>(src)[0];
    reinterpret_cast<qword_t *>(dest)[0] = merge(c, shift_1, d, shift_2);
  task0:
    b = reinterpret_cast<qword_t *>(src)[1];
    reinterpret_cast<qword_t *>(dest)[1] = merge(d, shift_1, a, shift_2);
  task3:
    c = reinterpret_cast<qword_t *>(src)[2];
    reinterpret_cast<qword_t *>(dest)[2] = merge(a, shift_1, b, shift_2);
  task2:
    d = reinterpret_cast<qword_t *>(src)[3];
    reinterpret_cast<qword_t *>(dest)[3] = merge(b, shift_1, c, shift_2);

    src += 4 * sizeof(qword_t);
    dest += 4 * sizeof(qword_t);
    len -= 4;
  } while (len != 0);

exit:
  reinterpret_cast<qword_t *>(dest)[0] = merge(c, shift_1, d, shift_2);
}

void copy_backward_aligned(uintptr_t dest, uintptr_t src, size_t len) {
  qword_t a, b;

  switch (len % 8) {
    case 0:
      if ((threshold <= 3 * sizeof(qword_t)) && (len == 0)) {
        return;
      }

      src -= 8 * sizeof(qword_t);
      dest -= 7 * sizeof(qword_t);
      a = reinterpret_cast<qword_t *>(src)[7];

      goto task0;
    case 1:
      src -= 9 * sizeof(qword_t);
      dest -= 8 * sizeof(qword_t);
      b = reinterpret_cast<qword_t *>(src)[8];
      len--;

      if ((threshold <= 3 * sizeof(qword_t)) && (len == 0)) {
        goto exit;
      }

      goto task1;
    case 2:
      src -= 2 * sizeof(qword_t);
      dest -= sizeof(qword_t);
      a = reinterpret_cast<qword_t *>(src)[1];
      len += 6;

      goto task2;
    case 3:
      src -= 3 * sizeof(qword_t);
      dest -= 2 * sizeof(qword_t);
      b = reinterpret_cast<qword_t *>(src)[2];
      len += 5;

      goto task3;
    case 4:
      src -= 4 * sizeof(qword_t);
      dest -= 3 * sizeof(qword_t);
      a = reinterpret_cast<qword_t *>(src)[3];
      len += 4;

      goto task4;
    case 5:
      src -= 5 * sizeof(qword_t);
      dest -= 4 * sizeof(qword_t);
      b = reinterpret_cast<qword_t *>(src)[4];
      len += 3;

      goto task5;
    case 6:
      src -= 6 * sizeof(qword_t);
      dest -= 5 * sizeof(qword_t);
      a = reinterpret_cast<qword_t *>(src)[5];
      len += 2;

      goto task6;
    case 7:
      src -= 7 * sizeof(qword_t);
      dest -= 6 * sizeof(qword_t);
      b = reinterpret_cast<qword_t *>(src)[6];
      len++;

      goto task7;
  }

  do {
  task1:
    a = reinterpret_cast<qword_t *>(src)[7];
    reinterpret_cast<qword_t *>(dest)[7] = b;
  task0:
    b = reinterpret_cast<qword_t *>(src)[6];
    reinterpret_cast<qword_t *>(dest)[6] = a;
  task7:
    a = reinterpret_cast<qword_t *>(src)[5];
    reinterpret_cast<qword_t *>(dest)[5] = b;
  task6:
    b = reinterpret_cast<qword_t *>(src)[4];
    reinterpret_cast<qword_t *>(dest)[4] = a;
  task5:
    a = reinterpret_cast<qword_t *>(src)[3];
    reinterpret_cast<qword_t *>(dest)[3] = b;
  task4:
    b = reinterpret_cast<qword_t *>(src)[2];
    reinterpret_cast<qword_t *>(dest)[2] = a;
  task3:
    a = reinterpret_cast<qword_t *>(src)[1];
    reinterpret_cast<qword_t *>(dest)[1] = b;
  task2:
    b = reinterpret_cast<qword_t *>(src)[0];
    reinterpret_cast<qword_t *>(dest)[0] = a;

    src -= 8 * sizeof(qword_t);
    dest -= 8 * sizeof(qword_t);
    len -= 8;
  } while (len != 0);

exit:
  reinterpret_cast<qword_t *>(dest)[7] = b;
}

void copy_backward_dest_aligned(uintptr_t dest, uintptr_t src, size_t len) {
  qword_t a, b, c, d;

  int shift_1 = 8 * (src % sizeof(qword_t));
  int shift_2 = 8 * sizeof(qword_t) - shift_1;

  src &= -sizeof(qword_t);
  src += sizeof(qword_t);

  switch (len % 4) {
    case 0:
      if ((threshold <= 3 * sizeof(qword_t)) && (len == 0)) {
        return;
      }

      src -= 5 * sizeof(qword_t);
      dest -= 3 * sizeof(qword_t);
      a = reinterpret_cast<qword_t *>(src)[4];
      d = reinterpret_cast<qword_t *>(src)[3];

      goto task0;
    case 1:
      src -= 6 * sizeof(qword_t);
      dest -= 4 * sizeof(qword_t);
      b = reinterpret_cast<qword_t *>(src)[5];
      a = reinterpret_cast<qword_t *>(src)[4];
      len--;

      if ((threshold <= 3 * sizeof(qword_t)) && (len == 0)) {
        goto exit;
      }

      goto task1;
    case 2:
      src -= 3 * sizeof(qword_t);
      dest -= sizeof(qword_t);
      c = reinterpret_cast<qword_t *>(src)[2];
      b = reinterpret_cast<qword_t *>(src)[1];
      len += 2;

      goto task2;
    case 3:
      src -= 4 * sizeof(qword_t);
      dest -= 2 * sizeof(qword_t);
      d = reinterpret_cast<qword_t *>(src)[3];
      c = reinterpret_cast<qword_t *>(src)[2];
      len++;

      goto task3;
  }

  do {
  task1:
    d = reinterpret_cast<qword_t *>(src)[3];
    reinterpret_cast<qword_t *>(dest)[3] = merge(a, shift_1, b, shift_2);
  task0:
    c = reinterpret_cast<qword_t *>(src)[2];
    reinterpret_cast<qword_t *>(dest)[2] = merge(d, shift_1, a, shift_2);
  task3:
    b = reinterpret_cast<qword_t *>(src)[1];
    reinterpret_cast<qword_t *>(dest)[1] = merge(c, shift_1, d, shift_2);
  task2:
    a = reinterpret_cast<qword_t *>(src)[0];
    reinterpret_cast<qword_t *>(dest)[0] = merge(b, shift_1, c, shift_2);

    src -= 4 * sizeof(qword_t);
    dest -= 4 * sizeof(qword_t);
    len -= 4;
  } while (len != 0);

exit:
  reinterpret_cast<qword_t *>(dest)[3] = merge(a, shift_1, b, shift_2);
}
}  // namespace string