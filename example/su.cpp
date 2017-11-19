
#include <su.h>
#include <list>

int main()
{
	std::string v1 = "123";
	su::printf(v1, "real %+.2f %llf .", 3.141596f, 3.141596);

  std::string v2;
  su::printf(v2, "integer %lld %d %hd %hhi .", 7000000000000, 70000, 70000, 70000);

  std::string v3;
  su::printf(v3, "hex %llX %x %hX %hhx .", 0x123456789ABCDEF, 0x89ABCDEF, 0x89ABCDEF, 0x89ABCDEF);

  std::string v4;
  su::printf(v4, "oct %llo %o %ho %hho .", 01234567012345, 012345670123, 012345670123, 012345670123);

  std::string v5;
  su::printf(v5, "string %*.*s .", 30, 30, "abcdefghijklmnopqrstuvwxyz0123456789");

  std::string v6;
  su::printf(v6, "string %-*.*s .", 30, 30, "abcdef");

  std::string v7 = su::to_lower(std::string("123 ABCDEFGH"));

  std::string v8 = "   13     4  ";
  su::make_trim(v8);

  std::string v9 = ",,,13,,,,,4,,";
  std::list<std::string> c;
  uint32_t cnt = su::split(c, v9, " ,", true);

  return 0;
}

