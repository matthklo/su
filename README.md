# su - Header only std::string utilities
To integrate into existing projects, just include su.h and it's all done.

NOTE: C++11 is required.

## Feateures:

### su::printf()

A printf() over a std::string.  
Supported format specifiers: d,i,u,o,x,X,c,s,p,%  
Supported flags: -,+,(space),#,0  
Supported width: (number),*  
Supported .precision: .(number),.*  
Supported length sub-specifiers: hh,h,l,ll  

Example:
```
    std::string a = su.printf("%d, %12.12s", 123, "abcdefghijklm");
```

### su::to_lower(), su::to_upper()

Return a new std::string with content cloned from the input std::string but translating all letters to lower-case or upper-case.

### su::make_lower(), su::make_upper()

Translate all letters of input std::string to lower-case or upper-case in-place.

### su::trim()

Return a new std::string with content cloned from the input std::string but has all leading and trailing space letters trimmed.

### su::make_trim()

Make all leading and trailing space letters of the input std::string trimmed in-place.

### su::split()

Take a container, an input std::string, and a string of characters used as delimiters. Split the content into tokens by the delimiters and store in the container. The container can be any class which supports clear(), push_back(), and size().

Example:
```
    std::string to_split("abc def,123");
    std::vector<std::string> tokens;
    
    auto cnt = su.split(tokens, to_split, " ,");
    // 'cnt' is 3 and 'tokens' has 3 members: "abc", "def", "123"
```
