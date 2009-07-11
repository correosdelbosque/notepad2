
=======================================================================
=                                                                     =
=                                                                     =
=   Notepad2 - light-weight Scintilla-based text editor for Windows   =
=                                                                     =
=                                                                     =
=                                                   Notepad2 3.1.21   =
=                                      (c) Florian Balmer 2004-2009   =
=                                       http://www.flos-freeware.ch   =
=                                                                     =
=                                                                     =
=======================================================================


The Notepad2 Source Code

  This package contains the full source code of Notepad2 3.1.21 for
  Windows. Project files for Visual C++ 7.0 are included. Chances are
  that Notepad2 can be rebuilt with other development tools, including
  the free Visual C++ Express Edition, but I haven't tested this.


Rebuilding from the Source Code

  To be able to rebuild Notepad2, the source code of the Scintilla
  editing component [1] has to be unzipped to the "Scintilla"
  subdirectory of the Notepad2 source code directory.

  [1] http://www.scintilla.org

  Notepad2 3.1.21 has been created with Scintilla 1.77. The following
  modification to the Scintilla source code is necessary:

  Scintilla/src/KeyWords.cxx:

      #define LINK_LEXER(lexer) extern LexerModule lexer; ...

    must be replaced with:

      #define LINK_LEXER(lexer) void(0)


Shrinking the Executable Program File

  To reduce the size of the Notepad2.exe program file, dynamic linking
  to the Visual C++ runtime library is enabled. To use the system C
  runtime library present on every Windows computer (msvcrt.dll), the
  Visual C++ 7.0 version of msvcrt.lib has to be replaced by msvcrt.lib
  version 6.0. This file can be obtained by downloading the Visual
  Studio 6.0 Service Pack 6 from Microsoft [2]. Run the downloaded
  archive and find msvcrt.lib in one of the extracted cab-files.

  Batch files to rebuild Notepad2 with the system C runtime library can
  be found in the "vc6build" subdirectory. The environment variables
  need to be adapted to your system. The first LIB path has to be the
  directory containing msvcrt.lib version 6.0.

  Special thanks to Kai Liu [3] for providing useful information about
  using the system C runtime library!

  The resulting executable file is compressed with UPX [4].

  [2] http://www.google.ch/search?q=Visual+Studio+6.0+Service+Pack+6
  [3] http://www.kailiu.com/notepad2/
  [4] http://upx.sourceforge.net


How to add or modify Syntax Schemes

  The Scintilla documentation has an overview of syntax highlighting,
  and how to write your own lexing module, in case the language you
  would like to add is not currently supported by Scintilla.

  Add your own lexer data structs to the global pLexArray (Styles.c),
  then adjust NUMLEXERS (Styles.h) to the new total number of syntax
  schemes. The style definitions can be found in SciLexer.h of the
  Scintilla source code. Include the Lex*.cxx file from Scintilla
  required for your language into your project.


Copyright

  See License.txt for details about distribution and modification.

  If you have any comments or questions, please drop me a note:
  florian.balmer@gmail.com

  (c) Florian Balmer 2004-2009
  http://www.flos-freeware.ch

###
