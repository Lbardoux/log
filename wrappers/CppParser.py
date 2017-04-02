#!/usr/bin/python3
#-*- encoding:utf-8 -*-

# This script may parse logs.hpp and generate a datastructure usable to generate other language code.
# In order to do so, respect the logs.hpp construction (with macro and shits).

class LogMethod:
    def __init__(self, macro_list):
        self.name, self.tag, self.color, self.suffix = macro_list

class OptionsAttribute:
    def __init__(self, macro_list):
        self.Type, self.name, self.default = macro_list

class OptionsGetSet:
    def __init__(self, macro_list):
        self.Type, self.suffix, self.ivar = macro_list

class Color:
    def __init__(self, macro_list):
        self.name, self.string = macro_list

class HeaderTag:
    def __init__(self, macro_list):
        self.tag, self.id = macro_list
        self.fname = macro_list[0][2:len(macro_list[0])-2]
        self.includefname = self.fname + ".include"

class DumpLine:
    def __init__(self, line):
        self.line = line

class ReadLine:
    def __init__(self, suffix):
        self.suffix = suffix

class LogContent:
    def __init__(self):
        self.methods    = list()
        self.attributes = list()
        self.getSets    = list()
        self.colors     = list()
        self.tags       = list()
        self.dumpLines  = list()
        self.readLines  = list()

    def _macroContent(self, line):
        return [l.strip() for l in line.split("(")[1].split(")")[0].split(",")]

    def read(self, fname):
        with open(fname, "r") as fidin:
            for line in fidin:
                line = line.strip()
                if line.find("MTL_LOG_METHOD") == 0:
                    self.methods.insert(0, LogMethod(self._macroContent(line)))
                elif line.find("MTL_LOG_STATIC_DECLARATION") == 0:
                    self.attributes.insert(0, OptionsAttribute(self._macroContent(line)))
                elif line.find("MTL_LOG_GET_SET") == 0:
                    self.getSets.insert(0, OptionsGetSet(self._macroContent(line)))
                elif line.find("MTL_LOG_COLOR_CODE") == 0:
                    self.colors.insert(0, Color(self._macroContent(line)))
                elif line.find("MTL_LOG_CASE_TAG") == 0:
                    self.tags.insert(0, HeaderTag(self._macroContent(line)))
                elif line.find("MTL_LOG_DUMP_LINE") == 0:
                    self.dumpLines.append(DumpLine(self._macroContent(line)[1]))
                elif line.find("MTL_LOG_READ_BOOL") == 0:
                    self.readLines.append(ReadLine(self._macroContent(line)[0]))
