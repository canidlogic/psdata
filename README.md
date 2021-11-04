# psdata

Utility for embedding binary data into PostScript with Base-85 encoding.

Supports embedding both raw Base-85 streams as well as Base-85 streams that are appropriately wrapped for use in PostScript programs following the Document Structuring Conventions.

Properly supports setting binary mode on standard input and output on the Windows platform at the start of the program.  On Windows, all line breaks will be CR+LF in output instead of LF.

## Syntax

    psdata [options]

The program reads raw binary data from standard input and outputs Base-85 ASCII data to standard output, with line breaks inserted to keep line lengths limited.  The following options are available:

    -dsc

Wrap the Base-85 data stream in `%%BeginData` and `%%EndData` tags so that it can be properly embedded within PostScript files that are following the Document Structuring Conventions.  Only use this option if the whole generated PostScript file is using the Document Structuring Conventions, see _PostScript Language Document Structuring Conventions Specification_ (Version 3.0, 1992) for further information.

Using the `-dsc` tag will force `psdata` to buffer all Base-85 data in a temporary file before output, because the `%%BeginData` tag requires the total count of lines before any of the Base-85 data is written.

    -head [text]

Add a header line to the Base-85 data stream.  `[text]` is the header line to add.  This may only include visible, printing US-ASCII characters, and the space character (range [0x20, 0x7e]).  It may furthermore have at most 255 characters.  Finally, its length may not exceed the maximum line length, which is either the default value of 72 or a different value specified with the `-len` option.

The header line, exactly as it is, followed by one line break, will be printed before the start of the Base-85 data stream.  If the `-dsc` option is also specified, the header line will occur after the `%%BeginData` tag line, and the header line and the following blank line will be included in the line count given in the `%%BeginData` tag line.

When accessing the embedded Base-85 data stream in the PostScript program using the `currentfile` operator, the Document Structuring Conventions recommend that the operator that will actually read from the `currentfile` be wrapped within the `%%BeginData` and `%%EndData` section.  To accomplish this with `psdata`, specify the `-dsc` option to turn on Document Structuring Conventions support, and then specify the `-head` option with the operator that will actually be reading from the stream, for example `-head image`

    -len [count]

Set the maximum line length on output.  `[count]` is the maximum number of characters per line, excluding the line break.  If this option is not specified, a default value of 72 is used.

The valid range of `[count]` values is [16, 255].  Lines are not allowed to be longer than 255 characters (excluding line break) according to the Document Structuring Conventions.

## Compilation

The whole program is contained in `psdata.c` which has no dependencies beyond the standard C library.  You can compile it with GCC like this:

    gcc -O2 -o psdata psdata.c

The source file preprocessor detects whether it is being compiled on Windows by checking for one of the following predefined constants:

- `_WIN32`
- `_WIN64`
- `__WIN32__`
- `__TOS_WIN__`
- `__WINDOWS__`

If any of those constants are defined by the environment, then the source file preprocessor will define a constant `PSDATA_WIN` which indicates that the file is being compiled on a Windows environment.  You can force the preprocessor to assume a Windows environment by defining the `PSDATA_WIN` constant during compilation.

If `PSDATA_WIN` gets defined, then the `<io.h>` and `<fcntl.h>` headers will also be imported.  Furthermore, the extension functions `_setmode()` and `_fileno()` will be used to set binary mode on standard input and standard output at the beginning of the program.  (This is not necessary on POSIX platforms, where there is no difference between text and binary modes.)  Finally, the output function will change LF characters into CR+LF sequences on Windows.

Normally, Windows platform support should be automatic so just compile the file normally as a C console program on Windows.
