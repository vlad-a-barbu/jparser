#include "stdio.h"

typedef unsigned long long u64;
typedef unsigned int u32;
typedef unsigned short int u16;
typedef unsigned char u8;

#define ARRAY_SIZE(array) (sizeof(array) / sizeof(array[0]))

struct PState {
  const char *stream;
  u64 pos;
};

enum JTokenType : u8 {
  Error,
  
  JInt,
  JString,
  JArray,
};

struct JToken {
  JTokenType type;
  u64 pos;
  u64 len;
};

void strdump(char *buff, const char *str) {
  u64 len = 0;
  for (const char *ptr = str; *ptr != '\0'; ++ptr) {
    buff[len++] = *ptr;
  }
  buff[len++] = '\0';
}

void token_type_to_string(JTokenType type, char *buff) {
  switch (type) {
  case JInt:
    strdump(buff, "JInt");
    break;
  case JString:
    strdump(buff, "JString");
    break;
  case JArray:
    strdump(buff, "JArray");
    break;
  default:
    strdump(buff, "Invalid");
    break;
  }
}

void print_token(JToken token) {
  char type_buff[20];
  token_type_to_string(token.type, type_buff);
  printf("type = %s; pos = %llu; len = %llu;\n",
	 type_buff,
	 token.pos,
	 token.len);
}

PState init_pstate(const char *stream) {
  return (PState) { stream, .pos = 0 };
}

JToken init_token(u64 pos) {
  return (JToken) { .type = Error, pos, .len = 0 };
}

u64 skip_whitespace(PState *state) {
  u64 skipped = 0;
  for (;state->stream[state->pos] != '\0'; ++state->pos) {
    char c = state->stream[state->pos];
    if (c == ' ' || c == '\r' || c == '\n' || c == '\t') {
      ++skipped;
      continue;
    } else {
      break;
    }
  }
  return skipped;
}

JToken parse_int(PState state) {
  JToken token = init_token(state.pos);
  for (; state.stream[state.pos] != '\0'; ++state.pos) {
    char c = state.stream[state.pos];
    if (c >= '0' && c <= '9') {
      ++token.len;
    } else {
      break;
    }
  }
  if (token.len > 0) {
    token.type = JInt;
  }
  return token;
}

JToken parse_string(PState state) {
  JToken token = init_token(state.pos);
  for (; state.stream[state.pos] != '\0'; ++state.pos) {
    char c = state.stream[state.pos];
    if (c == '"') {
      ++token.len;
      if (token.len == 1) {
	continue;
      } else {
	break;
      }
    } else if (token.len > 0) {
      ++token.len;
    } else {
      break;
    }
  }
  if (token.len > 1 &&
      state.stream[token.pos + token.len - 1] == '"') {
    token.type = JString;
  }
  return token;
}

JToken parse(PState);

JToken parse_array(PState state) {
  JToken token = init_token(state.pos);
  u8 accept_next = 0;
  for (; state.stream[state.pos] != '\0'; ++state.pos) {
    char c = state.stream[state.pos];
    if (c == '[') {
      ++token.len;
      accept_next = 1;
    } else if (token.len > 0) {
      u64 skipped = skip_whitespace(&state);
      if (skipped) {
	token.len += skipped;
	c = state.stream[state.pos];
      }
      if (c == ']') {
	++token.len;
	break;
      }
      if (accept_next) {
	JToken child_token = parse(state);
	if (child_token.type != Error) {
	  state.pos += child_token.len - 1; // +1 loop increment
	  token.len += child_token.len;
	  accept_next = 0;
	  continue;
	} else {
	  break;
	}
      } else if (c == ',') {
	++token.len;
	accept_next = 1;
      } else {
	break;
      }
    } else {
      break;
    }
  }
  if (token.len > 1 &&
      state.stream[token.pos + token.len - 1] == ']') {
    token.type = JArray;
  }
  return token;
}

JToken parse(PState state) {
  JToken token;
  skip_whitespace(&state);
  
  token = parse_array(state);
  if (token.type != Error) {
    return token;
  }
  
  token = parse_string(state);
  if (token.type != Error) {
    return token;
  }

  token = parse_int(state);
  if (token.type != Error) {
    return token;
  }

  return token;
}

void run_tests (const char *tests[], u8 count) {
  u8 passed = 1;
  for (u8 i = 0; i < count; ++i) {
    const char *test = tests[i];
    PState pstate = init_pstate(test);
    JToken token = parse(pstate);
    if (token.type != Error) {
      printf("[TEST %d PASSED] <<%s>>\n", i + 1, test);
      print_token(token);
      printf("\n\n");
      continue;
    }
    else {
      passed = 0;
      printf("[TEST %d FAILED] <<%s>>\n", i + 1, test);
    }
  }
  if (passed) {
    puts("!!! ALL TESTS PASSED !!!");
  }
}

const char *tests[] = {
  "0",
  "123",
  "   123 ",
  " \n\t  123  \r\n    ",
  "\"\"",
  "\"works\"",
  "\" with spaces ! \"",
  "\" w o r k s \n \t \"",
  "[]",
  " [    ] ",
  "[1 , 2\n,3 ]",
  "[\"abc\",   \n 123, [] ]",
  "    \n \r\n [  \t\t [    1   \n, 2 ], [], \"\"     ]"
};

int main(void) {
  run_tests(tests, ARRAY_SIZE(tests));
  return 0;
}

