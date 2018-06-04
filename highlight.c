/*
 * About this program: highlight version 1.1; colorize text on terminals
 * Copyright (C) 2012 Michal Kosek
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */ 

#define _GNU_SOURCE

/* ************************************************************************** */
/* includes  */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <getopt.h>
#include <regex.h>

#include "ansicolor.h"

/* ************************************************************************** */
/* constants  */

static int INPUT_BUFFER_SIZE = (16*1024);
static int SEQUENCE_STRING_LENGTH = 20;

static char* highlight_version="Highlight version 1.1";
static char* highlight_copyright="Copyright (c) 2012 Michal Kosek.";

/* ************************************************************************** */
/* globals  */

/* program return values  */
static struct {
  int value_ok;
  int value_warning;
  int value_error;
  bool flag_warning;
} retval={ 0, 1, 2, false};

/* basename  */
char* highlight_basename;

/* ************************************************************************** */
/* prototypes  */

/* copy 'length' bytes from 'source to 'dest' AND terminate 'dest' with zero  */
void
_strncpy(char* dest, char* source, size_t length);

/* print version information  */
void 
version(FILE* ostream);

/* print short usage message  */
void 
usage(FILE* ostream);

/* print hint short message about how to invoke help  */
void 
hint(FILE* ostream);

/* print complete help  */
void 
help(FILE* ostream);

