#!/usr/bin/python3
#-*- encoding:utf-8 -*-

from CppParser import *

def write_ln(out, text):
    out.write(text + '\n')

def writeImports(out):
    write_ln(out, "import java.io.IOException;")
    write_ln(out, "import java.io.PrintStream;\n\n")

def writeClassName(out, namespace):
    write_ln(out, "public final class {0}".format(namespace) + " {")

def writeHeader(out, namespace):
    write_ln(out, "\t\tprivate final class Header {")
    write_ln(out, "\t\t\tprivate static String format;")
    write_ln(out, "\t\t\tprivate static String header;")
    write_ln(out, "\t\t\tHeader(String format) {")
    write_ln(out, "\t\t\t\t")
    write_ln(out, "\t\t\t\t")
    write_ln(out, "\t\t\t\t")
    write_ln(out, "\t\t\t}")
    #display with out.format(header, what1, what2, etc)
    
    
    
    write_ln(out, "\t\t}")

def writeOptions(out, namespace, parser):
    write_ln(out, "\tpublic static final class Options {")
    write_ln(out, "\t\tprivate Options() {")
    write_ln(out, "\t\t}\n")
    writeHeader(out, namespace)
    for att in parser.attributes:
        if att.Type == "bool":
            write_ln(out, "\t\tprivate static boolean {0} = {1};".format(att.name, att.default))
        elif att.name == "FORMAT":
            pass #todo
    write_ln(out, "\n\t\tprivate static final Object MUTEX = new Object();")
    write_ln(out, "\t\tprivate static PrintStream OUT = System.out;\n")

    for color in parser.colors:
        write_ln(out, "\t\tprivate static String {0} = new String({1});".format(color.name, color.string))
    write_ln(out, "")
    # add HEADER, NAMES
    for f in parser.getSets:
        write_ln(out, "\t\t//! Getter and setter for {}.".format(f.suffix))
        write_ln(out, "\t\tpublic static void enable{}(boolean value)".format(f.suffix) + " {")
        write_ln(out, "\t\t\tsynchronized({}.Options.MUTEX)".format(namespace) + " {")
        write_ln(out, "\t\t\t\t{0}.Options.{1} = value;".format(namespace, f.ivar))
        write_ln(out, "\t\t\t}")
        write_ln(out, "\t\t}")
        write_ln(out, "\t\tpublic static boolean is{}Enabled()".format(f.suffix) + " {")
        write_ln(out, "\t\t\tboolean value;")
        write_ln(out, "\t\t\tsynchronized({}.Options.MUTEX)".format(namespace) + " {")
        write_ln(out, "\t\t\t\tvalue = {0}.Options.{1};".format(namespace, f.ivar))
        write_ln(out, "\t\t\t}")
        write_ln(out, "\t\t\treturn value;")
        write_ln(out, "\t\t}")
    write_ln(out, "\n\t\t//! Getter and setter for the stream you wanna log in.")
    write_ln(out, "\t\tpublic static void setOutputStream(PrintStream stream) {")
    write_ln(out, "\t\t\tsynchronized({}.Options.MUTEX)".format(namespace) + " {")
    write_ln(out, "\t\t\t\t{}.Options.OUT = stream;".format(namespace))
    write_ln(out, "\t\t\t}\n\t\t}")
    write_ln(out, "\t\tpublic static PrintStream getOutputStream() {")
    write_ln(out, "\t\t\tPrintStream value = null;")
    write_ln(out, "\t\t\tsynchronized({}.Options.MUTEX)".format(namespace) + " {")
    write_ln(out, "\t\t\t\tvalue = {}.Options.OUT;".format(namespace))
    write_ln(out, "\t\t\t}return value;\n\t\t}")

    write_ln(out, "\t\t//! Getter and setter for the format String.")
    write_ln(out, "\t\tpublic static void setFormat(String format) {")
    write_ln(out, "\t\t\tsynchronized({}.Options.MUTEX)".format(namespace) + " {")
    #todo
    #write_ln(out, "\t\t\t\t = format;")
    #write_ln(out, "\t\t\t\t .build();")
    write_ln(out, "\t\t\t}")
    write_ln(out, "\t\t}")
    write_ln(out, "\t\tpublic static String getFormat() {")
    write_ln(out, "\t\t\tString value = null;")
    write_ln(out, "\t\t\tsynchronized({}.Options.MUTEX)".format(namespace) + " {")
    #todo
    #write_ln(out, "\t\t\t\tvalue = ;")
    write_ln(out, "\t\t\t}")
    write_ln(out, "\t\t\treturn value;")
    write_ln(out, "\t\t}")
    write_ln(out, "\t}")

def writeGenericMethod(out, namespace):
    write_ln(out, "\tprivate static void _log(String tag, String color, Object... args) throws IOException {")
    write_ln(out, "\t\tsynchronized({}.Options.MUTEX)".format(namespace) + " {")
    write_ln(out, "\t\t\tif ({0}.Options.ENABLE_HEADER)".format(namespace) + " {")
    #Print HEADER
    write_ln(out, "\t\t\t}")
    write_ln(out, "\t\t\tfor(int i=0;i<args.length;i++) {")
    write_ln(out, "\t\t\t\t{}.Options.OUT.print(args[i]);".format(namespace))
    write_ln(out, "\t\t\t\tif ({}.Options.ENABLE_SPACING)".format(namespace) + " {")
    write_ln(out, "\t\t\t\t\t{}.Options.OUT.print(' ');".format(namespace))
    write_ln(out, "\t\t\t\t}")
    write_ln(out, "\t\t\t}")
    write_ln(out, "\t\t\t{}.Options.OUT.println(\"\");".format(namespace))
    write_ln(out, "\t\t}\n\t}")

def writeLogMethods(out, methods, namespace):
    for method in methods:
        write_ln(out, "\t/**")
        write_ln(out, "\t * Display a log line which type is {}.".format(method.name))
        write_ln(out, "\t * @param args A bunch of arguments, which require the toString statement.")
        write_ln(out, "\t */")
        write_ln(out, "\tpublic static void {}(Object... args) throws IOException".format(method.name) + " {")
        write_ln(out, "\t\tif ({0}.Options.isLogEnabled() && {0}.Options.is{1}Enabled())".format(namespace, method.suffix) + " {")
        write_ln(out, "\t\t\t{0}._log({1}, {0}.Options.{2}, args);".format(namespace, method.tag, method.color))
        write_ln(out, "\t\t}")
        write_ln(out, "\t}")

def writeLoadConfiguration(namespace, out, parser):
    write_ln(out, "//! Loads fname if founded, else create it")
    write_ln(out, "//! @return true if the file was founded, false otherwise")
    write_ln(out, "\tpublic static boolean loadConfiguration(String fname) {")
    write_ln(out, "\t\t")
    write_ln(out, "\t\t")
    write_ln(out, "\t\t")
    write_ln(out, "\t\treturn false;")
    write_ln(out, "\t}")



def wrap(fname, outputname, namespace="mlog"):
    parser = LogContent()
    parser.read(fname)
    with open(namespace + ".java", "w") as output:
        writeImports(output)
        writeClassName(output, namespace)
        writeOptions(output, namespace, parser)
        writeGenericMethod(output, namespace)
        writeLogMethods(output, parser.methods, namespace)
        writeLoadConfiguration(namespace, output, parser)
        write_ln(output, "}")


wrap("src/logs.hpp", "logs.java")
