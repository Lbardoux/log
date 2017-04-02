#!/usr/bin/python3
#-*- encoding:utf-8 -*-

from CppParser import *



def createHeaderClass(output, namespace):
    """
    Edit here to add some new tags inside python wrapping.
    Do not forget to import within the addHeader() function.
    """
    output.write("    class _Header:\n")
    output.write("      def __init__(self, format):\n")
    output.write("         self._format   = format\n")
    output.write("         self._formated = format\n")
    output.write("         self._build()\n\n")
    output.write("      def _build(self):\n")
    output.write("        self._formated = self._formated.replace('{TYPE}',   '{0}')\n")
    output.write("        self._formated = self._formated.replace('{DATE}',   '{1}')\n")
    output.write("        self._formated = self._formated.replace('{TIME}',   '{2}')\n")
    output.write("        self._formated = self._formated.replace('{THREAD}', '{3}')\n")
    output.write("        \n")
    output.write("      def _f(self, num):\n")
    output.write("        return str(num) if num >= 10 else '0' + str(num)\n")
    output.write("      def getDate(self):\n")
    output.write("        now = datetime.datetime.now()\n")
    output.write("        return self._f(now.month) + '/' + self._f(now.day) + '/' + str(now.year)\n\n")
    output.write("      def getTime(self):\n")
    output.write("        now = datetime.datetime.now()\n")
    output.write("        return self._f(now.hour) + ':' + self._f(now.minute) + ':' + self._f(now.second)\n\n")
    output.write("      def getThread(self):\n")
    output.write("        try:\n")
    output.write("          return {}.Options.NAMES[get_ident()]\n".format(namespace))
    output.write("        except KeyError:\n")
    output.write("          return get_ident()\n")
    output.write("        \n")
    output.write("      def display(self, output, tag, color):\n")
    output.write("        fulltag = str(color if {0}.Options.ENABLE_COLOR else \"\") + tag + str({0}.Options.C_BLANK if {0}.Options.ENABLE_COLOR else \"\")\n".format(namespace))
    output.write("        try:\n")
    output.write("          output.write(self._formated.format(fulltag, self.getDate(), self.getTime(), self.getThread()))\n")
    output.write("        except KeyError:\n")
    output.write("          pass\n\n")


def createFunction(method, output, namespace):
    output.write("  def {}(*params):\n".format(method.name))
    output.write("    \"\"\"\n")
    output.write("    @brief: Logs a line with {} type, and all additional parameters\n".format(method.name))
    output.write("    @param *params: A bunch of arguments, which must be displayable using __str__() statement.\n")
    output.write("    \"\"\"\n")
    output.write("    if {0}.Options.isLogEnabled() and {0}.Options.is{1}Enabled():\n".format(namespace, method.suffix))
    output.write("      {0}._print({1}, {0}.Options.{2}, *params)\n\n".format(namespace, method.tag, method.color))


