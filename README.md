## Introduction

Introducing *UNV* - a computer language based upon ideas from the univalent
foundations of mathematics project: [http://homotopytypetheory.org]
(http://homotopytypetheory.org)

## Motivation

The motivation for this project is as tool to study and understand *homotopy
type theory* from a computer science perspective.  The language is meant to
serve as a testing ground for working out the implications of homotopy type
theory.

## Design

TODO

## Implementation

One of the goals is to introduce dependently typed constructs to the c-like
language family of general purpose and systems programming.  The compiler
frontend uses the *QtCore module* of the [Qt C++ toolkit](http://qt-project.org/)
in order to quickly create a hand-written lexer/parser that will be easily
to manipulate as the language takes shape.  The backend of the compiler will
use the [LLVM (Low Level Virtual Machine)](http://llvm.org/) compiler
infrastructure and associated tooling to produce native binaries for systems
programming.
