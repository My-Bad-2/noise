#ifndef CTYPE_H
#define CTYPE_H 1

#ifdef __cplusplus
extern "C" {
#endif

int isalnum(int ch);
int isalpha(int ch);
int isblank(int ch);
int iscntrl(int ch);
int isdigit(int ch);
int isgraph(int ch);
int islower(int ch);
int isprint(int ch);
int ispunct(int ch);
int isspace(int ch);
int isupper(int ch);
int isxdigit(int ch);

int tolower(int ch);
int toupper(int ch);
int toascii(int ch);

#ifdef __cplusplus
}
#endif

#endif  // CTYPE_H