def createOptions(content, output, namespace):
    output.write("  class Options:\n")
    output.write("    \"\"\"Embeds settings for logs\"\"\"\n")
    createHeaderClass(output, namespace)
    for attribut in content.attributes:
        if attribut.Type == "bool":
            output.write("    {0} = {1}\n".format(attribut.name, "True" if attribut.default == "true" else "False"))
        elif attribut.name == "FORMAT":
            output.write("    FORMAT = _Header({})\n".format(attribut.default))
    output.write("    OUT = sys.stdout\n")
    output.write("    MUTEX = Lock()\n")
    output.write("    NAMES = dict()\n")

    for color in content.colors:
        output.write("    {0} = {1}\n".format(color.name, color.string))

    output.write("\n")
    for getset in content.getSets:
        output.write("    def enable{}(boolean):\n".format(getset.suffix))
        output.write("      {0}.Options.MUTEX.acquire()\n".format(namespace))
        output.write("      {0}.Options.{1} = boolean\n".format(namespace, getset.ivar))
        output.write("      {0}.Options.MUTEX.release()\n\n".format(namespace))
        output.write("    def is{}Enabled():\n".format(getset.suffix))
        output.write("      {0}.Options.MUTEX.acquire()\n".format(namespace))
        output.write("      v = {0}.Options.{1}\n".format(namespace, getset.ivar))
        output.write("      {0}.Options.MUTEX.release()\n".format(namespace))
        output.write("      return v\n\n")
    
    output.write("    def bindThreadName(name):\n")
    output.write("      {}.Options.MUTEX.acquire()\n".format(namespace))
    output.write("      {}.Options.NAMES[get_ident()] = name\n\n".format(namespace))
    output.write("      {}.Options.MUTEX.release()\n".format(namespace))
    output.write("    def unbindThreadName():\n")
    output.write("      {}.Options.MUTEX.acquire()\n".format(namespace))
    output.write("      if get_ident() in NAMES:\n")
    output.write("        del {}.Options.NAMES[get_ident()]\n\n".format(namespace))
    output.write("      {}.Options.MUTEX.release()\n".format(namespace))
    
    output.write("    def setOutputStream(stream):\n")
    output.write("      {0}.Options.MUTEX.acquire()\n".format(namespace))
    output.write("      {}.Options.OUT = stream\n".format(namespace))
    output.write("      {0}.Options.MUTEX.release()\n\n".format(namespace))
    output.write("    def getOutputStream():\n")
    output.write("      {0}.Options.MUTEX.acquire()\n".format(namespace))
    output.write("      v = {}.Options.OUT\n".format(namespace))
    output.write("      {0}.Options.MUTEX.release()\n".format(namespace))
    output.write("      return v\n\n".format(namespace))
    output.write("    def setFormat(string):\n")
    output.write("      {0}.Options.MUTEX.acquire()\n".format(namespace))
    output.write("      {}.Options.FORMAT._format   = string\n".format(namespace))
    output.write("      {}.Options.FORMAT._formated = string\n".format(namespace))
    output.write("      {}.Options.FORMAT._build()\n".format(namespace))
    output.write("      {0}.Options.MUTEX.release()\n\n".format(namespace))
    output.write("    def getFormat():\n")
    output.write("      {0}.Options.MUTEX.acquire()\n".format(namespace))
    output.write("      f = {}.Options.FORMAT._format\n".format(namespace))
    output.write("      {0}.Options.MUTEX.release()\n".format(namespace))
    output.write("      return f\n\n")


def addHeader(output):
    output.write("#!/usr/bin/python3\n#-*-encoding:utf-8-*-\n\n")
    output.write("import sys\n")
    output.write("from threading import Thread, Lock, get_ident\n")
    output.write("import datetime")
    output.write("\n\n\n")


def addPrototype(output, namespace):
    output.write("  def _print(tag, color, *params):\n")
    output.write("    {}.Options.MUTEX.acquire()\n".format(namespace))
    output.write("    if {}.Options.ENABLE_HEADER:\n".format(namespace))
    output.write("      {0}.Options.FORMAT.display({1}.Options.OUT, tag, color)\n".format(namespace, namespace))
    output.write("    for arg in params:\n")
    output.write("      {}.Options.OUT.write(arg)\n".format(namespace))
    output.write("      if {}.Options.ENABLE_SPACING:\n".format(namespace))
    output.write("        {}.Options.OUT.write(' ')\n".format(namespace))
    output.write("    {}.Options.OUT.write(\"\\n\")\n".format(namespace))
    output.write("    {}.Options.MUTEX.release()\n\n".format(namespace))

def createLoader(output, reads, dumps, namespace):
    output.write("  def loadConfiguration(fname):\n")
    output.write("    \"\"\"Loads fname, return true if loaded, false otherwise.\"\"\"\n")
    output.write("    try:\n")
    output.write("      with open(fname, \"r\") as config:\n")
    for line in reads:
        output.write("        {0}.Options.enable{1}(bool(fname.readline().split(\" \")[2]))\n".format(namespace, line.suffix))
    output.write("        {0}.Options.setFormat(fname.readline().split(\" =\")[1])\n".format(namespace))
    output.write("      return True\n")

    output.write("    except FileNotFoundError:\n")
    output.write("      with open(fname, \"w\") as config:\n")
    for line in dumps:
        output.write("        fname.write({} + \"\\n\")\n".format(line.line))
    output.write("      return False\n\n")



def wrap(fname, outputname, namespace="mlog"):
    parser = LogContent()
    parser.read(fname)
    with open(outputname, "w") as output:
        addHeader(output)
        output.write("class {}:\n".format(namespace))
        createOptions(parser, output, namespace)
        addPrototype(output, namespace)
        createLoader(output, parser.readLines, parser.dumpLines, namespace)
        for i in parser.methods:
            createFunction(i, output, namespace)
