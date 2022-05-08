# C++ Programmer's Test Task

It is necessary to write a class in pure C++ that can read huge text log files
(hundreds of megabytes, tens of gigabytes) as quickly as possible and produce lines
that satisfy the conditions of the simplest regexp: (at least the `*` and `?` operators,
a wider range of possibilities are welcome):

- character `*` - sequence of any characters of unlimited length;
- character `?` - any one character;
- masks should be processed correctly: `*Some*`, `*Some`, `Some*`, `*****Some***` -
  there are no restrictions on the position `*` in the mask.

The search result must be strings that match the mask.

For example:

- The mask `*abc*` selects all lines containing `abc` and starting and ending with any sequence of characters.
- The `abc*` mask matches all strings starting with `abc` and ending with any sequence of characters.
- The `abc?` mask selects all lines starting with `abc` and ending with any additional character.
- The `abc` mask selects all strings that are equal to this mask.

The class will not be extended and will not be a base class.

Character encoding: all files and regex patterns are ANSI (non-unicode single-byte encoding).

The class must have the following public interface:

```cpp
class CLogReader
  {
public:
           CLogReader(...);
          ~CLogReader(...);

   bool    Open(...);                       // opening a file, false - error
   void    Close();                         // close the file

   bool    SetFilter(const char *filter);   // setting the line filter, false - error
   bool    GetNextLine(char *buf,           // request for the next matched line,
                       const int bufsize);  // buf - buffer, bufsize - maximum length
                                            // false - end of file or error
  };
```

## Implementation Requirements

- Platform: Windows 7 and above
- Functionality: maximum performance (caching of search results and file is not required)
- Resources: memory costs should be minimal (within reasonable limits)
- Components: use of third-party libraries (including STL) and components is prohibited.
  Only WinAPI and CRT are allowed.
- Exceptions: Windows and C++ exceptions cannot be used
- The code must be absolutely "indestructible" and protected from errors

## Design Requirements

- The code should be as simple as possible
- The code should be as clean, beautiful and understandable as possible
- The result should be a finished class (`.cpp` + `.h` files) and a console example app
  which opens a text file of a couple of megabytes and greps `order*closed`.
- The console example app should receive the path to the text file (the first parameter)
  and the line filter(the second parameter) as command line parameters and output
  the found lines in full to the console. Run line example:  
  `tool.exe 20190102.log *bbb*`
- The project must be designed for building in MS Visual Studio 2015 and higher.

Any comments in the code are welcome! `:)`

---
