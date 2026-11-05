/*
 * legacy_result_parser.cpp - see header.
 *
 * Firmware v2 lines:
 *   R|pos|sample|assay|signal|flags|timestamp
 *   C|pos|control lot|assay|signal|flags|timestamp
 *   E|pos|sample|code|message
 *   H|...   heartbeat, ignored
 * Firmware v1 lines (still sent by old units in the field):
 *   R;pos;sample;assay;signal
 */
#include "legacy_result_parser.h"

#include <stdlib.h>
#include <string.h>
#include <ctype.h>

static char g_fld[10][80];
static int g_nfld;

static int split(const char *s, char d) {
  int i = 0, j = 0;
  g_nfld = 0;
  memset(g_fld, 0, sizeof(g_fld));
  while (s[i] != 0 && s[i] != '\n' && s[i] != '\r') {
    if (s[i] == d) {
      g_nfld++;
      j = 0;
      if (g_nfld >= 10) return -1;
    } else {
      if (j < 79) {
        g_fld[g_nfld][j] = s[i];
        j++;
      }
    }
    i++;
  }
  g_nfld++;
  return g_nfld;
}

static void trim(char *s) {
  int n = (int)strlen(s);
  while (n > 0 && (s[n - 1] == ' ' || s[n - 1] == '\t')) {
    s[n - 1] = 0;
    n--;
  }
  int k = 0;
  while (s[k] == ' ' || s[k] == '\t') k++;
  if (k > 0) memmove(s, s + k, strlen(s + k) + 1);
}

int lr_parse(const char *line, LEGACY_RESULT *r) {
  int n, i, fw = 2, ok;
  char d = '|';
  char *e;
  if (line == 0 || r == 0) return LR_E_EMPTY;
  memset(r, 0, sizeof(LEGACY_RESULT));
  if (strlen(line) < 2) return LR_E_EMPTY;
  if (line[0] == 'H') return LR_SKIP;
  if (line[0] == '#') return LR_SKIP;
  if (line[1] == ';') {
    d = ';';
    fw = 1;
  } else if (line[1] != '|') {
    return LR_E_TYPE;
  }
  n = split(line, d);
  if (n < 0) return LR_E_NFIELDS;
  for (i = 0; i < n; i++) trim(g_fld[i]);
  r->fw = fw;
  if (g_fld[0][0] == 'R' || g_fld[0][0] == 'C') {
    if (fw == 1) {
      if (n != 5) return LR_E_NFIELDS;
      if (g_fld[0][0] == 'C') return LR_E_TYPE; /* v1 had no controls */
    } else {
      if (n != 7) return LR_E_NFIELDS;
    }
    r->type = g_fld[0][0];
    ok = 1;
    for (i = 0; g_fld[1][i]; i++)
      if (!isdigit((unsigned char)g_fld[1][i])) ok = 0;
    if (!ok || strlen(g_fld[1]) == 0) return LR_E_POS;
    r->pos = atoi(g_fld[1]);
    if (r->pos < 1 || r->pos > 999) return LR_E_POS;
    strncpy(r->sid, g_fld[2], LR_MAXFLD - 1);
    strncpy(r->assay, g_fld[3], 7);
    if (strlen(g_fld[4]) == 0) return LR_E_SIGNAL;
    r->signal = strtod(g_fld[4], &e);
    if (*e != 0) return LR_E_SIGNAL;
    if (r->signal < 0) return LR_E_SIGNAL;
    if (fw == 2) {
      /* flags: "-" or list like HEM;CLOT */
      if (strcmp(g_fld[5], "-") != 0 && strlen(g_fld[5]) > 0) {
        char tmp[80];
        char *p, *q;
        strncpy(tmp, g_fld[5], 79);
        tmp[79] = 0;
        p = tmp;
        while (p != 0 && *p) {
          q = strchr(p, ';');
          if (q) *q = 0;
          if (strcmp(p, "HEM") == 0) r->flags |= LR_F_HEM;
          else if (strcmp(p, "CLOT") == 0) r->flags |= LR_F_CLOT;
          else if (strcmp(p, "ERR") == 0) r->flags |= LR_F_ERR;
          else if (strcmp(p, "OK") == 0) { /* old firmware sends OK, ignore */ }
          else return LR_E_FLAGS;
          if (q) p = q + 1; else p = 0;
        }
      }
    }
    return LR_OK;
  } else if (g_fld[0][0] == 'E') {
    if (fw == 1) return LR_E_TYPE;
    if (n < 5) return LR_E_NFIELDS;
    r->type = 'E';
    r->pos = atoi(g_fld[1]);
    strncpy(r->sid, g_fld[2], LR_MAXFLD - 1);
    r->errcode = atoi(g_fld[3]);
    strncpy(r->msg, g_fld[4], LR_MAXMSG - 1);
    r->flags = LR_F_ERR;
    return LR_OK;
  }
  return LR_E_TYPE;
}

const char *lr_errstr(int code) {
  switch (code) {
    case LR_OK: return "ok";
    case LR_SKIP: return "skipped";
    case LR_E_EMPTY: return "empty line";
    case LR_E_TYPE: return "unknown record type";
    case LR_E_NFIELDS: return "wrong number of fields";
    case LR_E_POS: return "bad rack position";
    case LR_E_SIGNAL: return "bad signal value";
    case LR_E_FLAGS: return "unknown flag";
  }
  return "unknown error";
}
