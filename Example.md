
From:

```
tagged union JSON {
  void null;
  bool boole;
  double number;
  string string;
  JSON *array[];
  Pair<String, JSON*> object[];
}
```

Yields:

```
typedef union JSON JSON;

typedef enum JSON_tag {
  JSON_null,
  JSON_boole,
  JSON_number,
  JSON_string,
  JSON_array,
  JSON_object
} JSON_tag;

struct JSON_null {
  JSON_tag tag;
};

struct JSON_boole {
  JSON_tag tag;
  bool val;
};

struct JSON_number {
  JSON_tag tag;
  double val;
};

struct JSON_string {
  JSON_tag tag;
  char* val;            // bad type
};

struct JSON_array {
  JSON_tag tag;
  JSON** val;            // bad type
};

struct JSON_object {
  JSON_tag tag;
  char** val_a;            // bad type
  JSON** val_b;            // bad type
};

union JSON {
  JSON_tag tag;
  struct JSON_null null;
  struct JSON_boole boole;
  struct JSON_number number;
  struct JSON_string string;
  struct JSON_array array;
  struct JSON_object object;
};
```

Notes:

1. `union JSON` retains its name, and gets a typedef.
2. Each constructor has a predictable name (`JSON_null`) and NO typedef.
3. Presently, I cheated with a few types. Just thinking about tagged unions right now.