/* ************************************************************************** */
/* main  */
int
main(int argc, char** argv)
{
  FILE* input = stdin;
  FILE* output = stdout;
  FILE* errput = stderr;

  struct {
    bool help_flag;
    bool hint_and_exit_flag;
    bool version_flag;
    bool ignore_case_flag;
    bool line_mode_flag;
    bool extended_regex_flag;
    bool attr_set_flag;
    bool fg_set_flag;
    bool bg_set_flag;
    bool pattern_set_flag;
    bool filename_set_flag;
    int attr_index;
    int fg_index;
    int bg_index;
    char* pattern;
    char* filename;
  } options = {false, false, false, false, false,
		false, false, false, false, false, false, 
		0, 0, 0, 
		NULL, NULL};

  char* input_buffer = NULL; // input buffer
  char* print_buffer = NULL; // temporary buffer used for printing

  char* input_buffer_tail; // current position in string
  char* p_match_position; // pointer to string match position

  regex_t regex_re; // compiled regular expression
  regmatch_t regex_match;
  int regex_rv; // regex return value
  size_t match_start, match_end; // beginning and end of pattern match
  
  /* ANSI escape sequences  */
  char start_sequence_string[SEQUENCE_STRING_LENGTH];  
  char end_sequence_string[SEQUENCE_STRING_LENGTH];

  highlight_basename = argv[0];

  int i;

  int getopt_rv;
  static struct option long_options[] = {
    {"help",     no_argument,       NULL,  'h'},
    {"version",  no_argument,       NULL,  'v'},
    {NULL,         0,        NULL,   0 }};

  /* parse command line parameters and store result in options  */
  while(!options.hint_and_exit_flag)
  {
    getopt_rv = getopt_long(argc, argv, "-ilhvEa:f:b:", long_options, NULL);
    if(getopt_rv == -1)
      break;

    switch(getopt_rv)
    {
      case 1: {
		if(!options.pattern_set_flag)
		{
		  options.pattern = argv[optind-1];
		  options.pattern_set_flag = true;
		}
		else if(!options.filename_set_flag)
		{
		  options.filename = argv[optind-1];
		  options.filename_set_flag = true;
		}
		else
		{
		  fprintf(errput, "%s: redundant parameter -- %s\n",
			    highlight_basename, argv[optind-1]);
		  options.hint_and_exit_flag = true;
		}
		break;
	      }
      case 'i': options.ignore_case_flag = true; break;
      case 'l': options.line_mode_flag = true; break;
      case 'h': options.help_flag = true; break;
      case 'v': options.version_flag = true; break;
      case 'E': options.extended_regex_flag = true; break;
      case 'a':	{
		  for(i = 0; i < ATTR_COUNT; i++)
		    if(strcmp(optarg, attr_string[i]) == 0)
		    {
		      options.attr_index = i;
		      options.attr_set_flag = true;
		      break;
		    }
		  /* additional argument not recognized  */
		  if(!options.attr_set_flag)
		  {
		    fprintf(errput, "%s: unknown attribute -- %s\n",
			    highlight_basename, optarg);
		    options.hint_and_exit_flag = true;
		  }
		  break;
		}
      case 'f':	{
		  for(i = 0; i < FG_COUNT; i++)
		    if(strcmp(optarg, fg_string[i]) == 0)
		    {
		      options.fg_index = i;
		      options.fg_set_flag = true;
		      break;
		    }
		  /* additional argument not recognized  */
		  if(!options.fg_set_flag)
		  {
		    fprintf(errput, "%s: unknown foreground color -- %s\n",
			    highlight_basename, optarg);
		    options.hint_and_exit_flag = true;
		  }
		  break;
		}
      case 'b':	{
		  for(i = 0; i < BG_COUNT; i++)
		    if(strcmp(optarg, bg_string[i]) == 0)
		    {
		      options.bg_index = i;
		      options.bg_set_flag = true;
		      break;
		    }
		  /* additional argument not recognized  */
		  if(!options.bg_set_flag)
		  {
		    fprintf(errput, "%s: unknown background color -- %s\n",
			    highlight_basename, optarg);
		    options.hint_and_exit_flag = true;
		  }
		  break;
		}
      default: options.hint_and_exit_flag = true; break;
    }
  }

  if(!options.pattern_set_flag && !options.version_flag && !options.help_flag)
    options.hint_and_exit_flag = true;
  
  /* print version info, hint and help message  */
  if(options.hint_and_exit_flag) 
  {
    hint(errput); 
    return retval.value_error; 
  }
  if(options.version_flag) 
  { 
    version(output);
    if(!options.help_flag) 
      return retval.value_ok;
  }
  if(options.help_flag) 
  {
    help(output); 
    return retval.value_ok; 
  }
  
  /* initialize highlighting control sequence strings  */
  start_sequence_string[0] = 0;
  end_sequence_string[0] = 0;

  if(options.attr_set_flag || options.fg_set_flag || options.bg_set_flag)
  {
    strcat(start_sequence_string, "\033[");
    strcat(end_sequence_string, "\033[0m");

    if(options.attr_set_flag)
    {
      strcat(start_sequence_string, attr_sequence[options.attr_index]);
      strcat(start_sequence_string, (options.fg_set_flag || options.bg_set_flag)
	    ? ";" : "");
    }
    if(options.fg_set_flag)
    {
      strcat(start_sequence_string, fg_sequence[options.fg_index]);
      strcat(start_sequence_string,(options.bg_set_flag) ? ";" : "");
    }
    if(options.bg_set_flag)
      strcat(start_sequence_string, bg_sequence[options.bg_index]);

    if(options.attr_set_flag || options.fg_set_flag || options.bg_set_flag)
      strcat(start_sequence_string, "m");
  }
  else
  {
    /* default highlighting */
    strcat(start_sequence_string, "\033[");
    strcat(start_sequence_string, fg_sequence[7]);
    strcat(start_sequence_string, ";");
    strcat(start_sequence_string, bg_sequence[1]);
    strcat(start_sequence_string, "m");
    strcat(end_sequence_string, "\033[0m");
  }

  /* open input file if filename is specified  */
  if (options.filename_set_flag)
  {
    input = fopen(options.filename, "r");
    if(input == NULL)
    {
      fprintf(errput, "%s: cannot open input file %s\n",
	      highlight_basename, options.filename);
      return retval.value_error;
    }
  }

  /* set line buffering mode for input and output stream */
  setvbuf(input, NULL, _IOLBF, INPUT_BUFFER_SIZE);
  setvbuf(output, NULL, _IOLBF, INPUT_BUFFER_SIZE);

  /* compile regular expression  */
  if(options.extended_regex_flag)
  {
    int regcomp_flags = REG_EXTENDED;
    if(options.line_mode_flag)
      regcomp_flags = regcomp_flags | REG_NOSUB;
    if(options.ignore_case_flag)
      regcomp_flags = regcomp_flags | REG_ICASE;
    regex_rv = regcomp(&regex_re, options.pattern, regcomp_flags);

    if(regex_rv != 0)
    {
      fprintf(errput, "%s: failed to compile regular expression",
	      highlight_basename);
      if(options.filename_set_flag)
	fclose(input);
      return retval.value_error;
    }
  }

  /* allocate buffers  */
  input_buffer = malloc(INPUT_BUFFER_SIZE*sizeof(char));
  print_buffer = malloc(INPUT_BUFFER_SIZE*sizeof(char));

  if(input_buffer == NULL || print_buffer == NULL)
  {
    if(input_buffer != NULL) free(input_buffer);
    if(print_buffer != NULL) free(print_buffer);

    if(options.filename_set_flag)
      fclose(input);
    fprintf(errput, "%s: cannot allocate buffers", highlight_basename);
    return retval.value_error;
  }

  /* begin main loop  */
  while(1)
  {
    /* read next line  */
    if(fgets(input_buffer, INPUT_BUFFER_SIZE, input) == NULL)
      break;

    /* check for lines longer than input buffer  */
    if(strstr(input_buffer, "\n") == NULL)
    {
      fprintf(errput, "%s: warning: line length exceeded buffer\n",
	      highlight_basename);
      retval.flag_warning = true;
    }

    input_buffer_tail = input_buffer;
    p_match_position = NULL;

    /* cycle to find all occurences on current line  */
    while(strlen(input_buffer_tail))
    {
      /* check for match  */
      if(options.extended_regex_flag)
      {
	if(options.line_mode_flag)
	  regex_rv = regexec(&regex_re, input_buffer_tail, 0, NULL, 0);
	else
	  regex_rv = regexec(&regex_re, input_buffer_tail,
			    1, &regex_match,
			    (input_buffer_tail == input_buffer)? 0 : REG_NOTBOL);
      }
      else
      {
	if(options.ignore_case_flag)
	  p_match_position = strcasestr(input_buffer_tail, options.pattern);
	else
	  p_match_position = strstr(input_buffer_tail, options.pattern);
      }
      
      /* no other match on current line, break cycle  */
      if(!options.extended_regex_flag && (p_match_position == NULL))
	break;
      if(options.extended_regex_flag && (regex_rv != 0))
	break;
      
      /* FOUND MATCH !!!  */
      
      /* if line mode flag is set, print whole line highlighted  */
      if(options.line_mode_flag)
      {
	/* check for EOL inside line  */
	/* if found, cut the line and print EOL after end_sequence_string  */
	char* p_EOL_position;
	p_EOL_position = strstr(input_buffer_tail, "\n");

	if(p_EOL_position == NULL)
	{
	  fputs(start_sequence_string, output);
	  fputs(input_buffer_tail, output);
	  fputs(end_sequence_string, output);
	}
	else
	{
	  *p_EOL_position = 0;
	  fputs(start_sequence_string, output);
	  fputs(input_buffer_tail, output);
	  fputs(end_sequence_string, output);
	  fprintf(output, "\n");
	}
	/* nullify input_buffer_tail to finish processing current line  */
	*input_buffer_tail = 0;
	break;
      }

      /* determine start and end position of a match  */
      if(options.extended_regex_flag)
      {
	match_start = regex_match.rm_so;
	match_end = regex_match.rm_eo;
      }
      else
      {
	match_start = strlen(input_buffer_tail) - strlen(p_match_position);
	match_end = match_start + strlen(options.pattern);
      }

      /* substract and print preceeding substring  */
      _strncpy(print_buffer, input_buffer_tail, match_start);
      input_buffer_tail += match_start * sizeof(char); // shift buffer
      fputs(print_buffer, output);

      /* substract string that is a match to temporary buffer  */
      _strncpy(print_buffer, input_buffer_tail, match_end-match_start);
      input_buffer_tail += (match_end - match_start) * sizeof(char); // shift buffer

      fputs(start_sequence_string, output);
      fputs(print_buffer, output);
      fputs(end_sequence_string, output);
    } /* processing line end  */

    /* print rest of current line or whole line with no match  */
    fprintf(output, "%s", input_buffer_tail);
  } /* end main loop  */

  /* free buffers and close input file  */
  free(input_buffer);
  free(print_buffer);
  if(options.extended_regex_flag)
    regfree(&regex_re);
  if(options.filename_set_flag)
    fclose(input);

  if(retval.flag_warning)
    return retval.value_warning;
  return retval.value_ok;
} /* main end  */

