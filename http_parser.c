#include "http_parser.h"
#include <string.h>
#include <malloc.h>

typedef char byte;

typedef struct {
    byte is_search;         /* Флаг, искать ли условие? */
    size_t off_len;         /* Позиция от начала */
} info;


typedef struct {
    info about;        /* Информация о ключевом слове */
    int pos;
    string key;                /* Ключевое слово */
} key;

static key *iterator_key (char ch, int pos, key *keys, const int n) {
    int count;
    register key *required_key = 0;
    for (count = 0; count < n; count++) {
        required_key = keys + count;
        if (required_key->about.is_search) {
            if (ch == required_key->key.pointer[required_key->about.off_len])
                required_key->about.off_len++;
            else required_key->about.off_len = 0;
            if (required_key->about.off_len == required_key->key.len) {
                required_key->pos = pos + 1;
                return required_key;
            }
        }
    }
    return 0;
}

static void read_key (key *_key, string buffer, int pos) {
    _key->pos = pos - 1;
    pos -= 2;
    for (_key->key.len = 0; buffer.pointer[pos] != '\n' && buffer.pointer[pos - 1] != '\0'; pos--)
        if(buffer.pointer[pos] != '\n')
            _key->key.len++;
    if (buffer.pointer[pos] == 10) {
        pos++;
        if(_key->key.len > 0) {
            _key->key.pointer = buffer.pointer + pos;
            _key->about.is_search = 1;
        }
    }
}

static key *search_key (string buffer, size_t pos, key *keys, const int n) {
    static key *required_key = 0;
    if(n == 0) {
        if(!required_key)
            required_key = (key*) malloc(sizeof(key));
    }
    else required_key = 0;
    for (; pos < buffer.len; pos++) {
        if(n) {
            if((required_key = iterator_key(buffer.pointer[pos], pos, keys, n)))
                return required_key;
        } else {
            if (buffer.pointer[pos] == 0x20 && buffer.pointer[pos - 1] == 0x3A) {
                memset(required_key, 0, sizeof(key));
                read_key(required_key, buffer, pos);
                return required_key;
            }
        }
    }
    return 0;
}

static string get_value (string buffer, key *required) {
    byte is_end = 0;
    size_t i = required->pos;
    string value;
    printf("I: %d\n", buffer.pointer[i]);
    memset(&value, 0, sizeof(string));
    if (required->about.is_search) {
        if (buffer.pointer[i] == 0x3A && buffer.pointer[i + 1] == 0x20) {
            i += 2;
            value.pointer = buffer.pointer + i;
            for (; i < buffer.len; i++) {
                if (is_end) {
                    if (buffer.pointer[i] == 0xA) {
                        printf("I2: %d\n", buffer.pointer[i]);
                        value.len = i > required->pos ? i - required->pos : required->pos;
                        break;
                    } else is_end = 0;
                }else if (buffer.pointer[i] == 0xD && !is_end)
                    is_end = 1;
                else
                    value.len++;
            }
        }
    }
    return value;
}

static int search_result (string buffer, byte **name_keys, int n, void *callback) {
    void (*result)(string, string) = callback;
    int counter_line = 0;
    key keys[n];
    int i;
    if (n) {
        for (i = 0; i < n; i++) {
            keys[i].pos = 0;
            keys[i].key.pointer= *(name_keys + i);
            keys[i].key.len = strlen(*(name_keys + i));
            keys[i].about.off_len = 0;
            keys[i].about.is_search = 1;
        }
    }
    i = 0;
    key *_key = 0, *old_key = 0;
    string value;
    while ((_key = search_key(buffer, i, keys, n)) != 0) {
        if (_key->about.is_search) {
            value = get_value(buffer, _key);
            result(_key->key, value);
            old_key = _key;
            i = _key->pos + value.len;
            _key->about.is_search = 0;
            counter_line++;
        }
    }
    return counter_line;
}


int http_parser (byte *buff, int buff_len, byte **keys, void *callback) {
    int a = 0;
    if(keys != 0)
        while(*(keys + a) != 0)
            a++;
    string buffer = {(size_t)buff_len, buff};
    return search_result (buffer, keys, a, callback);
}
