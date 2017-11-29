#ifndef _CEC278B6B50A_SU_HEADER_
#define _CEC278B6B50A_SU_HEADER_

/* C++11 is required */

#include <cstdint>
#include <cstdarg>
#include <cctype>
#include <string>
#include <type_traits>

namespace su {

  // T can only be a specialization of 
  //   std::basic_string< char, std::char_traits<char>, Alloc >
  template< typename T >
  int32_t printf( T& str, const char* format, ...)
  {
    static_assert(std::is_same<T::value_type, char>(), 
      "T should be a specialization of std::basic_string< char, std::char_traits<char>, Alloc >.");

    va_list ap;
    va_start(ap, format);
    
    str.clear();
    
    enum
    {
      ST_TEXT = 0,
      ST_FLAGS,
      ST_WIDTH,
      ST_PERCISION,
      ST_SUBSPEC,
      ST_SPEC,
    };
    
    enum
    {
      FLAG_LEFTJUSTIFY = 0x1,
      FLAG_FORCESIGN = 0x2,
      FLAG_BLANKSIGN = 0x4,
      FLAG_PREFIXSUFFIX = 0x8,
      FLAG_LEFTPADZERO = 0x10,

      FLAG_WIDTH_SPECIFIED = 0x100,
      FLAG_WIDTH_PARAM = 0x200,
      FLAG_PERCISION_SPECIFIED = 0x400,
      FLAG_PERCISION_PARAM = 0x800,

      FLAG_FMT_LONG     = 0x1000,
      FLAG_FMT_LONGLONG = 0x2000,
      FLAG_FMT_HALF     = 0x4000,
      FLAG_FMT_HALFHALF = 0x8000,
    };

    int state = ST_TEXT;
    int width = 0;
    int percision = 0;
    unsigned short flag = 0;
    const char *hextableL = "0123456789abcdef";
    const char *hextableU = "0123456789ABCDEF";
    
    for (const char* p = format; *p !=  0; ++p)
    {
      char c = static_cast<char>(*p);
      
      switch (state)
      {
      case ST_TEXT:
        if ('%' == c)
        {
          width = percision = 0;
          flag = 0;
          state = ST_FLAGS;
        }
        else
        {
          str.append(1, *p);
        }
        break;

      case ST_FLAGS:
        switch (c)
        {
        case '-':
          flag |= FLAG_LEFTJUSTIFY; break;
        case '+':
          flag |= FLAG_FORCESIGN; flag &= (~FLAG_BLANKSIGN); break;
        case ' ':
          flag |= FLAG_BLANKSIGN; flag &= (~FLAG_FORCESIGN); break;
        case '#':
          flag |= FLAG_PREFIXSUFFIX; break;
        case '0':
          flag |= FLAG_LEFTPADZERO; break;
        default:
          p--; state = ST_WIDTH; break;
        }
        break;

      case ST_WIDTH:
        if ((c >= '0') && (c <= '9'))
        {
          flag |= FLAG_WIDTH_SPECIFIED;
          if (width < 0) width = 0;
          width = width * 10 + (c - '0');
        }
        else if (c == '*')
        {
          flag |= FLAG_WIDTH_SPECIFIED;
          flag |= FLAG_WIDTH_PARAM;
        }
        else if (c == '.')
        {
          flag |= FLAG_PERCISION_SPECIFIED;
          state=ST_PERCISION;
        }
        else
        {
          p--; state = ST_SUBSPEC;
        }
        break;

      case ST_PERCISION:
        if ((c >= '0') && (c <= '9'))
        {
          if (percision < 0) percision = 0;
          percision = percision * 10 + (c - '0');
        }
        else if (c == '*')
        {
          flag |= FLAG_PERCISION_PARAM;
        }
        else
        {
          p--; state = ST_SUBSPEC;
        }
        break;

      case ST_SUBSPEC:
        // http://www.cplusplus.com/reference/cstdio/printf/
        // The sub-specifiers we're currently support is 'll', 'l', 'h', and 'hh', others are ignored.
        switch(c)
        {
        case 'l':
          if (*(p+1) == 'l')
          {
            flag |= FLAG_FMT_LONGLONG;
            p++;
          }
          else
          {
            flag |= FLAG_FMT_LONG;
          }
          break;

        case 'h':
          if (*(p+1) == 'h')
          {
            flag |= FLAG_FMT_HALFHALF;
            p++;
          }
          else
          {
            flag |= FLAG_FMT_HALF;
          }
          break;

        case 'j':
        case 'z':
        case 't':
        case 'L':
          // not yet implement
          break;

        default:
          p--;
          state = ST_SPEC;
          break;
        }
        break;
      
      case ST_SPEC:
        switch(c)
        {

        // Handy macro to conditionally load width and percision arguments from va_args
        #define _READ_PARAMS \
            if (flag & FLAG_WIDTH_PARAM)\
              width = va_arg(ap, int);\
            if (flag & FLAG_PERCISION_PARAM) \
              percision = va_arg(ap, int);

        case 'c':
          {
            // Note: For specifier 'c', we ignore any sub-specifier (i.e., 'l', 'h', etc.).
            _READ_PARAMS;

            int prepad = 0;
            int postpad = 0;
            if (width > 1)
            {
              if (flag & FLAG_LEFTJUSTIFY)
                postpad = width - 1;
              else
                prepad = width - 1;
            }

            if (prepad > 0)
              str.append(prepad, ' ');

            // per C99 standard: 'char', 'short' will be promoted to 'int' when passing through va_args
            // http://www.gnu.org/software/libc/manual/html_node/Argument-Macros.html#Argument-Macros
            int v = va_arg(ap, int);
            if (v != 0) // never append internal null
              str.append(1, v);

            if (postpad > 0)
              str.append(postpad, ' ');
          }
          state = ST_TEXT;
          break;

        case '%':
          {
            _READ_PARAMS;
            str.append(1, '%');
            state = ST_TEXT;
          }
          break;

        case 'p':
          // Keep FLAG_WIDTH_PARAM and FLAG_PERCISION_PARAM but clear all other flag bits to 0.
          // Add FLAG_WIDTH_SPECIFIED, and FLAG_PREFIXSUFFIX.
          flag = (flag & (FLAG_WIDTH_SPECIFIED | FLAG_WIDTH_PARAM | FLAG_PERCISION_SPECIFIED | FLAG_PERCISION_PARAM)) | FLAG_PERCISION_SPECIFIED | FLAG_PREFIXSUFFIX;

          if (percision < 8)
            percision = 8;
          if (sizeof(void*) > 4) // FIXME: better way to detect whether the host use a 64-bit pointer type?
          {
            if (percision < 16)
              percision = 16;
            flag |= FLAG_FMT_LONGLONG;
          }

          // fall-through ...
          c = 'x';

        case 'F': // all these specifiers are treated as 'f'
        case 'g':
        case 'G':
        case 'e':
        case 'E':
        case 'A':
        case 'a':
          if (c != 'x')
            c = 'f';
          // fall-through ...

        case 'f':
        case 'x':
        case 'X':
        case 'o':
        case 'd':
        case 'i':
        case 'u':
          {
            _READ_PARAMS;

            const bool isFloat = (c == 'f');
            const bool isSigned = ((c == 'd') || (c == 'i'));
            const bool isHex = ((c == 'x') || (c == 'X'));
            const bool isOct = (c == 'o');

            // Read param from va_arg according to the type hints by sub-specifier.
            // Apply necessary convertion. (hex, oct, integer)
            T valstr;
            if (isFloat)
            {
              double v = va_arg(ap, double);
              valstr = std::to_string(v);
            }
            else if (isSigned)
            {
              int64_t v = (flag & FLAG_FMT_LONGLONG) ?
                  (int64_t)va_arg(ap, int64_t) : (int64_t)va_arg(ap, int32_t);
              // Shrink to valid value width according to sub-specifiers
              if (flag & FLAG_FMT_HALF)
                v &= 0xFFFF;
              else if (flag & FLAG_FMT_HALFHALF)
                v &= 0xFF;
              valstr = std::to_string(v);
            }
            else // unsigned
            {
              uint64_t v = (flag & FLAG_FMT_LONGLONG) ?
                (uint64_t)va_arg(ap, uint64_t) : (uint64_t)va_arg(ap, uint32_t);
              // Shrink to valid value width according to sub-specifiers
              if (flag & FLAG_FMT_HALF)
                v &= 0xFFFF;
              else if (flag & FLAG_FMT_HALFHALF)
                v &= 0xFF;

              if (isHex || isOct)
              {
                bool leadingZero = true;

                if (isHex)
                {
                  int n = (flag & FLAG_FMT_LONGLONG) ? 16 :  8; // Maximum possible characters of output.
                  int d = (flag & FLAG_FMT_LONGLONG) ? 60 : 28; // How many bit to shift to get current digit.
                  const char *hextable = ('X' == c) ? hextableU : hextableL;
                  for (int i = 0; i < n; ++i)
                  {
                    const char digit = hextable[(v >> (d - (4 * i))) & 0xF];
                    if (leadingZero && digit == '0')
                      continue;
                    leadingZero = false;
                    valstr.append(1, digit);
                  }
                }
                else // Oct
                {
                  int n = (flag & FLAG_FMT_LONGLONG) ? 22 : 11; // Maximum possible characters of output.
                  int d = (flag & FLAG_FMT_LONGLONG) ? 63 : 30; // How many bit to shift to get current digit.
                  for (int i = 0; i < n; ++i)
                  {
                    const char digit = hextableL[(v >> (d - (3 * i))) & 0x7];
                    if (leadingZero && digit == '0')
                      continue;
                    leadingZero = false;
                    valstr.append(1, digit);
                  }
                }
                if (valstr.empty())
                  valstr.append(1, '0');
              }
              else
                valstr = std::to_string(v);
            }

            const bool neg = (valstr[0] == '-');

            // Layout: Neither width nor percision specified, this is the simplest case.
            if (!(flag & (FLAG_WIDTH_SPECIFIED | FLAG_PERCISION_SPECIFIED)))
            {
              if (isSigned || isFloat)
              {
                if ((flag & FLAG_FORCESIGN) && !neg)
                  str.append(1, '+');
                if ((flag & FLAG_BLANKSIGN) && !neg)
                  str.append(1, ' ');

                if (isFloat && (flag & FLAG_PREFIXSUFFIX))
                {
                  if (valstr.find_first_of('.') == T::npos)
                    valstr.append(1, '.');
                }
              }

              if ((isHex || isOct) && (flag & FLAG_PREFIXSUFFIX))
              {
                if (!((valstr[0]=='0') && (valstr[1]==0)))
                {
                  str.append(1, '0');
                  if (c != 'o')
                    str.append(1, c); // append either 'x' or 'X'
                }
              }

              str.append(valstr.c_str());
            }
            else 
            {
              if (flag & (FLAG_PERCISION_SPECIFIED | FLAG_LEFTJUSTIFY))
              {
                // FLAG_LEFTPADZERO will be ignored if either FLAG_PERCISION_SPECIFIED or FLAG_LEFTJUSTIFY present
                flag &= (~FLAG_LEFTPADZERO);
              }

              T prefix;
              if (isSigned || isFloat)
              {
                if (neg)
                  prefix = "-";
                else if (flag & FLAG_FORCESIGN)
                  prefix = "+";
                else if (flag & FLAG_BLANKSIGN)
                  prefix = " ";
              }
              else if ((isHex || isOct) && (flag & FLAG_PREFIXSUFFIX))
              {
                prefix = "0";
                if (isHex)
                  prefix.append(1, c);
              }

              if (flag & FLAG_PERCISION_SPECIFIED)
              {
                do
                {
                  // Per spec: If percision is 0, do not output any digit when val is also 0.
                  if (!isFloat && (percision == 0) && (valstr[0]=='0') && (valstr[1]==0))
                  {
                    valstr = "";
                    if (isSigned)
                    {
                      // Note: both msvc and gcc still output sign (either '+' or ' ') for such case.
                      if (flag & FLAG_FORCESIGN)
                        valstr = "+";
                      else if (flag & FLAG_BLANKSIGN)
                        valstr = " ";
                    }
                    break;
                  }

                  if (neg)
                    valstr = valstr.substr(1);

                  if (isFloat) // trim or pad for float values
                  {
                    size_t idx = valstr.find_first_of('.');
                    int padcnt = 0;
                    if (idx == T::npos) // no '.' in content, only possible to pad
                    {
                      if ((percision > 0) || (flag & FLAG_PREFIXSUFFIX))
                      {
                        valstr.append(1, '.');
                        padcnt = percision;
                      }
                    }
                    else // trim or pad
                    {
                      // trim
                      int restcnt = (int)valstr.size() - (int)idx - 1;
                      if (restcnt > percision)
                      {
                        // if percision is 0, only show tailing '.' if FLAG_PREFIXSUFFIX has been set. 
                        if ((percision == 0) && !(flag & FLAG_PREFIXSUFFIX))
                          valstr = valstr.substr(0, (int)idx);
                        else
                          valstr = valstr.substr(0, (int)idx + percision + 1);
                      }
                      else
                        padcnt = percision - restcnt;
                    }

                    if (padcnt > 0)
                      valstr.append(padcnt, '0');

                    valstr = prefix + valstr;
                  }
                  else // integer based values
                  {
                    // compute padding count
                    const int cnt = (percision > (int)valstr.size())? (percision - (int)valstr.size()): 0;
                    if (cnt > 0)
                    {
                      T padding;
                      padding.append(cnt, '0');
                      valstr = prefix + padding + valstr;
                    }
                    else
                      valstr = prefix + valstr;
                  }

                } while (0);

                // layout in given width
                const int cnt = (width > (int)valstr.size())? (width - (int)valstr.size()): 0;
                if (flag & FLAG_LEFTJUSTIFY)
                {
                  str.append(valstr.c_str());
                  if (cnt > 0)
                    str.append(cnt, ' ');
                }
                else
                {
                  if (cnt > 0)
                    str.append(cnt, ' ');
                  str.append(valstr.c_str());
                }
              }
              else // width specified but no percision
              {
                if (neg)
                  valstr = valstr.substr(1);

                // Per spec: float values has a default percision of 6
                // However, the irrString constructor of the version 
                // which takes a double param already guarantee this.

                const int fulllen = (int)(prefix.size() + valstr.size());
                const int padlen = (width > fulllen)? (width - fulllen): 0;
                if (flag & FLAG_LEFTJUSTIFY)
                {
                  str.append(prefix.c_str());
                  str.append(valstr.c_str());
                  if (padlen > 0)
                    str.append(padlen, ' ');
                }
                else if (flag & FLAG_LEFTPADZERO)
                {
                  str.append(prefix.c_str());
                  if (padlen > 0)
                    str.append(padlen, '0');
                  str.append(valstr.c_str());
                }
                else
                {
                  if (padlen > 0)
                    str.append(padlen, ' ');
                  str.append(prefix.c_str());
                  str.append(valstr.c_str());
                }
              }
            }
          }
          state = ST_TEXT;
          break;

        case 's':
          {
            _READ_PARAMS;
            char* v = va_arg(ap, char*);

            // Neither width specified nor percision, the simplest case.
            if (!(flag & (FLAG_WIDTH_SPECIFIED | FLAG_PERCISION_SPECIFIED)))
            {
              int cnt = 0;
              for (char* vp = v; *vp != 0; ++vp, ++cnt);

              if (cnt > 0)
                str.append(v, cnt);
            }
            else
            {
              // look for the string length (must not exceeds percision, if present)
              int cnt = 0;
              for (char* vp = v; *vp != 0; ++vp, ++cnt)
              {
                if ((flag & FLAG_PERCISION_SPECIFIED) && (cnt >= percision))
                {
                  cnt = percision; break;
                }
              }

              int pad = ((flag & FLAG_WIDTH_SPECIFIED) && (width > cnt))? (width - cnt): 0;
              if (flag & FLAG_LEFTJUSTIFY)
              {
                str.append(v, cnt);
                if (pad > 0)
                  str.append(pad, ' ');
              }
              else
              {
                if (pad > 0)
                  str.append(pad, ' ');
                str.append(v, cnt);
              }
            }
          }
          state = ST_TEXT;
          break;

        default:
          state = ST_TEXT;
          break;
        }
        break;

      default:
        state = ST_TEXT;
        break;
      }

      #undef _READ_PARAMS

    }
    va_end(ap);
    return (int32_t)str.size();
  } // end of printf()

