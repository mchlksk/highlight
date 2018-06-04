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

#pragma once

static int ATTR_COUNT = 7;
static int FG_COUNT = 8;
static int BG_COUNT = 8;

const char* attr_string[] = {"reset", "bright", "dim", "underline", 
			    "blink", "reverse", "hidden"};
const char* attr_sequence[] = {"0", "1", "2", "3", "4", "7", "8"};

const char* fg_string[] = {"black", "red", "green", "yellow", "blue",
                            "magenta", "cyan", "white"};
const char* fg_sequence[] = {"30", "31", "32", "33", "34", "35", "36", "37"};

const char* bg_string[] = {"black", "red", "green", "yellow", "blue",
                            "magenta", "cyan", "white"};
const char* bg_sequence[] = {"40", "41", "42", "43", "44", "45", "46", "47"};
