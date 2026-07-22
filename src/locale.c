#include <stdlib.h>
#include <stdint.h>
#include <limits.h>
#include <stdio.h>
#include <tutils.h>

typedef struct mo_header {
	uint32_t magic;
	uint32_t revision;
	uint32_t num_strings;
	uint32_t orig_table_offset;
	uint32_t trans_table_offset;
	uint32_t hash_size;
	uint32_t hash_offset;
} __attribute__((packed)) mo_header_t;

typedef struct mo_desc {
	uint32_t length;
	uint32_t offset;
} __attribute__((packed)) mo_desc_t;

#define MO_MAGIC 0x950412de

typedef struct translation {
	const char *msgid;
	const char *msgstr;
} translation_t;

static translation_t *translations;
static size_t translations_count;

// GNU gettext string hashing algorithm
uint32_t hash_pjw(const char *str) {
	uint32_t hval = 0;
	while (*str) {
	        hval = (hval << 4) + *str;
		uint32_t g = hval & 0xF0000000;
		if (g != 0) {
			hval ^= g >> 24;
		}
		hval ^= g;
	}
	return hval;
}


static char *load_str(FILE *file, mo_desc_t *desc) {
	fseek(file, desc->offset, SEEK_SET);
	char *buf = malloc(desc->length + 1);
	fread(buf, desc->length, 1, file);
	buf[desc->length] = '\0';
	return buf;
}

void setup_locale(void) {
	const char *lang_env = getenv("LANG");
	if (!lang_env) return;
	char lang[64];
	if (strlen(lang_env) + 1 > sizeof(lang)) return;
	strcpy(lang, lang_env);
	if (strchr(lang, '.')) *strchr(lang, '.') = '\0';
	char path[PATH_MAX];
	snprintf(path, sizeof(path), LOCALEDIR"/tutils/%s.mo", lang);
	FILE *file = fopen(path, "r");
	if (!file) return;

	mo_header_t header;
	if (fread(&header, sizeof(header), 1, file) != 1) return;
	if (header.magic != MO_MAGIC) return;

	translations_count = header.num_strings;
	translations = malloc(sizeof(translation_t) * translations_count);
	for (size_t i=0; i<header.num_strings; i++) {
		fseek(file, header.orig_table_offset + sizeof(mo_desc_t) * i, SEEK_SET);
		mo_desc_t orig_desc;
		fread(&orig_desc, sizeof(mo_desc_t), 1, file);
		translations[i].msgid = load_str(file, &orig_desc);
		fseek(file, header.trans_table_offset + sizeof(mo_desc_t) * i, SEEK_SET);
		mo_desc_t trans_desc;
		fread(&trans_desc, sizeof(mo_desc_t), 1, file);
		translations[i].msgstr = load_str(file, &trans_desc);
	}

	fclose(file);
}

const char *locale_getstr(const char *msgid) {
	if (!translations) return msgid;
	for (size_t i=0; i<translations_count; i++) {
		if (!strcmp(translations[i].msgid, msgid)) {
			return translations[i].msgstr;
		}
	}
	return msgid;
}
