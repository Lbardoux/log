#!/usr/bin/python3
#-*- encoding:utf-8 -*-

from CppParser import *

def write_ln(out, text):
    out.write(text + '\n')

def writeImports(out):
    write_ln(out, "import java.io.IOException;")
    write_ln(out, "")

def writeClassName(out, namespace):
    write_ln(out, "public final class {0}".format(namespace))
    write_ln(out, "{")

def writeHeader(out, namespace):
    pass

def writeOptions(out, namespace, parser):
    write_ln(out, "\tpublic final class Options")
    write_ln(out, "\t{")
    for att in parser.attributes:
        if att.Type == "bool":
            write_ln(out, "\t\tprivate static boolean {0} = {1};".format(att.name, att.default))
        elif att.name == "FORMAT":
            pass #todo
    # add HEADER, MUTEX, NAMES
    write_ln(out, "")
    for f in parser.getSets:
        write_ln(out, "\t\t//! Getter and setter for {}.".format(f.suffix))
        write_ln(out, "\t\tpublic static void enable{}(boolean value)".format(f.suffix))
        write_ln(out, "\t\t{")
        write_ln(out, "\t\t\t")
        write_ln(out, "\t\t\tOptions.{} = value;".format(f.ivar))
        write_ln(out, "\t\t\t")
        write_ln(out, "\t\t}")
        write_ln(out, "\t\tpublic static boolean is{}Enabled()".format(f.suffix))
        write_ln(out, "\t\t{")
        write_ln(out, "\t\t\t")
        write_ln(out, "\t\t\tboolean value = Options.{};".format(f.ivar))
        write_ln(out, "\t\t\t")
        write_ln(out, "\t\t\treturn value;")
        write_ln(out, "\t\t}")
    #GET/SET for format && output 
    write_ln(out, "\t}")


def writeLogMethods(out, methods, namespace):
    for method in methods:
        write_ln(out, "\t/**")
        write_ln(out, "\t * Display a log line which type is {}.".format(method.name))
        write_ln(out, "\t * @param args A bunch of arguments, which require the toString statement.")
        write_ln(out, "\t */")
        write_ln(out, "\tpublic static void {}(Object[] args) throws IOException".format(method.name))
        write_ln(out, "\t{")
        write_ln(out, "\t}")



def wrap(fname, outputname, namespace="mlog"):
    parser = LogContent()
    parser.read(fname)
    with open(outputname, "w") as output:
        writeImports(output)
        writeClassName(output, namespace)
        writeHeader(output, namespace)
        writeOptions(output, namespace, parser)
        
        writeLogMethods(output, parser.methods, namespace)
        
        
        
        
        write_ln(output, "}")


wrap("src/logs.hpp", "logs.java")
