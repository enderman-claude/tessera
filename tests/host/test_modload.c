/* Host tests for module manifests, init order, and import checks. */
#include <stdio.h>
#include <string.h>
#include "tessera/modload/manifest_parse.h"
#include "tessera/modload/init_order.h"
#include "tessera/modload/check_imports.h"
#include "tessera/modload/export_table.h"
#include "tessera/modload/run_init.h"

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

static char trace[64];
static int tlen;
static int init_a(void) { trace[tlen++] = 'a'; return 0; }
static int init_b(void) { trace[tlen++] = 'b'; return 0; }
static int init_c(void) { trace[tlen++] = 'c'; return 0; }
static int init_bad(void) { trace[tlen++] = 'x'; return 1; }

static void test_run_init(void)
{
    struct mod_manifest mods[3];
    uint32_t order[3];
    uint8_t status[3];
    parse("name=c\n", &mods[0]);
    parse("name=b\nrequires=c\n", &mods[1]);
    parse("name=a\nrequires=b\nrequired=1\n", &mods[2]);
    init_order(mods, 3, order);
    mod_init_fn inits[3] = { init_c, init_b, init_a };
    tlen = 0;
    CHECK(run_init(mods, 3, order, inits, status) == MOD_OK, "run: all succeed");
    trace[tlen] = 0;
    CHECK(strcmp(trace, "cba") == 0, "run: inits run in dependency order");
    CHECK(status[0] == MOD_STATUS_OK && status[1] == MOD_STATUS_OK, "run: status ok");

    parse("name=opt\n", &mods[1]);
    mod_init_fn inits2[3] = { init_c, init_bad, init_a };
    tlen = 0;
    CHECK(run_init(mods, 3, order, inits2, status) == MOD_OK, "run: optional failure does not halt");
    CHECK(status[1] == MOD_STATUS_SKIPPED, "run: optional failure recorded as skipped");

    struct mod_manifest rmods[2];
    parse("name=one\nrequired=1\n", &rmods[0]);
    parse("name=two\n", &rmods[1]);
    uint32_t rorder[2] = { 0, 1 };
    mod_init_fn rinits[2] = { init_bad, init_a };
    uint8_t rstatus[2];
    tlen = 0;
    CHECK(run_init(rmods, 2, rorder, rinits, rstatus) == MOD_ERR_REQUIRED_FAILED, "run: required failure halts");
    CHECK(tlen == 1, "run: nothing after a required failure runs");
}

static void test_exports(void)
{
    uint32_t n = 0;
    const char *const *names = export_names(&n);
    CHECK(n == 4 && strcmp(names[0], "log_write") == 0, "exports: table lists log_write first");
    struct mod_manifest m;
    parse("name=ok\nimports=log_write,timer_read\n", &m);
    uint32_t bad = 0;
    CHECK(check_imports(&m, names, n, &bad) == MOD_OK, "exports: real imports resolve");
    parse("name=evil\nimports=log_write,port_poke\n", &m);
    CHECK(check_imports(&m, names, n, &bad) == MOD_ERR_UNRESOLVED_IMPORT, "exports: unknown import rejected");
}

int main(void)
{
    test_parse();
    test_order();
    test_imports();
    test_run_init();
    test_exports();
    printf("%s (%d failures)\n", failures ? "FAILED" : "PASSED", failures);
    return failures ? 1 : 0;
}
