/* lsdtrace.c - list available probes on system.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 * 
 * Copyright (c) 2015 Andrew Griffiths
*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <libgen.h>
#include <ctype.h>
#include <getopt.h>
#include <fnmatch.h>
#include <errno.h>
#include <string.h>

#include <dtrace.h>

typedef struct {
  char *provider;
  char *mod;
  char *func;
  char *name;
} probefilter_t;

/* callback for dtrace iterator. optionally filters then dumps probe info */
static int
probeinfo(dtrace_hdl_t *dh, const dtrace_probedesc_t *desc, void *ud)
{
  probefilter_t *filter = ud;

#define FILTER_OUT(F, V) (F && fnmatch(F, V, 0))
  if (FILTER_OUT(filter->provider, desc->dtpd_provider)
    ||FILTER_OUT(filter->mod, desc->dtpd_mod)
    ||FILTER_OUT(filter->func, desc->dtpd_func)
    ||FILTER_OUT(filter->name, desc->dtpd_name)
  ) return 0;

  printf("%s\t%s\t%s\t%s\n", 
    desc->dtpd_provider,
    desc->dtpd_mod,
    desc->dtpd_func,
    desc->dtpd_name);

  (void)dh; // silence warnings - we don't  use this
  return 0;
}

static int
find_probes(probefilter_t *filter)
{
  int err, rc;
  int flags = 0;

  err = 0;
  /* get dtrace handle */
  dtrace_hdl_t *dh = dtrace_open(DTRACE_VERSION, flags, &err);
  if (err) {
    printf("%s: %s\n", strerror(errno), dtrace_errmsg(dh, err));
    return err;
  }

  /* start iteration, results go to callback above */
  rc = dtrace_probe_iter(dh, NULL, probeinfo, filter);
  dtrace_close(dh);
  return rc;
}

int
main(int argc, char *argv[])
{
  int c;
  probefilter_t filter;
  
  static struct option longopts[] = {
    {"provider", required_argument, NULL, 'p'},
    {"module", required_argument, NULL, 'm'},
    {"function", required_argument, NULL, 'f'},
    {"name", required_argument, NULL, 'n'},
    {NULL, 0, NULL, 0}
  };

  opterr = 0;
  memset(&filter, 0, sizeof(probefilter_t));
  while ((c = getopt_long(argc, argv, ":p:m:f:n:", longopts, NULL)) != -1){
  switch(c){
    case 'p': filter.provider = optarg; break;
    case 'm': filter.mod = optarg; break;
    case 'f': filter.func = optarg; break;
    case 'n': filter.name = optarg; break;
    default: opterr = 1; break;
  }}

#define name argv[0]

  if (opterr)
  {
    printf(
      "usage: %s [options]\n"
      "options:\n", name);
    puts(
      "    -p pattern        --provider=pattern\n"
      "    -m pattern        --module=pattern\n"
      "    -f pattern        --function=pattern\n"
      "    -n pattern        --name=pattern\n");
    printf("%s lists dtrace probes. The list maybe filtered to those "
           "matching user provided glob patterns.\n", name);
    return EXIT_FAILURE;
  }

  return find_probes(&filter);
}
