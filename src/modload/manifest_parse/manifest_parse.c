#include "tessera/modload/manifest_parse.h"
#include "tessera/lib/str_eq.h"
#include "tessera/lib/str_bounded_copy.h"

/* Appends one comma-separated item list to a fixed array. */
static int add_list(const char *v, size_t vlen, char out[][MOD_NAME_MAX], uint32_t *n, uint32_t max)
{
    size_t start = 0;
    for (size_t i = 0; i <= vlen; i++) {
        if (i == vlen || v[i] == ',') {
            if (i == start) {
                return MOD_ERR_SYNTAX;
            }
            if (*n >= max) {
                return MOD_ERR_TOO_MANY;
            }
            if (str_bounded_copy(out[*n], MOD_NAME_MAX, v + start, i - start) != 0) {
                return MOD_ERR_TOO_LONG;
            }
            (*n)++;
            start = i + 1;
        }
    }
    return MOD_OK;
}

static int set_key(struct mod_manifest *m, const char *k, size_t klen, const char *v, size_t vlen)
{
    char key[16];
    if (klen >= sizeof key || str_bounded_copy(key, sizeof key, k, klen) != 0) {
        return MOD_ERR_UNKNOWN_KEY;
    }
    if (str_eq(key, "name")) {
        return str_bounded_copy(m->name, MOD_NAME_MAX, v, vlen) == 0 ? MOD_OK : MOD_ERR_TOO_LONG;
    }
    if (str_eq(key, "version")) {
        uint32_t n = 0;
        if (vlen == 0) return MOD_ERR_SYNTAX;
        for (size_t i = 0; i < vlen; i++) {
            if (v[i] < '0' || v[i] > '9') return MOD_ERR_SYNTAX;
            n = n * 10 + (uint32_t)(v[i] - '0');
        }
        m->version = n;
        return MOD_OK;
    }
    if (str_eq(key, "requires")) return add_list(v, vlen, m->requires, &m->n_requires, MOD_LIST_MAX);
    if (str_eq(key, "provides")) return add_list(v, vlen, m->provides, &m->n_provides, MOD_LIST_MAX);
    if (str_eq(key, "imports")) return add_list(v, vlen, m->imports, &m->n_imports, MOD_IMPORT_MAX);
    if (str_eq(key, "required")) {
        if (vlen != 1 || (v[0] != '0' && v[0] != '1')) return MOD_ERR_SYNTAX;
        m->required = v[0] == '1';
        return MOD_OK;
    }
    return MOD_ERR_UNKNOWN_KEY;
}

int manifest_parse(const char *text, size_t len, struct mod_manifest *out)
{
    *out = (struct mod_manifest){0};
    size_t pos = 0;
    while (pos < len) {
        size_t end = pos;
        while (end < len && text[end] != '\n') end++;
        size_t line_len = end - pos;
        if (line_len > 0) {
            const char *line = text + pos;
            size_t eq = 0;
            while (eq < line_len && line[eq] != '=') eq++;
            if (eq == line_len || eq == 0) return MOD_ERR_SYNTAX;
            int rc = set_key(out, line, eq, line + eq + 1, line_len - eq - 1);
            if (rc != MOD_OK) return rc;
        }
        pos = end + 1;
    }
    if (out->name[0] == '\0') {
        return MOD_ERR_NAME_MISSING;
    }
    return MOD_OK;
}