/* ************************************************************************** */
/* definitions  */

void 
_strncpy(char* dest, char* source, size_t length)
{
  strncpy(dest, source, length);
  *(dest + (length * sizeof(char))) = 0;
}

void 
version(FILE* ostream)
{
  fprintf(ostream, "%s\n%s\n", highlight_version, highlight_copyright);
}

void 
usage(FILE* ostream)
{
  fprintf(ostream, "Usage: %s [OPTION]... PATTERN [FILE]\n", 
	    highlight_basename);
}

void 
hint(FILE* ostream)
{
  usage(ostream);
  fprintf(ostream, "Try `%s --help' for more information.\n", 
	  highlight_basename, highlight_basename);
}

void 
help(FILE* ostream)
{
  int i;

  usage(ostream);
  fprintf(ostream,
      "Highlight PATTERN using ANSI escape sequence,"
      " read from FILE or standard input.\n"
      "Example: %s -f red 'hello world' main.c\n", highlight_basename);
  fprintf(ostream, 
      "\n"
      "Pattern interpretation:\n"
      "-i               ignore case\n"
      "-E               PATTERN is an extended regular expression\n"
      "\n"
      "Highlighting:\n"
      "-a attribute     set attribute of highlighted text\n"
      "                 possible attributes are:");
  for(i=0; i < ATTR_COUNT; i++)
    fprintf(ostream, " %s", attr_string[i]);
  fprintf(ostream, 
      "\n"
      "-f color         set foreground color of highlighted text\n"
      "                 possible colors are:");
  for(i=0; i < FG_COUNT; i++)
    fprintf(ostream, " %s", fg_string[i]);
  fprintf(ostream,
      "\n"
      "-b color         set background color of highlighted text\n"
      "                 possible colors are:");
  for(i=0; i < BG_COUNT; i++)
    fprintf(ostream, " %s", bg_string[i]);
  fprintf(ostream,
      "\n"
      "-l               line mode (highlight whole lines)\n"
      "\n"
      "-v, --version    display program version and exit\n"
      "-h, --help       display this help and exit\n"
      "\n"
      "With no FILE read standard input.\n"
      "Exit status is %d if no error, %d on warning, %d on fatal error.\n"
      "\n"
      "Report bugs to <mkmk@email.cz>\n",
      retval.value_ok, retval.value_warning, retval.value_error);
}

/* ************************************************************************** */
/* EOF  */
