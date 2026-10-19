/* Host tests for module manifests, init order, and import checks. */
#include <stdio.h>
#include <string.h>
#include "tessera/modload/manifest_parse.h"
#include "tessera/modload/init_order.h"
#include "tessera/modload/check_imports.h"

static int failures;
#define CHECK(cond, name) do { if (cond) printf("ok   %s\n", name); else { printf("FAIL %s\n", name); failures++; } } while (0)

static int parse(const char *s, struct mod_manifest *m)
{
    return manifest_parse(s, strlen(s), m);
}

static void test_parse(void)
{
    struct mod_manifest m;
    const char *good =
        "name=acpi\nversion=2\nrequires=pci,timer\nprovides=acpi_tables\n"
        "imports=log_write,mem_alloc\nrequired=1\n";
    CHECK(parse(good, &m) == MOD_OK, "parse: good manifest");
    CHECK(strcmp(m.name, "acpi") == 0 && m.version == 2, "parse: name and version");
    CHECK(m.n_requires == 2 && strcmp(m.requires[1], "timer") == 0, "parse: requires list");
    CHECK(m.n_imports == 2 && m.required == 1, "parse: imports and required flag");

    CHECK(parse("version=1\n", &m) == MOD_ERR_NAME_MISSING, "parse: missing name rejected");
    CHECK(parse("name=a\nbogus=1\n", &m) == MOD_ERR_UNKNOWN_KEY, "parse: unknown key rejected");
    CHECK(parse("name=a\nrequired=yes\n", &m) == MOD_ERR_SYNTAX, "parse: bad flag rejected");
    CHECK(parse("name=a\nrequires=x,,y\n", &m) == MOD_ERR_SYNTAX, "parse: empty list item rejected");
    CHECK(parse("name=a\nno-equals-here\n", &m) == MOD_ERR_SYNTAX, "parse: line without = rejected");
    CHECK(parse("name=a\nversion=1x\n", &m) == MOD_ERR_SYNTAX, "parse: non-numeric version rejected");

    char longname[64];
    memset(longname, 'x', sizeof longname - 1);
    longname[sizeof longname - 1] = '\0';
    char toolong[128];
    snprintf(toolong, sizeof toolong, "name=%s\n", longname);
    CHECK(parse(toolong, &m) == MOD_ERR_TOO_LONG, "parse: name over limit rejected");

    char many[256] = "name=a\nrequires=";
    for (int i = 0; i < MOD_LIST_MAX + 1; i++) strcat(many, i ? ",d" : "d");
    strcat(many, "\n");
    CHECK(parse(many, &m) == MOD_ERR_TOO_MANY, "parse: too many requires rejected");
}

static void test_order(void)
{
    struct mod_manifest mods[4];
    uint32_t order[4];
    parse("name=c\n", &mods[0]);
    parse("name=b\nrequires=c\n", &mods[1]);
    parse("name=a\nrequires=b\n", &mods[2]);
    parse("name=z\n", &mods[3]);
    CHECK(init_order(mods, 3, order) == MOD_OK, "order: chain resolves");
    CHECK(order[0] == 0 && order[1] == 1 && order[2] == 2, "order: c, b, a");

    CHECK(init_order(mods, 4, order) == MOD_OK, "order: independent module added");
    /* After c (0), ready set is {b (1), z (3)}; lowest index first gives b, then a (2), then z. */
    CHECK(order[0] == 0 && order[1] == 1 && order[2] == 2 && order[3] == 3,
          "order: lowest ready index first, deps first");

    struct mod_manifest cyc[2];
    parse("name=x\nrequires=y\n", &cyc[0]);
    parse("name=y\nrequires=x\n", &cyc[1]);
    CHECK(init_order(cyc, 2, order) == MOD_ERR_CYCLE, "order: cycle rejected");

    struct mod_manifest miss[1];
    parse("name=p\nrequires=nothere\n", &miss[0]);
    CHECK(init_order(miss, 1, order) == MOD_ERR_MISSING_DEP, "order: missing dependency rejected");

    struct mod_manifest dup[2];
    parse("name=same\n", &dup[0]);
    parse("name=same\n", &dup[1]);
    CHECK(init_order(dup, 2, order) == MOD_ERR_DUPLICATE, "order: duplicate name rejected");

    struct mod_manifest prov[2];
    parse("name=p1\nprovides=clock\n", &prov[0]);
    parse("name=p2\nprovides=clock\n", &prov[1]);
    CHECK(init_order(prov, 2, order) == MOD_ERR_PROVIDES_CONFLICT, "order: two providers of one facility rejected");
}

static void test_imports(void)
{
    struct mod_manifest m;
    parse("name=iommu\nimports=log_write,mem_alloc\n", &m);
    const char *exports[] = { "log_write", "mem_alloc", "timer_read" };
    uint32_t bad = 99;
    CHECK(check_imports(&m, exports, 3, &bad) == MOD_OK, "imports: all resolved");

    parse("name=iommu\nimports=log_write,secret_poke\n", &m);
    CHECK(check_imports(&m, exports, 3, &bad) == MOD_ERR_UNRESOLVED_IMPORT && bad == 1,
          "imports: undeclared export rejected, index reported");
}

int main(void)
{
    test_parse();
    test_order();
    test_imports();
    printf("%s (%d failures)\n", failures ? "FAILED" : "PASSED", failures);
    return failures ? 1 : 0;
}
