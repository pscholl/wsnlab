

typedef char* data_type;

struct arraylist {
  int size;
  data_type* data;
};

data_type list_get (const struct arraylist list, int index);
void list_init (struct arraylist *list);
void list_setData (struct arraylist *list, data_type* data);
int list_get_size (const struct arraylist list);
void list_add (struct arraylist *list, data_type data);
int list_indexof (const struct arraylist list, data_type data);
void addToListWithCheck (struct arraylist *list, data_type value);
char* getLeftOf (const struct arraylist list, data_type value);
char* getRightOf (const struct arraylist list, data_type value);