  // T can be any std::basic_string derived class
  template< typename T >
  void make_lower( T& str )
  {
    for (T::size_type i = 0; i < str.size(); ++i)
    {
      str[i] = static_cast<T::value_type>(std::tolower((int)str[i]));
    }
  }

  // T can be any std::basic_string derived class
  template< typename T >
  T to_lower( const T& str )
  {
    T r(str);
    make_lower(r);
    return r;
  }

  // T can be any std::basic_string derived class
  template< typename T >
  void make_upper( T& str )
  {
    for (T::size_type i = 0; i < str.size(); ++i)
    {
      str[i] = static_cast<T::value_type>(std::toupper((int)str[i]));
    }
  }

  // T can be any std::basic_string derived class
  template< typename T >
  T to_upper( const T& str )
  {
    T r(str);
    make_upper(r);
    return r;
  }

  // T can be any std::basic_string derived class and S should the same type as T::value_type
  template< typename T, typename S >
  void make_trim( T& str, const S* chars )
  {
    static_assert(std::is_same<T::value_type, S>(), "T::value_type should be the same type as S.");

    T::size_type cnt = 0;
    for (T::size_type i = 0; i < str.size(); ++i)
    {
      bool trimmed = false;
      for (const S* p = chars; *p != S(); ++p)
      {
        if (str[i] == *p)
        {
          trimmed = true;
          break;
        }
      }
      if (trimmed)
        cnt++;
      else
        break;
    }
    str.erase(0, cnt);
    cnt = 0;
    for (int32_t i = (int32_t)(str.size() - 1); i >= 0; --i)
    {
      bool trimmed = false;
      for (const S* p = chars; *p != S(); ++p)
      {
        if (str[i] == *p)
        {
          trimmed = true;
          break;
        }
      }
      if (trimmed)
        cnt++;
      else
        break;
    }
    str.erase(str.size() - cnt, cnt);
  }

