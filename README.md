This package contains highlight version 1.1.

Highlight is a command line filter to highlight given pattern 
on ANSI compatible terminals using colors.

Prerequisities:
  gcc, make

Installation instructions:

  1. (optional) edit Makefile according to your needs.
  2. Type: `make'
  3. Type: `make install' as superuser

Known bugs and limitations:
  * Input buffer for one line (including EOL) is limited to 16383 characters.
    Longer lines will still be printed (preceeded by warning on error console)
    and pattern will be highlighted UNLESS it lies across the 16383 characters boundary.
  * ANSI escape sequences are used to highlight text, not compatible with all terminals.
  * If a highlighted background is wrapped on terminal because the terminal width
    is exceeded, rest of the line with no text is printed in selected background color.
