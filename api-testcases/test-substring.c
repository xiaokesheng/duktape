/*===
*** test_1
blen=5, clen=3, str="o\xe1\x88\xb4a"
blen=6, clen=4, str="o\xe1\x88\xb4ar"
blen=0, clen=0, str=""
blen=0, clen=0, str=""
final top: 1
rc=0, result=undefined
*** test_2
rc=1, result=TypeError: incorrect type, expected tag 5
*** test_3
rc=1, result=Error: invalid index: -2
*** test_4
rc=1, result=Error: invalid index: -2147483648
===*/

void dump_string(duk_context *ctx) {
	const char *buf;
	size_t i, len;

	buf = duk_get_lstring(ctx, -1, &len);
	printf("blen=%d, clen=%d, str=\"", (int) len, (int) duk_get_length(ctx, -1));
	for (i = 0; i < len; i++) {
		char c = buf[i];
		if (c >= 0x20 && c <= 0x7e) {
			printf("%c", c);
		} else {
			printf("\\x%02x", ((int) c) & 0xff);
		}
	}
	printf("\"\n");

	duk_pop(ctx);
}

int test_1(duk_context *ctx) {
	/*
	 *  Test with a string containing non-ASCII to ensure indices are
	 *  treated correctly as char indices.
	 *
	 *  >>> u'foo\u1234ar'.encode('utf-8').encode('hex')
	 *  '666f6fe188b46172'
	 */
	const char *teststr = "666f6fe188b46172";

	duk_set_top(ctx, 0);

	duk_push_string(ctx, (const char *) teststr);
	duk_hex_decode(ctx, -1);
	duk_to_string(ctx, -1);

	/* basic case */
	duk_dup_top(ctx);
	duk_push_int(ctx, 123);  /* dummy */
	duk_substring(ctx, -2, 2, 5);  /* test index other than stack top */
	duk_pop(ctx);
	dump_string(ctx);

	/* end is clamped */
	duk_dup_top(ctx);
	duk_substring(ctx, -1, 2, 8);
	dump_string(ctx);

	/* start and end are clamped */
	duk_dup_top(ctx);
	duk_substring(ctx, -1, 10, 20);
	dump_string(ctx);

	/* start > end */
	duk_dup_top(ctx);
	duk_substring(ctx, -1, 4, 2);
	dump_string(ctx);

	printf("final top: %d\n", duk_get_top(ctx));
	return 0;
}

/* non-string -> error */
int test_2(duk_context *ctx) {
	duk_set_top(ctx, 0);

	duk_push_int(ctx, 123456);
	duk_substring(ctx, -1, 2, 4);

	printf("final top: %d\n", duk_get_top(ctx));
	return 0;
}

/* invalid index */
int test_3(duk_context *ctx) {
	duk_set_top(ctx, 0);

	duk_push_string(ctx, "foobar");
	duk_substring(ctx, -2, 2, 4);

	printf("final top: %d\n", duk_get_top(ctx));
	return 0;
}

/* invalid index */
int test_4(duk_context *ctx) {
	duk_set_top(ctx, 0);

	duk_push_string(ctx, "foobar");
	duk_substring(ctx, DUK_INVALID_INDEX, 2, 4);

	printf("final top: %d\n", duk_get_top(ctx));
	return 0;
}

#define  TEST(func)  do { \
		printf("*** %s\n", #func); \
		rc = duk_safe_call(ctx, (func), 0, 1, DUK_INVALID_INDEX); \
		printf("rc=%d, result=%s\n", rc, duk_to_string(ctx, -1)); \
		duk_pop(ctx); \
	} while (0)

void test(duk_context *ctx) {
	int rc;

	TEST(test_1);
	TEST(test_2);
	TEST(test_3);
	TEST(test_4);  /* FIXME: exposes value of DUK_INVALID_INDEX */
}