  template< typename T >
  void make_trim(T& str)
  {
    make_trim(str, " \t\f\v\r\n");
  }

  template< typename T, typename S >
  T trim( const T& str, const S* chars )
  {
    static_assert(std::is_same<T::value_type, S>(), "T::value_type should be the same type as S.");

    T r(str);
    make_trim(r, chars);
    return r;
  }

  template< typename T >
  T trim(const T& str)
  {
    T r(str);
    make_trim(r);
    return r;
  }

  // T should be a container type which supports push_back() and size(), 
  // and the element type should be S.
  // S should be any std::basic_string derived type.
  // R should be the same type as S::value_type.
  template< typename T, typename S, typename R >
  uint32_t split( T& out, const S& str, const R* delim, bool keepEmptyTokens = false )
  {
    static_assert(std::is_same<T::value_type, S>(), "T::value_type should be the same type as S.");
    static_assert(std::is_same<S::value_type, R>(), "S::value_type should be the same type as R.");

    S::size_type head = 0;
    for (S::size_type i = 0; i < str.size(); ++i)
    {
      for (const R* p = delim; *p != R(); ++p)
      {
        if (str[i] == *p)
        {
          if ((i != head) || keepEmptyTokens)
            out.push_back( S(&str[head], i - head) );
          if (keepEmptyTokens && (i == str.size() - 1))
            out.push_back( S() );
          head = i + 1;
          break;
        }
      }
    }
    return (uint32_t)out.size();
  }
}


#endif // _CEC278B6B50A_SU_HEADER_