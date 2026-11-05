/*
 * legacy_result_parser.h
 *
 * Parser for the result lines sent by the analyzer firmware over the serial link.
 * Ported from the old acquisition daemon (2011), kept as is because the
 * firmware protocol has not changed since.
 *
 * DO NOT CHANGE THE STRUCT LAYOUT - the old daemon dumped it to disk.
 */
#ifndef LEGACY_RESULT_PARSER_H
#define LEGACY_RESULT_PARSER_H

#define LR_MAXFLD 32
#define LR_MAXMSG 64

#define LR_OK 1
#define LR_SKIP 0
#define LR_E_EMPTY (-1)
#define LR_E_TYPE (-2)
#define LR_E_NFIELDS (-3)
#define LR_E_POS (-4)
#define LR_E_SIGNAL (-5)
#define LR_E_FLAGS (-6)

#define LR_F_HEM 0x10
#define LR_F_CLOT 0x20
#define LR_F_ERR 0x08

typedef struct {
  char type; /* 'R' result, 'C' control, 'E' error */
  int pos;
  char sid[LR_MAXFLD];
  char assay[8];
  double signal;
  unsigned int flags;
  int errcode;
  char msg[LR_MAXMSG];
  int fw; /* 1 or 2 */
} LEGACY_RESULT;

int lr_parse(const char *line, LEGACY_RESULT *r);
const char *lr_errstr(int code);

#endif
